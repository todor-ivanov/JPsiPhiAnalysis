import FWCore.ParameterSet.Config as cms
process = cms.Process('PSIKK')

input_file = "file:0EE0B583-7A9C-E711-82D7-B083FED00117.root"

process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load("Configuration.StandardSequences.MagneticField_AutoFromDBCurrent_cff")
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')
process.load("SimTracker.TrackerHitAssociation.tpClusterProducer_cfi")
from Configuration.AlCa.GlobalTag_condDBv2 import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '80X_dataRun2_2016LegacyRepro_v4')

process.options   = cms.untracked.PSet( wantSummary = cms.untracked.bool(True))

process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 500

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(input_file)
)

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(-1))

process.TFileService = cms.Service("TFileService",
        fileName = cms.string('rootuple-2016-dimuonditrak.root'),
)

process.load("jpsiphi.jpsiphi.slimmedMuonsTriggerMatcher2016_cfi")
# process.load("jpsiphi.jpsiphi.slimmedTracksTriggerMatcher2017_cfi")

charmoniumHLT = [
#JPsi
'HLT_DoubleMu4_JpsiTrk_Displaced',
'HLT_DoubleMu4_3_Jpsi_Displaced',
'HLT_Dimuon20_Jpsi',
'HLT_Dimuon16_Jpsi',
'HLT_Dimuon10_Jpsi_Barrel',
]

hltList = charmoniumHLT

hltpaths = cms.vstring(hltList)

hltpathsV = cms.vstring([h + '_v*' for h in hltList])

filters = cms.vstring(#HLT_DoubleMu4_JpsiTrk_Displaced_v4
                      'hltDoubleMu4JpsiDisplacedL3Filtered',
                      'hltJpsiTkVertexFilter',
                      #HLT_DoubleMu4_3_Jpsi_Displaced_v4
                      'hltDoubleMu43JpsiDisplacedL3Filtered',
                      'hltDisplacedmumuFilterDoubleMu43Jpsi',
                      #HLT_Dimuon20_Jpsi_v3
                      'hltDimuon20JpsiL3Filtered',
                      'hltDisplacedmumuFilterDimuon20Jpsi',
                      #HLT_Dimuon16_Jpsi_v3
                      'hltDimuon16JpsiL3Filtered',
                      'hltDisplacedmumuFilterDimuon16Jpsi',
                      #HLT_Dimuon10_Jpsi_Barrel_v4
                      'hltDimuon10JpsiBarrelL3Filtered',
                      'hltDisplacedmumuFilterDimuon10JpsiBarrel'
                                )

process.triggerSelection = cms.EDFilter("TriggerResultsFilter",
                                        triggerConditions = cms.vstring(hltpathsV),
                                        hltResults = cms.InputTag( "TriggerResults", "", "HLT" ),
                                        l1tResults = cms.InputTag( "" ),
                                        throw = cms.bool(False)
                                        )


process.softMuons = cms.EDFilter('PATMuonSelector',
   src = cms.InputTag('slimmedMuonsWithTrigger'),
   cut = cms.string('muonID(\"TMOneStationTight\")'
                    ' && abs(innerTrack.dxy) < 0.3'
                    ' && abs(innerTrack.dz)  < 20.'
                    ' && innerTrack.hitPattern.trackerLayersWithMeasurement > 5'
                    ' && innerTrack.hitPattern.pixelLayersWithMeasurement > 0'
                    ' && innerTrack.quality(\"highPurity\")'
   ),
   filter = cms.bool(True)
)
process.JPsi2MuMuPAT = cms.EDProducer('DiMuonProducerPAT',
        muons                       = cms.InputTag('softMuons'),
        primaryVertexTag            = cms.InputTag('offlineSlimmedPrimaryVertices'),
        beamSpotTag                 = cms.InputTag('offlineBeamSpot'),
        higherPuritySelection       = cms.string(""),
        lowerPuritySelection        = cms.string(""),
        dimuonSelection             = cms.string("2.95 < mass && mass < 3.25 && charge==0"),
        addCommonVertex             = cms.bool(True),
        addMuonlessPrimaryVertex    = cms.bool(False),
        addMCTruth                  = cms.bool(False),
        resolvePileUpAmbiguity      = cms.bool(True),
        HLTFilters                  = filters
)

process.JPsi2MuMuFilter = cms.EDProducer('DiMuonFilter',
      OniaTag             = cms.InputTag("JPsi2MuMuPAT"),
      singlemuonSelection = cms.string(""),
      dimuonSelection     = cms.string("2.95 < mass && mass < 3.25 && userFloat('vProb') > 0.01 && pt > 2.0"),
      do_trigger_match    = cms.bool(False),
      HLTFilters          = filters
)

process.PsiPhiProducer = cms.EDProducer('DiMuonDiTrakProducer',
    DiMuon = cms.InputTag('JPsi2MuMuPAT'),
    PFCandidates = cms.InputTag('packedPFCandidates'),
    DiMuonMassCuts = cms.vdouble(2.95,3.25),      # J/psi mass window 3.096916 +/- 0.150
    TrakTrakMassCuts = cms.vdouble(1.0,1.04),  # phi mass window 1.019461 +/- .015
    DiMuonDiTrakMassCuts = cms.vdouble(4.0,5.8),            # b-hadron mass window
    MassTraks = cms.vdouble(0.493677,0.493677),         # traks masses
    OnlyBest  = cms.bool(False)
)

process.PsiPhiFitter = cms.EDProducer('DiMuonDiTrakKinematicFit',
    DiMuonDiTrak        = cms.InputTag('PsiPhiProducer','DiMuonDiTrakCandidates'),
    DiMuonMass          = cms.double(3.096916),              # J/psi mass in GeV
    DiMuonTrakTrakMassCuts    = cms.vdouble(4.0,5.8),            # b-hadron mass window
    MassTraks           = cms.vdouble(0.493677,0.493677),         # traks masses
    Product             = cms.string('DiMuonDiTrakCandidatesRef')
)

process.rootuple = cms.EDAnalyzer('DiMuonDiTrakRootupler',
    dimuonditrk_cand = cms.InputTag('PsiPhiProducer','DiMuonDiTrakCandidates'),
    dimuonditrk_rf_cand = cms.InputTag("PsiPhiFitter","DiMuonDiTrakCandidatesRef"),
    beamSpotTag = cms.InputTag("offlineBeamSpot"),
    primaryVertices = cms.InputTag("offlineSlimmedPrimaryVertices"),
    TriggerResults = cms.InputTag("TriggerResults", "", "HLT"),
    isMC = cms.bool(False),
    OnlyBest = cms.bool(False),
    HLTs = hltpaths,
    Filters = filters,
    TreeName = cms.string('JPsi Phi Tree')
)

process.rootupleMuMu = cms.EDAnalyzer('DiMuonRootupler',
                          dimuons = cms.InputTag("JPsi2MuMuFilter"),
                          muons = cms.InputTag("replaceme"),
                          primaryVertices = cms.InputTag("offlinePrimaryVertices"),
                          TriggerResults = cms.InputTag("TriggerResults", "", "HLT"),
                          dimuon_pdgid = cms.uint32(443),
                          dimuon_mass_cuts = cms.vdouble(2.5,3.5),
                          isMC = cms.bool(False),
                          OnlyBest = cms.bool(False),
                          OnlyGen = cms.bool(False),
                          HLTs = hltpaths
                          )

process.p = cms.Path(process.triggerSelection *
                     process.slimmedMuonsWithTriggerSequence *
                     # process.slimmedPFCandsWithTriggerSequence *
                     process.softMuons *
                     process.JPsi2MuMuPAT *
                     process.JPsi2MuMuFilter*
                     process.PsiPhiProducer *
                     process.PsiPhiFitter *
                     process.rootuple *
                     process.rootupleMuMu)# * process.Phi2KKPAT * process.patSelectedTracks *process.rootupleKK)