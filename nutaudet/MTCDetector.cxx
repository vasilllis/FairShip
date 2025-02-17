#include "MTCDetector.h"
#include "TGeoManager.h"
#include "TGeoVolumeAssembly.h"
#include "TGeoBBox.h"
#include "TGeoTranslation.h"
#include "TString.h"
#include "ShipUnit.h"  // For unit definitions (e.g. cm, m)
#include <iostream>
using std::cout;
using std::endl;

// Helper function to create a segmented layer (SciFi or scintillator)
// Parameters:
//   name           : Base name (e.g., "SciFi" or "Scint")
//   width, height  : Active area dimensions (cm)
//   thickness      : Layer thickness (cm)
//   cellSizeX, cellSizeY : Cell dimensions (cm), here 1×1 cm
//   material       : Material used (retrieved from gGeoManager)
//   color          : Visualization color
TGeoVolume* CreateSegmentedLayer(const char* name, Double_t width, Double_t height,
                                 Double_t thickness, Double_t cellSizeX, Double_t cellSizeY,
                                 TGeoMedium* material, Int_t color)
{
  // Mother volume (the full active area)
  TGeoBBox* motherSolid = new TGeoBBox(Form("%sMotherSolid",name), width/2.0, height/2.0, thickness/2.0);
  TGeoVolume* motherVol = new TGeoVolume(Form("%sMotherLogical",name), motherSolid, material);
  motherVol->SetLineColor(0); // transparent frame

  // Determine number of cells along x and y:
  Int_t nCellsX = static_cast<Int_t>(width / cellSizeX);
  Int_t nCellsY = static_cast<Int_t>(height / cellSizeY);

  // Create one cell (a box)
  TGeoBBox* cellSolid = new TGeoBBox(name, cellSizeX/2.0, cellSizeY/2.0, thickness/2.0);
  TGeoVolume* cellVol = new TGeoVolume(Form("%sCell",name), cellSolid, material);
  cellVol->SetLineColor(color);
  
  // Place the cells in a grid filling the mother volume.
  for (Int_t i = 0; i < nCellsX; i++) {
    for (Int_t j = 0; j < nCellsY; j++) {
      Double_t x = -width/2.0 + cellSizeX/2.0 + i * cellSizeX;
      Double_t y = -height/2.0 + cellSizeY/2.0 + j * cellSizeY;
      motherVol->AddNode(cellVol, i * nCellsY + j, new TGeoTranslation(x, y, 0));
    }
  }
  return motherVol;
}

//
// MTCDetector implementation
//

ClassImp(MTCDetector)

MTCDetector::MTCDetector(const char* name, const Double_t zCenter, Bool_t Active, const char* Title)
  : FairDetector(name, Active),
    fWidth(0), fHeight(0), fIronThick(0), fSciFiThick(0), fScintThick(0),
    fLayers(0), fZCenter(zCenter),
    fMTCDetectorPointCollection(new TClonesArray("MTCDetectorPoint"))
{
}

MTCDetector::MTCDetector()
  : FairDetector("MTCDetector", kTRUE),
    fWidth(0), fHeight(0), fIronThick(0), fSciFiThick(0), fScintThick(0),
    fLayers(0), fZCenter(0),
    fMTCDetectorPointCollection(new TClonesArray("MTCDetectorPoint"))
{
}

MTCDetector::~MTCDetector() {
  if (fMTCDetectorPointCollection) {
    fMTCDetectorPointCollection->Delete();
    delete fMTCDetectorPointCollection;
  }
}

void MTCDetector::SetMTCParameters(Double_t width, Double_t height,
                                   Double_t ironThick, Double_t sciFiThick, Double_t scintThick,
                                   Int_t nLayers, Double_t zCenter)
{
  fWidth       = width;
  fHeight      = height;
  fIronThick   = ironThick;
  fSciFiThick  = sciFiThick;
  fScintThick  = scintThick;
  fLayers      = nLayers;
  fZCenter     = zCenter;
}

void MTCDetector::ConstructGeometry() {
  // Retrieve the top volume
  TGeoVolume *top = gGeoManager->GetTopVolume();
  // Create an assembly for the complete MTC detector.
  TGeoVolumeAssembly* mtcAssembly = new TGeoVolumeAssembly("MTCDetectorAssembly");
  top->AddNode(mtcAssembly, 100, new TGeoTranslation(0,0,0));

  // Build the sandwich layers.
  // Each layer unit is composed of three sub-layers.
  Double_t unitThick = fIronThick + fSciFiThick + fScintThick;
  // Start stacking from z = 0.
  Double_t currentZ = 0.0;
  for (Int_t i = 0; i < fLayers; i++) {
    // Create an assembly for layer unit i.
    TGeoVolumeAssembly* unit = new TGeoVolumeAssembly(Form("MTCUnit_%d", i+1));

    // --- 1. Magnetised Iron Layer ---
    // Create a box representing the iron layer.
    TGeoBBox* ironBox = new TGeoBBox("MTCIronBox", fWidth/2.0, fHeight/2.0, fIronThick/2.0);
    TGeoMedium* ironMed = gGeoManager->GetMedium("iron"); // Must be defined in your media file.
    TGeoVolume* ironVol = new TGeoVolume("MTCMagnetisedIron", ironBox, ironMed);
    ironVol->SetLineColor(kGray);
    // Place the iron layer at z = - (fSciFiThick + fScintThick)/2.
    unit->AddNode(ironVol, 1, new TGeoTranslation(0, 0, - (fSciFiThick + fScintThick)/2.0));

    // --- 2. Segmented SciFi Layer ---
    // Create SciFi layer as a segmented box using our helper function.
    TGeoMedium* scintMed = gGeoManager->GetMedium("scintillator"); // For example, use scintillator for fiber core.
    TGeoVolume* sciFiLayer = CreateSegmentedLayer("SciFi", fWidth, fHeight, fSciFiThick, 1.0, 1.0, scintMed, kBlue);
    // Place SciFi layer at z = - fScintThick/2.
    unit->AddNode(sciFiLayer, 1, new TGeoTranslation(0, 0, - fScintThick/2.0));

    // --- 3. Segmented Scintillator Layer ---
    // Create scintillator layer similarly.
    TGeoVolume* scintLayer = CreateSegmentedLayer("Scint", fWidth, fHeight, fScintThick, 1.0, 1.0, scintMed, kOrange);
    // Place scintillator layer at z = + fScintThick/2.
    unit->AddNode(scintLayer, 1, new TGeoTranslation(0, 0, fScintThick/2.0));

    // Place the entire unit at its proper z position.
    Double_t unitCenter = currentZ + unitThick/2.0;
    mtcAssembly->AddNode(unit, i+2, new TGeoTranslation(0, 0, unitCenter));
    currentZ += unitThick;
  }

  // Finally, shift the entire assembly to its global z placement.
  TGeoTranslation* globalTrans = new TGeoTranslation(0, 0, fZCenter);
  mtcAssembly->SetMatrix(globalTrans);
}

void MTCDetector::Initialize() {
  FairDetector::Initialize();
}

Bool_t MTCDetector::ProcessHits(FairVolume* /*vol*/) {
  // This passive detector does not process hits.
  return kTRUE;
}

void MTCDetector::Register() {
  FairRootManager::Instance()->Register("MTCDetectorPoint", "MTCDetector",
                                        fMTCDetectorPointCollection, kTRUE);
}

TClonesArray* MTCDetector::GetCollection(Int_t iColl) const {
  if(iColl == 0) return fMTCDetectorPointCollection;
  return 0;
}

void MTCDetector::Reset() {
  fMTCDetectorPointCollection->Clear();
}

MTCDetectorPoint* MTCDetector::AddHit(Int_t trackID, Int_t detID,
                                       TVector3 pos, TVector3 mom,
                                       Double_t time, Double_t length,
                                       Double_t eLoss, Int_t pdgCode)
{
  TClonesArray &clref = *fMTCDetectorPointCollection;
  Int_t size = clref.GetEntriesFast();
  return new(clref[size]) MTCDetectorPoint(trackID, detID, pos, mom, time, length, eLoss, pdgCode);
}
