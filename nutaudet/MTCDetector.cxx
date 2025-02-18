// Include TGeoMatrix.h instead of TGeoTranslation.h
#include "TGeoMatrix.h"

#include "MTCDetector.h"
//#include "MTCDetectorPoint.h"  // Full definition of our hit point class

// Standard ROOT and FairRoot includes used in FairShip geometry
#include "TGeoManager.h"
#include "TGeoVolume.h"
#include "TGeoBBox.h"
#include "TGeoTrd1.h"
#include "TGeoTube.h"
#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGeoUniformMagField.h"
#include "TClonesArray.h"
#include "TVirtualMC.h"
#include "ShipDetectorList.h"
#include "ShipUnit.h"
#include "ShipStack.h"

#include <iostream>
using std::cout;
using std::endl;

// Helper function: Create a segmented layer (used for SciFi and scintillator)
// This creates a mother volume (a TGeoBBox) subdivided into cells of size cellSizeX × cellSizeY,
// placing each cell with a new TGeoTranslation.
TGeoVolume* CreateSegmentedLayer(const char* name, Double_t width, Double_t height,
                                 Double_t thickness, Double_t cellSizeX, Double_t cellSizeY,
                                 TGeoMedium* material, Int_t color)
{
  // Create the mother solid.
  TGeoBBox* motherSolid = new TGeoBBox(Form("%sMotherSolid", name), width/2.0, height/2.0, thickness/2.0);
  TGeoVolume* motherVol = new TGeoVolume(Form("%sMotherLogical", name), motherSolid, material);
  motherVol->SetLineColor(0); // Do not draw the mother volume

  Int_t nCellsX = static_cast<Int_t>(width / cellSizeX);
  Int_t nCellsY = static_cast<Int_t>(height / cellSizeY);

  // Create the cell solid.
  TGeoBBox* cellSolid = new TGeoBBox(name, cellSizeX/2.0, cellSizeY/2.0, thickness/2.0);
  TGeoVolume* cellVol = new TGeoVolume(Form("%sCell", name), cellSolid, material);
  cellVol->SetLineColor(color);

  // Place each cell using a TGeoTranslation.
  for (Int_t i = 0; i < nCellsX; i++) {
    for (Int_t j = 0; j < nCellsY; j++) {
      Double_t x = -width/2.0 + cellSizeX/2.0 + i * cellSizeX;
      Double_t y = -height/2.0 + cellSizeY/2.0 + j * cellSizeY;
      TGeoTranslation* trans = new TGeoTranslation(x, y, 0);
      motherVol->AddNode(cellVol, i*nCellsY+j, trans);
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
    fLayers(0), fZCenter(zCenter)
 //   fMTCDetectorPointCollection(new TClonesArray("MTCDetectorPoint"))
{
}

MTCDetector::MTCDetector()
  : FairDetector("MTCDetector", kTRUE),
    fWidth(0), fHeight(0), fIronThick(0), fSciFiThick(0), fScintThick(0),
    fLayers(0), fZCenter(0)
 //   fMTCDetectorPointCollection(new TClonesArray("MTCDetectorPoint"))
{
}

MTCDetector::~MTCDetector() {
 // if (fMTCDetectorPointCollection) {
   // fMTCDetectorPointCollection->Delete();
  //  delete fMTCDetectorPointCollection;
  //}
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
  // Compute the thickness of one sandwich unit and total detector thickness.
  Double_t unitThick = fIronThick + fSciFiThick + fScintThick;
  Double_t totalZ = fLayers * unitThick;
  
  // Create an envelope volume for the detector.
  TGeoBBox* envSolid = new TGeoBBox("MTCEnvSolid", fWidth/2.0, fHeight/2.0, totalZ/2.0);
  TGeoMedium* air = gGeoManager->GetMedium("air");
  TGeoVolume* envVol = new TGeoVolume("MTCDetectorVolume", envSolid, air);
  envVol->SetLineColor(kGreen);

  // Place the envelope into the top volume using a TGeoTranslation (shifting by fZCenter along z).
  gGeoManager->GetTopVolume()->AddNode(envVol, 1, new TGeoTranslation(0, 0, fZCenter));

  // Create subcomponent volumes:
  // 1. Magnetised Iron Layer
  TGeoBBox* ironBox = new TGeoBBox("MTCIronBox", fWidth/2.0, fHeight/2.0, fIronThick/2.0);
  TGeoMedium* ironMed = gGeoManager->GetMedium("iron");
  TGeoVolume* ironVol = new TGeoVolume("MTCMagnetisedIron", ironBox, ironMed);
  ironVol->SetLineColor(kGray);

  // 2. Segmented SciFi Layer
  // (For SciFi, we use the scintillator material as an example; adjust as needed.)
  TGeoMedium* sciFiMed = gGeoManager->GetMedium("scintillator");
  TGeoVolume* sciFiLayer = CreateSegmentedLayer("SciFi", fWidth, fHeight, fSciFiThick, 1.0, 1.0, sciFiMed, kBlue);

  // 3. Segmented Scintillator Layer
  TGeoVolume* scintLayer = CreateSegmentedLayer("Scint", fWidth, fHeight, fScintThick, 1.0, 1.0, sciFiMed, kOrange);

  // Loop over layers to place each sandwich unit.
  for (Int_t i = 0; i < fLayers; i++) {
    Double_t unitCenter = -totalZ/2.0 + unitThick/2.0 + i * unitThick;
    
    // Place the iron layer at z = unitCenter - (fSciFiThick+fScintThick)/2.
    envVol->AddNode(ironVol, i*3+1, new TGeoTranslation(0, 0, unitCenter - (fSciFiThick+fScintThick)/2.0));
    
    // Place the SciFi layer at z = unitCenter - (fScintThick)/2.
    envVol->AddNode(sciFiLayer, i*3+2, new TGeoTranslation(0, 0, unitCenter - fScintThick/2.0));
    
    // Place the scintillator layer at z = unitCenter + (fScintThick)/2.
    envVol->AddNode(scintLayer, i*3+3, new TGeoTranslation(0, 0, unitCenter + fScintThick/2.0));
  }
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

//TClonesArray* MTCDetector::GetCollection(Int_t iColl) const {
 // if(iColl == 0) return fMTCDetectorPointCollection;
 // return 0;
//}

void MTCDetector::Reset() {
 // fMTCDetectorPointCollection->Clear();
}

//MTCDetectorPoint* MTCDetector::AddHit(Int_t trackID, Int_t detID,
  //                                     TVector3 pos, TVector3 mom,
   //                                    Double_t time, Double_t length,
     //                                  Double_t eLoss, Int_t pdgCode)
//{
 // TClonesArray &clref = *fMTCDetectorPointCollection;
 // Int_t size = clref.GetEntriesFast();
  //return new(clref[size]) MTCDetectorPoint(trackID, detID, pos, mom, time, length, eLoss, pdgCode);
//}
