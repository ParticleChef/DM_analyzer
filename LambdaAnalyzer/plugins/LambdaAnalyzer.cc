// -*- C++ -*-
//
// Package:    edmAna/LambdaAnayzer
// Class:      LambdaAnayzer
// 
/**\class LambdaAnayzer LambdaAnayzer.cc edmAna/LambdaAnayzer/plugins/LambdaAnayzer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Siew Yan Hoh
//         Created:  Tue, 12 Feb 2019 20:23:44 GMT
//
//

#include "LambdaAnalyzer.h"

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
LambdaAnalyzer::LambdaAnalyzer(const edm::ParameterSet& iConfig):
  GenPSet(iConfig.getParameter<edm::ParameterSet>("genSet")),
  JetPSet(iConfig.getParameter<edm::ParameterSet>("jetSet")),
  ElectronPSet(iConfig.getParameter<edm::ParameterSet>("electronSet")),
  MuonPSet(iConfig.getParameter<edm::ParameterSet>("muonSet")),
  HistFile(iConfig.getParameter<std::string>("histFile"))
{
   //now do what ever initialization is needed
   usesResource("TFileService");
   //TFileDirectory allDir=fs->mkdir("All/");

   // Initialize Objects
   theGenAnalyzer      = new GenAnalyzer(GenPSet, consumesCollector());
   theJetAnalyzer      = new JetAnalyzer(JetPSet, consumesCollector());
   theElectronAnalyzer = new ElectronAnalyzer(ElectronPSet, consumesCollector());
   theMuonAnalyzer     = new MuonAnalyzer(MuonPSet, consumesCollector());

   //  ---------- Plots Initialization ----------
   TFileDirectory allDir=fs->mkdir("All/");
   TFileDirectory genDir=fs->mkdir("Gen/");
   TFileDirectory recoDir=fs->mkdir("Reco/");
   
   //make TH1F
   //std::vector<std::string> nLabels={"All", "Trigger", "Iso Lep #geq 2", "Z cand ", "Jets #geq 2", "Z mass ", "h mass ", "Top veto", "bJets #geq 1", "bJets #geq 2"};
   std::vector<std::string> nLabels={"All","MET > 160", "MET > 200", "MET > 250"}; //, "Jets #geq 2", "Z mass ", "h mass ", "Top veto","bJets #geq 0", "bJets #geq 1", "bJets #geq 2"};

   int nbins;
   float min, max;
   std::string name, title, opt;

   std::ifstream histFile(HistFile);
   if(!histFile.is_open()) {
     throw cms::Exception("Analyzer", HistFile + " file not found");
   }
   while(histFile >> name >> title >> nbins >> min >> max >> opt) {
       //std::cout << name << ", " << title << ", " << nbins << ", " << min << ", " << ", " << max << ", " << opt << std::endl;

     if(name.find('#')==std::string::npos) {
       while(title.find("~")!=std::string::npos) title=title.replace(title.find("~"), 1, " "); // Remove ~
       if(name.substr(0, 2)=="a_") Hist[name] = allDir.make<TH1F>(name.c_str(), title.c_str(), nbins, min, max);
       if(name.substr(0, 2)=="g_") Hist[name] = genDir.make<TH1F>(name.c_str(), title.c_str(), nbins, min, max);
       if(name.substr(0, 2)=="r_") Hist[name] = recoDir.make<TH1F>(name.c_str(), title.c_str(), nbins, min, max); 

     Hist[name]->Sumw2();
     Hist[name]->SetOption(opt.c_str());
     if(name=="a_nEvents" || name=="e_nEvents" || name=="m_nEvents") for(unsigned int i=0; i<nLabels.size(); i++) Hist[name]->GetXaxis()->SetBinLabel(i+1, nLabels[i].c_str());
     }
   }
   histFile.close();

   std::cout << "---------- STARTING ----------" << std::endl;
}

LambdaAnalyzer::~LambdaAnalyzer(){
  
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)
  std::cout << "---------- ENDING  ----------" << std::endl;
  delete theGenAnalyzer;
  delete theJetAnalyzer;
  delete theElectronAnalyzer;
  delete theMuonAnalyzer;
  
}


//
// member functions
//

float LambdaAnalyzer::deltaPhi(float phi1, float phi2){
  float PHI = fabs(phi1-phi2);
  if (PHI<=3.14159265)
    return PHI;
  else
    return 2*3.14159265-PHI; 
}

std::vector<const reco::GenParticle*> LambdaAnalyzer::IndexByPtGen(std::vector<const reco::GenParticle*> vector)
{
  compgen comparatorgen;

  std::sort (vector.begin() , vector.end() , comparatorgen);
  return vector;
}

std::vector<pat::Jet> LambdaAnalyzer::IndexByPtPat(std::vector<pat::Jet> vector)
{
  comppat comparatorpat;

  std::sort (vector.begin() , vector.end() , comparatorpat);
  return vector;
}

float LambdaAnalyzer::deltaR(float phi1, float eta1, float phi2, float eta2){
  return sqrt(pow((eta2-eta1),2)+pow(deltaPhi(phi1,phi2),2));
}

// ------------ method called for each event  ------------
void
LambdaAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;
 
   //Initialize event variable
   nJets=0; EventWeight=1.;
  
   Hist["a_nEvents"]->Fill(1.,EventWeight);
   //std::cout << "Event filled" << std::endl;
   //std::cout << std::endl;

   //GEN
   // Gen Weight
   //std::map<int, float> GenWeight = theGenAnalyzer->FillWeightsMap(iEvent);

   // Lhe Particles 
   //std::map<std::string, float> LheMap = theGenAnalyzer->FillLheMap(iEvent);

   // Gen Particle
   std::vector<reco::GenParticle> GenPVect = theGenAnalyzer->FillGenVector(iEvent);

   for(unsigned int i = 0; i < GenPVect.size(); i++){
       std::cout<<"Gen ID: "<<GenPVect[i].pdgId()<<std::endl;
   }
  
   //std::cout << GenPVect.size() << std::endl;
   //for(unsigned int k = 0; k < GenPVect.size(); k++) {
   //    if( abs(GenPVect[k].pdgId()) == 18 ) std::cout << GenPVect[k].pdgId() << std::endl;
   //}

   // Gen candidates
   //reco::Candidate* theGenZ = theGenAnalyzer->FindGenParticle(GenPVect, 23);
   //reco::Candidate* theGenW = theGenAnalyzer->FindGenParticle(GenPVect, 24);
   //reco::Candidate* theGenTop     = theGenAnalyzer->FindGenParticle(GenPVect, 6);
   //reco::Candidate* theGenAntiTop = theGenAnalyzer->FindGenParticle(GenPVect, -6);

   //reco::Candidate* theDM = theGenAnalyzer->FindGenParticle(GenPVect, 18);
   //reco::Candidate* theZp = theGenAnalyzer->FindGenParticle(GenPVect, 55);
   
   //std::vector<reco::Candidate*> theDM = theGenAnalyzer->FindGenParticleVector(GenPVect, 18);
   //std::vector<reco::Candidate*> theZp = theGenAnalyzer->FindGenParticleVector(GenPVect, 55);
   std::vector<reco::GenParticle> theDM = theGenAnalyzer->SelectGenVector(GenPVect, 18);
   std::vector<reco::GenParticle> theTop = theGenAnalyzer->SelectGenVector(GenPVect, 6);
   std::vector<reco::GenParticle> theEle = theGenAnalyzer->SelectGenVector(GenPVect, 11);
   std::vector<reco::GenParticle> theMu = theGenAnalyzer->SelectGenVector(GenPVect, 13);

   TLorentzVector vector1;
   TLorentzVector vector2;
   TLorentzVector sumVect;

//   std::cout << "The dm size: " << theDM.size() << std::endl;
   for(unsigned int k = 0; k < theDM.size(); k++) {
       if( k == 0 ) vector1.SetPtEtaPhiM(theDM[k].pt(), theDM[k].eta(), theDM[k].phi(), theDM[k].mass());
       if( k == 1 ) vector2.SetPtEtaPhiM(theDM[k].pt(), theDM[k].eta(), theDM[k].phi(), theDM[k].mass());
       Hist["g_DMmass"]->Fill(theDM[k].mass(),EventWeight);
       Hist["g_DMpt"]->Fill(theDM[k].pt(),EventWeight);
       Hist["g_DMeta"]->Fill(theDM[k].eta(),EventWeight);
       Hist["g_DMphi"]->Fill(theDM[k].phi(),EventWeight);
       //std::cout<<"DM pt: "<<theDM[k].pt()<<std::endl;
   }

   sumVect = vector1 + vector2;

   Hist["g_Medmass"]->Fill(sumVect.M(),EventWeight);
   Hist["g_Medpt"]->Fill(sumVect.Pt(),EventWeight);
   Hist["g_Medeta"]->Fill(sumVect.Eta(),EventWeight);
   Hist["g_Medphi"]->Fill(sumVect.Phi(),EventWeight);

   for(unsigned int k = 0; k < theTop.size(); k++) {
       Hist["g_Topmass"]->Fill(theTop[k].mass(),EventWeight);
       Hist["g_Toppt"]->Fill(theTop[k].pt(),EventWeight);
       Hist["g_TopEta"]->Fill(theTop[k].eta(),EventWeight);
       Hist["g_TopPhi"]->Fill(theTop[k].phi(),EventWeight);
   }

   for(unsigned int k = 0; k < theEle.size(); k++) {
       Hist["g_Elemass"]->Fill(theEle[k].mass(),EventWeight);
       Hist["g_Elept"]->Fill(theEle[k].pt(),EventWeight);
       Hist["g_EleEta"]->Fill(theEle[k].eta(),EventWeight);
       Hist["g_ElePhi"]->Fill(theEle[k].phi(),EventWeight);
   }
   for(unsigned int k = 0; k < theMu.size(); k++) {
       Hist["g_Mumass"]->Fill(theMu[k].mass(),EventWeight);
       Hist["g_Mupt"]->Fill(theMu[k].pt(),EventWeight);
       Hist["g_MuEta"]->Fill(theMu[k].eta(),EventWeight);
       Hist["g_MuPhi"]->Fill(theMu[k].phi(),EventWeight);
   }

   /*
   std::vector<int> LepIds = {11,13,15,-11,-13,-15};
   std::vector<int> NeuIds = {12,14,16,-12,-14,-16};
   std::vector<int> HadIds = {1,2,3,4,5,-1,-2,-3,-4,-5};

   reco::GenParticle* theGenLep = theGenAnalyzer->FindGenParticleGenByIds(GenPVect, LepIds);
   reco::GenParticle* theGenNeu = theGenAnalyzer->FindGenParticleGenByIds(GenPVect, NeuIds);
   reco::GenParticle* theGenHad = theGenAnalyzer->FindGenParticleGenByIds(GenPVect, HadIds);
   */

   //std::cout << "Call candidates" << std::endl;
   //std::cout << std::endl;

   // Electron
   std::vector<pat::Electron> ElecVect = theElectronAnalyzer->FillElectronVector(iEvent);

   // Muon
   std::vector<pat::Muon> MuonVect = theMuonAnalyzer->FillMuonVector(iEvent);

   // Jet
   std::vector<pat::Jet> JetsVect = theJetAnalyzer->FillJetVector(iEvent);
   Hist["r_nJetb"]->Fill(JetsVect.size(),EventWeight);
   theJetAnalyzer->CleanJetsFromMuons(JetsVect, MuonVect, 0.4);
   theJetAnalyzer->CleanJetsFromElectrons(JetsVect, ElecVect, 0.4);

   // GenJet
   std::vector<reco::GenJet> GenJetsVect = theJetAnalyzer->FillGenJetVector(iEvent);
   
   //MET
   pat::MET MET = theJetAnalyzer->FillMetVector(iEvent);   
   
   Hist["a_met"]->Fill(MET.pt(), EventWeight);
   // Save MET phi
   METphi = MET.phi();
   METeta = MET.eta();
   //std::cout << "MET filled" << std::endl;
   //std::cout << std::endl;
   
   // Fill number of events when MET > 200 GeV
   if ( MET.pt() > 160. ) Hist["a_nEvents"]->Fill(2.,EventWeight);
   if ( MET.pt() > 200. ) Hist["a_nEvents"]->Fill(3.,EventWeight);
   if ( MET.pt() > 250. ) Hist["a_nEvents"]->Fill(4.,EventWeight);

   //JET MC Truth
   //The pruned genParticles are the ones pointed by the MC matching of the high level patObjectes (e.g. pat::Electron::genParticle()) 
   //https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookMiniAOD2016#MC_Truth

   //Matching by R may not work reliably in dense environments, such as jets. For studies needing high quality matching of reconstructed tracks with true tracks, it is possible to base the matching either on the number of hits that they share in common, or on a comparison of the 5 helix parameters describing the track. 

   std::vector<const reco::GenParticle*> HardGenJetsVect;
   std::vector<pat::Jet> HardJetsVect;

   for(unsigned int i = 0; i < JetsVect.size(); i++){
     //std::cout<<"jet pt = "<<JetsVect[i].pt()<<std::endl;
     pat::Jet j = JetsVect[i];
     if (j.genParton() == NULL)
       continue;
     /*
     std::cout<<"genParton pdgid = "<< ( j.genParton() )->pdgId() <<std::endl;
     std::cout<<"Robust Flags"<<std::endl;
     std::cout<<"isPromptFinalState() = "<< ( j.genParton() )->isPromptFinalState() <<std::endl;
     std::cout<<"Less robust Flags"<<std::endl;
     std::cout<<"isHardProcess() = "<< ( j.genParton() )->isHardProcess() <<std::endl;
     std::cout<<"fromHardProcessFinalState() = "<< ( j.genParton() )->fromHardProcessFinalState() <<std::endl;
     std::cout<<"fromHardProcessDecayed() = "<< ( j.genParton() )->fromHardProcessDecayed() <<std::endl;
     std::cout<<"isLastCopy() = "<< ( j.genParton() )->isLastCopy() <<std::endl;

     std::cout<<"hadronFlavour = "<<(JetsVect[i].hadronFlavour())<<std::endl;
     std::cout<<"partonFlavour = "<<(JetsVect[i].partonFlavour())<<std::endl;
     */

     if (j.genParton()->isHardProcess()){
       HardGenJetsVect.push_back( j.genParton() );
       HardJetsVect.push_back(j);
     }
   }

   //std::cout << "Before Sorting" << std::endl; 
   //std::cout<<"Total number of match reco jet = "<< HardJetsVect.size() <<std::endl;
   for(unsigned int i = 0; i < HardJetsVect.size(); i++){
     pat::Jet j = HardJetsVect[i];
     //std::cout<<"Jet number "<<i<<" with j.pt() = "<<j.pt()<<std::endl;
   }
   //Sort vector
   JetsVect.clear();
   std::vector<const reco::GenParticle*> JetsMCmatch;
   JetsVect = IndexByPtPat(HardJetsVect);
   JetsMCmatch = IndexByPtGen(HardGenJetsVect);
   //std::cout << "After Sorting" << std::endl;
   //std::cout<<"Total number of match reco jet = "<< JetsVect.size() <<std::endl;
   for(unsigned int i = 0; i < JetsVect.size(); i++){
     pat::Jet j = JetsVect[i];
     //std::cout<<"Jet number "<<i<<" with j.pt() = "<<j.pt()<<std::endl;
   }

   //Filling
   
   //GenJet
   Hist["g_nJet"]->Fill(JetsMCmatch.size(), EventWeight);
   for(unsigned int i = 0; i < JetsMCmatch.size(); i++){
     if (i>2) break;
     Hist[("g_Jet"+std::to_string(i+1)+"pt").c_str()]->Fill(JetsMCmatch[i]->pt(), EventWeight);
     Hist[("g_Jet"+std::to_string(i+1)+"eta").c_str()]->Fill(JetsMCmatch[i]->eta(), EventWeight);
     Hist[("g_Jet"+std::to_string(i+1)+"phi").c_str()]->Fill(JetsMCmatch[i]->phi(), EventWeight);
     if (JetsMCmatch.size() >= 2 )
     {
         Hist["g_J1J2dPhi"]->Fill(deltaPhi(JetsMCmatch[0]->phi(), JetsMCmatch[1]->phi()), EventWeight);
         Hist["g_J1J2dEta"]->Fill(JetsMCmatch[0]->eta() - JetsMCmatch[1]->eta(), EventWeight);
         Hist["g_J1J2dR"]->Fill(deltaR(JetsMCmatch[0]->phi(), JetsMCmatch[0]->eta(), JetsMCmatch[1]->phi(), JetsMCmatch[1]->eta()), EventWeight);
     }
    }
   
   //std::cout << "Genjet finished" << std::endl;
   //std::cout << std::endl;
  
   nJets=JetsVect.size();
   //RecoJet
   Hist["r_nJet"]->Fill(nJets, EventWeight);
   for(unsigned int i = 0; i < JetsVect.size(); i++){ 
     if (i>2) break;
     Hist[("r_Jet"+std::to_string(i+1)+"pt").c_str()]->Fill(JetsVect[i].pt(), EventWeight); 
     Hist[("r_Jet"+std::to_string(i+1)+"eta").c_str()]->Fill(JetsVect[i].eta(), EventWeight); 
     RecoJphi = JetsVect[0].phi();
     RecoJeta = JetsVect[0].eta();
     if (JetsVect.size() >= 2 )
     {
         Hist["r_J1J2dPhi"]->Fill(deltaPhi(JetsVect[0].phi(), JetsVect[1].phi()), EventWeight);
         Hist["r_J1J2dEta"]->Fill(JetsVect[0].eta() - JetsVect[1].eta(), EventWeight);
         Hist["r_J1J2dR"]->Fill(deltaR(JetsVect[0].phi(), JetsVect[0].eta(), JetsVect[1].phi(), JetsVect[1].eta()), EventWeight);
     }
   }
   //std::cout << "Recojet finished" << std::endl;
   //std::cout << std::endl;

   for(unsigned int i = 0; i < ElecVect.size(); i++){
	   Hist["r_Elept" ]->Fill(ElecVect[i].pt(), EventWeight);
	   Hist["r_EleEta"]->Fill(ElecVect[i].eta(), EventWeight);
	   Hist["r_ElePhi"]->Fill(ElecVect[i].phi(), EventWeight);
   }

   for(unsigned int i = 0; i < MuonVect.size(); i++){
	   Hist["r_Mupt" ]->Fill( MuonVect[i].pt(), EventWeight);
	   Hist["r_MuEta"]->Fill(MuonVect[i].eta(), EventWeight);
	   Hist["r_MuPhi"]->Fill(MuonVect[i].phi(), EventWeight);
   }

   // Calculate angle difference (delta phi) of MET and 
   Hist["r_dPhi"]->Fill(deltaPhi(RecoJphi, METphi), EventWeight);
   Hist["r_dR"]->Fill(deltaR(METphi, METeta, RecoJphi, RecoJeta), EventWeight);

   tree->Fill();
   
   //std::cout << "Filling finished" << std::endl;
   //std::cout << std::endl;
}


// ------------ method called once each job just before starting event loop  ------------
void 
LambdaAnalyzer::beginJob()
{
  tree=fs->make<TTree>("tree", "tree");
}

// ------------ method called once each job just after ending the event loop  ------------
void 
LambdaAnalyzer::endJob() 
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
LambdaAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(LambdaAnalyzer);
