//=============================================================================
// EfficiencyPlots.h
//
// Gleb Sinev, Duke, 2015
// This class reads trees located in 
// /pnfs/lbne/persistent/users/gvsinev/photon_detectors/efficiency/
// and makes efficiency plots and time distributions of flashes
// (make_efficiency_plots.C turned class)
//=============================================================================

// C++ includes
#include <map>
#include <vector>
#include <string>

// Forward declarations
class TH1S;
class TH1F;
class TH2F;
class TTree;
class TEfficiency;

class EfficiencyPlots {

  public:

    // Constructor
    // with option being "nobg", "ar39", or "rn222"
    EfficiencyPlots(std::string const& option, int const minimumNPDs, 
                                            bool const debug = false);

    // Destructor
    ~EfficiencyPlots();

    // Process the data, fill the histograms
    void Fill();

  private:

    // Process one ROOT file, filling every histogram we have
    void AnalyzeRootFile(std::string const& filename);

    // Get a list of all ROOT files in one directory
    // that have "flashes_" in their names
    std::vector< std::string > GetRootFiles(std::string const& directory) const;

    // Cuts used mostly to calculate efficiencies
    bool FlashTimeCut(float const flashTime) const;

    // Cuts used to fill histograms
    bool NSignalPDCut(unsigned int const NSignalPDs) const;

    // Get the number of PDs that have hits
    unsigned int GetNSignalPDs
      (std::vector< float > const& PEsPerFlashPerChannel,
       int const flashID, int const NFlashes, int const NChannels) const;

    // Improve the histogram by adjusting its width, etc.
    void ImproveHist(TH1F* const hist);
    void ImproveHist(TH1S* const hist);

    // Get the maximum value for a certain int branch in a TTree
    int TTreeGetMaxIntValue(std::string const& branch, 
                                    TTree* const tree) const;
    
    // Depending on whether we want to process statistics with or without Ar39
    // this string is set to "nobg", "ar39", or "rn222"
    std::string const fOption;

    // Mininmum number of photon detectors with some signal on them
    // that a flash has to have in order to not be discarded
    unsigned int const fMinimumNPDs;

    // Assume that all photon detectors have the same number of channels
    int const fNChannelsPerPD;

    // Vector containing different optical flash threshold values
    std::vector< int > const fThresholdValues;

    // Vector containing different simulated energy values
    std::vector< int > const fEnergyValues;

    // Flash time distributions in the background region 
    // for different threshold values
    std::map< int, TH1F* > fBackgroundHists;

    // PEs-vs-NSignalPDs 2D distribution for background flashes
    // for different threshold values
    std::map< int, TH2F* > fBackgroundPEVsNPDHistMap;
    
    std::map< int, TH2F* > fBackgroundPEVsYWidthHistMap;
    std::map< int, TH2F* > fBackgroundPEVsZWidthHistMap;

    // Flash time distributions in the signal region 
    // for different energy and threshold values
    std::map< int, std::map< int, TH2F* > > fSignalHists;

    // PEs-vs-NSignalPDs 2D distribution for flashes in the signal region
    // for different energy and threshold values
    std::map< int, std::map< int, TH2F* > > fSignalPEVsNPDHistMap;

    std::map< int, std::map< int, TH2F* > > fSignalPEVsYWidthHistMap;
    std::map< int, std::map< int, TH2F* > > fSignalPEVsZWidthHistMap;

    // Efficiency distributions in the singal range 
    // for different energy and threshold values
    std::map< int, std::map< int, TEfficiency* > > fEfficiencyHists;

    // Number of flashes in the signal range distributions 
    // for different energy and threshold values
    std::map< int, std::map< int, TH1S* > > fNumberOfFlashesHists;

    bool const fDebug;

};
