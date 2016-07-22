//=============================================================================
// ThresholdPlots.h
//
// Gleb Sinev, Duke, 2015
// This class reads the output of the EfficiencyPlots class
// and produces efficiency and background vs threshold graphs
//=============================================================================

// ROOT includes
#include "TFile.h"

// C++ includes
#include <map>
#include <vector>
#include <string>

class TH1F;
class TGraphErrors;
class TGraphAsymmErrors;

class ThresholdPlots {

  public: 

    // Constructor
    ThresholdPlots(std::string const& option,
                   unsigned int const minimumNPDs);

    // Destructor
    ~ThresholdPlots();

    // Process the data, fill the histograms
    void Fill();

  private:

    // Fill the graphs after everything else is filled
    void FillEfficiencyVSThreshold();
    void FillBackgroundVSThreshold();

    unsigned int GetNumberOfEvents(TFile &file) const;

    float GetXRange(TH1F *hist) const;

    // Vector containing different optical flash threshold values
    std::vector< int > const fThresholdValues;

    // Vector containing different simulated energy values
    std::vector< int > const fEnergyValues;

    // Vector containing strings for additional options 
    // (to specify whether argon-39 background was used in simulation)
    std::string const fOption;

    // Minimum number of PDs with signal on them
    unsigned int const fMinimumNPDs;

    // Efficiency versus flash threshold
    // for different energy values
    std::map< int, TGraphAsymmErrors* > fEfficiencyVSThreshold;

    // Background rate versus flash threshold
    // (to make this a rate we have to scale the Y axis by
    //  1/(N events * time of the event used to measure the background
    //                              * fraction of the full event time))
    TGraphErrors* fBackgroundVSThreshold;

    // Output filename of the EfficiencyPlots class
    std::string fInputFilename;

    // Number of photon detectors in the geometry
    unsigned int const fNPDs;

};
