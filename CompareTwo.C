//=============================================================================
// CompareTwo.C
// A macro to make a decent-looking plot
// with background and efficiency plots overlayed on top of each other
// for two output files of ThresholdPlots.C
//
// Gleb Sinev, Duke, 2016
//=============================================================================

// ROOT includes
#include "TH1F.h"
#include "TGraphErrors.h"
#include "TGraphAsymmErrors.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TGaxis.h"
#include "TLegend.h"

// C++ includes
#include <string>
#include <sstream>

void DivideGraphByN(TGraphErrors *graph, double n);

// The format of options is "minimumNPDs_background"
// Energy values are { 8, 17, 333, 833 }
// Example: CompareTwo("1_nobg", "2_nobg", 8, 3.5)
void CompareTwo(std::string const& option1, std::string const& option2,
                unsigned int const energy, float const yMax) {

  std::string filename1 = "background_and_efficiency_" + option1 + ".root";
  TFile *file1 = new TFile(filename1.c_str());

  std::stringstream efficiencyPlotName;
  efficiencyPlotName << "efficiency_vs_threshold_" << energy;
  TGraphAsymmErrors *efficiency1 = dynamic_cast< TGraphAsymmErrors* >
    (file1->Get(efficiencyPlotName.str().c_str()));
  std::string backgroundPlotName("background_vs_threshold");
  TGraphErrors      *background1 = dynamic_cast< TGraphErrors* >
    (file1->Get(backgroundPlotName.c_str()));

  std::string filename2 = "background_and_efficiency_" + option2 + ".root";
  TFile *file2 = new TFile(filename2.c_str());

  TGraphAsymmErrors *efficiency2 = dynamic_cast< TGraphAsymmErrors* >
    (file2->Get(efficiencyPlotName.str().c_str()));
  TGraphErrors      *background2 = dynamic_cast< TGraphErrors* >
    (file2->Get(backgroundPlotName.c_str()));

  TCanvas *canvas = new TCanvas("canvas", "", 800, 600);

  TPad *padLeft  = new TPad("padLeft", "", 0, 0, 1, 1);
  padLeft->Draw();
  padLeft->cd();

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

  efficiency1->SetLineWidth(3);
  efficiency1->Draw("LP");
  efficiency2->SetLineWidth(3);
  efficiency2->SetLineStyle(7);
  efficiency2->Draw("LPsame");
  
  canvas->cd();
  TPad *overlayLeft = new TPad("overlayLeft", "", 0, 0, 1, 1);
  overlayLeft->SetFillStyle(0);
  overlayLeft->SetFillColor(0);
  overlayLeft->SetFrameFillStyle(0);
  overlayLeft->Draw("FA");
  overlayLeft->cd();

  double factor = 1000.0;
  DivideGraphByN(background1, factor);
  DivideGraphByN(background2, factor);
  background1->SetLineColor(kRed);
  background1->SetLineWidth(3);
  background2->SetLineWidth(3);
  background2->SetLineStyle(7);
  background2->SetLineColor(kRed);
  Double_t xMin = padLeft->GetUxmin();
  Double_t yMin = 0;
  Double_t xMax = padLeft->GetUxmax();

  TH1F *axesOverlayLeft = overlayLeft->DrawFrame(xMin, yMin, xMax, yMax);
  axesOverlayLeft->GetXaxis()->SetLabelOffset(99);
  axesOverlayLeft->GetYaxis()->SetLabelOffset(99);
  axesOverlayLeft->GetXaxis()->SetNdivisions(0);
  axesOverlayLeft->GetYaxis()->SetNdivisions(0);

  background1->Draw("LP");
  background2->Draw("LPsame");

  TGaxis *axisLeft = new TGaxis(xMax, yMin, xMax, yMax, 
                                 yMin, yMax, 510, "+L");
  axisLeft->SetLineColor(kRed);
  axisLeft->SetLabelColor(kRed);
  axisLeft->SetTitle("Background rate per PD [kHz]");
  axisLeft->SetTitleOffset(0.6);
  axisLeft->CenterTitle();
  axisLeft->SetTitleSize(axesLeft->GetTitleSize());
  axisLeft->SetLabelSize(axesLeft->GetLabelSize());
  axisLeft->SetTitleFont(axesLeft->GetTitleFont());
  axisLeft->SetLabelFont(axesLeft->GetLabelFont());
  axisLeft->SetTitleColor(kRed);
  axisLeft->SetNdivisions(505);
  axisLeft->SetMaxDigits(4);
  axisLeft->Draw();

  TLegend *legend = new TLegend(0.55, 0.70, 0.85, 0.85);
  legend->AddEntry(efficiency1, option1.c_str(), "l");
  legend->AddEntry(efficiency2, option2.c_str(), "l");
  legend->SetLineColor(0);
  legend->SetTextSize(axesLeft->GetXaxis()->GetTitleSize());
  legend->SetTextFont(axesLeft->GetXaxis()->GetTitleFont());
  legend->Draw();
}

//-----------------------------------------------------------------------------
// Divide all Y-values (and Y-errors) of a TGraph by n
void DivideGraphByN(TGraphErrors *graph, double n) {

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
