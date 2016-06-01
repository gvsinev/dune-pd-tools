//==============================================================================
// diff_thresholds_2_3.C
//
// Gleb Sinev, Duke, 2016
// Look for flashes that pass the 3 PE threshold cut,
// but not the 2 PE threshold one
// (also require hits on at least 3 photon detectors)
//
// Some functions are copied from the EfficiencyPlots class
//==============================================================================

// ROOT includes
#include "TFile.h"
#include "TTree.h"

// C++ includes
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>

// Other includes
#include <dirent.h>

// Declare the functions used
void AnalyzeRootFile(std::string const& filename);
std::vector< std::string > GetRootFiles (std::string const& dir_name);
bool NPDsCut(std::vector< float > const& PEsPerFlashPerChannel,
             int const flashID, int const NFlashes, int const NChannels);

//-----------------------------------------------------------------------------
// Main function
void diff_thresholds_2_3() {

  std::string option("rn222");

  // Directory where the data is (or even are) kept
  std::string dataDir = 
    "/pnfs/lbne/scratch/users/gvsinev/photon_detectors/"
    "efficiency/dune4apa_" + option + "/root";

  // Get a vector containing names of datafiles
  std::vector< std::string > filenames = GetRootFiles(dataDir);

  // Analyze each file
  int counter = 0;
  for (auto const& filename : filenames) {
    std::cout << counter++ << ". ";
    AnalyzeRootFile(filename);
  }

}


//-----------------------------------------------------------------------------
// Function to analyze a single ROOT file
void AnalyzeRootFile(std::string const& filename) {

  std::cout << "Processing " << filename << "...\n";

  TFile *file = new TFile(filename.c_str());

  std::string flashDirectory3("opflashana3");
  std::string flashDirectory2("opflashana2");
  TTree *flashTree3 = (TTree*)file->GetDirectory(flashDirectory3.c_str())
                                 ->Get         ("PerEventFlashTree");
  TTree *flashTree2 = (TTree*)file->GetDirectory(flashDirectory2.c_str())
                                 ->Get         ("PerEventFlashTree");

  // Variables to read from the tree
  int NFlashes3;
  int NChannels3;
  std::vector< float >* flashTimeVector3             = nullptr;
  std::vector< float >* PEsPerFlashPerChannelVector3 = nullptr;
  std::vector< float >* totalPEVector3               = nullptr;
  flashTree3->SetBranchAddress("NFlashes",        &NFlashes3       );
  flashTree3->SetBranchAddress("NChannels",       &NChannels3      );
  flashTree3->SetBranchAddress("FlashTimeVector", &flashTimeVector3);
  flashTree3->SetBranchAddress("PEsPerFlashPerChannelVector", 
                                      &PEsPerFlashPerChannelVector3);
  flashTree3->SetBranchAddress("TotalPEVector",   &totalPEVector3  );
  int NFlashes2;
  int NChannels2;
  std::vector< float >* flashTimeVector2             = nullptr;
  std::vector< float >* PEsPerFlashPerChannelVector2 = nullptr;
  std::vector< float >* totalPEVector2               = nullptr;
  flashTree2->SetBranchAddress("NFlashes",        &NFlashes2       );
  flashTree2->SetBranchAddress("NChannels",       &NChannels2      );
  flashTree2->SetBranchAddress("FlashTimeVector", &flashTimeVector2);
  flashTree2->SetBranchAddress("PEsPerFlashPerChannelVector", 
                                      &PEsPerFlashPerChannelVector2);
  flashTree2->SetBranchAddress("TotalPEVector",   &totalPEVector2  );

  // Loop through the events filling the histograms
  Long64_t nEntries = flashTree3->GetEntries();
  for (Long64_t entry = 0; entry < nEntries; ++entry) {
    flashTree3->GetEntry(entry);
    flashTree2->GetEntry(entry);

    std::cout << '\n' << "Entry number: " << entry << '\n';

    for (int flashCounter3 = 0; flashCounter3 < NFlashes3; ++flashCounter3) {
      if (NPDsCut(*PEsPerFlashPerChannelVector3, flashCounter3, 
                                         NFlashes3, NChannels3)) continue;
      bool flash3ExistsIn2 = false;
      for (int flashCounter2 = 0; flashCounter2 < NFlashes2; ++flashCounter2) {
        if (NPDsCut(*PEsPerFlashPerChannelVector2, flashCounter2, 
                                           NFlashes2, NChannels2)) continue;
        if (flashTimeVector3->at(flashCounter3) == 
            flashTimeVector2->at(flashCounter2)) {
          flash3ExistsIn2 = true;
          break;
        }
      }

      // Output the flash if it isn't found among flashes
      // that passed the 2-PE flash threshold 
      if (!flash3ExistsIn2) {
        // Due to the calculation of PE in optical hits being
        // uncalibrated, actual threshold is something like 2/3*threshold
        // (don't remember the exact coefficient)
        std::cout << "\nEntry: "       << entry        
                  << " Flash number: " << flashCounter3 
                  << " Total: "        << totalPEVector3->at(flashCounter3) 
                  << " PE Time: "      << flashTimeVector3->at(flashCounter3) 
                  << " us\n";
        int NChannelsPerPD = 12;
        int firstChannel   = flashCounter3*NChannels3;
        int nextFlash      = firstChannel + NChannels3;
        float PEsPerPD = 0.0;
        for (int channelCounter = firstChannel; channelCounter < nextFlash;
                                                          ++channelCounter) {
          float PEs = PEsPerFlashPerChannelVector3->at(channelCounter);
          PEsPerPD += PEs;
          std::cout << std::setw(10) << PEs << ' ';
          if (!((channelCounter - firstChannel + 1)%NChannelsPerPD)) {
            std::cout << ": " << PEsPerPD << '\n';
            PEsPerPD = 0;
          }
        }
      }

    }

  }

  delete file;

}
//-----------------------------------------------------------------------------
// Function to return a vector of strings with names of all .root files 
// starting with "reco_" in the directory
std::vector< std::string > GetRootFiles (std::string const& dir_name) {

  std::vector< std::string > filenames;
  DIR *directory;
  struct dirent *entry;
  if (directory = opendir(dir_name.c_str())) 
    while (entry = readdir(directory)) {
      std::string filename    = entry->d_name;
      std::string extension   = ".root";
      std::string common_part = "flashes_";
      if (filename.find(extension)   != std::string::npos && 
          filename.find(common_part) != std::string::npos)
        filenames.emplace_back(dir_name + "/" + filename);
    }
  else std::cout << "Something's wrong with " << dir_name;

  return filenames;

}

//-----------------------------------------------------------------------------
// Return true if number of PDs fired is greater than fMinimumNPDs
bool NPDsCut(std::vector< float > const& PEsPerFlashPerChannel,
             int const flashID, int const NFlashes, int const NChannels) {

  int minimumNPDs    = 3;
  int NChannelsPerPD = 12;
                        
  // Assume that a PD has some signal on it 
  // if its number of PEs is greater than this
  float minimumPEs = 0.1;

  float PEsPerPD = 0.0;
  int firstChannel = flashID*NChannels;
  int nextFlash    = firstChannel + NChannels;
  int NSignalPDs   = 0;
  for (int channelCounter = firstChannel; channelCounter < nextFlash;
                                                    ++channelCounter) {
    float PEs = PEsPerFlashPerChannel.at(channelCounter);
    PEsPerPD += PEs;
    if (!((channelCounter - firstChannel + 1)%NChannelsPerPD)) {
      if (PEsPerPD > minimumPEs) ++NSignalPDs;
      PEsPerPD = 0;
    }
  }

  if (NSignalPDs >= minimumNPDs) return true;
  else                           return false;

}
