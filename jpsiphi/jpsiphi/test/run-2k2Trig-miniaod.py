import FWCore.ParameterSet.Config as cms
process = cms.Process('PSIKK')

input_file = "file:049F2D32-26F2-E711-A162-00266CFFC664.root"

process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load("Configuration.StandardSequences.MagneticField_AutoFromDBCurrent_cff")
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')
process.load("SimTracker.TrackerHitAssociation.tpClusterProducer_cfi")
from Configuration.AlCa.GlobalTag_condDBv2 import GlobalTag
#process.GlobalTag = GlobalTag(process.GlobalTag, '94X_dataRun2_ReReco_EOY17_v1')
process.GlobalTag = GlobalTag(process.GlobalTag, '94X_dataRun2_ReReco_EOY17_v2') #F

process.options   = cms.untracked.PSet( wantSummary = cms.untracked.bool(True))

process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 500

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(input_file)
)

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(-1))

process.TFileService = cms.Service("TFileService",
        fileName = cms.string('rootuple-2017-ditraktrigger.root'),
)

kaonmass = 0.493677
pionmass = 0.13957061
muonmass = 0.1056583715;

process.load("jpsiphi.jpsiphi.slimmedMuonsTriggerMatcher2017_cfi")
# process.load("jpsiphi.jpsiphi.slimmedTracksTriggerMatcher2017_cfi")

charmoniumHLT = [
#Phi
'HLT_DoubleMu2_Jpsi_DoubleTrk1_Phi',
#JPsi
'HLT_DoubleMu4_JpsiTrkTrk_Displaced',
'HLT_DoubleMu4_JpsiTrk_Displaced'
# 'HLT_DoubleMu4_Jpsi_Displaced',
# 'HLT_DoubleMu4_3_Jpsi_Displaced',
# 'HLT_Dimuon20_Jpsi_Barrel_Seagulls',
# 'HLT_Dimuon25_Jpsi',
]

hltList = charmoniumHLT #muoniaHLT

hltpaths = cms.vstring(hltList)

hltpathsV = cms.vstring([h + '_v*' for h in hltList])

filters = cms.vstring(
                                #HLT_DoubleMu2_Jpsi_DoubleTrk1_Phi
                                'hltJpsiTkTkVertexFilterPhiDoubleTrk1v2',
                                'hltJpsiTkTkVertexFilterPhiKstar',
                                'hltJpsiTkVertexFilter'
                                )

process.triggerSelection = cms.EDFilter("TriggerResultsFilter",
                                        triggerConditions = cms.vstring(hltpathsV),
                                        hltResults = cms.InputTag( "TriggerResults", "", "HLT" ),
                                        l1tResults = cms.InputTag( "" ),
                                        throw = cms.bool(False)
                                        )

process.unpackPatTriggers = cms.EDProducer("PATTriggerObjectStandAloneUnpacker",
  patTriggerObjectsStandAlone = cms.InputTag( 'slimmedPatTrigger' ),
  triggerResults              = cms.InputTag( 'TriggerResults::HLT' ),
  unpackFilterLabels          = cms.bool( True )
)



process.JPsi2MuMuPAT = cms.EDProducer('DiMuonProducerHLTPAT',
        muons                       = cms.InputTag('slimmedMuonsWithTrigger'),
        primaryVertexTag            = cms.InputTag('offlineSlimmedPrimaryVertices'),
        beamSpotTag                 = cms.InputTag('offlineBeamSpot'),
        higherPuritySelection       = cms.string(""),
        lowerPuritySelection        = cms.string(""),
        dimuonSelection             = cms.string("2.82 < mass && mass < 3.32 && charge==0"),
        addCommonVertex             = cms.bool(True),
        addMuonlessPrimaryVertex    = cms.bool(False),
        addMCTruth                  = cms.bool(False),
        resolvePileUpAmbiguity      = cms.bool(True),
        HLTFilters                  = filters
)


process.JPsi2MuMuFilter = cms.EDProducer('DiMuonFilter',
      OniaTag             = cms.InputTag("JPsi2MuMuPAT"),
      singlemuonSelection = cms.string(""),
      dimuonSelection     = cms.string("2.82 < mass && mass < 3.32 && userFloat('vProb') > 0.0"),
      do_trigger_match    = cms.bool(True),
      HLTFilters          = filters
)

process.DiMuonCounterJPsi = cms.EDFilter('CandViewCountFilter',
    src       = cms.InputTag("JPsi2MuMuFilter"),
    minNumber = cms.uint32(1),
    filter    = cms.bool(True)
)

# process.DiTrakHLT  = cms.EDAnalyzer('DiTrakHLT',
#     PFCandidates        = cms.InputTag("packedPFCandidates"),
#     TriggerInput        = cms.InputTag("unpackPatTriggers"),
#     primaryVertices     = cms.InputTag("offlineSlimmedPrimaryVertices"),
#     TriggerResults      = cms.InputTag("TriggerResults", "", "HLT"),
#     TrakTrakMassCuts    = cms.vdouble(0.5,1.5),
#     MassTraks           = cms.vdouble(kaonmass,kaonmass),
#     isMC                = cms.bool(False),
#     OnlyBest            = cms.bool(False),
#     HLTs                = hltpaths,
#     Filters             = filters
# )

process.DiMuonDiTrakProducerVertexHLT = cms.EDProducer('DiMuonDiTrakProducerVertexHLT',
    DiMuon                  = cms.InputTag('JPsi2MuMuPAT'),
    PFCandidates            = cms.InputTag('packedPFCandidates'),
    TriggerInput            = cms.InputTag("unpackPatTriggers"),
    TriggerResults          = cms.InputTag("TriggerResults", "", "HLT"),
    DiMuonMassCuts          = cms.vdouble(2.88,3.32),      # J/psi mass window 3.096916 +/- 0.150
    TrakTrakMassCuts        = cms.vdouble(0.93,1.32),  # phi mass window 1.019461 +/- .015
    DiMuonDiTrakMassCuts    = cms.vdouble(5.1,5.8),            # b-hadron mass window
    MassTraks               = cms.vdouble(kaonmass,kaonmass),         # traks masses
    MaxDeltaRPt             = cms.vdouble(0.01,2.0),
    OnlyBest                = cms.bool(False),
    Product                 = cms.string("DiMuonDiTrakCandidatesHLT"),
    HLTs                    = hltpaths,
    Filters                 = filters,
    CandsMasses             = cms.vdouble(muonmass,muonmass,kaonmass,kaonmass),
)
#
# process.DiMuonDiTrakKinematicFit = cms.EDProducer('DiMuonDiTrakKinematicFit',
#     DiMuonDiTrak                = cms.InputTag('DiMuonDiTrakProducerHLT','DiMuonDiTrakCandidatesHLT'),
#     DiMuonMass                  = cms.double(3.096916),              # J/psi mass in GeV
#     DiMuonTrakTrakMassCuts      = cms.vdouble(4.0,5.8),            # b-hadron mass window
#     MassTraks                   = cms.vdouble(kaonmass,kaonmass),         # traks masses
#     Product                     = cms.string('DiMuonDiTrakCandidatesRef')
# )

process.DiMuonDiTrakVertexRootuplerHLT = cms.EDAnalyzer('DiMuonDiTrakVertexRootuplerHLT',
    dimuonditrk_cand = cms.InputTag('DiMuonDiTrakProducerVertexHLT','DiMuonDiTrakCandidatesHLT'),
    # dimuonditrk_rf_cand = cms.InputTag("DiMuonDiTrakKinematicFit","DiMuonDiTrakCandidatesRef"),
    # beamSpotTag = cms.InputTag("offlineBeamSpot"),
    primaryVertices = cms.InputTag("offlineSlimmedPrimaryVertices"),
    TriggerResults = cms.InputTag("TriggerResults", "", "HLT"),
    OnlyBest = cms.bool(False),
    HLTs = hltpaths,
    Filters = filters,
)



process.p = cms.Path(process.triggerSelection *
                     process.slimmedMuonsWithTriggerSequence *
                     process.unpackPatTriggers *
                     # process.slimmedPFCandsWithTriggerSequence *
                     # process.softMuons *
                     process.JPsi2MuMuPAT *
                     process.JPsi2MuMuFilter*
                     process.DiMuonCounterJPsi*
                     process.DiMuonDiTrakProducerVertexHLT *
                     # process.DiMuonDiTrakKinematicFit *
                     process.DiMuonDiTrakVertexRootuplerHLT *
                     # process.DiTrakHLT*
                     # process.DiMuonRootuplerHLT)
                     # process.rootupleMuMu)# * process.Phi2KKPAT * process.patSelectedTracks *process.rootupleKK)