#include "ElectronAnalyzer.h"



ElectronAnalyzer::ElectronAnalyzer(const edm::ParameterSet& PSet, edm::ConsumesCollector&& CColl):
    ElectronToken(CColl.consumes<std::vector<pat::Electron> >(PSet.getParameter<edm::InputTag>("electrons"))),
    VertexToken(CColl.consumes<reco::VertexCollection>(PSet.getParameter<edm::InputTag>("vertices")))
{
    
    std::cout << " --- ElectronAnalyzer initialization ---" << std::endl;
    std::cout << std::endl;
}

ElectronAnalyzer::~ElectronAnalyzer() {

}

std::vector<pat::Electron> ElectronAnalyzer::FillElectronVector(const edm::Event& iEvent) {
    std::vector<pat::Electron> Vect;
    // Declare and open collection
    edm::Handle<std::vector<pat::Electron> > EleCollection;
    iEvent.getByToken(ElectronToken, EleCollection);
    
    //edm::Handle<std::vector<pat::Conversion> > EleConv;
    //iEvent.getByToken(edm::InputTag("patConversions"), EleConv);
    
    edm::Handle<reco::VertexCollection> PVCollection;
    iEvent.getByToken(VertexToken, PVCollection);
    const reco::Vertex* vertex=&PVCollection->front();
   
    // Loop on Electron collection
    for(std::vector<pat::Electron>::const_iterator it=EleCollection->begin(); it!=EleCollection->end(); ++it) {
        pat::Electron el=*it;
        // Fill vector
	//if(el.pt()<15 || fabs(el.eta())>2.4) continue;
        Vect.push_back(el);
    }
    return Vect;
}
