# dune-pd-tools
A collections of tools used (and, usually, created) by me to work on DUNE photon detector R&amp;D.

## Efficiency Chain

### EfficiencyPlots
This class reads the output of the `AnaTree` and `OpFlashAna` LArSoft modules
and produces background and signal flash time plots, efficiency histograms, and number of flashes plots
for different flash thresholds and simulated (signal) electron energies.

### ThresholdPlots
This class reads the output of the `EfficiencyPlots` class and produces plots
of background rate and single electron efficiency (for different primary electron energies)
as a function of the flash threshold.

### CompareTwo
This script reads the output of the `ThresholdPlots` class
and produces a final plot of single electron efficiency (for one primary electron energy)
and background rate (overlaid) as functions of the flash threshold.

## Other tools

### plot\_background\_flashes.C
This script plots background flash time plots for different flash thresholds on top of each other.
It runs on the output of the `EfficiencyPlots` class (for now the input file is hardcoded).

### diff\_thresholds\_2\_3.C
This script outputs information about flashes that appear in the 3-PE-threshold sample, but not in the 2-PE-threshold one.
