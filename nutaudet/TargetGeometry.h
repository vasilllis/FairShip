// TargetGeometry.h
#ifndef TARGET_GEOMETRY_H
#define TARGET_GEOMETRY_H

#include "TGeoVolume.h"

// Create a target volume with the given width, height, and thickness.
TGeoVolume* CreateTargetGeometry(Double_t width, Double_t height, Double_t thickness);

#endif

