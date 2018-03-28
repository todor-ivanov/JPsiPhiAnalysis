// -*- C++ -*-
//
// Package:    DiMuonDiTrakMLAnalyzer
// Class:      DiMuonDiTrakMLAnalyzer
//
// Author:  Adriano Di Florio
//

#include "../interface/DiMuonDiTrakMLAnalyzer.h"
#include "../interface/DiMuonVtxReProducer.h"


//
// constructors and destructor
//

DiMuonDiTrakMLAnalyzer::DiMuonDiTrakMLAnalyzer(const edm::ParameterSet& iConfig):
muons_(consumes<reco::MuonCollection>(iConfig.getParameter<edm::InputTag>("Muons"))),
traks_(consumes<reco::TrackCollection>(iConfig.getParameter<edm::InputTag>("Tracks"))),
thebeamspot_(consumes<reco::BeamSpot>(iConfig.getParameter<edm::InputTag>("BeamSpot"))),
thePVs_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("PrimaryVertex"))),
DiMuonMassCuts_(iConfig.getParameter<std::vector<double>>("DiMuonCuts")),
DiTrakMassCuts_(iConfig.getParameter<std::vector<double>>("DiTrakCuts"))
// thebeamspot_(consumes<reco::BeamSpot>(iConfig.getParameter<edm::InputTag>("BeamSpot"))),
// thePVs_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("PrimaryVertex"))),
// DiMuonMassCuts_(iConfig.getParameter<std::vector<double>>("DiMuonCuts")),
// DiTrakMassCuts_(iConfig.getParameter<std::vector<double>>("DiTrakCuts")),
// DiMuonDiTrakMassCuts_(iConfig.getParameter<std::vector<double>>("DiMuonDiTrakCuts")),
// massCands_(iConfig.getParameter<std::vector<double>>("CandsMasses"))
{
  // produces<pat::CompositeCandidateCollection>();
  muon_mass = 0.1056583715;

  edm::Service < TFileService > fs;
  ml_tree = fs->make < TTree > ("DiMuonDiTrackML", "Tree of DiTrakDiMuon");

  ml_tree->Branch("run",      &run,      "run/i");
  ml_tree->Branch("event",    &event,    "event/l");
  ml_tree->Branch("lumiblock",&lumiblock,"lumiblock/i");

  //
  // ml_tree->Branch("nditrak",  &nditrak,    "nditrak/i");
  // ml_tree->Branch("trigger",  &trigger,  "trigger/i");
  // ml_tree->Branch("charge",   &charge,   "charge/I");
  //
  // ml_tree->Branch("isBest",   &isBest,   "isBest/O");
  //
  // if(addTrigger_)
  // {
  //   ml_tree->Branch("trigs_pt",   &trigs_pt);
  //   ml_tree->Branch("trigs_eta",   &trigs_eta);
  //   ml_tree->Branch("trigs_phi",   &trigs_phi);
  //   ml_tree->Branch("trigs_m",   &trigs_m);
  //   ml_tree->Branch("trigs_filters", &trigs_filters);
  // }
  // ml_tree->Branch("ditrak_p4", "TLorentzVector", &ditrak_p4);
  // ml_tree->Branch("trakP_p4",  "TLorentzVector", &trakP_p4);
  // ml_tree->Branch("trakN_p4",  "TLorentzVector", &trakN_p4);
  //
  // ml_tree->Branch("MassErr",   &MassErr,    "MassErr/F");
  // ml_tree->Branch("vProb",     &vProb,      "vProb/F");
  // ml_tree->Branch("DCA",       &DCA,        "DCA/F");
  // ml_tree->Branch("ctauPV",    &ctauPV,     "ctauPV/F");
  // ml_tree->Branch("ctauErrPV", &ctauErrPV,  "ctauErrPV/F");
  // ml_tree->Branch("cosAlpha",  &cosAlpha,   "cosAlpha/F");
  // ml_tree->Branch("lxy",       &lxyPV,      "lxy/F");
  // ml_tree->Branch("lxyErrPV",    &lxyErrPV,      "lxyErr/F");
  //
  // ml_tree->Branch("numPrimaryVertices", &numPrimaryVertices, "numPrimaryVertices/i");


}

DiMuonDiTrakMLAnalyzerAnalyzer::~DiMuonDiTrakMLAnalyzerAnalyzer() {}

//
// member functions
//

const reco::Candidate* DiMuonDiTrakMLAnalyzerAnalyzer::GetAncestor(const reco::Candidate* p) {
   if (p->numberOfMothers()) {
      if  ((p->mother(0))->pdgId() == p->pdgId()) return GetAncestor(p->mother(0));
      else return p->mother(0);
   }
   return p;
}


/* Grab Trigger information. Save it in variable trigger, trigger is an uint between 0 and 256, in binary it is:
   (pass 2)(pass 1)(pass 0)
   ex. 7 = pass 0, 1 and 2
   ex. 6 = pass 1, 2
   ex. 1 = pass 0
*/


UInt_t DiMuonDiTrakMLAnalyzerAnalyzer::getTriggerBits(const edm::Event& iEvent ) {

  UInt_t trigger = 0;

  edm::Handle< edm::TriggerResults > triggerResults_handle;
  iEvent.getByToken( triggerResults_Label , triggerResults_handle);

  if (triggerResults_handle.isValid()) {
     const edm::TriggerNames & TheTriggerNames = iEvent.triggerNames(*triggerResults_handle);
     unsigned int NTRIGGERS = HLTs_.size();

     for (unsigned int i = 0; i < NTRIGGERS; i++) {
        for (int version = 1; version < 20; version++) {
           std::stringstream ss;
           ss << HLTs_[i] << "_v" << version;
           unsigned int bit = TheTriggerNames.triggerIndex(edm::InputTag(ss.str()).label());
           if (bit < triggerResults_handle->size() && triggerResults_handle->accept(bit) && !triggerResults_handle->error(bit)) {
              trigger += (1<<i);
              break;
           }
        }
     }
   } else std::cout << "*** NO triggerResults found " << iEvent.id().run() << "," << iEvent.id().event() << std::endl;

   return trigger;
}

// ------------ method called for each event  ------------
void DiMuonDiTrakMLAnalyzerAnalyzer::analyze(const edm::Event & iEvent, const edm::EventSetup & iSetup) {

  std::vector<int> pixelDets{0,1,2,3,14,15,16,29,30,31};

  std::vector<double> mmMasses;
  mmMasses.push_back( 0.1056583715 );
  mmMasses.push_back( 0.1056583715 );

  edm::Handle<reco::TrackCollection> tracks;
  iEvent.getByToken(traks_,tracks);

  edm::Handle<reco::MuonCollection> muons;
  iEvent.getByToken(muons_,muons);

  reco::Vertex thePrimaryV;

  edm::ESHandle<MagneticField> magneticField;
  iSetup.get<IdealMagneticFieldRecord>().get(magneticField);
  const MagneticField* field = magneticField.product();

  edm::Handle<reco::BeamSpot> theBeamSpot;
  iEvent.getByToken(thebeamspot_,theBeamSpot);
  reco::BeamSpot bs = *theBeamSpot;

  edm::Handle<reco::VertexCollection> priVtxs;
  iEvent.getByToken(thePVs_, priVtxs);
  if ( priVtxs->begin() != priVtxs->end() ) {
    thePrimaryV = reco::Vertex(*(priVtxs->begin()));
  }
  else {
    thePrimaryV = reco::Vertex(bs.position(), bs.covariance3D());
  }


  edm::ESHandle<TransientTrackBuilder> theTTBuilder;
  iSetup.get<TransientTrackRecord>().get("TransientTrackBuilder",theTTBuilder);
  KalmanVertexFitter vtxFitter(true);

  int max = 0;

  float vProb, vNDF, vChi2, minDz = 999999.;
  float vProb_mumu, vNDF_mumu, vChi2_mumu;
  float cosAlpha, ctauPV, ctauErrPV, dca;
  float l_xy, lErr_xy;

  for(reco::MuonCollection::const_iterator mPos = muons->begin();mPos != muons->end(); ++mPos )
  {
    if(mPos->charge()<=0.0) continue;
    if (!(mPos->bestTrackRef().isNonnull())) continue;
    if (!(mPos->innerTrack().isNonnull())) continue;

    for(reco::MuonCollection::const_iterator mNeg = muons->begin();mNeg != muons->end(); ++mNeg )
    {
      if(mNeg->charge()>=0.0) continue;
      if (!(mNeg->bestTrackRef().isNonnull())) continue;
      if (!(mNeg->innerTrack().isNonnull())) continue;

      std::vector<TransientVertex> vDiMuon;

      // Candidate::LorentzVector mumu = mNeg->p4() + mPos->p4();

      TLorentzVector mu1, mu2,mumuP4;

      mu1.SetXYZM(mNeg->track()->px(),mNeg->track()->py(),mNeg->track()->pz(),muon_mass);
      mu2.SetXYZM(mPos->track()->px(),mPos->track()->py(),mPos->track()->pz(),muon_mass);

      mumuP4=mu1+mu2;
      // mumucand.setP4(mumu);
      // mumucand.setCharge(mNeg->charge()+mPos->charge());

      if(mumuP4.M() < DiMuonMassCuts_[1]) continue;
      if(mumuP4.M() > DiMuonMassCuts_[0]) continue;

      std::vector<reco::TransientTrack> mm_ttks;

      mm_ttks.push_back(theTTBuilder->build(mNeg->track()));  // pass the reco::Track, not  the reco::TrackRef (which can be transient)
      mm_ttks.push_back(theTTBuilder->build(mPos->track()));

      TransientVertex mumuVertex = vtxFitter.vertex(mm_ttks);
      CachingVertex<5> VtxForInvMass = vtxFitter.vertex( mm_ttks );

      Measurement1D MassWErr(mPos->mass(),-9999.);
      if ( field->nominalValue() > 0 )
          MassWErr = massCalculator.invariantMass( VtxForInvMass, mmMasses );
      else
          mumuVertex = TransientVertex();                      // with no arguments it is invalid

      if (!(mumuVertex.isValid()))
          continue;

      vChi2_mumu = mumuVertex.totalChiSquared();
      vNDF_mumu  = mumuVertex.degreesOfFreedom();
      vProb_mumu = TMath::Prob(vChi2_mumu,(int)vNDF_mumu);

      if(vProb_mumu < 0.0) continue;

      for(reco::TrackCollection::const_iterator posTrack = tracks->begin();posTrack != tracks->end(); ++posTrack )
      {
        if(!(posTrack->extra())) continue;
        if(posTrack->charge()<=0.0) continue;
        if(!(posTrack->extra().isNonnull())) continue;
        // if(!(posTrack->isNonnull())) continue;

        for(reco::TrackCollection::const_iterator negTrack = tracks->begin();negTrack != tracks->end(); ++negTrack )
        {
          if(!(negTrack->extra())) continue;
          if(negTrack->charge()>=0.0) continue;
          if(!(negTrack->extra().isNonnull())) continue;
          // if(!(negTrack->isNonnull())) continue;


          float deltaphi  = posTrack->p4().Phi() - it2->p4().Phi();
          while (deltaphi < -M_PI) deltaphi += 2*M_PI;
          while (deltaphi >  M_PI) deltaphi -= 2*M_PI;
          float deltaeta  = posTrack->p4().Eta() - it2->p4().Eta();
          float deltar    = sqrt(pow(deltaphi,2) + pow(deltaeta,2));

      //     pat::CompositeCandidate myPhi;
      //     vector<TransientVertex> vDiKaon;
      //
      //     TLorentzVector k1, k2, kkP4;
      //
      //     k1.SetXYZM(negTrack->px(),negTrack->py(),negTrack->pz(),muon_mass);
      //     k2.SetXYZM(posTrack->px(),posTrack->py(),posTrack->pz(),muon_mass);
      //
      //     kkP4=k1+k2;
      //     // mumucand.setP4(mumu);
      //     // mumucand.setCharge(mNeg->charge()+mPos->charge());
      //
      //     if(kkP4.M() < DiTrakMassCuts_[1]) continue;
      //     if(kkP4.M() > DiTrakMassCuts_[0]) continue;
      //
      //     if (!(posTrack->track().isNonnull() && it2->track().isNonnull())) continue;
      //
      //     // ---- fit vertex using Tracker tracks (if they have tracks) ----
      //     if (posTrack->track().isNonnull() && it2->track().isNonnull()) {
      //
    	// //build the dikaon secondary vertex
    	// vector<TransientTrack> t_tks;
    	// t_tks.push_back(theTTBuilder->build(posTrack->track()));  // pass the reco::Track, not  the reco::TrackRef (which can be transient)
    	// t_tks.push_back(theTTBuilder->build(it2->track())); // otherwise the vertex will have transient refs inside.
    	// TransientVertex myVertex = vtxFitter.vertex(t_tks);
      //
    	// CachingVertex<5> VtxForInvMass = vtxFitter.vertex( t_tks );
      //
      //       Measurement1D MassWErr(phi.M(),-9999.);
      //       if ( field->nominalValue() > 0 ) MassWErr = massCalculator.invariantMass( VtxForInvMass, kMasses );
      //       else myVertex = TransientVertex();                   // this is an invalid vertex by definition
      //
    	// myPhi.addUserFloat("MassErr",MassWErr.error());
      //
    	// if (myVertex.isValid()) {
    	//   float vChi2 = myVertex.totalChiSquared();
    	//   float vNDF  = myVertex.degreesOfFreedom();
    	//   float vProb(TMath::Prob(vChi2,(int)vNDF));
      //
    	//   myPhi.addUserFloat("vNChi2",vChi2/vNDF);
    	//   myPhi.addUserFloat("vProb",vProb);
      //
    	//   TVector3 vtx;
      //         TVector3 pvtx;
      //         VertexDistanceXY vdistXY;
      //
    	//   vtx.SetXYZ(myVertex.position().x(),myVertex.position().y(),0);
    	//   TVector3 pperp(phi.px(), phi.py(), 0);
    	//   AlgebraicVector3 vpperp(pperp.x(),pperp.y(),0);
      //
    	//   float minDz = 99999.;
    	//   float extrapZ=-9E20;
      //
    	//   if (resolveAmbiguity_) {
    	//     TwoTrackMinimumDistance ttmd;
    	//     bool status = ttmd.calculate( GlobalTrajectoryParameters(
      //                                                                    GlobalPoint(myVertex.position().x(), myVertex.position().y(), myVertex.position().z()),
      //                                                                    GlobalVector(myPhi.px(),myPhi.py(),myPhi.pz()),TrackCharge(0),&(*magneticField)),
    	// 				  GlobalTrajectoryParameters(
    	// 							     GlobalPoint(bs.position().x(), bs.position().y(), bs.position().z()),
    	// 							     GlobalVector(bs.dxdz(), bs.dydz(), 1.),TrackCharge(0),&(*magneticField)));
    	//     if (status) extrapZ=ttmd.points().first.z();
      //
    	//       int ii_pv = -1;
    	//       for (VertexCollection::const_iterator itv = priVtxs->begin(), itvend = priVtxs->end(); itv != itvend; ++itv) {
    	// 	ii_pv++;
    	// 	float deltaZ = fabs(extrapZ - itv->position().z()) ;
    	// 	if ( deltaZ < minDz ) {
    	// 	  minDz = deltaZ;
    	// 	  thePrimaryV = Vertex(*itv);
    	// 	  pv_index = ii_pv;
    	// 	}
    	//       }
    	//   } else {
      //           minDz = -1;
      //           pv_index = which_vertex;
      //           thePrimaryV = (*priVtxs)[which_vertex];
      //           extrapZ = thePrimaryV.position().z();
      //         }
      //
      //         myPhi.addUserInt("oniaPV",which_vertex);
    	//   myPhi.addUserInt("iPV",pv_index);
    	//   myPhi.addUserFloat("dzPV",minDz);
    	//   myPhi.addUserFloat("extrapZPV",extrapZ);
      //
    	//   // count the number of high Purity tracks with pT > 500 MeV attached to the chosen vertex
    	//   double vertexWeight = 0., sumPTPV = 0.;
    	//   int countTksOfPV = 0;
      //         for (size_t kk=1; kk<(size_t)ntracks_pv; kk++) {
      //             const pat::GenericParticle *it3 = &(kaons->at(kk));
      //             if (!it3->track().isNonnull())                  continue;
      //             reco::Track track = *it3->track();
      //             if(track.pt() < 0.5)                            continue;
      //             if(!track.quality(reco::TrackBase::highPurity)) continue;
      //             TransientTrack tt = theTTBuilder->build(track);
      //             pair<bool,Measurement1D> tkPVdist = IPTools::absoluteImpactParameter3D(tt,thePrimaryV);
      //             if (!tkPVdist.first)                  continue;
      //             if (tkPVdist.second.significance()>3) continue;
      //             if (track.ptError()/track.pt()>0.1)   continue;
      //             if (it3 == it2 || it3 == it)          continue;
      //             countTksOfPV++;
      //             sumPTPV += track.pt();
      //             vertexWeight += thePrimaryV.trackWeight(it3->track());
      //         }
      //
    	//   myPhi.addUserInt("countTksOfPV", countTksOfPV);
    	//   myPhi.addUserFloat("vertexWeight", (float) vertexWeight);
    	//   myPhi.addUserFloat("sumPTPV", (float) sumPTPV);
      //
    	//   ///DCA
    	//   TrajectoryStateClosestToPoint k1TS = t_tks[0].impactPointTSCP();
    	//   TrajectoryStateClosestToPoint k2TS = t_tks[1].impactPointTSCP();
    	//   float dca = 1E20;
    	//   if (k1TS.isValid() && k2TS.isValid()) {
    	//     ClosestApproachInRPhi cApp;
    	//     cApp.calculate(k1TS.theState(), k2TS.theState());
    	//     if (cApp.status() ) dca = cApp.distance();
    	//   }
    	//   myPhi.addUserFloat("DCA", dca );
    	//   ///end DCA
      //
    	//   myPhi.addUserData("PVwithkaons",thePrimaryV);
    	//   npvtracks = thePrimaryV.nTracks();
      //
    	//   // lifetime using PV
      //         pvtx.SetXYZ(thePrimaryV.position().x(),thePrimaryV.position().y(),0);
    	//   TVector3 vdiff = vtx - pvtx;
    	//   double cosAlpha = vdiff.Dot(pperp)/(vdiff.Perp()*pperp.Perp());
    	//   Measurement1D distXY = vdistXY.distance(Vertex(myVertex), thePrimaryV);
    	//   double ctauPV = distXY.value()*cosAlpha * myPhi.mass()/pperp.Perp();
    	//   GlobalError v1e = (Vertex(myVertex)).error();
    	//   GlobalError v2e = thePrimaryV.error();
      //         AlgebraicSymMatrix33 vXYe = v1e.matrix()+ v2e.matrix();
    	//   double ctauErrPV = sqrt(ROOT::Math::Similarity(vpperp,vXYe))*myPhi.mass()/(pperp.Perp2());
      //
    	//   myPhi.addUserFloat("ppdlPV",ctauPV);
      //         myPhi.addUserFloat("ppdlErrPV",ctauErrPV);
    	//   myPhi.addUserFloat("cosAlpha",cosAlpha);
      //
    	//   // lifetime using BS
      //         pvtx.SetXYZ(theBeamSpotV.position().x(),theBeamSpotV.position().y(),0);
    	//   vdiff = vtx - pvtx;
    	//   cosAlpha = vdiff.Dot(pperp)/(vdiff.Perp()*pperp.Perp());
    	//   distXY = vdistXY.distance(Vertex(myVertex), theBeamSpotV);
    	//   double ctauBS = distXY.value()*cosAlpha*myPhi.mass()/pperp.Perp();
    	//   GlobalError v1eB = (Vertex(myVertex)).error();
    	//   GlobalError v2eB = theBeamSpotV.error();
      //         AlgebraicSymMatrix33 vXYeB = v1eB.matrix()+ v2eB.matrix();
    	//   double ctauErrBS = sqrt(ROOT::Math::Similarity(vpperp,vXYeB))*myPhi.mass()/(pperp.Perp2());
      //
    	//   myPhi.addUserFloat("ppdlBS",ctauBS);
      //         myPhi.addUserFloat("ppdlErrBS",ctauErrBS);
      //
    	//   if (addCommonVertex_) myPhi.addUserData("commonVertex",Vertex(myVertex));
      //         myPhi.addUserInt("npvtracks", npvtracks);
    	//   myPhi.addUserInt("ntracks_pv",ntracks_pv );
      //
      //         // ---- If here push back to output ----
      //         phiOutput->push_back(myPhi);
      //
    	// }


        }


      }


    }

  }

// 	int padHalfSize = 8;
// 	int padSize = padHalfSize*2;
//   int maxpix = 0;
//
//   for(reco::TrackCollection::const_iterator itTrack = tracks->begin();itTrack != tracks->end(); ++itTrack )
//     {
// 	int noPixels= 0,noStripOne = 0, noStripTwo = 0;
// 	int counter = 0;
// 	float clusterSize = 0.0;
// //	std::cout<<"On "<< itTrack->found() ;
// 	for ( trackingRecHit_iterator recHit = (*itTrack).recHitsBegin();recHit != (*itTrack).recHitsEnd(); ++recHit )
// 	{
// 		counter++;
// /*		 if(!(*recHit))
//                  continue;
//
//                  if (!((*recHit)->isValid()))
//                  continue;
//
//                  if(!((*recHit)->hasPositionAndError()))
//                  continue;
// */
// 		TrackerSingleRecHit const * hit= dynamic_cast<TrackerSingleRecHit const *>(*recHit);
// //    BaseTrackerRecHit const * bhit = dynamic_cast<BaseTrackerRecHit const *>(recHit);
//
// 		DetId detid = (*recHit)->geographicalId();
// 		unsigned int subdetid = detid.subdetId();
//
//
// 	        //if(!(siPix))
// 	        //continue;
// 	        //
//
//     	 if(detid.det() != DetId::Tracker) continue;
// //	 if (!((subdetid==1) || (subdetid==2))) continue;
// // 	 if()
// 		if (dynamic_cast<SiPixelRecHit const *>(hit))
// 		{		noPixels++;
// 				clusterSize += float(dynamic_cast<SiPixelRecHit const *>(hit)->cluster()->size());
// 				clusterSize /= float(counter);
//
// 		auto clust = dynamic_cast<SiPixelRecHit const *>(hit)->cluster();
// 		TH2F hClust("hClust","hClust",
//               padSize,
//               clust->x()-padHalfSize,
//               clust->x()+padHalfSize,
//               padSize,
//               clust->y()-padHalfSize,
//               clust->y()+padHalfSize);
//
// 		for (int nx = 0; nx < padSize; ++nx)
//               for (int ny = 0; ny < padSize; ++ny)
//               hClust.SetBinContent(nx,ny,0.0);
//
//               for (int k = 0; k < clust->size(); ++k)
//               hClust.SetBinContent(hClust.FindBin((float)clust->pixel(k).x, (float)clust->pixel(k).y),(float)clust->pixel(k).adc);
//
// 		for (int ny = padSize; ny>0; --ny)
//               {
//                 for(int nx = 0; nx<padSize; nx++)
//                 {
//                   int n = (ny+2)*(padSize + 2) - 2 -2 - nx - padSize; //see TH2 reference for clarification
//
//        //          std::cout << hClust.GetBinContent(n) << " ";
// 		}
//               }
// 	//	std::cout << std::endl;
//
// 		}
// 		if (dynamic_cast<SiStripRecHit1D const *>(hit))
// 		noStripOne++;
//
// 	if (dynamic_cast<SiStripRecHit2D const *>(hit))
// 			noStripTwo++;
//
// 	}
// //	std::cout << " n. pixels = " << noPixels<< " 1DStrips = " << noStripOne << " 2DStrips = " << noStripTwo<< " clustsize : "<< clusterSize <<std::endl;
//
// 	     max = std::max(max,int(itTrack->found()));
// 		maxpix = std::max(maxpix,noPixels);
// }
//   std::cout<<"Max = " << max<< " Max pixels " << maxpix << std::endl;


}

// ------------ method called once each job just before starting event loop  ------------
void DiMuonDiTrakMLAnalyzerAnalyzer::beginJob() {}

// ------------ method called once each job just after ending the event loop  ------------
void DiMuonDiTrakMLAnalyzerAnalyzer::endJob() {}

// ------------ method called when starting to processes a run  ------------
void DiMuonDiTrakMLAnalyzerAnalyzer::beginRun(edm::Run const &, edm::EventSetup const &) {}

// ------------ method called when ending the processing of a run  ------------
void DiMuonDiTrakMLAnalyzerAnalyzer::endRun(edm::Run const &, edm::EventSetup const &) {}

// ------------ method called when starting to processes a luminosity block  ------------
void DiMuonDiTrakMLAnalyzerAnalyzer::beginLuminosityBlock(edm::LuminosityBlock const &, edm::EventSetup const &) {}

// ------------ method called when ending the processing of a luminosity block  ------------
void DiMuonDiTrakMLAnalyzerAnalyzer::endLuminosityBlock(edm::LuminosityBlock const &, edm::EventSetup const &) {}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void DiMuonDiTrakMLAnalyzerAnalyzer::fillDescriptions(edm::ConfigurationDescriptions & descriptions) {
	//The following says we do not know what parameters are allowed so do no validation
	// Please change this to state exactly what you do use, even if it is no parameters
	edm::ParameterSetDescription desc;
	desc.setUnknown();
	descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(DiMuonDiTrakMLAnalyzerAnalyzer);