//=============================================================================
// CompareTwo.h
//
// Gleb Sinev, Duke, 2015
// This class reads the output of EfficiencyPlots with and without argon-39
// and produces graphs to compare the two
//=============================================================================

// C++ includes
#include <map>
#include <vector>
#include <string>

class TGraphErrors;
class TGraphAsymmErrors;

class CompareTwo {

  public: 

    // Constructor
    CompareTwo(std::string const& option1, std::string const& option2,
                          std::string const& minimumNPDs, int NEvents);

    // Destructor
    ~CompareTwo();

    // Process the data, fill the histograms
    void Fill();

    // Produce final figures
    void Draw();

  private:

    // Fill the graphs after everything else is filled
    void FillEfficiencyVSThreshold(std::string const& option);
    void FillBackgroundVSThreshold(std::string const& option);

    void DivideGraphByN(TGraphErrors *graph, double n);

    // Vector containing different optical flash threshold values
    std::vector< int > fThresholdValues;

    // Vector containing different simulated energy values
    std::vector< int > fEnergyValues;

    // Vector containing strings for additional options 
    // (to specify whether argon-39 background was used in simulation)
    std::vector< std::string > const fOptions;

    // Minimum number of PDs with signal on them
    std::string const fMinimumNPDs;

    // Efficiency versus flash threshold
    // for different energy values
    std::map< std::string, std::map< int, TGraphAsymmErrors* > >
                                                     fEfficiencyVSThreshold;

    // Background rate versus flash threshold
    // for different energy values
    // (to make this a rate we have to scale the Y axis by
    //  1/(N events * time of the event used to measure the background
    //                              * fraction of the full event time))
    std::map< std::string, TGraphErrors* > fBackgroundVSThreshold;

    // Common part in the name of the directories where the input files are
    std::string fDirectoryName;

    // Common part in the name of the input files
    std::string fInputFilename;

    // Simulation parameters required to calculate the backround rate
    float fBackgroundReadoutWindow;
    float fEventReadoutWindow;
    unsigned int const fNumberOfEvents;
    unsigned int fNPDs;

};
