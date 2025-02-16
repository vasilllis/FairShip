#!/usr/bin/env python
import os
import sys
import ROOT
ROOT.gSystem.Load('libEGPythia8')

import shipunit as u
import shipRoot_conf
import rootUtils as ut
from ShipGeoConfig import ConfigRegistry
from argparse import ArgumentParser

DownScaleDiMuon = False

# Default HNL parameters
theHNLMass   = 1.0*u.GeV
theProductionCouplings = theDecayCouplings = None

# Default dark photon parameters
theDPmass    = 0.2*u.GeV

motherMode = True

mcEngine     = "TGeant4"
simEngine    = "Pythia8"  # Options: "Pythia8", "Genie", "Ntuple", etc.
inclusive    = "c"

MCTracksWithHitsOnly   = False
MCTracksWithEnergyCutOnly = True
MCTracksWithHitsOrEnergyCut = False

charmonly    = False
HNL          = True

inputFile    = "/eos/experiment/ship/data/Charm/Cascade-parp16-MSTP82-1-MSEL4-978Bpot.root"
defaultInputFile = True

globalDesigns = {
     '2016' : { 'dy' : 10., 'dv' : 5, 'ds' : 7, 'nud' : 1, 'caloDesign' : 0, 'strawDesign' : 4 },
     '2018' : { 'dy' : 10., 'dv' : 6, 'ds' : 9, 'nud' : 3, 'caloDesign' : 3, 'strawDesign' : 10 },
     '2022' : { 'dy' : 8.,  'dv' : 6, 'ds' : 9, 'nud' : 3, 'caloDesign' : 3, 'strawDesign' : 10 },
     '2023' : { 'dy' : 6.,  'dv' : 6, 'ds' : 8, 'nud' : 4, 'caloDesign' : 3, 'strawDesign' : 10 }
}
default = '2023'

inactivateMuonProcesses = False

# Parse command-line arguments (unchanged)
parser = ArgumentParser()
group = parser.add_mutually_exclusive_group()
parser.add_argument("--evtcalc", help="Use EventCalc", action="store_true")
parser.add_argument("--Pythia6", dest="pythia6", help="Use Pythia6", required=False, action="store_true")
parser.add_argument("--Pythia8", dest="pythia8", help="Use Pythia8", required=False, action="store_true")
parser.add_argument("--PG",      dest="pg",      help="Use Particle Gun", required=False, action="store_true")
parser.add_argument("--pID",     dest="pID",     help="Particle ID (default=22)", required=False, default=22, type=int)
parser.add_argument("--Estart", help="Start energy (default=10 GeV)", default=10, type=float)
parser.add_argument("--Eend", help="End energy (default=10 GeV)", default=10, type=float)
parser.add_argument("-A", dest="A", help="A option", required=False, default='c')
parser.add_argument("--Genie", dest="genie", help="Use Genie", action="store_true")
parser.add_argument("--NuRadio", dest="nuradio", help="Use NuRadio", action="store_true")
parser.add_argument("--Ntuple", dest="ntuple", help="Use ntuple", action="store_true")
parser.add_argument("--MuonBack", dest="muonback", help="Use muon background file", action="store_true")
parser.add_argument("--FollowMuon", dest="followMuon", help="Follow muons", action="store_true")
parser.add_argument("--FastMuon", dest="fastMuon", help="Fast muon only", action="store_true")
parser.add_argument("--Nuage", dest="nuage", help="Use Nuage", action="store_true")
parser.add_argument("--phiRandom", dest="phiRandom", help="Random phi", action="store_true")
parser.add_argument("--Cosmics", dest="cosmics", help="Use cosmic generator", required=False, default=None)
parser.add_argument("--MuDIS", dest="mudis", help="Use muonDIS", action="store_true")
parser.add_argument("--RpvSusy", dest="RPVSUSY", help="Use RPVSUSY", action="store_true")
parser.add_argument("--DarkPhoton", dest="DarkPhoton", help="Use dark photon generator", action="store_true")
parser.add_argument("--SusyBench", dest="RPVSUSYbench", help="SusyBench parameter", required=False, default=2)
parser.add_argument("-m", "--mass", dest="theMass", help="Mass of hidden particle", required=False, default=None, type=float)
parser.add_argument("-c", "--couplings", dest="thecouplings", help="HNL couplings", required=False, default="0.447e-9,7.15e-9,1.88e-9")
parser.add_argument("-cp", "--production-couplings", dest="theprodcouplings", help="Production couplings", required=False, default=None)
parser.add_argument("-cd", "--decay-couplings", dest="thedeccouplings", help="Decay couplings", required=False, default=None)
parser.add_argument("-e", "--epsilon", dest="theDPepsilon", help="Epsilon", required=False, default=0.00000008, type=float)
parser.add_argument("-n", "--nEvents", dest="nEvents", help="Number of events", required=False, default=100, type=int)
parser.add_argument("-i", "--firstEvent", dest="firstEvent", help="First event", required=False, default=0, type=int)
parser.add_argument("-s", "--seed", dest="theSeed", help="Random seed", required=False, default=0, type=int)
parser.add_argument("-S", "--sameSeed", dest="sameSeed", help="Same seed", required=False, default=False, type=int)
group.add_argument("-f", dest="inputFile", help="Input file", required=False, default=False)
parser.add_argument("-g", dest="geofile", help="geofile for muon shield", required=False, default=None)
parser.add_argument("-o", "--output", dest="outputDir", help="Output directory", required=False, default=".")
parser.add_argument("-Y", dest="dy", help="max height of vacuum tank", required=False, default=globalDesigns[default]['dy'])
parser.add_argument("--tankDesign", dest="dv", help="Tank design", required=False, default=globalDesigns[default]['dv'], type=int)
parser.add_argument("--muShieldDesign", dest="ds", help="Muon shield design", required=False, choices=range(7,11), default=globalDesigns[default]['ds'], type=int)
parser.add_argument("--nuTauTargetDesign", dest="nud", help="nuTauTargetDesign", required=False, default=globalDesigns[default]['nud'], type=int)
parser.add_argument("--caloDesign", help="Calo design", default=globalDesigns[default]['caloDesign'], type=int, choices=[0,2,3])
parser.add_argument("--strawDesign", dest="strawDesign", help="Straw design", required=False, default=globalDesigns[default]['strawDesign'], type=int)
parser.add_argument("-F", dest="deepCopy", help="Deep copy option", required=False, action="store_true")
parser.add_argument("-t", "--test", dest="testFlag", help="Quick test", required=False, action="store_true")
parser.add_argument("--dry-run", dest="dryrun", help="Dry run (stop after init)", required=False, action="store_true")
parser.add_argument("-D", "--display", dest="eventDisplay", help="Store trajectories", required=False, action="store_true")
parser.add_argument("--stepMuonShield", dest="muShieldStepGeo", help="Step geometry for muon shield", required=False, action="store_true", default=False)
parser.add_argument("--coMuonShield", dest="muShieldWithCobaltMagnet", help="Replace a magnet with cobalt", required=False, type=int, default=0)
parser.add_argument("--noSC", dest="SC_mag", help="Deactivate SC muon shield", action='store_false')
parser.add_argument("--scName", help="SC shield name", default="sc_v6")
parser.add_argument("--MesonMother", dest="MM", help="Meson production source", required=False, default=True)
parser.add_argument("--debug", help="Debug option", required=False, default=0, type=int, choices=range(0,3))
parser.add_argument("--helium", dest="decayVolMed", help="Set Decay Volume medium to helium", action="store_const", const="helium", default="helium")
parser.add_argument("--vacuums", dest="decayVolMed", help="Set Decay Volume medium to vacuum", action="store_const", const="vacuums", default="helium")
parser.add_argument("--SND", dest="SND", help="Activate SND", action='store_true')
parser.add_argument("--noSND", dest="SND", help="Deactivate SND", action='store_false')

options = parser.parse_args()

if options.evtcalc:  simEngine = "EvtCalc"
if options.pythia6:  simEngine = "Pythia6"
if options.pythia8:  simEngine = "Pythia8"
if options.pg:       simEngine = "PG"
if options.genie:    simEngine = "Genie"
if options.nuradio:  simEngine = "nuRadiography"
if options.ntuple:   simEngine = "Ntuple"
if options.muonback: simEngine = "MuonBack"
if options.nuage:    simEngine = "Nuage"
if options.mudis:    simEngine = "muonDIS"
if options.A != 'c':
    inclusive = options.A
    if options.A =='b': inputFile = "/eos/experiment/ship/data/Beauty/Cascade-run0-19-parp16-MSTP82-1-MSEL5-5338Bpot.root"
    if options.A.lower() == 'charmonly':
           charmonly = True
           HNL = False
    if options.A not in ['b','c','bc','meson','pbrem','qcd']: inclusive = True
if options.MM:
    motherMode = options.MM
if options.cosmics:
    simEngine = "Cosmics"
    Opt_high = int(options.cosmics)
if options.inputFile:
    if options.inputFile == "none": options.inputFile = None
    inputFile = options.inputFile
    defaultInputFile = False
if options.RPVSUSY: HNL = False
if options.DarkPhoton: HNL = False
if not options.theMass:
    if options.DarkPhoton: options.theMass  = theDPmass
    else:                  options.theMass  = theHNLMass
if options.thecouplings:
    theCouplings = [float(c) for c in options.thecouplings.split(",")]
if options.theprodcouplings:
    theProductionCouplings = [float(c) for c in options.theprodcouplings.split(",")]
if options.thedeccouplings:
    theDecayCouplings = [float(c) for c in options.thedeccouplings.split(",")]
if options.testFlag:
    inputFile = "$FAIRSHIP/files/Cascade-parp16-MSTP82-1-MSEL4-76Mpot_1_5000.root"

# Sanity check (unchanged)
if (HNL and options.RPVSUSY) or (HNL and options.DarkPhoton) or (options.DarkPhoton and options.RPVSUSY):
    print("cannot have HNL and SUSY or DP at the same time, abort")
    sys.exit(2)

if (simEngine == "Genie" or simEngine == "nuRadiography") and defaultInputFile:
    inputFile = "/eos/experiment/ship/data/GenieEvents/genie-nu_mu.root"
if simEngine == "muonDIS" and defaultInputFile:
    print('input file required if simEngine = muonDIS')
    sys.exit()
if simEngine == "Nuage" and not inputFile:
    inputFile = 'Numucc.root'

print("FairShip setup for", simEngine, "to produce", options.nEvents, "events")
if (simEngine == "Ntuple" or simEngine == "MuonBack") and defaultInputFile:
    print('input file required if simEngine = Ntuple or MuonBack')
    sys.exit()
ROOT.gRandom.SetSeed(options.theSeed)
shipRoot_conf.configure(0)

# -----------------------------------------------------------------------------
# NEW GEOMETRY: load the new configuration file for the MTS detector.
# We now load our minimal configuration file "mts_config.py" (located in $FAIRSHIP/geometry/)
ship_geo = ConfigRegistry.loadpy(
    "$FAIRSHIP/geometry/mts_config.py",  # NEW: use our new minimal configuration file
    Yheight=options.dy,
    tankDesign=options.dv,
    muShieldDesign=options.ds,
    nuTauTargetDesign=options.nud,
    CaloDesign=options.caloDesign,
    strawDesign=options.strawDesign,
    muShieldGeo=options.geofile,
    muShieldStepGeo=options.muShieldStepGeo,
    muShieldWithCobaltMagnet=options.muShieldWithCobaltMagnet,
    SC_mag=options.SC_mag,
    scName=options.scName,
    DecayVolumeMedium=options.decayVolMed,
    SND=options.SND,
)
# -----------------------------------------------------------------------------
if simEngine == "PG":
    tag = simEngine + "_" + str(options.pID) + "-" + mcEngine
else:
    tag = simEngine + "-" + mcEngine
if charmonly:
    tag = simEngine + "CharmOnly-" + mcEngine
if options.eventDisplay:
    tag = tag + '_D'
if options.dv > 4:
    tag = 'conical.' + tag
if not os.path.exists(options.outputDir):
    os.makedirs(options.outputDir)
outFile = "%s/ship.%s.root" % (options.outputDir, tag)
for x in os.listdir(options.outputDir):
    if not x.find(tag) < 0:
        os.system("rm %s/%s" % (options.outputDir, x))
parFile = "%s/ship.params.%s.root" % (options.outputDir, tag)

timer = ROOT.TStopwatch()
timer.Start()
run = ROOT.FairRunSim()
run.SetName(mcEngine)
run.SetSink(ROOT.FairRootFileSink(outFile))
run.SetUserConfig("g4Config.C")
rtdb = run.GetRuntimeDb()

# Create geometry.
import shipDet_conf
modules = shipDet_conf.configure(run, ship_geo)

primGen = ROOT.FairPrimaryGenerator()
if simEngine == "Pythia8":
    primGen.SetTarget(ship_geo.target.z0, 0.)
    if HNL or options.RPVSUSY:
        P8gen = ROOT.HNLPythia8Generator()
        import pythia8_conf
        if HNL:
            print('Generating HNL events of mass %.3f GeV' % options.theMass)
            if theProductionCouplings is None and theDecayCouplings is None:
                print('and with couplings=', theCouplings)
                theProductionCouplings = theDecayCouplings = theCouplings
            elif theProductionCouplings is not None and theDecayCouplings is not None:
                print('and with couplings', theProductionCouplings, 'at production')
                print('and', theDecayCouplings, 'at decay')
            else:
                raise ValueError('Either both production and decay couplings must be specified, or neither.')
            pythia8_conf.configure(P8gen, options.theMass, theProductionCouplings, theDecayCouplings, inclusive, options.deepCopy)
        if options.RPVSUSY:
            print('Generating RPVSUSY events of mass %.3f GeV' % theHNLMass)
            print('and with couplings=[%.3f,%.3f]' % (theCouplings[0], theCouplings[1]))
            print('and with stop mass=%.3f GeV\n' % theCouplings[2])
            pythia8_conf.configurerpvsusy(P8gen, options.theMass, [theCouplings[0], theCouplings[1]], theCouplings[2], options.RPVSUSYbench, inclusive, options.deepCopy)
        P8gen.SetParameters("ProcessLevel:all = off")
        if inputFile:
            ut.checkFileExists(inputFile)
            P8gen.UseExternalFile(inputFile, options.firstEvent)
    if options.DarkPhoton:
        P8gen = ROOT.DPPythia8Generator()
        if inclusive == 'qcd':
            P8gen.SetDPId(4900023)
        else:
            P8gen.SetDPId(9900015)
        import pythia8darkphoton_conf
        passDPconf = pythia8darkphoton_conf.configure(P8gen, options.theMass, options.theDPepsilon, inclusive, motherMode, options.deepCopy)
        if passDPconf != 1:
            sys.exit()
    if HNL or options.RPVSUSY or options.DarkPhoton:
        P8gen.SetSmearBeam(1*u.cm)
        P8gen.SetLmin((ship_geo.Chamber1.z - ship_geo.chambers.Tub1length) - ship_geo.target.z0)
        P8gen.SetLmax(ship_geo.TrackStation1.z - ship_geo.target.z0)
    if charmonly:
        primGen.SetTarget(0., 0.)
        ut.checkFileExists(inputFile)
        if ship_geo.Box.gausbeam:
            primGen.SetBeam(0.,0., 0.5, 0.5)
            primGen.SmearGausVertexXY(True)
        else:
            primGen.SetBeam(0.,0., ship_geo.Box.TX-1., ship_geo.Box.TY-1.)
            primGen.SmearVertexXY(True)
        P8gen = ROOT.Pythia8Generator()
        P8gen.UseExternalFile(inputFile, options.firstEvent)
        P8gen.SetTarget("volTarget_1",0.,0.)
    primGen.AddGenerator(P8gen)

# (Remaining generator and run setup unchanged)
run.SetGenerator(primGen)
if options.eventDisplay:
    run.SetStoreTraj(ROOT.kTRUE)
else:
    run.SetStoreTraj(ROOT.kFALSE)
run.Init()
if options.dryrun:
    sys.exit(0)
gMC = ROOT.TVirtualMC.GetMC()
fStack = gMC.GetStack()
if MCTracksWithHitsOnly:
    fStack.SetMinPoints(1)
    fStack.SetEnergyCut(-100.*u.MeV)
elif MCTracksWithEnergyCutOnly:
    fStack.SetMinPoints(-1)
    fStack.SetEnergyCut(100.*u.MeV)
elif MCTracksWithHitsOrEnergyCut:
    fStack.SetMinPoints(1)
    fStack.SetEnergyCut(100.*u.MeV)
elif options.deepCopy:
    fStack.SetMinPoints(0)
    fStack.SetEnergyCut(0.*u.MeV)
if options.eventDisplay:
    trajFilter = ROOT.FairTrajFilter.Instance()
    trajFilter.SetStepSizeCut(1*u.mm)
    trajFilter.SetVertexCut(-20*u.m, -20*u.m, ship_geo.target.z0-1*u.m, 20*u.m, 20*u.m, 200.*u.m)
    trajFilter.SetMomentumCutP(0.1*u.GeV)
    trajFilter.SetEnergyCut(0., 400.*u.GeV)
    trajFilter.SetStorePrimaries(ROOT.kTRUE)
    trajFilter.SetStoreSecondaries(ROOT.kTRUE)
import geomGeant4
if hasattr(ship_geo.Bfield,"fieldMap"):
    fieldMaker = geomGeant4.addVMCFields(ship_geo, '', True)
if options.debug == 1:
    geomGeant4.printVMCFields()
    geomGeant4.printWeightsandFields(onlyWithField=True, exclude=['DecayVolume','Tr1','Tr2','Tr3','Tr4','Veto','Ecal','Hcal','MuonDetector','SplitCal'])
if inactivateMuonProcesses:
    ROOT.gROOT.ProcessLine('#include "Geant4/G4ProcessTable.hh"')
    mygMC = ROOT.TGeant4.GetMC()
    mygMC.ProcessGeantCommand("/process/inactivate muPairProd")
    mygMC.ProcessGeantCommand("/process/inactivate muBrems")
    mygMC.ProcessGeantCommand("/process/inactivate muIoni")
    mygMC.ProcessGeantCommand("/process/inactivate muonNuclear")
    mygMC.ProcessGeantCommand("/particle/select mu+")
    mygMC.ProcessGeantCommand("/particle/process/dump")
    gProcessTable = ROOT.G4ProcessTable.GetProcessTable()
    procmu = gProcessTable.FindProcess(ROOT.G4String('muIoni'), ROOT.G4String('mu+'))
    procmu.SetVerboseLevel(2)
run.Run(options.nEvents)
kParameterMerged = ROOT.kTRUE
parOut = ROOT.FairParRootFileIo(kParameterMerged)
parOut.open(parFile)
rtdb.setOutput(parOut)
rtdb.saveOutput()
rtdb.printParamContexts()
getattr(rtdb,"print")()
run.CreateGeometryFile("%s/geofile_full.%s.root" % (options.outputDir, tag))
import saveBasicParameters
saveBasicParameters.execute("%s/geofile_full.%s.root" % (options.outputDir, tag), ship_geo)
if options.debug == 2:
    fGeo = ROOT.gGeoManager
    fGeo.SetNmeshPoints(10000)
    fGeo.CheckOverlaps(0.1)
    fGeo.PrintOverlaps()
    for x in fGeo.GetTopNode().GetNodes():
        x.CheckOverlaps(0.0001)
        fGeo.PrintOverlaps()
timer.Stop()
rtime = timer.RealTime()
ctime = timer.CpuTime()
print(' ')
print("Macro finished succesfully.")
if "P8gen" in globals():
    if (HNL):
        print("number of retries, events without HNL ", P8gen.nrOfRetries())
    elif (options.DarkPhoton):
        print("number of retries, events without Dark Photons ", P8gen.nrOfRetries())
        print("total number of dark photons (including multiple meson decays per single collision) ", P8gen.nrOfDP())
print("Output file is ", outFile)
print("Parameter file is ", parFile)
print("Real time ", rtime, " s, CPU time ", ctime, "s")
if simEngine == "MuonBack":
    tmpFile = outFile + "tmp"
    xxx = outFile.split('/')
    check = xxx[len(xxx)-1]
    fin = False
    for ff in ROOT.gROOT.GetListOfFiles():
        nm = ff.GetName().split('/')
        if nm[len(nm)-1] == check:
            fin = ff
    if not fin:
        fin = ROOT.TFile.Open(outFile)
    t = fin.cbmsim
    fout = ROOT.TFile(tmpFile, 'recreate')
    fSink = ROOT.FairRootFileSink(fout)
    sTree = t.CloneTree(0)
    nEvents = 0
    pointContainers = []
    for x in sTree.GetListOfBranches():
        name = x.GetName()
        if not name.find('Point') < 0:
            pointContainers.append('sTree.'+name+'.GetEntries()')
    for n in range(t.GetEntries()):
        rc = t.GetEvent(n)
        empty = True
        for x in pointContainers:
            if eval(x) > 0:
                empty = False
        if not empty:
            rc = sTree.Fill()
            nEvents += 1
    branches = ROOT.TList()
    branches.SetName('BranchList')
    branches.Add(ROOT.TObjString('MCTrack'))
    branches.Add(ROOT.TObjString('vetoPoint'))
    branches.Add(ROOT.TObjString('ShipRpcPoint'))
    branches.Add(ROOT.TObjString('TargetPoint'))
    branches.Add(ROOT.TObjString('TTPoint'))
    branches.Add(ROOT.TObjString('ScoringPoint'))
    branches.Add(ROOT.TObjString('strawtubesPoint'))
    branches.Add(ROOT.TObjString('EcalPoint'))
    branches.Add(ROOT.TObjString('sEcalPointLite'))
    branches.Add(ROOT.TObjString('smuonPoint'))
    branches.Add(ROOT.TObjString('TimeDetPoint'))
    branches.Add(ROOT.TObjString('MCEventHeader'))
    branches.Add(ROOT.TObjString('sGeoTracks'))
    sTree.AutoSave()
    fSink.WriteObject(branches, "BranchList", ROOT.TObject.kSingleKey)
    fSink.SetOutTree(sTree)
    fout.Close()
    print("removed empty events, left with:", nEvents)
    rc1 = os.system("rm  " + outFile)
    rc2 = os.system("mv " + tmpFile + " " + outFile)
    fin.SetWritable(False)
import checkMagFields
def visualizeMagFields():
    checkMagFields.run()
def checkOverlapsWithGeant4():
    mygMC = ROOT.TGeant4.GetMC()
    mygMC.ProcessGeantCommand("/geometry/test/recursion_start 0")
    mygMC.ProcessGeantCommand("/geometry/test/recursion_depth 2")
    mygMC.ProcessGeantCommand("/geometry/test/run")

