// SegmentedScintillatorLayer.cxx
#include "SegmentedScintillatorLayer.h"
#include "TGeoManager.h"
#include "TGeoBBox.h"
#include "TGeoVolumeAssembly.h"
#include "ShipUnit.h"

TGeoVolume* CreateSegmentedScintillatorLayer(Double_t width, Double_t height, Double_t thickness, Double_t cellX, Double_t cellY) {
    int nCellsX = static_cast<int>(width/cellX);
    int nCellsY = static_cast<int>(height/cellY);
    TGeoVolumeAssembly *segLayer = new TGeoVolumeAssembly("SegmentedScintillatorLayer");
    // Retrieve the "scintillator" medium.
    TGeoMedium *scint = gGeoManager->GetMedium("scintillator");
    TGeoBBox *cellBox = new TGeoBBox("ScintCellBox", cellX/2, cellY/2, thickness/2);
    TGeoVolume *cellVol = new TGeoVolume("ScintCell", cellBox, scint);
    cellVol->SetLineColor(kGreen);
    // Center the grid.
    Double_t startX = -width/2 + cellX/2;
    Double_t startY = -height/2 + cellY/2;
    for (int ix = 0; ix < nCellsX; ix++) {
        Double_t xPos = startX + ix * cellX;
        for (int iy = 0; iy < nCellsY; iy++) {
            Double_t yPos = startY + iy * cellY;
            int copyNo = ix * nCellsY + iy;
            segLayer->AddNode(cellVol, copyNo, new TGeoTranslation(xPos, yPos, 0));
        }
    }
    return segLayer;
}

