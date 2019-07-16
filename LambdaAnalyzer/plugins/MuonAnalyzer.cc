#include "MuonAnalyzer.h"
  
MuonAnalyzer::MuonAnalyzer(edm::ParameterSet& PSet, edm::ConsumesCollector&& CColl):
    MuonToken(CColl.consumes<std::vector<pat::Muon> >(PSet.getParameter<edm::InputTag>("muons")))
{
    std::cout << " --- MuonAnalyzer initialization ---" << std::endl;
    std::cout << std::endl;
}

MuonAnalyzer::~MuonAnalyzer() {
}

std::vector<pat::Muon> MuonAnalyzer::FillMuonVector(const edm::Event& iEvent) {

    std::vector<pat::Muon> Vect;
    // Declare and open collections
    edm::Handle<std::vector<pat::Muon> > MuonCollection;
    iEvent.getByToken(MuonToken, MuonCollection);

    // Loop on Muon collection
    for(std::vector<pat::Muon>::const_iterator it=MuonCollection->begin(); it!=MuonCollection->end(); ++it) {
        pat::Muon mu=*it;
	if ( mu.pt()<5 ||/* fabs(mu.eta())>2.4 ||*/ !mu.isLooseMuon() ) continue;
        Vect.push_back(mu);
    }
    return Vect;
}
