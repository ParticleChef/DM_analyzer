//#include "RecoilCorrector.hh" // From: https://github.com/cms-met/MetTools/tree/master/RecoilCorrections

#include "JetAnalyzer.h"


JetAnalyzer::JetAnalyzer(edm::ParameterSet& PSet, edm::ConsumesCollector&& CColl):
  JetToken(CColl.consumes<std::vector<pat::Jet> >(PSet.getParameter<edm::InputTag>("jets"))),
  MetToken(CColl.consumes<std::vector<pat::MET> >(PSet.getParameter<edm::InputTag>("met"))),
  VertexToken(CColl.consumes<reco::VertexCollection>(PSet.getParameter<edm::InputTag>("vertices"))),
  GenJetToken(CColl.consumes<std::vector<reco::GenJet> >(PSet.getParameter<edm::InputTag>("genjets")))
{
    
    std::cout << " --- JetAnalyzer initialization ---" << std::endl;
    /**
    std::cout << "  jet Id            :\t" << JetId << std::endl;
    std::cout << "  jet pT [1, 2]     :\t" << Jet1Pt << "\t" << Jet2Pt << std::endl;
    std::cout << "  jet eta           :\t" << JetEta << std::endl;
    std::cout << "  b-tagging algo    :\t" << BTag << std::endl;
    std::cout << "  b-tag cut [1, 2]  :\t" << Jet1BTag << "\t" << Jet2BTag << std::endl;
    std::cout << "  apply recoil corr :\t" << (UseRecoil ? "YES" : "NO") << std::endl;
    std::cout << "  recoil file MC    :\t" << RecoilMCFile << std::endl;
    std::cout << "  recoil file Data  :\t" << RecoilDataFile << std::endl;
    std::cout << std::endl;
    **/
}

JetAnalyzer::~JetAnalyzer() {

}
std::vector<reco::GenJet> JetAnalyzer::FillGenJetVector(const edm::Event& iEvent) {
  std::vector<reco::GenJet> Vect;
  // Declare and open collection
  edm::Handle<std::vector<reco::GenJet> > GenJetsCollection;
  iEvent.getByToken(GenJetToken, GenJetsCollection);

  // Loop on Jet collection                                                                                                                                                                                                                              
  for(std::vector<reco::GenJet>::const_iterator it=GenJetsCollection->begin(); it!=GenJetsCollection->end(); ++it) {
    reco::GenJet genjet=*it;
    Vect.push_back(genjet);
  }
  return Vect;
}

std::vector<pat::Jet> JetAnalyzer::FillJetVector(const edm::Event& iEvent) {
    std::vector<pat::Jet> Vect;
    // Declare and open collection
    edm::Handle<std::vector<pat::Jet> > PFJetsCollection;
    iEvent.getByToken(JetToken, PFJetsCollection);

    // Vertex collection
    edm::Handle<reco::VertexCollection> PVCollection;
    iEvent.getByToken(VertexToken, PVCollection);
 
    // Loop on Jet collection
    for(std::vector<pat::Jet>::const_iterator it=PFJetsCollection->begin(); it!=PFJetsCollection->end(); ++it) {
      
      pat::Jet jet=*it;
      //const reco::GenJet* genJet=jet.genJet();
      if (jet.pt()<30) continue;
      //if (abs(jet.eta())>2.5)continue;
      Vect.push_back(jet); // Fill vector
    }
    return Vect;
}

void JetAnalyzer::CleanJetsFromMuons(std::vector<pat::Jet>& Jets, std::vector<pat::Muon>& Muons, float angle) {
    for(unsigned int m = 0; m < Muons.size(); m++) {
        for(unsigned int j = 0; j < Jets.size(); ) {
            if(deltaR(Jets[j], Muons[m]) < angle) Jets.erase(Jets.begin() + j);
            else j++;
        }
    }
}

void JetAnalyzer::CleanJetsFromElectrons(std::vector<pat::Jet>& Jets, std::vector<pat::Electron>& Electrons, float angle) {
    for(unsigned int e = 0; e < Electrons.size(); e++) {
        for(unsigned int j = 0; j < Jets.size(); ) {
            if(deltaR(Jets[j], Electrons[e]) < angle) Jets.erase(Jets.begin() + j);
            else j++;
        }
    }
}

pat::MET JetAnalyzer::FillMetVector(const edm::Event& iEvent) {
    
    edm::Handle<std::vector<pat::MET> > MetCollection;
    iEvent.getByToken(MetToken, MetCollection);
    pat::MET MEt = MetCollection->front();
    MEt.addUserFloat("ptShiftJetResUp", MEt.shiftedPt(pat::MET::METUncertainty::JetResUp));
    MEt.addUserFloat("ptShiftJetResDown", MEt.shiftedPt(pat::MET::METUncertainty::JetResDown));
    MEt.addUserFloat("ptShiftJetEnUp", MEt.shiftedPt(pat::MET::METUncertainty::JetEnUp));
    MEt.addUserFloat("ptShiftJetEnDown", MEt.shiftedPt(pat::MET::METUncertainty::JetEnDown));
    MEt.addUserFloat("ptShiftUnclusteredEnUp", MEt.shiftedPt(pat::MET::METUncertainty::UnclusteredEnUp));
    MEt.addUserFloat("ptShiftUnclusteredEnDown", MEt.shiftedPt(pat::MET::METUncertainty::UnclusteredEnDown));
    MEt.addUserFloat("ptRaw", MEt.uncorPt());
    MEt.addUserFloat("phiRaw", MEt.uncorPhi());
    return MEt;
}
