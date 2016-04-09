//=============================================================================
// CompareTwo.C
//
// Gleb Sinev, Duke, 2015
//=============================================================================

#include "CompareTwo.h"

// ROOT includes
#include "TH1F.h"
#include "TEfficiency.h"
#include "TGraphErrors.h"
#include "TGraphAsymmErrors.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TGaxis.h"
#include "TLegend.h"

// C++ includes
#include <sstream>
#include <cmath>

//-----------------------------------------------------------------------------
// Constructor
CompareTwo::CompareTwo(std::string const& option1, std::string const& option2, 
                                  std::string const& minimumNPDs, int NEvents)
                      : fOptions       ( { option1, option2 } )
                      , fMinimumNPDs   ( minimumNPDs          ) 
                      , fNumberOfEvents( NEvents              ) {

  // Fill the vector with threshold values used in reconstruction
  fThresholdValues.emplace_back( 2);
  fThresholdValues.emplace_back( 3);
  fThresholdValues.emplace_back( 4);
  fThresholdValues.emplace_back( 5);
  fThresholdValues.emplace_back( 7);
  fThresholdValues.emplace_back(10);

  // Fill the vector with energy values used in simulation
  fEnergyValues.emplace_back(  8);
  fEnergyValues.emplace_back( 17);
  fEnergyValues.emplace_back(333);
  fEnergyValues.emplace_back(833);

  // Fill the vector with options
//  fOptions.emplace_back("nobg");
//  fOptions.emplace_back("ar39");

  //fDirectoryName = "dune4apa";
  fInputFilename = "flash_time_dune4apa_" + fMinimumNPDs + "_";

  fBackgroundReadoutWindow = 0.0020;          // In seconds
//  fEventReadoutWindow      = 0.001*4.492/2;   // In seconds
  fEventReadoutWindow      = 0.001*4.492;     // In seconds
  fNPDs                    = 40;     
//  fNumberOfEvents          = 100000;


  for (std::string const& option : fOptions) {
    for (int const& energy : fEnergyValues) {
      // Make efficiency versus flash threshold graphs
      std::stringstream efficiencyGraphName;
      efficiencyGraphName << "efficiency_vs_threshold_" << energy 
                                                 << '_' << option;
      fEfficiencyVSThreshold[option][energy] = 
                               new TGraphAsymmErrors(fThresholdValues.size());
      fEfficiencyVSThreshold[option][energy]->SetName(efficiencyGraphName
                                                           .str().c_str());

    }  
    // Make background versus flash threshold graphs
    std::stringstream backgroundGraphName;
    backgroundGraphName << "background_vs_threshold_" << option;
    fBackgroundVSThreshold[option] = new TGraphErrors(fThresholdValues.size());
    fBackgroundVSThreshold[option]->SetName(backgroundGraphName.str().c_str());
  }
 
}

//-----------------------------------------------------------------------------
// Destructor
CompareTwo::~CompareTwo() {

  // Detete the efficiency versus flash threshold graphs
  for (auto const& pairOptionMap : fEfficiencyVSThreshold) 
    for (auto const& pairEnergyGraph : pairOptionMap.second)
      delete pairEnergyGraph.second;
  // Detete the background versus flash threshold graphs
  for (auto const& pairOptionGraph : fBackgroundVSThreshold) 
    delete pairOptionGraph.second;

}

//-----------------------------------------------------------------------------
// Main function in the class used to process the data
void CompareTwo::Fill() {

  // Make an output file with histograms
  std::stringstream outputName;
  outputName << "background_and_efficiency";
  for (std::string const& option : fOptions) outputName << '_' << option;
  outputName << ".root";
  TFile output(outputName.str().c_str(), "RECREATE");

  // Fill the graphs
  for (std::string const& option : fOptions) {
    FillEfficiencyVSThreshold(option);
    FillBackgroundVSThreshold(option);
  }

  // Change ROOT directory to the output file
  output.cd();

  // Save the efficiency versus flash threshold graphs to the file
  for (auto const& pairOptionMap : fEfficiencyVSThreshold) 
    for (auto const& pairEnergyGraph : pairOptionMap.second)
      pairEnergyGraph.second->Write();
  // Save the background versus flash threshold graphs to the file
  for (auto const& pairOptionGraph : fBackgroundVSThreshold) 
    pairOptionGraph.second->Write();

}

//-----------------------------------------------------------------------------
// Use TEfficiencies (as functions of X) to fill the efficiency graphs
void CompareTwo::FillEfficiencyVSThreshold(std::string const& option) {

//  std::stringstream filename;
//  filename << fDirectoryName << option << "/" 
//           << fInputFilename << option << ".root";
  std::string filename = fInputFilename + option + ".root";

  TFile *file = new TFile(filename.c_str());

  for (int const& energy : fEnergyValues) {
    int counter = 0; // Counter to keep track of which graph point we set
    for (int const& threshold : fThresholdValues) {

      // Get the TEfficiency histogram
      std::stringstream efficiencyHistName;
      efficiencyHistName << "efficiency_" << threshold << "_" << energy;
      TEfficiency *efficiencyHist = 
        (TEfficiency*)file->Get(efficiencyHistName.str().c_str());

      // Get average efficiency for that histogram
      TH1F passed(*(TH1F*)efficiencyHist->GetPassedHistogram());
      passed.Rebin(passed.GetNbinsX());
      TH1F total (*(TH1F*)efficiencyHist->GetTotalHistogram() );
      total .Rebin(total .GetNbinsX());
      TEfficiency efficiency(passed, total);

      // Set a point in the efficiency graph
      fEfficiencyVSThreshold[option][energy]->SetPoint(counter, threshold, 
                                              efficiency.GetEfficiency(1));
      fEfficiencyVSThreshold[option][energy]->SetPointEYlow(counter,
                                efficiency.GetEfficiencyErrorLow(1));
      fEfficiencyVSThreshold[option][energy]->SetPointEYhigh(counter,
                                  efficiency.GetEfficiencyErrorUp(1));

      counter++;
    }
  }
  
  delete file;

}

//-----------------------------------------------------------------------------
// Use background TH1Fs (as functions of X) to fill the background graphs
void CompareTwo::FillBackgroundVSThreshold(std::string const& option) {

  std::string filename = fInputFilename + option + ".root";
  //filename << fDirectoryName << option << "/" 
  //         << fInputFilename << option << ".root";

  TFile *file = new TFile(filename.c_str());

  int counter = 0; // Counter to keep track of which graph point we set

  for (int const& threshold : fThresholdValues) {
    // Get the TH1F background histogram
    std::stringstream backgroundHistName;
    backgroundHistName << "background_" << threshold;
    TH1F *backgroundHist = (TH1F*)file->Get(backgroundHistName.str().c_str());

    // Calculate the background rate
    float events      = backgroundHist->Integral();
    float scale       = 1.0/(fBackgroundReadoutWindow*fNumberOfEvents*
                          fBackgroundReadoutWindow/fEventReadoutWindow*fNPDs);
    float rate        = events*scale;
    float uncertainty = std::sqrt(events)*scale;

    // Set a point in the background graph
    fBackgroundVSThreshold[option]->SetPoint(counter, threshold, rate);
    fBackgroundVSThreshold[option]->SetPointError(counter, 0.0, uncertainty);

    ++counter;
  }

  delete file;

}

//-----------------------------------------------------------------------------
// Produce final figures
void CompareTwo::Draw() {

  TCanvas *canvas = new TCanvas("canvas", "", 800, 600);
  //canvas->Divide(2, 1);

  //canvas->cd(1);
  TPad *padLeft  = new TPad("padLeft", "", 0, 0, 1, 1);
  padLeft->Draw();
  padLeft->cd();

  const int energy = fEnergyValues.at(0);
  std::stringstream YAxisTitle;
  YAxisTitle << energy << " MeV electron efficiency";

  TH1F *axesLeft = canvas->DrawFrame(1.0, 0.0, 11.0, 1.1);
  axesLeft->Draw();
  axesLeft->SetXTitle("PE threshold");
  axesLeft->SetYTitle(YAxisTitle.str().c_str());
  axesLeft->GetXaxis()->SetTitleSize(0.05);
  axesLeft->GetYaxis()->SetTitleSize(axesLeft->GetXaxis()->GetTitleSize());
  axesLeft->GetXaxis()->SetLabelSize(axesLeft->GetXaxis()->GetTitleSize());
  axesLeft->GetYaxis()->SetLabelSize(axesLeft->GetXaxis()->GetTitleSize());
  axesLeft->GetXaxis()->SetNdivisions(205);
  axesLeft->GetXaxis()->CenterTitle();
  axesLeft->GetYaxis()->CenterTitle();

  fEfficiencyVSThreshold[fOptions.at(1)][energy]->SetLineWidth(3);
  fEfficiencyVSThreshold[fOptions.at(1)][energy]->Draw("LP");
  fEfficiencyVSThreshold[fOptions.at(0)][energy]->SetLineWidth(3);
  fEfficiencyVSThreshold[fOptions.at(0)][energy]->SetLineStyle(7);
  fEfficiencyVSThreshold[fOptions.at(0)][energy]->Draw("LPsame");
  
  //canvas->cd(1);
  canvas->cd();
  TPad *overlayLeft = new TPad("overlayLeft", "", 0, 0, 1, 1);
  overlayLeft->SetFillStyle(0);
  overlayLeft->SetFillColor(0);
  overlayLeft->SetFrameFillStyle(0);
  //overlayLeft->SetFrameFillColor(0);
  overlayLeft->Draw("FA");
  overlayLeft->cd();

  //fBackgroundVSThreshold[fOptions.at(1)]->SetMarkerColor(kRed);
  double factor = 1000.0;
  DivideGraphByN(fBackgroundVSThreshold[fOptions.at(1)], factor);
  DivideGraphByN(fBackgroundVSThreshold[fOptions.at(0)], factor);
  fBackgroundVSThreshold[fOptions.at(1)]->SetLineColor(kRed);
  fBackgroundVSThreshold[fOptions.at(1)]->SetLineWidth(3);
  fBackgroundVSThreshold[fOptions.at(0)]->SetLineWidth(3);
  fBackgroundVSThreshold[fOptions.at(0)]->SetLineStyle(7);
  fBackgroundVSThreshold[fOptions.at(0)]->SetLineColor(kRed);
  Double_t xMin = padLeft->GetUxmin();
  Double_t yMin = 0;
  Double_t xMax = padLeft->GetUxmax();
  //Double_t yMax = 15.0;
  Double_t yMax = 3.5;

  TH1F *axesOverlayLeft = overlayLeft->DrawFrame(xMin, yMin, xMax, yMax);
  axesOverlayLeft->GetXaxis()->SetLabelOffset(99);
  //axesOverlayLeft->GetXaxis()->SetAxisColor(0);
  axesOverlayLeft->GetYaxis()->SetLabelOffset(99);
  axesOverlayLeft->GetXaxis()->SetNdivisions(0);
  axesOverlayLeft->GetYaxis()->SetNdivisions(0);

  fBackgroundVSThreshold[fOptions.at(1)]->Draw("LP");
  fBackgroundVSThreshold[fOptions.at(0)]->Draw("LPsame");

  TGaxis *axisLeft = new TGaxis(xMax, yMin, xMax, yMax, 
                                 yMin, yMax, 510, "+L");
  axisLeft->SetLineColor(kRed);
  axisLeft->SetLabelColor(kRed);
  axisLeft->SetTitle("Background rate per PD [kHz]");
  axisLeft->CenterTitle();
  axisLeft->SetTitleSize(axesLeft->GetTitleSize());
  axisLeft->SetLabelSize(axesLeft->GetLabelSize());
  //axisLeft->SetTitleOffset(1.2);
  axisLeft->SetTitleFont(axesLeft->GetTitleFont());
  axisLeft->SetLabelFont(axesLeft->GetLabelFont());
  axisLeft->SetTitleColor(kRed);
  axisLeft->SetNdivisions(505);
  axisLeft->SetMaxDigits(4);
  axisLeft->Draw();

  TLegend *legend = new TLegend(0.55, 0.65, 0.85, 0.85);
  //legend->AddEntry(fEfficiencyVSThreshold[fOptions.at(1)][energy], 
  //                                                "With Ar39","l");
  legend->AddEntry(fEfficiencyVSThreshold[fOptions.at(1)][energy], 
                                  fOptions.at(1).c_str(),"l");
  //legend->AddEntry(fEfficiencyVSThreshold[fOptions.at(0)][energy], 
  //                                             "Without Ar39","l");
  legend->AddEntry(fEfficiencyVSThreshold[fOptions.at(0)][energy], 
                                  fOptions.at(0).c_str(),"l");
  legend->SetLineColor(0);
  legend->SetTextSize(axesLeft->GetXaxis()->GetTitleSize());
  legend->SetTextFont(axesLeft->GetXaxis()->GetTitleFont());
  legend->Draw();

}

//-----------------------------------------------------------------------------
// Divide all Y-values (and Y-errors) of a TGraph by n
void CompareTwo::DivideGraphByN(TGraphErrors *graph, double n) {

  int nPoints = graph->GetN();

  for (int point = 0; point < nPoints; ++point) {
    double x    = graph->GetX()[point];
    double y    = graph->GetY()[point]/n;
    double errX = graph->GetErrorX(point);
    double errY = graph->GetErrorY(point)/n;
    graph->SetPoint(point, x, y);
    graph->SetPointError(point, errX, errY);
  }

}
