// MagnetisedIronLayer.cxx
#include "MagnetisedIronLayer.h"
#include "TGeoManager.h"
#include "TGeoBBox.h"
#include "TGeoUniformMagField.h"
#include "ShipUnit.h"  // provides u.cm, u.m, etc.

TGeoVolume* CreateMagnetisedIronLayer(Double_t width, Double_t height, Double_t thickness) {
    // Retrieve the "iron" medium (ensure it's defined in your media file)
    TGeoMedium *iron = gGeoManager->GetMedium("iron");
    // Create a box with half-dimensions.
    TGeoBBox *box = new TGeoBBox("MagIronBox", width/2, height/2, thickness/2);
    TGeoVolume *vol = new TGeoVolume("MagnetisedIronLayer", box, iron);
    vol->SetLineColor(kGray+1);
    // Apply a uniform magnetic field of 1.7 Tesla along the y-axis.
    TGeoUniformMagField *field = new TGeoUniformMagField(0, 1.7, 0);
    vol->SetField(field);
    return vol;
}

