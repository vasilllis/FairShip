// TargetGeometry.cxx
#include "TargetGeometry.h"
#include "TGeoManager.h"
#include "TGeoBBox.h"
#include "ShipUnit.h"

TGeoVolume* CreateTargetGeometry(Double_t width, Double_t height, Double_t thickness) {
    // Assume the target is made of lead (make sure "lead" is defined in your media file)
    TGeoMedium *targetMed = gGeoManager->GetMedium("lead");
    TGeoBBox *box = new TGeoBBox("TargetBox", width/2, height/2, thickness/2);
    TGeoVolume *vol = new TGeoVolume("TargetGeometry", box, targetMed);
    vol->SetLineColor(kRed);
    return vol;
}

