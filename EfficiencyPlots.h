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
class TEfficiency;

class EfficiencyPlots {

  public:

    // Constructor
    // with option being "nobg", "ar39", or "rn222"
    EfficiencyPlots(std::string const& option);

    // Destructor
    ~EfficiencyPlots();

    // Process the data, fill the histograms
    void Fill();

  private:

    // Process one ROOT file, filling every histogram we have
    void AnalyzeRootFile(std::string const& filename);

    // Get a list of all ROOT files in one directory
    // that have "reco_" in their names
    std::vector< std::string > GetRootFiles(std::string const& directory) const;

    // A cut used mostly to calculate efficiencies
    bool CutOnFlashTime(float const flashTime) const;

    // Improve the histogram by adjusting its width, etc.
    void ImproveHist(TH1F* const hist);
    void ImproveHist(TH1S* const hist);

    // Depending on whether we want to process statistics with or without Ar39
    // this string is set to "nobg", "ar39", or "rn222"
    std::string const fOption;

    // Vector containing different optical flash threshold values
    std::vector< int > const fThresholdValues;

    // Vector containing different simulated energy values
    std::vector< int > const fEnergyValues;

    // Flash time distributions in the background range 
    // for different threshold values
    std::map< int, TH1F* > fBackgroundHists;

    // Flash time distributions in the singal range 
    // for different energy and threshold values
    std::map< int, std::map< int, TH2F* > > fSignalHists;

    // Efficiency distributions in the singal range 
    // for different energy and threshold values
    std::map< int, std::map< int, TEfficiency* > > fEfficiencyHists;

    // Number of flashes in the signal range distributions 
    // for different energy and threshold values
    std::map< int, std::map< int, TH1S* > > fNumberOfFlashesHists;

};
