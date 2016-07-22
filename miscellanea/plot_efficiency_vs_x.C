//=============================================================================
// plot_efficiency_vs_x.C
//
// Gleb Sinev, Duke, 2016
// Make presentable the efficiency-versus-X plot
// from /dune/app/users/gvsinev/larsoft_pd/
//        test_pd/dune-pd-tools/flash_time_dune1x2x6_1_nobg.root
// for a certain flash threshold
//=============================================================================

// ROOT includes
#include "TFile.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TEfficiency.h"

// C++ includes
#include <string>

// Function declarations

void plot_efficiency_vs_x(unsigned int threshold) {

  // File with the histograms
  std::string inputFileName("/dune/app/users/gvsinev/larsoft_pd/test_pd/"
                         "dune-pd-tools/flash_time_dune1x2x6_1_nobg.root");
  TFile inputFile(inputFileName.c_str());

  TCanvas canvas("canvas", 
                 "Efficiency versus flash threshold for 8-MeV electrons", 
                                                       0, 10, 2000, 1000);

  float xMin = 0.0;
  float xMax = 360.0;
  float yMin = 0.0;
  float yMax = 1.05;
  TH1F frame(*canvas.DrawFrame(xMin, yMin, xMax, yMax));
  frame.GetXaxis()->SetTitle("X (cm)");
  frame.GetYaxis()->SetTitle("8-MeV-electron flash efficiency");
  frame.Draw();

  canvas.SetTitle("");

  std::string efficiencyGraphName("efficiency_" + 
                                  std::to_string(threshold) + "_8");
  TEfficiency efficiencyGraph(*dynamic_cast< TEfficiency* >
               (inputFile.Get(efficiencyGraphName.c_str())));

  efficiencyGraph.SetLineColor(kBlack);
  efficiencyGraph.SetLineWidth(3);

  efficiencyGraph.Draw("same");

  // Create a PNG file with the output
  std::string outputFileName("efficiency_vs_x_" + std::to_string(threshold) + ".png");
  canvas.Print(outputFileName.c_str());

}
