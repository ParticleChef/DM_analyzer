#include "GenAnalyzer.h"


GenAnalyzer::GenAnalyzer(edm::ParameterSet& PSet, edm::ConsumesCollector&& CColl):
  GenToken(CColl.consumes<GenEventInfoProduct>(PSet.getParameter<edm::InputTag>("genProduct"))),
  LheToken(CColl.consumes<LHEEventProduct>(PSet.getParameter<edm::InputTag>("lheProduct"))),
  GenParticlesToken(CColl.consumes<std::vector<reco::GenParticle> >(PSet.getParameter<edm::InputTag>("genParticles"))),
  ParticleList(PSet.getParameter<std::vector<int> >("pdgId"))
{    
  std::cout << " --- GenAnalyzer initialization ---" << std::endl; 
}

GenAnalyzer::~GenAnalyzer() {

}

// ---------- GEN WEIGHTS ----------

std::map<int, float> GenAnalyzer::FillWeightsMap(const edm::Event& iEvent) {
    std::map<int, float> Weights;
    Weights[-1] = 1.; // EventWeight
    if(iEvent.isRealData() or PythiaLOSample) return Weights;
    // Declare and open collection
    edm::Handle<GenEventInfoProduct> GenEventCollection;
    iEvent.getByToken(GenToken, GenEventCollection);
    // Declare and open collection
    edm::Handle<LHEEventProduct> LheEventCollection;
    iEvent.getByToken(LheToken, LheEventCollection);
    const LHEEventProduct* Product = LheEventCollection.product();
    
    Weights[-1] = GenEventCollection->weight()/fabs(GenEventCollection->weight());
     
    for(unsigned int i = 0; i < Product->weights().size(); i++) {
        Weights[ i ] = Product->weights()[i].wgt / Product->originalXWGTUP();
    }

    return Weights;
}

// similar to previous function but using ids instead of indexes
std::map<int, float> GenAnalyzer::LHEWeightsMap(const edm::Event& iEvent) {

    std::map<int, float> weights;
    weights[-1] = 1.; // default eventWeight

    if(iEvent.isRealData() or PythiaLOSample) return weights;
    // Declare and open collection
    edm::Handle<GenEventInfoProduct> GenEventCollection;
    iEvent.getByToken(GenToken, GenEventCollection);
    // Declare and open collection
    edm::Handle<LHEEventProduct> LheEventCollection;
    iEvent.getByToken(LheToken, LheEventCollection);

    // get  product and original weight
    const LHEEventProduct* lhe_product = LheEventCollection.product();
    const auto original_w = lhe_product->originalXWGTUP();
    
    // default will be the sign for MC 
    weights[-1] = GenEventCollection->weight()/fabs(GenEventCollection->weight());
     
    for(const auto & weight : lhe_product->weights()) {
        weights[std::stoi(weight.id)] = weight.wgt / original_w;
        //std::cout<<"weight.id = "<<weight.id<<", weight.wgt = "<<weight.wgt<<std::endl;
    }

    std::cout<<"LHEweightmap"<<std::endl;
    return weights;
}



// ---------- GEN PARTICLES ----------

std::vector<reco::GenParticle> GenAnalyzer::FillGenVector(const edm::Event& iEvent) {

    std::vector<reco::GenParticle> Vect;
    iEvent.getByToken(GenParticlesToken, GenCollection);
    // Loop on Gen Particles collection
    for(std::vector<reco::GenParticle>::const_iterator it = GenCollection->begin(); it != GenCollection->end(); ++it) {
        //        std::cout << it->pdgId() << "  " << it->status() << "  " << it->pt() << "  " << it->eta() << "  " << it->phi() << "  " << it->mass() << std::endl;
        //        if(it->numberOfDaughters()>0) std::cout << "  " << it->daughter(0)->pdgId() << "  " << it->daughter(0)->status() << "  " << it->daughter(0)->pt() << std::endl;
        //        if(it->numberOfDaughters()>1) std::cout << "  " << it->daughter(1)->pdgId() << "  " << it->daughter(1)->status() << "  " << it->daughter(1)->pt() << std::endl;
        //
        reco::GenParticle genP = *it;
        for(unsigned int i = 0; i < ParticleList.size(); i++) {
            //if(abs(it->pdgId()) == ParticleList[i]) Vect.push_back(*it); // Fill vector
            if(abs(it->pdgId()) == ParticleList[i]) Vect.push_back(genP); // Fill vector
        }
    }
    //    std::cout << "\n\n\n" << std::endl;
    return Vect;
}

std::vector<reco::GenParticle> GenAnalyzer::SelectGenVector(std::vector<reco::GenParticle>& Vect, int pdg) {

    std::vector<reco::GenParticle> GenVector;
    
    //for(unsigned int i = 0; i < Vect.size(); i++) {
    for(std::vector<reco::GenParticle>::const_iterator it = Vect.begin(); it != Vect.end(); ++it) {
        //std::cout << Vect[i].pdgId() << std::endl;
        if(abs(it->pdgId()) == pdg) GenVector.push_back(*it);
    }

    return GenVector;	
}

std::map<std::string, float> GenAnalyzer::FillLheMap(const edm::Event& iEvent) {
    std::map<std::string, float> Var;
    if(iEvent.isRealData() or PythiaLOSample) return Var;
    
    int lhePartons(0), lheBPartons(0);
    float lheHT(0.), lhePtZ(0.), lhePtW(0.), pt(0.);
    
    // Declare and open collection
    edm::Handle<LHEEventProduct> LheEventCollection;
    iEvent.getByToken(LheToken, LheEventCollection);
    const lhef::HEPEUP hepeup = LheEventCollection->hepeup();
    
    for(int i = 0; i < hepeup.NUP; ++i) {
        int id=abs(hepeup.IDUP[i]);
        // Lab frame momentum (Px, Py, Pz, E and M in GeV) for the particle entries in this event
        //reco::Candidate::LorentzVector P4(hepeup.PUP[i][0], hepeup.PUP[i][1], hepeup.PUP[i][2], hepeup.PUP[i][3]);
        pt = sqrt(hepeup.PUP[i][0]*hepeup.PUP[i][0] + hepeup.PUP[i][1]*hepeup.PUP[i][1]);
        if(hepeup.ISTUP[i]==1 && (id<6 || id==21)) {
            lheHT += pt; //P4.pt() 
            lhePartons++;
            if(id==5) lheBPartons++;
        }
        if(hepeup.ISTUP[i]==2 && abs(hepeup.IDUP[i])==23) lhePtZ = pt;
        if(hepeup.ISTUP[i]==2 && abs(hepeup.IDUP[i])==24) lhePtW = pt;
    }
    
    Var["lhePartons"] = lhePartons;
    Var["lheBPartons"] = lheBPartons;
    Var["lheHT"] = lheHT;
    Var["lhePtZ"] = lhePtZ;
    Var["lhePtW"] = lhePtW;
    return Var;
}

reco::Candidate* GenAnalyzer::FindGenParticle(std::vector<reco::GenParticle>& Vect, int pdg) {
    for(unsigned int i = 0; i < Vect.size(); i++) {
        if(Vect[i].pdgId() == pdg) return FindLastDaughter(dynamic_cast<reco::Candidate*>(&Vect[i]));
    }
    return NULL;
}

std::vector<reco::Candidate*> GenAnalyzer::FindGenParticleVector(std::vector<reco::GenParticle>& Vect, int pdg) {

  std::vector<reco::Candidate*> GenVect;

  for(unsigned int i = 0; i < Vect.size(); i++) {
    if(Vect[i].pdgId() == pdg)
      GenVect.push_back(FindLastDaughter(dynamic_cast<reco::Candidate*>(&Vect[i])));
  }
  return GenVect;
}

// Recursive function to find the last particle in the chain before decay: e.g. 23 -> 23 -> *23* -> 13 -13. Returns a reco Candidate
reco::Candidate* GenAnalyzer::FindLastDaughter(reco::Candidate* p) {
    if(p->numberOfDaughters() <= 0 || !p->daughter(0)) return p;
    if(p->daughter(0)->pdgId() != p->pdgId()) return p;
    return FindLastDaughter(p->daughter(0));
}

reco::GenParticle* GenAnalyzer::FindGenParticleGenByIds(std::vector<reco::GenParticle>& Vect, std::vector<int> pdgs, int motherId) {
    for(unsigned int i = 0; i < Vect.size(); i++) {
        for(unsigned int j=0; j<pdgs.size();j++) {
            if( Vect[i].pdgId() == pdgs[j] ) {
                if(motherId<=0) return FindLastDaughterGen(&Vect[i]);
                else {
                    if(Vect[i].mother() && Vect[i].mother()->pdgId() == motherId) return &Vect[i];
                }
            }
        }
    }
    return NULL;
}

// Recursive function to find the last particle in the chain before decay: e.g. 23 -> 23 -> *23* -> 13 -13.  Returns a GenParticle
reco::GenParticle* GenAnalyzer::FindLastDaughterGen(reco::GenParticle* p) {
    if(p->numberOfDaughters() <= 0 || !p->daughter(0)) return p;
    if(p->daughter(0)->pdgId() != p->pdgId()) return p;
    return FindLastDaughterGen( dynamic_cast<reco::GenParticle*>( p->daughter(0) ));
}

// Recursive function to find the last particle in the chain before decay: e.g. 23 -> 23 -> *23* -> 13 -13.  Returns a GenParticle
const reco::GenParticle* GenAnalyzer::FindLastDaughterGen(const reco::GenParticle* p) {
    if(p->numberOfDaughters() <= 0 || !p->daughter(0)) return p;
    if(p->daughter(0)->pdgId() != p->pdgId()) return p;
    return FindLastDaughterGen( dynamic_cast<const reco::GenParticle*>( p->daughter(0) ));
}

// Some particles radiate other particles with the same pdgId but a different status. Method to find a mother with different pdgId
const reco::Candidate* GenAnalyzer::FindMother(reco::GenParticle* p) {
    int pId = p->pdgId();
    const reco::Candidate* mom = p->mother();
    while (mom != 0 && mom->pdgId() == pId)
        mom = mom->mother();
    return mom;
}

// In Pythia 8, final state heavy bosons with kinematical info enclosed have status 62.
reco::Candidate* GenAnalyzer::FindGenParticleByIdAndStatus(std::vector<reco::GenParticle>& Vect, int pdg, int stat) {
    for(unsigned int i = 0; i < Vect.size(); i++) {
      if( (fabs(Vect[i].pdgId()) == pdg) && (Vect[i].status() == stat) ) return dynamic_cast<reco::Candidate*>(&Vect[i]);
    }
    return NULL;
}


// ---------- Pileup ----------

float GenAnalyzer::GetPUWeight(const edm::Event& iEvent) {
  //  int nPT(0);
  //  // https://twiki.cern.ch/twiki/bin/view/CMS/SWGuideCMSDataAnalysisSchool2012PileupReweighting
  //  if(!iEvent.isRealData()) {
  //    edm::Handle<std::vector<PileupSummaryInfo> > PUInfo;
  //    iEvent.getByLabel(edm::InputTag("PileupSummaryInfo"), PUInfo);
  //    for(std::vector<PileupSummaryInfo>::const_iterator pvi=PUInfo->begin(), pvn=PUInfo->end(); pvi!=pvn; ++pvi) {
  //      if(pvi->getBunchCrossing()==0) nPT=pvi->getTrueNumInteractions(); // getPU_NumInteractions();
  //    }
  //    return LumiWeights->weight( nPT );
  //  }
    return 1.;
}


// ---------- PDF ----------

std::pair<float, float> GenAnalyzer::GetQ2Weight(const edm::Event& iEvent) {
  //  float Q, id1, id2, x1, x2;
  //  // Open LHE event
  //  edm::Handle<LHEEventProduct> lheProduct;
  //  iEvent.getByLabel(edm::InputTag("source"), lheProduct);
  //  // Access event info from LHE
  //  if(lheProduct.isValid()) {
  //    const lhef::HEPEUP hepeup=lheProduct->hepeup();
  //    // PDF
  //    Q   = hepeup.SCALUP;
  //    // id of the particle: 0 is for gluons
  //    id1 = hepeup.IDUP[0]==21 ? 0 : hepeup.IDUP[0];
  //    id2 = hepeup.IDUP[1]==21 ? 0 : hepeup.IDUP[1];
  //    x1  = fabs(hepeup.PUP[0][2]/6500.);
  //    x2  = fabs(hepeup.PUP[1][2]/6500.);
  //    //xfx1 = hepeup.XPDWUP.first;
  //    //xfx2 = hepeup.XPDWUP.second;
  //  }
  //  // Access event info from Gen if LHE info are not available
  //  else {
  //    edm::Handle<GenEventInfoProduct> genProduct;
  //    iEvent.getByLabel(edm::InputTag("generator"), genProduct);
  //    Q   = genProduct->pdf()->scalePDF;
  //    id1 = genProduct->pdf()->id.first;
  //    id2 = genProduct->pdf()->id.second;
  //    x1  = genProduct->pdf()->x.first;
  //    x2  = genProduct->pdf()->x.second;
  //    //pdf1 = genProduct->pdf()->xPDF.first;
  //    //pdf2 = genProduct->pdf()->xPDF.second;
  //  }
  //  //LHAPDF::usePDFMember(1, 0);
  //  float pdf1 = LHAPDF::xfx(1, x1, Q, id1)/x1;
  //  float pdf2 = LHAPDF::xfx(1, x2, Q, id2)/x2;
  //  // Q2 scale
  //  float newqcd1_up = LHAPDF::xfx(1, x1, Q*2, id1)/x1;
  //  float newqcd2_up = LHAPDF::xfx(1, x2, Q*2, id2)/x2;
  //  float newqcd1_down = LHAPDF::xfx(1, x1, Q/2, id1)/x1;
  //  float newqcd2_down = LHAPDF::xfx(1, x2, Q/2, id2)/x2;
  //  float Q2ScaleWeightUp = (newqcd1_up/pdf1)*(newqcd2_up/pdf2);
  //  float Q2ScaleWeightDown = (newqcd1_down/pdf1)*(newqcd2_down/pdf2);
    float Q2ScaleWeightUp = 1.;
    float Q2ScaleWeightDown = 1.;
    return std::pair<float, float>(Q2ScaleWeightUp, Q2ScaleWeightDown);
}

//float GenAnalyzer::GetPDFWeight(const edm::Event& iEvent) {
//  float Q, id1, id2, x1, x2;
//  // Open LHE event
//  edm::Handle<LHEEventProduct> lheProduct;
//  iEvent.getByLabel(edm::InputTag("source"), lheProduct);
//  // Access event info from LHE
//  if(lheProduct.isValid()) {
//    const lhef::HEPEUP hepeup=lheProduct->hepeup();
//    // PDF
//    Q   = hepeup.SCALUP;
//    // id of the particle: 0 is for gluons
//    id1 = hepeup.IDUP[0]==21 ? 0 : hepeup.IDUP[0];
//    id2 = hepeup.IDUP[1]==21 ? 0 : hepeup.IDUP[1];
//    x1  = fabs(hepeup.PUP[0][2]/6500.);
//    x2  = fabs(hepeup.PUP[1][2]/6500.);
//    //xfx1 = hepeup.XPDWUP.first;
//    //xfx2 = hepeup.XPDWUP.second;
//  }
//  // Access event info from Gen if LHE info are not available
//  else {
//    edm::Handle<GenEventInfoProduct> genProduct;
//    iEvent.getByLabel(edm::InputTag("generator"), genProduct);
//    Q   = genProduct->pdf()->scalePDF;
//    id1 = genProduct->pdf()->id.first;
//    id2 = genProduct->pdf()->id.second;
//    x1  = genProduct->pdf()->x.first;
//    x2  = genProduct->pdf()->x.second;
//    //pdf1 = genProduct->pdf()->xPDF.first;
//    //pdf2 = genProduct->pdf()->xPDF.second;
//  }
//  //LHAPDF::usePDFMember(1, 0);
//  float pdf1 = LHAPDF::xfx(1, x1, Q, id1)/x1;
//  float pdf2 = LHAPDF::xfx(1, x2, Q, id2)/x2;
//  // New PDF, if not working type <scramv1 setup lhapdffull> to enable more than one LHAPDF set
//  //LHAPDF::usePDFMember(2, 0);
//  float newpdf1 = LHAPDF::xfx(2, x1, Q, id1)/x1;
//  float newpdf2 = LHAPDF::xfx(2, x2, Q, id2)/x2;
//  return (newpdf1/pdf1)*(newpdf2/pdf2);
//}

// ---------- LHE Event ----------

float GenAnalyzer::GetStitchWeight(std::map<std::string, float> Map) {

    if(Map.size() <= 1) return 1.;
    if(Sample=="" || (SampleDYJetsToLL.size()==0 && SampleZJetsToNuNu.size()==0 && SampleWJetsToLNu.size()==0)) return 1.;
    if(Sample.find("JetsToLL") == std::string::npos && Sample.find("JetsToNuNu") == std::string::npos && Sample.find("JetsToLNu") == std::string::npos) return 1.;
    if(hPartons.find(Sample) == hPartons.end()) return 1.;
    
    // Find bins
    float StitchWeight(1.);
    int binPartons = hPartons[Sample]->FindBin(Map["lhePartons"]);
    int binBPartons = hBPartons[Sample]->FindBin(Map["lheBPartons"]);
    int binHT = hHT[Sample]->FindBin(Map["lheHT"]);
    int binPtZ = hPtV[Sample]->FindBin(Map["lhePtZ"] > 0. ? Map["lhePtZ"] : Map["lhePtW"]);
    
    // Calculate numerator and denominator
    if(Sample.find("JetsToLL") != std::string::npos) {
        float num(0.), den(0.);
        for(unsigned int i = 0; i < SampleDYJetsToLL.size(); i++) {
            den += hPartons[SampleDYJetsToLL[i]]->GetBinContent(binPartons);
            den += hBPartons[SampleDYJetsToLL[i]]->GetBinContent(binBPartons);
            den += hHT[SampleDYJetsToLL[i]]->GetBinContent(binHT);
            den += hPtV[SampleDYJetsToLL[i]]->GetBinContent(binPtZ);
        }
        num += hPartons[Sample]->GetBinContent(binPartons);
        num += hBPartons[Sample]->GetBinContent(binBPartons);
        num += hHT[Sample]->GetBinContent(binHT);
        num += hPtV[Sample]->GetBinContent(binPtZ);
        if(!(den != den || num != num) && den > 0.) StitchWeight = num / den;
    }
    return StitchWeight;
}

/// returning the topPt weight for each top/antitop
/// the total ttbar weight is implemented as sqrt(w_t1*w_t2)
float GenAnalyzer::GetTopPtWeight(float toppt) {
    if(!ApplyTopPtReweigth) return 1.;
    if(toppt <= 0) return 1.;
    return (toppt<400) ? sqrt(exp(0.0615-0.0005*toppt)) : sqrt(0.87066325126911626); // 0.87066325126911626 = exp(0.0615-0.0005*400)
}

float GenAnalyzer::GetZewkWeight(float zpt) {
    if(!ApplyEWK) return 1.;
    if(zpt <= 0) return 1.;
    return fZEWK->Eval(zpt);
}

float GenAnalyzer::GetWewkWeight(float wpt) {
    if(!ApplyEWK) return 1.;
    if(wpt <= 0) return 1.;
    return fWEWK->Eval(wpt);
}


// returns a vector with the gen particles from the decays of the particles in the list
std::vector<reco::GenParticle> GenAnalyzer::PartonsFromDecays(const std::vector<int> & pdgIds)  {
  
  // vector to save the partons
  std::vector<reco::GenParticle> partons;

  if(isRealData or PythiaLOSample) return partons;

  for (auto & genParticle : *GenCollection ) {
    // check if pdgid in vector
    if (std::find(pdgIds.begin(), pdgIds.end(), genParticle.pdgId()) != pdgIds.end()) {
      if (genParticle.numberOfDaughters() > 1) {
        if (genParticle.daughter(0) && genParticle.daughter(1)) {
          // emplace genParticles in output vector
          partons.emplace_back(*FindLastDaughterGen(dynamic_cast<const reco::GenParticle*>(genParticle.daughter(0))));
          partons.emplace_back(*FindLastDaughterGen(dynamic_cast<const reco::GenParticle*>(genParticle.daughter(1))));
        }
      }
    }
  }

  return partons;
}

// returns a vector with the gen particles from the decays of the particles in the list
// saves decaying particles in vector passed a reference
std::vector<reco::GenParticle> GenAnalyzer::PartonsFromDecays(const std::vector<int> & pdgIds, std::vector<reco::GenParticle> & genDecay)  {
  
  // vector to save the partons
  std::vector<reco::GenParticle> partons;

  if(isRealData or PythiaLOSample) return partons;

  for (auto & genParticle : *GenCollection ) {
    // check if pdgid in vector
    if (std::find(pdgIds.begin(), pdgIds.end(), genParticle.pdgId()) != pdgIds.end()) {
      if (genParticle.numberOfDaughters() > 1) {
        if (genParticle.daughter(0) && genParticle.daughter(1)) {
          genDecay.emplace_back(genParticle);
          // emplace genParticles in output vector
          partons.emplace_back(*FindLastDaughterGen(dynamic_cast<const reco::GenParticle*>(genParticle.daughter(0))));
          partons.emplace_back(*FindLastDaughterGen(dynamic_cast<const reco::GenParticle*>(genParticle.daughter(1))));
        }
      }
    }
  }

  return partons;
}

// returns a vector with the first n particles which have pdgIDs
std::vector<reco::GenParticle> GenAnalyzer::FirstNGenParticles(const std::vector<int> & pdgIds, std::size_t n)  {
  
  // vector to save the rtons
  std::vector<reco::GenParticle> particles;

  if(isRealData or PythiaLOSample) return particles;

  for (auto & genParticle : *GenCollection ) {
    // check if pdgid in vector
    if (std::find(pdgIds.begin(), pdgIds.end(), genParticle.pdgId()) != pdgIds.end()) {
          particles.emplace_back(genParticle);
    }
    if (particles.size() == n) return particles;
  }

  return particles;
}
