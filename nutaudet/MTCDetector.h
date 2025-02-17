#ifndef MTCDETECTOR_H
#define MTCDETECTOR_H

#include "FairDetector.h"     // Base detector class from FairShip
#include "TClonesArray.h"     // For storing hit points
#include "TVector3.h"
#include "TLorentzVector.h"

// Forward declaration for the hit point class (implement similar to ShipRpcPoint)
class MTCDetectorPoint;

class MTCDetector : public FairDetector {
public:
  // Constructors and destructor
  MTCDetector(const char* name, const Double_t zCenter, Bool_t Active, const char* Title="MTCDetector");
  MTCDetector();
  virtual ~MTCDetector();

  /**
   * Set all detector parameters.
   * All dimensions are in centimeters.
   *
   * @param width         Active area width (e.g. 50 cm)
   * @param height        Active area height (e.g. 50 cm)
   * @param ironThick     Thickness of the magnetised iron layer (e.g. 5 cm)
   * @param sciFiThick    Thickness of the SciFi layer (e.g. 0.5 cm)
   * @param scintThick    Thickness of the scintillator layer (e.g. 2 cm)
   * @param nLayers       Number of sandwich layers (e.g. 50)
   * @param zCenter       Global placement along z (in cm)
   */
  void SetMTCParameters(Double_t width, Double_t height,
                        Double_t ironThick, Double_t sciFiThick, Double_t scintThick,
                        Int_t nLayers, Double_t zCenter);

  /**
   * Constructs the complete detector geometry.
   * This method builds a sandwich of nLayers.
   * Each sandwich unit consists of:
   *   • A magnetised iron layer (with a 1.7 T field along the y axis)
   *   • A segmented SciFi layer (built as a 50×50 cm² box subdivided into 1×1 cm cells)
   *   • A segmented scintillator layer (similarly segmented)
   * Finally, the entire assembly is shifted to the global z position.
   */
  virtual void ConstructGeometry();

  // Standard FairDetector methods
  virtual void Initialize();
  virtual Bool_t ProcessHits(FairVolume* vol=0);
  virtual void Register();
  virtual TClonesArray* GetCollection(Int_t iColl) const;
  virtual void Reset();

  // Example method to add a hit point (if needed)
  MTCDetectorPoint* AddHit(Int_t trackID, Int_t detID,
                           TVector3 pos, TVector3 mom,
                           Double_t time, Double_t length,
                           Double_t eLoss, Int_t pdgCode);

private:
  // Detector parameters (in centimeters)
  Double_t fWidth;       // Active area width
  Double_t fHeight;      // Active area height
  Double_t fIronThick;   // Magnetised iron layer thickness
  Double_t fSciFiThick;  // SciFi layer thickness
  Double_t fScintThick;  // Scintillator layer thickness
  Int_t    fLayers;      // Number of sandwich layers
  Double_t fZCenter;     // Global z placement

  // Collection for hit points
  TClonesArray* fMTCDetectorPointCollection;

  // Copy constructor and assignment operator (not implemented)
  MTCDetector(const MTCDetector&);
  MTCDetector& operator=(const MTCDetector&);

  ClassDef(MTCDetector,1)
};

#endif // MTCDETECTOR_H
