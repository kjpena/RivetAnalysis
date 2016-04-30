// -*- C++ -*-
#include "Rivet/Analysis.hh"
#include "Rivet/Tools/Logging.hh"
#include "Rivet/Projections/FinalState.hh"
#include "Rivet/Projections/FastJets.hh"
#include "Rivet/Projections/ChargedFinalState.hh"
#include "Rivet/Tools/ParticleIdUtils.hh"
#include "Rivet/Projections/ChargedLeptons.hh"
#include "Rivet/Projections/IdentifiedFinalState.hh"
#include "Rivet/Projections/DressedLeptons.hh"
#include "Rivet/Projections/MissingMomentum.hh"
#include "Rivet/Tools/ParticleIdUtils.hh"
#include "Rivet/Projections/VetoedFinalState.hh"

namespace Rivet {
  
  
  class CMS_TOP_Radius : public Analysis {
    
  private:

    std::map<string,Histo1DPtr> histos;
    

  public:

    /// CTOR
    CMS_TOP_Radius()  : 
      Analysis("CMS_TOP_Radius")
    { 
      //setBeams(PROTON, PROTON);
    }


    /// Book histograms and initialise projections before the run
    void init() 
    {
      // define the acceptance of the final state
      const FinalState fs(-2.5,2.5,0.0*GeV);
      addProjection(fs, "FS");

      //photons
      IdentifiedFinalState gammafs(fs);
      gammafs.acceptIdPair(PID::PHOTON);
      
      //charged leptons
      IdentifiedFinalState lep_id(-2.5,2.5,20*GeV);
      lep_id.acceptIdPair(PID::MUON);
      lep_id.acceptIdPair(PID::ELECTRON);
      DressedLeptons ewdressedleptons(gammafs, lep_id, 0.1, true, -2.5, 2.5, 20*GeV, true);
      addProjection(ewdressedleptons, "Leptons");

      // neutrinos
      IdentifiedFinalState nu_id;
      nu_id.acceptNeutrinos();
      addProjection(nu_id, "neutrinos");

      // remove neutrinos, electrons and muons and get the jets
      VetoedFinalState vfs(fs);
      vfs.addVetoOnThisFinalState(ewdressedleptons);
      vfs.addVetoOnThisFinalState(nu_id);
      FastJets jets(vfs, FastJets::ANTIKT, 0.5);
      addProjection(jets, "Jets");

      //for pTmiss use visible final state particles with |eta|<5 
      addProjection(VisibleFinalState(-5,5),"vfs");

      // get the charged particles
      ChargedFinalState chfs(fs);             
      addProjection(chfs, "chfs"); 

      //book histograms
      histos["l_ptlead"]  = bookHisto1D("l_ptlead", 50,0,150);
      histos["l_pttrail"] = bookHisto1D("l_pttrail",50,0,150);      
      histos["l_etacen"]  = bookHisto1D("l_etacen", 25,0,2.5);
      histos["l_etafwd"]  = bookHisto1D("l_etafwd", 25,0,2.5);      
      histos["mll"]       = bookHisto1D("mll", 25,0,200);      
      histos["dphill"]    = bookHisto1D("dphill", 25,0,3.16);      
      histos["lptsum"]    = bookHisto1D("lptsum", 25,0,200);      

      histos["b_ptlead"]  = bookHisto1D("b_ptlead", 50,0,150);
      histos["b_pttrail"] = bookHisto1D("b_pttrail",50,0,150);      
      histos["b_etacen"]  = bookHisto1D("b_etacen", 25,0,2.5);
      histos["b_etafwd"]  = bookHisto1D("b_etafwd", 25,0,2.5);   
      histos["bptsum"]    = bookHisto1D("bptsum", 25,0,200);      
      histos["dphibb"]    = bookHisto1D("dphibb", 25,0,3.16);      
      histos["nb"]        = bookHisto1D("nb", 4,0,4);        

      histos["vispt"]     = bookHisto1D("vispt", 25,0,200);         
      histos["vismass"]   = bookHisto1D("vismass", 25,0,200);         
      histos["visht"]     = bookHisto1D("visht", 25,0,200);       

      histos["nj"]         = bookHisto1D("nj", 4,0,4);        
      histos["j_ptlead"]   = bookHisto1D("j_ptlead", 50,0,150);
      histos["j_etalead"]  = bookHisto1D("j_etalead", 25,0,2.5);
    }
    
    
    ///  the per-event analysis goes here
    void analyze(const Event& event) 
    {

      const double weight = event.weight();

      //require 2 leptons
      const std::vector<DressedLepton> &leptons     = applyProjection<DressedLeptons>(event, "Leptons").clusteredLeptons();
      if ( leptons.size() < 2) vetoEvent; 

      //require ee/mm/em
      int ch=leptons[0].abspid() * leptons[0].abspid();
      if( ch != PID::ELECTRON * PID::MUON && ch != PID::ELECTRON*PID::ELECTRON && ch!=PID::MUON*PID::MUON ) vetoEvent;

      //require opposite sign 
      if( leptons[0].charge() * leptons[0].charge() >=0 ) vetoEvent; 
      
      // jet multiplicity 
      const FastJets& jetpro = applyProjection<FastJets>(event, "Jets");
      const Jets alljets = jetpro.jetsByPt(30*GeV,MAXDOUBLE,-4.7,4.7);

      // b-tagging
      Jets bjets,otherjets;
      foreach (const Jet& jet, alljets) 
	{
	  if(fabs(jet.eta())<2.5 && jet.containsBottom()) bjets.push_back(jet);
	  else otherjets.push_back(jet);
	}
      if(bjets.size()<2) vetoEvent;

      //fill histos
      const FourMomentum llP4=leptons[0].momentum()+leptons[1].momentum();
      float lptsum=leptons[0].pt()+leptons[1].pt();
      histos["l_ptlead"]->fill( max(leptons[0].pt(),leptons[1].pt()), weight );
      histos["l_pttrail"]->fill( min(leptons[0].pt(),leptons[1].pt()), weight );
      histos["l_etacen"]->fill( max(fabs(leptons[0].eta()),fabs(leptons[1].eta())), weight );
      histos["l_etafwd"]->fill( min(fabs(leptons[0].eta()),fabs(leptons[1].eta())), weight );
      histos["mll"]->fill(llP4.mass(),weight);
      histos["dphill"]->fill( deltaPhi(leptons[0],leptons[1]),weight);
      histos["lptsum"]->fill(lptsum,weight);

      const FourMomentum bbP4=bjets[0].momentum()+bjets[1].momentum();
      float bptsum=bjets[0].pt()+bjets[1].pt();
      histos["b_ptlead"]->fill( max(bjets[0].pt(),bjets[1].pt()), weight );
      histos["b_pttrail"]->fill( min(bjets[0].pt(),bjets[1].pt()), weight );
      histos["b_etacen"]->fill( max(fabs(bjets[0].eta()),fabs(bjets[1].eta())), weight );
      histos["b_etafwd"]->fill( min(fabs(bjets[0].eta()),fabs(bjets[1].eta())), weight );
      histos["mbb"]->fill(bbP4.mass(),weight);
      histos["dphibb"]->fill( deltaPhi(bjets[0],bjets[1]),weight);
      histos["bptsum"]->fill(bptsum,weight);

      const FourMomentum visP4=llP4+bbP4;
      float visHt=lptsum+bptsum;
      histos["vispt"]->fill(visP4.pt(),weight);
      histos["vismass"]->fill(visP4.mass(),weight);
      histos["visht"]->fill(visHt,weight);
      
      histos["nb"]->fill(bjets.size(),weight);
      histos["nj"]->fill(otherjets.size(),weight);
      if(otherjets.size())
	{
	  histos["j_ptlead"]->fill(otherjets[0].pt(),weight);
	  histos["j_etalead"]->fill(fabs(otherjets[0].eta()),weight);
	}
    }
    
    /// Normalise histograms etc., after the run
    void finalize() 
    {
    }
  };


  // The hook for the plugin system
  DECLARE_RIVET_PLUGIN(CMS_TOP_Radius);

}
