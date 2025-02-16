#!/usr/bin/env python
"""
Minimal configuration file for testing the new MTS detector geometry in FairShip.

This configuration defines:
  - MTS detector parameters:
      * Active area: 50 x 50 cm²
      * Sandwich unit:
          - Magnetised iron layer: 5 cm thick (will be magnetized with 1.7 T along y)
          - SciFi layer: 0.5 cm thick (segmented into 1×1 cm² cells)
          - Scintillator layer: 2 cm thick (segmented into 1×1 cm² cells)
      * Total number of layers: 50
      * Optional target thickness: 10 cm
  - New detector placement: zC = -20 m (i.e. placed at -20 m along z)

Load this configuration with:
    ship_geo = ConfigRegistry.loadpy("$FAIRSHIP/geometry/mts_config.py", ...)
Then run:
    python $FAIRSHIP/macro/run_simScriptMTS.py
"""

import shipunit as u
from ShipGeoConfig import AttrDict, ConfigRegistry

# Create configuration dictionary "c"
c = AttrDict()

# -----------------------------
# New MTS Detector Parameters
# -----------------------------
c.MTS = AttrDict()
c.MTS.width           = 50 * u.cm       # Overall active area width: 50 cm
c.MTS.height          = 50 * u.cm       # Overall active area height: 50 cm
c.MTS.ironThickness   = 5.0 * u.cm      # Magnetised iron layer thickness
c.MTS.scifiThickness  = 0.5 * u.cm      # SciFi layer thickness
c.MTS.scintThickness  = 2.0 * u.cm      # Scintillator layer thickness
# Granularity for the segmented layers (each cell 1x1 cm²)
c.MTS.scintCellX      = 1.0 * u.cm      
c.MTS.scintCellY      = 1.0 * u.cm      
c.MTS.scifiCellX      = 1.0 * u.cm      
c.MTS.scifiCellY      = 1.0 * u.cm      
c.MTS.nLayers         = 50              # Total number of sandwich layers
c.MTS.targetThickness = 10.0 * u.cm     # Optional target thickness

# -----------------------------
# New Detector Placement
# -----------------------------
c.NewDetector = AttrDict()
# For testing, we place the new detector at z = -20 m.
c.NewDetector.zC = -20 * u.m

# -----------------------------
# Minimal global defaults (if needed)
# These values can be extended later.
c.muShieldDesign    = 7
c.nuTauTargetDesign = 1
c.tankDesign        = 6
c.strawDesign       = 4
c.caloDesign        = 0
c.Yheight           = 10.0

# -----------------------------
# Register this configuration under a new key ("mts_test")
# and set the global ship_geo pointer.
ConfigRegistry.register_config("mts_test", c)
ship_geo = c

# End of mts_config.py

