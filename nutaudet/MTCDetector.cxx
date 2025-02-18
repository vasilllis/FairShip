#include "MTCDetector.h"
#include "TGeoManager.h"
#include "TGeoVolume.h"
#include "TGeoBBox.h"
#include "TGeoMedium.h"
#include "TGeoUniformMagField.h"
#include "FairRootManager.h"

TGeoVolume* CreateSegmentedLayer(const char* name, Double_t width, Double_t height,
                                Double_t thickness, Double_t cellSizeX, Double_t cellSizeY,
                                TGeoMedium* material, Int_t color, Double_t transparency) {
    TGeoBBox* mother = new TGeoBBox(Form("%s_mother", name), width/2, height/2, thickness/2);
    TGeoVolume* motherVol = new TGeoVolume(Form("%s_mother", name), mother, material);
    motherVol->SetLineColor(color);
    motherVol->SetTransparency(transparency);

    TGeoBBox* cell = new TGeoBBox(Form("%s_cell", name), cellSizeX/2, cellSizeY/2, thickness/2);
    TGeoVolume* cellVol = new TGeoVolume(Form("%s_cell", name), cell, material);
    cellVol->SetLineColor(color);
    cellVol->SetTransparency(transparency);

    Int_t nX = Int_t(width/cellSizeX);
    Int_t nY = Int_t(height/cellSizeY);
    
    for(Int_t i=0; i<nX; i++) {
        for(Int_t j=0; j<nY; j++) {
            Double_t x = -width/2 + cellSizeX*(i+0.5);
            Double_t y = -height/2 + cellSizeY*(j+0.5);
            motherVol->AddNode(cellVol, i*nY+j, new TGeoTranslation(x, y, 0));
        }
    }
    return motherVol;
}

MTCDetector::MTCDetector(const char* name, Double_t zCenter, Bool_t Active, const char* Title, Int_t DetId)
    : FairDetector(name, Active, DetId),
      fWidth(0), fHeight(0), fIronThick(0), fSciFiThick(0), fScintThick(0),
      fLayers(0), fZCenter(zCenter), fFieldY(0), fMTCDetectorPointCollection(nullptr) {}

MTCDetector::~MTCDetector() {
    if(fMTCDetectorPointCollection) {
        fMTCDetectorPointCollection->Delete();
        delete fMTCDetectorPointCollection;
    }
}

void MTCDetector::SetMTCParameters(Double_t w, Double_t h, Double_t iron, 
                                  Double_t sciFi, Double_t scint, Int_t layers,
                                  Double_t z, Double_t field) {
    fWidth = w;
    fHeight = h;
    fIronThick = iron;
    fSciFiThick = sciFi;
    fScintThick = scint;
    fLayers = layers;
    fZCenter = z;
    fFieldY = field;
}

void MTCDetector::ConstructGeometry() {
    Double_t layerThickness = fIronThick + fSciFiThick + fScintThick;
    Double_t totalLength = fLayers * layerThickness;

    // Envelope (green transparent)
    TGeoBBox* env = new TGeoBBox("MTC_env", fWidth/2, fHeight/2, totalLength/2);
    TGeoVolume* envVol = new TGeoVolume("MTC", env, gGeoManager->GetMedium("air"));
    envVol->SetLineColor(kGreen);
    envVol->SetTransparency(50);

    // Iron layer with magnetic field (gray)
    TGeoBBox* iron = new TGeoBBox("MTC_iron", fWidth/2, fHeight/2, fIronThick/2);
    TGeoVolume* ironVol = new TGeoVolume("MTC_iron", iron, gGeoManager->GetMedium("iron"));
    ironVol->SetLineColor(kGray+1);
    ironVol->SetTransparency(20);
    ironVol->SetField(new TGeoUniformMagField(0, fFieldY, 0));

    // SciFi layer (green)
    TGeoVolume* sciFiVol = CreateSegmentedLayer("MTC_sciFi", fWidth, fHeight, 
                                               fSciFiThick, 1.0, 1.0, 
                                               gGeoManager->GetMedium("SciFiMat"),
                                               kGreen+2, 40);  

    // Scintillator layer (blue)
    TGeoVolume* scintVol = CreateSegmentedLayer("MTC_scint", fWidth, fHeight,
                                               fScintThick, 1.0, 1.0,
                                               gGeoManager->GetMedium("SciFiMat"),
                                               kAzure+7, 30);  

    // Layer assembly
    for(Int_t i=0; i<fLayers; i++) {
        Double_t zPos = -totalLength/2 + (i+0.5)*layerThickness;
        envVol->AddNode(ironVol, i, new TGeoTranslation(0, 0, zPos - (fSciFiThick + fScintThick)/2));
        envVol->AddNode(sciFiVol, i, new TGeoTranslation(0, 0, zPos - fScintThick/2));
        envVol->AddNode(scintVol, i, new TGeoTranslation(0, 0, zPos + fIronThick/2));
    }

    gGeoManager->GetTopVolume()->AddNode(envVol, 1, new TGeoTranslation(0, 0, fZCenter));
}

// Standard FairDetector methods
void MTCDetector::Initialize() { FairDetector::Initialize(); }
Bool_t MTCDetector::ProcessHits(FairVolume*) { return kTRUE; }
void MTCDetector::Register() { /* Implementation if needed */ }
TClonesArray* MTCDetector::GetCollection(Int_t) const { return nullptr; }
void MTCDetector::Reset() { /* Implementation if needed */ }

ClassImp(MTCDetector)