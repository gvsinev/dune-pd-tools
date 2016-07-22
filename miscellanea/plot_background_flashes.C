//=============================================================================
// plot_background_flashes.C
//
// Gleb Sinev, Duke, 2016
// Plot background flash distributions for different flash thresholds 
// from /dune/app/users/gvsinev/
//        larsoft_pd/test_pd/dune-pd-tools/flash_time_dune4apa_3_rn222.root
// overlaid on top of each other
//=============================================================================

// ROOT includes
#include "TFile.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TLegend.h"

// C++ includes
#include <vector>
#include <string>
#include <iostream>

// Declare the functions used
void ImproveHist(TH1F* const hist, int color);

void plot_background_flashes() {

  // File with the histograms
  std::string inputFileName("/dune/app/users/gvsinev/larsoft_pd/test_pd/"
                         "dune-pd-tools/flash_time_dune4apa_3_rn222.root");

  // Make a vector containing different optical flash threshold values
  std::vector< int > thresholdValuesVector{ 2, 3, 4, 5, 7, 10 };

  TFile inputFile(inputFileName.c_str());

  TCanvas canvas("canvas", "Background flashes", 0, 10, 2000, 1000);

  float yMin = 0.0;
  float yMax = 1.0;
  TH1F frame(*canvas.DrawFrame(100.0, yMin, 2100.0, yMax));
  frame.Draw();
  frame.GetXaxis()->SetTitle("Flash time (#mus)");
  frame.GetYaxis()->SetTitle("Flashes");

  TLegend legend(0.70, 0.50, 0.90, 0.90);

  int color = 1;
  for (int const& threshold : thresholdValuesVector) { 
    std::string histogramName("background_" + to_string(threshold));
    TH1F *histogram = 
      dynamic_cast< TH1F* >(inputFile.Get(histogramName.c_str()));
    ImproveHist(histogram, color);
    histogram->Draw("same");
    std::string legendEntry(to_string(threshold) + " PE threshold");
    legend.AddEntry(histogram, legendEntry.c_str(), "l");
    float currentYMax = histogram->GetMaximum();
    if (yMax < currentYMax) yMax = currentYMax;
    frame.SetMaximum(1.1*yMax);
    legend.Draw();
    canvas.Update();
    ++color;
  }

  // Create a PNG file with the output
  canvas.Print("background_flashes.png");

}

//-----------------------------------------------------------------------------
// Make histograms prettier
void ImproveHist(TH1F* const hist, int color) {

  hist->SetLineWidth(2);
  hist->SetStats(kFALSE);
  hist->SetMinimum(0);
  hist->SetLineColor(color);

}
