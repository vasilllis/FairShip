// SegmentedSciFiLayer.h
#ifndef SEGMENTED_SCIFI_LAYER_H
#define SEGMENTED_SCIFI_LAYER_H

#include "TGeoVolume.h"

// Create a segmented SciFi layer with overall dimensions and cell sizes.
TGeoVolume* CreateSegmentedSciFiLayer(Double_t width, Double_t height, Double_t thickness, Double_t cellX, Double_t cellY);

#endif

