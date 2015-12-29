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

// Other includes
#include <dirent.h>


//-----------------------------------------------------------------------------
// Constructor
EfficiencyPlots::EfficiencyPlots(std::string const& option) 
                                : fOption(option)
                                , fThresholdValues( { 2, 3, 4, 5, 7, 10 } )
                                , fEnergyValues   ( { 8, 17, 333, 833   } ) {

  // Create all the histograms
  for (int const& threshold : fThresholdValues) {
    // Make flash time histograms for the background range
    std::stringstream backgroundHistName;
    backgroundHistName << "background_" << threshold;
    fBackgroundHists[threshold] = new TH1F(backgroundHistName.str().c_str(), 
                                           "; t [#mus]; Flashes", 20, 100.0, 
                                                                     2100.0);
    ImproveHist(fBackgroundHists[threshold]);

    for (int const& energy : fEnergyValues) {
      // Make flash time histograms for the signal range
      std::stringstream signalHistName;
      signalHistName << "signal_" << threshold << "_" << energy;
      fSignalHists[threshold][energy] = 
        new TH2F(signalHistName.str().c_str(), "; X [cm]; t [#mus];", 
                                    35, -350.0, 350.0, 20, 0.0, 20.0);

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
                         "; Number of Flashes; Events", 1000, 0, 1000);
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
  // Delete the signal histograms
  for (auto const& signalHistThreshold : fSignalHists) 
    for (auto const& signalHist : signalHistThreshold.second)
      delete signalHist.second;
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
  std::string output_name = "flash_time_dune4apa" + fOption + ".root";
  TFile output(output_name.c_str(), "RECREATE");

  // Directory where we keep our data
  std::string data_dir = 
    "/pnfs/lbne/scratch/users/gvsinev/photon_detectors/"
    "efficiency/dune4apa" + fOption + "/root";

  // Get a vector containing names of datafiles
  std::vector< std::string > filenames = GetRootFiles(data_dir);

  // Analyze each file
  for (auto const& filename : filenames) AnalyzeRootFile(filename);

  // Change ROOT directory to the output file
  output.cd();

  // Save the background histograms to the file
  for (auto const& backgroundHist : fBackgroundHists)
    backgroundHist.second->Write();
  // Save the signal histograms to the file
  for (auto const& signalHistThreshold : fSignalHists) 
    for (auto const& signalHist : signalHistThreshold.second)
      signalHist.second->Write();
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

  for (int const& threshold : fThresholdValues) { 
    std::stringstream directory;
    directory << "anatree" << threshold;
    TTree *tree = (TTree*)file->GetDirectory(directory.str().c_str())
                              ->Get("anatree");

    // Variables to read from the tree
    Int_t   flashTotal;
    Float_t flashTime[1000];
    Float_t trkMomentum[1000];
    Float_t trkStartX[1000];
    tree->SetBranchAddress("flash_total", &flashTotal);
    tree->SetBranchAddress("flash_time",    flashTime);
    tree->SetBranchAddress("trkmom_MC",   trkMomentum);
    tree->SetBranchAddress("trkstartx_MC",  trkStartX);

    // Loop through the events filling the histograms
    Long64_t nEntries = tree->GetEntries();
    for (Long64_t entry = 0; entry < nEntries; ++entry) {
      tree->GetEntry(entry);

      // Find energy of the primary particle 
      // and fill respective histogram
      for (int const& energy : fEnergyValues)
        // Assume that energy values are quite spread, so we can be sure 
        // that there is only one value within [0.8, 1.2] of any energy value
        // (trkMomentum is in GeV, while energy is in MeV)
        if ((1000*trkMomentum[0] > 0.8*float(energy)) 
            && (1000*trkMomentum[0] < 1.2*float(energy))) {
          bool flash_signal = false;
          short number_of_flashes = 0;
          for (int counter = 0; counter < flashTotal; ++counter) {
            fSignalHists[threshold][energy]->Fill(trkStartX[0],
                                                  flashTime[counter]);
            // Assume we see the signal if there is at least one flash
            // with FlashTime < 10 us
            if (flashTime[counter] < 10.0) {
              flash_signal = true;
              ++number_of_flashes;
            }
          }
          fEfficiencyHists[threshold][energy]->Fill(flash_signal, 
                                                    trkStartX[0]);
          fNumberOfFlashesHists[threshold][energy]->Fill(number_of_flashes);
        }

      for (int counter = 0; counter < flashTotal; ++counter)
        fBackgroundHists[threshold]->Fill(flashTime[counter]);
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
      std::string common_part = "anatree_";
      if (filename.find(extension)   != std::string::npos && 
          filename.find(common_part) != std::string::npos)
        filenames.emplace_back(dir_name + "/" + filename);
    }
  else std::cout << "Something's wrong with " << dir_name;

  return filenames;

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
