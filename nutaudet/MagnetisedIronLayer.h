// MagnetisedIronLayer.h
#ifndef MAGNETISED_IRON_LAYER_H
#define MAGNETISED_IRON_LAYER_H

#include "TGeoVolume.h"

// Create a magnetised iron layer with given width, height, and thickness.
TGeoVolume* CreateMagnetisedIronLayer(Double_t width, Double_t height, Double_t thickness);

#endif

