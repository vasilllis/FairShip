// SegmentedScintillatorLayer.h
#ifndef SEGMENTED_SCINTILLATOR_LAYER_H
#define SEGMENTED_SCINTILLATOR_LAYER_H

#include "TGeoVolume.h"

// Create a segmented scintillator layer with overall width x height, thickness, and cell sizes.
TGeoVolume* CreateSegmentedScintillatorLayer(Double_t width, Double_t height, Double_t thickness, Double_t cellX, Double_t cellY);

#endif

