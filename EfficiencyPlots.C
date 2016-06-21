//=============================================================================
// EfficiencyPlots.C
//
// Gleb Sinev, Duke, 2015
//=============================================================================

#include "EfficiencyPlots.h"

// ROOT includes
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TEfficiency.h"

// C++ includes
#include <iostream>
#include <sstream>
#include <iomanip>

// Other includes
#include <dirent.h>

//-----------------------------------------------------------------------------
// Constructor
EfficiencyPlots::EfficiencyPlots(std::string const& option, 
                                 int const  minimumNPDs, bool const debug) 
                                : fOption         ( option                )
                                , fMinimumNPDs    ( minimumNPDs           )
                                , fNChannelsPerPD ( 12                    )
                                , fThresholdValues( { 2, 3, 4, 5, 7, 10 } )
                                , fEnergyValues   ( { 8, 17, 333, 833   } ) 
                                , fDebug          ( debug                 ) {

  // Create all the histograms
  for (int const& threshold : fThresholdValues) {
    // Make flash time histograms for the background region
    std::stringstream backgroundHistName;
    backgroundHistName << "background_" << threshold;
    fBackgroundHists[threshold] = new TH1F(backgroundHistName.str().c_str(), 
                                           "; t [#mus]; Flashes", 20, 100.0, 
                                                                     2100.0);
    ImproveHist(fBackgroundHists[threshold]);
    // Make PEs-vs-NSignalPDs 2D histograms for background flashes
    std::string backgroundPEVsNPDHistName("background_pe_vs_npd_" 
                                          + std::to_string(threshold));
    fBackgroundPEVsNPDHistMap.emplace
      (threshold, new TH2F(backgroundPEVsNPDHistName.c_str(), 
                           ";PEs;number of PDs with signal", 200, 0.0, 20.0,
                                                              20,   0, 20  ));

    for (int const& energy : fEnergyValues) {
      // Make flash time histograms for the signal region
      std::stringstream signalHistName;
      signalHistName << "signal_" << threshold << "_" << energy;
      fSignalHists[threshold][energy] = 
        new TH2F(signalHistName.str().c_str(), "; X [cm]; t [#mus];", 
                                    35, -350.0, 350.0, 20, 0.0, 20.0);

      // Make PEs-vs-NSignalPDs 2D histograms for flashes in the signal region
      std::string signalPEVsNPDHistName("signal_pe_vs_npd_" 
                                        + std::to_string(threshold) + '_' 
                                                 + std::to_string(energy));
      fSignalPEVsNPDHistMap[threshold].emplace
        (energy, new TH2F(signalPEVsNPDHistName.c_str(), 
                          ";PEs;number of PDs with signal", 400, 0.0, 40.0,
                                                             40,   0, 40  ));

      // Make efficiency histograms
      std::stringstream efficiencyHistName;
      efficiencyHistName << "efficiency_" << threshold << "_" << energy;
      fEfficiencyHists[threshold][energy] = 
        new TEfficiency(efficiencyHistName.str().c_str(), 
                        "; X [cm]; Efficiency;", 35, -350.0, 350.0);

      // Make number of flashes histograms
      std::stringstream numberOfFlashesHistName;
      numberOfFlashesHistName << "number_of_flashes_" << threshold 
                                                      << "_" << energy;
      fNumberOfFlashesHists[threshold][energy] = 
        new TH1S(numberOfFlashesHistName.str().c_str(), 
                         "; Number of Flashes; Events", 100, 0, 100);
      ImproveHist(fNumberOfFlashesHists[threshold][energy]);
    }
  }

}

//-----------------------------------------------------------------------------
// Destructor
EfficiencyPlots::~EfficiencyPlots() {

  // Delete the background histograms
  for (auto const& backgroundHist : fBackgroundHists)
    delete backgroundHist.second;
  for (auto const& intHistPair : fBackgroundPEVsNPDHistMap)
    delete intHistPair.second;
  // Delete the signal histograms
  for (auto const& signalHistThreshold : fSignalHists) 
    for (auto const& signalHist : signalHistThreshold.second)
      delete signalHist.second;
  for (auto const& intIntHistPair : fSignalPEVsNPDHistMap)
    for (auto const& intHistPair : intIntHistPair.second)
      delete intHistPair.second;
  // Delete the efficiency histograms
  for (auto const& efficiencyHistThreshold : fEfficiencyHists) 
    for (auto const& efficiencyHist : efficiencyHistThreshold.second)
      delete efficiencyHist.second;
  // Delete the number of flashes histograms
  for (auto const& numberOfFlashesHistThreshold : fNumberOfFlashesHists) 
    for (auto const& numberOfFlashesHist : numberOfFlashesHistThreshold.second)
      delete numberOfFlashesHist.second;
 
}

//-----------------------------------------------------------------------------
// Main function in the class used to process the data
void EfficiencyPlots::Fill() {

  // Make an output file with histograms
  std::stringstream outputName;
  outputName << "flash_time_dune4apa_" << fMinimumNPDs << '_' 
                                       << fOption      << ".root";
  TFile output(outputName.str().c_str(), "RECREATE");

  // Directory where the data is (or even are) kept
  std::string dataDir = 
    "/pnfs/lbne/scratch/users/gvsinev/photon_detectors/"
    "efficiency/dune4apa_" + fOption + "/root";

  // Get a vector containing names of datafiles
  std::vector< std::string > filenames = GetRootFiles(dataDir);

  // Analyze each file
  int counter = 0;
  for (auto const& filename : filenames) {
    std::cout << counter++ << ". ";
    AnalyzeRootFile(filename);
  }

  // Change ROOT directory to the output file
  output.cd();

  // Save the background histograms to the file
  for (auto const& backgroundHist : fBackgroundHists)
    backgroundHist.second->Write();
  for (auto const& intHistPair : fBackgroundPEVsNPDHistMap)
    intHistPair.second->Write();
  // Save the signal histograms to the file
  for (auto const& signalHistThreshold : fSignalHists) 
    for (auto const& signalHist : signalHistThreshold.second)
      signalHist.second->Write();
  for (auto const& intIntHistPair : fSignalPEVsNPDHistMap)
    for (auto const& intHistPair : intIntHistPair.second)
      intHistPair.second->Write();
  // Save the efficiency histograms to the file
  for (auto const& efficiencyHistThreshold : fEfficiencyHists) 
    for (auto const& efficiencyHist : efficiencyHistThreshold.second)
      efficiencyHist.second->Write();
  // Save the number of flashes histograms to the file
  for (auto const& numberOfFlashesHistThreshold : fNumberOfFlashesHists) 
    for (auto const& numberOfFlashesHist : numberOfFlashesHistThreshold.second)
      numberOfFlashesHist.second->Write();
  
}

//-----------------------------------------------------------------------------
// Function to fill the histograms with data from one root file
void EfficiencyPlots::AnalyzeRootFile(std::string const& filename) {

  std::cout << "Processing " << filename << "...\n";

  TFile *file = new TFile(filename.c_str());

  std::string anaTreeDirectory = "anatree";
  TTree *anaTree = (TTree*)file->GetDirectory(anaTreeDirectory.c_str())
                               ->Get("anatree");

  for (int const& threshold : fThresholdValues) { 
    if (fDebug) std::cout << '\n' << "Threshold: " << threshold << "\n\n";
    std::stringstream flashDirectory;
    flashDirectory << "opflashana" << threshold;
    TTree *flashTree = (TTree*)file->GetDirectory(flashDirectory.str().c_str())
                                   ->Get         ("PerEventFlashTree");

    // Variables to read from the tree
    float trkMomentum[1000];
    float trkStartX[1000];
    anaTree->SetBranchAddress("trkmom_MC",    trkMomentum);
    anaTree->SetBranchAddress("trkstartx_MC", trkStartX  );
    int NFlashes;
    int NChannels;
    std::vector< float >* flashTimeVector             = nullptr;
    std::vector< float >* PEsPerFlashPerChannelVector = nullptr;
    std::vector< float >* totalPEVector               = nullptr;
    flashTree->SetBranchAddress("NFlashes",        &NFlashes       );
    flashTree->SetBranchAddress("NChannels",       &NChannels      );
    flashTree->SetBranchAddress("FlashTimeVector", &flashTimeVector);
    flashTree->SetBranchAddress("PEsPerFlashPerChannelVector", 
                                       &PEsPerFlashPerChannelVector);
    flashTree->SetBranchAddress("TotalPEVector",   &totalPEVector  );

    // Loop through the events filling the histograms
    Long64_t nEntries = flashTree->GetEntries();
    for (Long64_t entry = 0; entry < nEntries; ++entry) {
      anaTree  ->GetEntry(entry);
      flashTree->GetEntry(entry);

      if (fDebug) std::cout << '\n' << "Entry number: " << entry << '\n';

      // Find energy of the primary particle 
      // and fill respective histogram
      for (int const& energy : fEnergyValues) 
        // Assume that energy values are quite spread, so we can be sure 
        // that there is only one value within [0.8, 1.2] of any energy value
        // (trkMomentum is in GeV, while energy is in MeV)
        if ((1000*trkMomentum[0] > 0.8*float(energy)) && 
            (1000*trkMomentum[0] < 1.2*float(energy))) {
          bool  flashSignal     = false;
          short numberOfFlashes = 0;

          // Should I make this loop a function?
          for (int flashCounter = 0; flashCounter < NFlashes; 
                                              ++flashCounter) {
            // Due to the calculation of PE in optical hits being
            // uncalibrated, actual threshold is something like 2/3*threshold
            // (don't remember the exact coefficient)
            if (fDebug) std::cout << "\nThreshold: " << threshold    << 
                                       " PE Entry: " << entry        << 
                                   " Flash number: " << flashCounter << 
                     " Total: " <<   totalPEVector->at(flashCounter) << 
                   " PE Time: " << flashTimeVector->at(flashCounter) << " us\n";
            unsigned int NSignalPDs = 
              GetNSignalPDs(*PEsPerFlashPerChannelVector, flashCounter, 
                                                   NFlashes, NChannels);
            if (NSignalPDCut(NSignalPDs)) {
              fSignalHists[threshold][energy]->Fill(trkStartX[0], 
                               flashTimeVector->at(flashCounter));
              fBackgroundHists[threshold]
                        ->Fill(flashTimeVector->at(flashCounter));
              // Assume we see the signal if there is at least one flash
              // passing the cut
              if (fDebug) std::cout << "Accepted\n";
              if (FlashTimeCut(flashTimeVector->at(flashCounter))) {
                flashSignal = true;
                ++numberOfFlashes;
                fSignalPEVsNPDHistMap[threshold][energy]
                  ->Fill(totalPEVector->at(flashCounter), NSignalPDs);
              }
              else {
                fBackgroundPEVsNPDHistMap[threshold]
                  ->Fill(totalPEVector->at(flashCounter), NSignalPDs);
              }
            }
            if (fDebug) std::cout << '\n';
          }

          fEfficiencyHists[threshold][energy]->Fill(flashSignal, trkStartX[0]);
          fNumberOfFlashesHists[threshold][energy]->Fill(numberOfFlashes);
        }

    }

  }

  delete file;

}

//-----------------------------------------------------------------------------
// Function to return a vector of strings with names of all .root files 
// starting with "reco_" in the directory
std::vector< std::string > EfficiencyPlots::GetRootFiles
                                         (std::string const& dir_name) const {

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
// Return true if -1 us < flashTime < 10 us, false otherwise
bool EfficiencyPlots::FlashTimeCut(float const flashTime) const {

  return ((flashTime < 10.0) && (flashTime > -1.0));

}

//-----------------------------------------------------------------------------
// Return true if the number of PDs fired is greater than fMinimumNPDs
bool EfficiencyPlots::NSignalPDCut(unsigned int const NSignalPDs) const {
                        
  return (NSignalPDs >= fMinimumNPDs);

}

//-----------------------------------------------------------------------------
// Return the number of PDs with hits
unsigned int EfficiencyPlots::GetNSignalPDs
       (std::vector< float > const& PEsPerFlashPerChannel, 
        int const flashID, int const NFlashes, int const NChannels) const {
                        
  // Assume that a PD has some signal on it 
  // if its number of PEs is greater than this
  float minimumPEs = 0.1;

  float PEsPerPD          = 0.0;
  int firstChannel        = flashID*NChannels;
  int nextFlash           = firstChannel + NChannels;
  unsigned int NSignalPDs = 0;
  for (int channelCounter = firstChannel; channelCounter < nextFlash;
                                                    ++channelCounter) {
    float PEs = PEsPerFlashPerChannel.at(channelCounter);
    PEsPerPD += PEs;
    if (fDebug) std::cout << std::setw(10) << PEs << ' ';
    if (!((channelCounter - firstChannel + 1)%fNChannelsPerPD)) {
      if (PEsPerPD > minimumPEs) ++NSignalPDs;
      if (fDebug) std::cout << ": " << PEsPerPD << '\n';
      PEsPerPD = 0;
    }
  }

  return NSignalPDs;

}

//-----------------------------------------------------------------------------
// Make the histogram prettier by adjusting parameters like line width
void EfficiencyPlots::ImproveHist(TH1F* const hist) {

  hist->SetLineWidth(2);
  hist->SetStats(kFALSE);
  hist->SetMinimum(0);

}

void EfficiencyPlots::ImproveHist(TH1S* const hist) {

  hist->SetLineWidth(2);
  hist->SetMinimum(0);

}
