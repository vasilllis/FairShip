#include "TGeoManager.h"
#include "TGeoVolumeAssembly.h"
#include "TGeoTranslation.h"
#include "TString.h"
#include "ShipUnit.h"   // provides u.cm, u.m, etc.
#include "DetectorAssembly.h"  // Declaration of BuildDetectorGeometry()

// Include the module headers that build individual parts of the detector:
#include "MagnetisedIronLayer.h"
#include "SegmentedSciFiLayer.h"
#include "SegmentedScintillatorLayer.h"
#include "TargetGeometry.h"

// The global configuration pointer is provided by your Python configuration
// (loaded via ConfigRegistry.loadpy). Its type is assumed to be ShipGeoConfig.
extern const ShipGeoConfig *ship_geo;

//-----------------------------------------------------------------------------
// BuildDetectorGeometry() - Build the new detector geometry using configuration
//-----------------------------------------------------------------------------
void BuildDetectorGeometry() {
    // Get the top-level volume from the global TGeoManager.
    TGeoVolume *top = gGeoManager->GetTopVolume();

    // Create an assembly for the entire detector.
    TGeoVolumeAssembly *detector = new TGeoVolumeAssembly("DetectorAssembly");
    top->AddNode(detector, 1, new TGeoTranslation(0, 0, 0));

    // Retrieve dimensions from the configuration (defined in geometry_config.py)
    Double_t width           = ship_geo->MTS.width;
    Double_t height          = ship_geo->MTS.height;
    Double_t thicknessIron   = ship_geo->MTS.ironThickness;
    Double_t thicknessSciFi  = ship_geo->MTS.scifiThickness;
    Double_t thicknessScint  = ship_geo->MTS.scintThickness;
    Double_t scintCellX      = ship_geo->MTS.scintCellX;
    Double_t scintCellY      = ship_geo->MTS.scintCellY;
    Double_t scifiCellX      = ship_geo->MTS.scifiCellX;
    Double_t scifiCellY      = ship_geo->MTS.scifiCellY;
    Int_t    nLayers         = ship_geo->MTS.nLayers;
    Double_t targetThickness = ship_geo->MTS.targetThickness;

    // --- Create and place the target ---
    TGeoVolume *targetVol = CreateTargetGeometry(width, height, targetThickness);
    // Place the target so that its center is at z = -targetThickness/2.
    detector->AddNode(targetVol, 1, new TGeoTranslation(0, 0, -targetThickness/2));

    // --- Build the sandwich units ---
    // Each sandwich unit consists of:
    // 1. A magnetised iron layer (5 cm thick)
    // 2. A segmented SciFi layer (0.5 cm thick)
    // 3. A segmented scintillator layer (2 cm thick)
    // The total thickness of one sandwich unit is:
    Double_t layerTotal = thicknessIron + thicknessSciFi + thicknessScint;
    // We start placing layers just in front of the target.
    Double_t currentZ = targetThickness/2;

    for (int i = 0; i < nLayers; i++) {
        // Create an assembly for this sandwich unit.
        TGeoVolumeAssembly *layerAssembly = new TGeoVolumeAssembly(Form("Layer_%d", i+1));

        // Create each component using our helper functions.
        TGeoVolume *ironLayer  = CreateMagnetisedIronLayer(width, height, thicknessIron);
        TGeoVolume *scifiLayer = CreateSegmentedSciFiLayer(width, height, thicknessSciFi, scifiCellX, scifiCellY);
        TGeoVolume *scintLayer = CreateSegmentedScintillatorLayer(width, height, thicknessScint, scintCellX, scintCellY);

        // Place the components within the sandwich unit:
        // Convention (local z within the unit):
        // - Iron layer at bottom: center at -(scifiThickness + scintThickness)/2
        Double_t zIron = - (thicknessSciFi + thicknessScint) / 2.0;
        layerAssembly->AddNode(ironLayer, 1, new TGeoTranslation(0, 0, zIron));

        // - SciFi layer above iron: center at -thicknessScint/2.
        Double_t zSciFi = - thicknessScint / 2.0;
        layerAssembly->AddNode(scifiLayer, 1, new TGeoTranslation(0, 0, zSciFi));

        // - Scintillator layer on top: center at +thicknessScint/2.
        Double_t zScint = thicknessScint / 2.0;
        layerAssembly->AddNode(scintLayer, 1, new TGeoTranslation(0, 0, zScint));

        // Place the sandwich unit in the overall detector.
        Double_t layerCenter = currentZ + layerTotal/2.0;
        detector->AddNode(layerAssembly, i+2, new TGeoTranslation(0, 0, layerCenter));

        // Update the current z position for the next sandwich unit.
        currentZ += layerTotal;
    }

    // Optionally, if you want to reposition your detector (for instance, at the old NuTau detector location)
    // you can translate the entire detector assembly using the zC value from the configuration.
    // For example:
    // detector->SetMatrix(new TGeoTranslation(0, 0, ship_geo->NewDetector.zC));
}

//-----------------------------------------------------------------------------
// Main function: Build geometry and launch visualization
//-----------------------------------------------------------------------------
#ifdef BUILD_DETECTOR_ASSEMBLY_MAIN
#include "TApplication.h"
#include "TCanvas.h"
#include <iostream>
int main(int argc, char** argv) {
    // Create a ROOT application to support the GUI.
    TApplication app("DetectorAssemblyApp", &argc, argv);

    // Build the detector geometry.
    BuildDetectorGeometry();

    // Close the geometry to lock it.
    gGeoManager->CloseGeometry();

    // Create a canvas for visualization.
    TCanvas* c1 = new TCanvas("c1", "New Detector Geometry", 800, 600);

    // Draw the top volume using the OpenGL option ("ogl") for interactive viewing.
    gGeoManager->GetTopVolume()->Draw("ogl");

    // Enter the ROOT event loop.
    app.Run();

    return 0;
}
#endif
