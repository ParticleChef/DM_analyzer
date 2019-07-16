import os
import FWCore.ParameterSet.Config as cms
import FWCore.Utilities.FileUtils as FileUtils
mylist = FileUtils.loadListFromFile('monotops.txt')

process = cms.Process("lambda")

process.load("FWCore.MessageService.MessageLogger_cfi")

#process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(100) )
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )

process.option = cms.untracked.PSet( wantSummary = cms.untracked.bool(True) )

process.source = cms.Source("PoolSource",
    # replace 'myfile.root' with the source file you want to use
    fileNames = cms.untracked.vstring(
        #'file:myfile.root'
        *mylist
    )
)

process.lamb = cms.EDAnalyzer("LambdaAnalyzer",
                              genSet = cms.PSet(
                                                genProduct = cms.InputTag('generator'),
                                                lheProduct = cms.InputTag('externalLHEProducer'),
                                                genParticles = cms.InputTag('prunedGenParticles'),
                                                pdgId = cms.vint32(1, 2, 3, 4, 5, 6, 11, 12, 13, 14, 15, 16, 18, 21, 23, 24, 25, 55, 118), #18 DM; 55 Zp; 
                                                ),
                              electronSet = cms.PSet(
                                                electrons = cms.InputTag('slimmedElectrons'),
                                                vertices = cms.InputTag('offlineSlimmedPrimaryVertices'),
                                                ),
                              muonSet = cms.PSet(
                                                muons = cms.InputTag('slimmedMuons'),
                                                ),
                              jetSet = cms.PSet(
                                                vertices = cms.InputTag('offlineSlimmedPrimaryVertices'),
                                                jets = cms.InputTag('slimmedJets'),
                                                met = cms.InputTag('slimmedMETs'),
                                                genjets = cms.InputTag('slimmedGenJets'),
                                                ),
                              #fatJetSet = cms.PSet(
                              #                  jets = cms.InputTag('slimmedJetsAK8'),
                              #                  ),
                              histFile = cms.string('%s/src/DM_analyzer/LambdaAnalyzer/data/HistList.dat' % os.environ['CMSSW_BASE']),
                              )

process.TFileService = cms.Service("TFileService",
                                    fileName = cms.string("results_VectorMonotop_31481514_Mphi_2000_Mchi_1500.root"),
                                    closeFileFast = cms.untracked.bool(True)
                                    )

process.p = cms.Path(process.lamb)
