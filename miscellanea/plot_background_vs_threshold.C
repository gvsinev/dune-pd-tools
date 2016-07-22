//=============================================================================
// plot_background_vs_threshold.C
//
// Gleb Sinev, Duke, 2016
// Make presentable the background-versus-threshold plot
// from /dune/app/users/gvsinev/larsoft_pd/
//        test_pd/dune-pd-tools/background_and_efficiency_1_ar39.root
// Also plot background per APA per drift window instead
// instead of background per PD in Hz
//=============================================================================

// ROOT includes
#include "TFile.h"
#include "TH1F.h"
#include "TGraphErrors.h"
#include "TCanvas.h"

// C++ includes
#include <string>

// Function declarations
void RenormalizeGraph(TGraphErrors& graph);

void plot_background_vs_threshold() {

  // File with the histograms
  std::string inputFileName("/dune/app/users/gvsinev/larsoft_pd/test_pd/"
                    "dune-pd-tools/background_and_efficiency_1_ar39.root");
  TFile inputFile(inputFileName.c_str());

  TCanvas canvas("canvas", "^{39}Ar Background vs Threshold", 
                                           0, 10, 2000, 1000);
  canvas.SetLogy();

  float yMin = 0.01;
  float yMax = 30.0;
  TH1F frame(*canvas.DrawFrame(1, yMin, 11, yMax));
  frame.GetXaxis()->SetTitle("Flash threshold (PE)");
  frame.GetYaxis()->SetTitle("Flashes per APA per drift window");
  frame.Draw();

  canvas.SetTitle("");

  std::string backgroundGraphName("background_vs_threshold");
  TGraphErrors backgroundGraph(*dynamic_cast< TGraphErrors* >
                 (inputFile.Get(backgroundGraphName.c_str())));

  RenormalizeGraph(backgroundGraph);

  backgroundGraph.SetLineColor(kRed);
  backgroundGraph.SetLineWidth(3);

  backgroundGraph.Draw("same");

  // Create a PNG file with the output
  canvas.Print("background_vs_threshold.png");

}

void RenormalizeGraph(TGraphErrors& graph) {

  unsigned int PDPerAPA    = 10;
  float        driftWindow = 4.492/2.0*0.001;
  float        scale       = PDPerAPA*driftWindow;

  int nPoints = graph.GetN();

  for (int point = 0; point < nPoints; ++point) {
    double x    = graph.GetX()[point];
    double y    = graph.GetY()[point]*scale;
    double errX = graph.GetErrorX(point);
    double errY = graph.GetErrorY(point)*scale;
    graph.SetPoint(point, x, y);
    graph.SetPointError(point, errX, errY);
  }

}
