//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Sun Mar  4 12:09:32 2018 by ROOT version 6.10/09
// from TTree ditrakTree/Tree of DiTrak
// found on file: ../../../../hltstudies/CMSSW_9_4_0/src/jpsiphi/jpsiphi/test/rootuple-2017-ditraktrigger.root
//////////////////////////////////////////////////////////

#ifndef DiTrakSkim_h
#define DiTrakSkim_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TTreeReaderArray.h>

// Headers needed by this particular selector
#include <vector>

#include "TLorentzVector.h"

#include <TNtuple.h>
#include <TString.h>
#include <TSelector.h>

#include <TProof.h>
#include <TProofOutputFile.h>

#include <bitset>
#include <math>

class DiTrakSkim : public TSelector {
public :
   TTreeReader     fReader;  //!the tree reader
   TTree          *fChain = 0;   //!pointer to the analyzed TTree or TChain

   TNtuple *outTuple;

   // Readers to access the data (delete the ones you do not need).
   TTreeReaderValue<UInt_t> run = {fReader, "run"};
   TTreeReaderValue<ULong64_t> event = {fReader, "event"};
   TTreeReaderValue<UInt_t> lumiblock = {fReader, "lumiblock"};
   TTreeReaderValue<UInt_t> nditrak = {fReader, "nditrak"};
   TTreeReaderValue<UInt_t> trigger = {fReader, "trigger"};
   TTreeReaderValue<Int_t> charge = {fReader, "charge"};
   TTreeReaderValue<Bool_t> isBest = {fReader, "isBest"};
   TTreeReaderArray<float> trigs_pt = {fReader, "trigs_pt"};
   TTreeReaderArray<float> trigs_eta = {fReader, "trigs_eta"};
   TTreeReaderArray<float> trigs_phi = {fReader, "trigs_phi"};
   TTreeReaderArray<float> trigs_m = {fReader, "trigs_m"};
   TTreeReaderArray<unsigned int> trigs_filters = {fReader, "trigs_filters"};
   TTreeReaderValue<TLorentzVector> ditrak_p4 = {fReader, "ditrak_p4"};
   TTreeReaderValue<TLorentzVector> trakP_p4 = {fReader, "trakP_p4"};
   TTreeReaderValue<TLorentzVector> trakN_p4 = {fReader, "trakN_p4"};
   TTreeReaderValue<Float_t> MassErr = {fReader, "MassErr"};
   TTreeReaderValue<Float_t> vProb = {fReader, "vProb"};
   TTreeReaderValue<Float_t> DCA = {fReader, "DCA"};
   TTreeReaderValue<Float_t> ctauPV = {fReader, "ctauPV"};
   TTreeReaderValue<Float_t> ctauErrPV = {fReader, "ctauErrPV"};
   TTreeReaderValue<Float_t> cosAlpha = {fReader, "cosAlpha"};
   TTreeReaderValue<Float_t> lxy = {fReader, "lxy"};
   TTreeReaderValue<Float_t> lxyErr = {fReader, "lxyErrPV"};
   TTreeReaderValue<UInt_t> numPrimaryVertices = {fReader, "numPrimaryVertices"};


   DiTrakSkim(TTree * /*tree*/ =0) { }
   virtual ~DiTrakSkim() { }
   virtual Int_t   Version() const { return 2; }
   virtual void    Begin(TTree *tree);
   virtual void    SlaveBegin(TTree *tree);
   virtual void    Init(TTree *tree);
   virtual Bool_t  Notify();
   virtual Bool_t  Process(Long64_t entry);
   virtual Int_t   GetEntry(Long64_t entry, Int_t getall = 0) { return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0; }
   virtual void    SetOption(const char *option) { fOption = option; }
   virtual void    SetObject(TObject *obj) { fObject = obj; }
   virtual void    SetInputList(TList *input) { fInput = input; }
   virtual TList  *GetOutputList() const { return fOutput; }
   virtual void    SlaveTerminate();
   virtual void    Terminate();

   bool MatchByDRDPt(const TLorentzVector t1, const TLorentzVector t2);
   float DeltaR(const TLorentzVector t1, const TLorentzVector t2);

   TProofOutputFile *OutFile;
   TFile            *fOut;

   float M_PI = 3.14159265359;
   float maxDPtRel = 2.0;
   float maxDeltaR  = 0.01;

   ClassDef(DiTrakSkim,0);

};

#endif

#ifdef DiTrakSkim_cxx
void DiTrakSkim::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the reader is initialized.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   fReader.SetTree(tree);
}

Bool_t DiTrakSkim::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}


#endif // #ifdef DiTrakSkim_cxx
