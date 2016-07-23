//=============================================================================
// ThresholdPlots.C
//
// Gleb Sinev, Duke, 2015
//=============================================================================

#include "ThresholdPlots.h"

// ROOT includes
#include "TH1F.h"
#include "TH1S.h"
#include "TEfficiency.h"
#include "TGraphErrors.h"
#include "TGraphAsymmErrors.h"

// C++ includes
#include <sstream>
#include <cmath>

//-----------------------------------------------------------------------------
// Constructor
ThresholdPlots::ThresholdPlots(std::string const& option,
                               unsigned int const minimumNPDs)
                      : fThresholdValues( { 2, 3, 4, 5, 6, 7, 8, 9, 10 } )
                      , fEnergyValues   ( { 8, 17, 333, 833 } )
                      , fOption         ( option      )
                      , fMinimumNPDs    ( minimumNPDs ) 
                      , fNPDs           ( 120         ) {

  fInputFilename = "flash_time_dune1x2x6" + std::to_string(fMinimumNPDs) + "_" 
                                          + fOption + ".root";

  for (int const& energy : fEnergyValues) {
    // Make efficiency versus flash threshold graphs
    std::stringstream efficiencyGraphName;
    efficiencyGraphName << "efficiency_vs_threshold_" << energy;
    fEfficiencyVSThreshold.emplace
                      (energy, new TGraphAsymmErrors(fThresholdValues.size()));
    fEfficiencyVSThreshold[energy]->SetName(efficiencyGraphName.str().c_str());
  }  

  // Make background versus flash threshold graphs
  std::stringstream backgroundGraphName;
  backgroundGraphName << "background_vs_threshold";
  fBackgroundVSThreshold = new TGraphErrors(fThresholdValues.size());
  fBackgroundVSThreshold->SetName(backgroundGraphName.str().c_str());
 
}

//-----------------------------------------------------------------------------
// Destructor
ThresholdPlots::~ThresholdPlots() {

  // Detete the efficiency versus flash threshold graphs
  for (auto const& pairEnergyGraph : fEfficiencyVSThreshold)
    delete pairEnergyGraph.second;
  // Detete the background versus flash threshold graph
  delete fBackgroundVSThreshold;

}

//-----------------------------------------------------------------------------
// Main function in the class used to process the data
void ThresholdPlots::Fill() {

  // Make an output file with histograms
  std::stringstream outputName;
  outputName << "background_and_efficiency_" << fMinimumNPDs
             << '_' << fOption << ".root";
  TFile output(outputName.str().c_str(), "RECREATE");

  // Fill the graphs
  FillEfficiencyVSThreshold();
  FillBackgroundVSThreshold();

  // Change ROOT directory to the output file
  output.cd();

  // Save the efficiency versus flash threshold graphs to the file
  for (auto const& pairEnergyGraph : fEfficiencyVSThreshold)
    pairEnergyGraph.second->Write();
  // Save the background versus flash threshold graph to the file
  fBackgroundVSThreshold->Write();

}

//-----------------------------------------------------------------------------
// Use TEfficiencies (as functions of X) to fill the efficiency graphs
void ThresholdPlots::FillEfficiencyVSThreshold() {

  TFile file(fInputFilename.c_str());

  for (int const& energy : fEnergyValues) {
    int counter = 0; // Counter to keep track of which graph point we set
    for (int const& threshold : fThresholdValues) {

      // Get the TEfficiency histogram
      std::stringstream efficiencyHistName;
      efficiencyHistName << "efficiency_" << threshold << "_" << energy;
      TEfficiency *efficiencyHist = dynamic_cast< TEfficiency* >
                           (file.Get(efficiencyHistName.str().c_str()));

      // Get average efficiency for that histogram
      TH1F passed(*(TH1F*)efficiencyHist->GetPassedHistogram());
      passed.Rebin(passed.GetNbinsX());
      TH1F total (*(TH1F*)efficiencyHist->GetTotalHistogram() );
      total .Rebin(total .GetNbinsX());
      TEfficiency efficiency(passed, total);

      // Set a point in the efficiency graph
      fEfficiencyVSThreshold[energy]->SetPoint(counter, threshold, 
                                      efficiency.GetEfficiency(1));
      fEfficiencyVSThreshold[energy]->SetPointEYlow(counter,
                        efficiency.GetEfficiencyErrorLow(1));
      fEfficiencyVSThreshold[energy]->SetPointEYhigh(counter,
                          efficiency.GetEfficiencyErrorUp(1));

      counter++;
    }
  }
  
}

//-----------------------------------------------------------------------------
// Use background TH1Fs (as functions of X) to fill the background graph
void ThresholdPlots::FillBackgroundVSThreshold() {

  TFile file(fInputFilename.c_str());

  int counter = 0; // Counter to keep track of which graph point we set

  for (int const& threshold : fThresholdValues) {
    // Get the TH1F background histogram
    std::stringstream backgroundHistName;
    backgroundHistName << "background_" << threshold;
    TH1F *backgroundHist = dynamic_cast< TH1F* >
                           (file.Get(backgroundHistName.str().c_str()));

    unsigned int numberOfEvents = GetNumberOfEvents(file);

    // Calculate the background rate
    float backgroundReadoutWindow = GetXRange(backgroundHist);
    float events      = backgroundHist->Integral();
    float scale       = 1.0/(backgroundReadoutWindow*numberOfEvents*fNPDs);
    float rate        = events*scale;
    float uncertainty = std::sqrt(events)*scale;

    // Set a point in the background graph
    fBackgroundVSThreshold->SetPoint(counter, threshold, rate);
    fBackgroundVSThreshold->SetPointError(counter, 0.0, uncertainty);

    ++counter;
  }

}

//-----------------------------------------------------------------------------
// Get the total number of events by combining the number of entries
// in number_of_flashes_2_{8,17,333,833} for different energy values
// (the flash threshold doesn't matter here, so I chose 2 PE)
unsigned int ThresholdPlots::GetNumberOfEvents(TFile &file) const {

  unsigned int numberOfEvents(0);

  for (int const& energy : fEnergyValues) {
    std::string histogramName("number_of_flashes_2_" + std::to_string(energy));
    TH1S histogram(*dynamic_cast< TH1S* >(file.Get(histogramName.c_str())));
    numberOfEvents += histogram.GetEntries();
  }

  return numberOfEvents;

}

//-----------------------------------------------------------------------------
// Calculate the X range of the histogram
float ThresholdPlots::GetXRange(TH1F *hist) const {

  unsigned int nBins = hist->GetNbinsX();
  float us2s  = 0.001*0.001;
  float range = us2s*(hist->GetBinCenter(nBins) - hist->GetBinCenter(0));

  return range;

}
