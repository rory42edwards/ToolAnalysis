#include "MyWCSimHitPlotter.h"

MyWCSimHitPlotter::MyWCSimHitPlotter():Tool(){}


bool MyWCSimHitPlotter::Initialise(std::string configfile, DataModel &data){

  /////////////////// Useful header ///////////////////////
  if(configfile!="") m_variables.Initialise(configfile); // loading config file
  //m_variables.Print();

  m_data= &data; //assigning transient data pointer
  /////////////////////////////////////////////////////////////////

  //TimeHist = new TH1D("timeplot","Frequency of measured hit time;Time / ns;no. hits",50,0,60);
  TimeTree = new TTree("timingtree", "timingtree");
  ChargeTree = new TTree("chargetree", "chargetree");
  auto TimingBranch = TimeTree->Branch("timing", &HitTime);
  auto ChargeBranch = ChargeTree->Branch("charge", &EventCharge);
  auto PhotonBranch = ChargeTree->Branch("photon_energy", &EventEnergyPhotons);
  auto ElectronBranch = ChargeTree->Branch("electron_energy", &EventEnergyElectrons);
  auto MuonBranch = ChargeTree->Branch("muon_energy", &EventEnergyMuons);
  auto LAPPDTimeBranch = ChargeTree->Branch("lappd_time", &LAPPDTime);
  auto LAPPDChargeBranch = ChargeTree->Branch("lappd_charge", &LAPPDCharge);


  return true;
}


bool MyWCSimHitPlotter::Execute(){
    int AnnieEventExists = m_data->Stores.count("ANNIEEvent");
    if (!AnnieEventExists) {std::cerr<<"No ANNIEEvent store!"<<std::endl; return false;};
    
    // Get the MC information
    m_data->Stores["ANNIEEvent"]->Get("MCParticles", MCParticles);
    m_data->Stores["ANNIEEvent"]->Get("MCHits", MCHits);
    m_data->Stores["ANNIEEvent"]->Get("MCTriggernum",MCTriggernum);
    m_data->Stores["ANNIEEvent"]->Get("MCFile",MCFile);
    m_data->Stores["ANNIEEvent"]->Get("MCEventNum",MCEventNum);

    // we want total charge and energy per event
    EventCharge = 0;
    EventEnergyPhotons = 0;
    EventEnergyElectrons = 0;
    EventEnergyMuons = 0;

    // loop through MCHits
    for (auto pair : *MCHits) {
        for (auto hit : pair.second) {
            HitTime = hit.GetTime(); // get timing info for each hit and fill tree
            TimeTree->Fill();

            double HitCharge = hit.GetCharge();
            EventCharge += HitCharge;
            //TimeHist->Fill(HitTime);
        }
    }

    std::cout<<"Length of MCParticles vector: "<<MCParticles->size()<<std::endl;
    // loop through the particles
    for (auto particle : *MCParticles){
        //particle.Print();
        double start_energy = particle.GetStartEnergy();
        double stop_energy = particle.GetStopEnergy();
        int pdg = particle.GetPdgCode();
        // check if particle is photon
        if (pdg == 22) {
            photons += 1;
            std::cout<<"gamma start energy: "<<start_energy<<std::endl;
            std::cout<<"gamma stop energy: "<<stop_energy<<std::endl;
            std::cout<<"particle pdg: "<<pdg<<std::endl;
            EventEnergyPhotons += start_energy;
        }
        // check if particle is electron
        if (pdg == 11) {
            electrons += 1;
            std::cout<<"electron start energy: "<<start_energy<<std::endl;
            std::cout<<"electron stop energy: "<<stop_energy<<std::endl;
            std::cout<<"particle pdg: "<<pdg<<std::endl;
            EventEnergyElectrons += start_energy;
        }
        // check if particle is muon
        if (pdg == 13){
            muons += 1;
            std::cout<<"muon start energy: "<<start_energy<<std::endl;
            std::cout<<"muon stop energy: "<<stop_energy<<std::endl;
            std::cout<<"particle pdg: "<<pdg<<std::endl;
            EventEnergyMuons += start_energy;
        }
    }

    std::cout<<"Total muon energy in this event: "<<EventEnergyMuons<<std::endl;

    ChargeTree->Fill();

    std::cout<<"Printing MCLAPPDHits information!"<<std::endl;
    std::cout<<"Printing MCLAPPDHits information!"<<std::endl;
    std::cout<<"Printing MCLAPPDHits information!"<<std::endl;
    // get MCLAPPDHits
    m_data->Stores.at("ANNIEEvent")->Get("MCLAPPDHits",MCLAPPDHits);
    for (auto& pair : *MCLAPPDHits) {
        std::cout<<"channelno: "<<pair.first<<std::endl;
        std::cout<<"number of MCLAPPDHits: "<<pair.second.size()<<std::endl;
        for (auto& hit : pair.second) {
            double hittime = hit.GetTime();
            LAPPDTime = hit.GetTime();
            LAPPDCharge = hit.GetCharge();
            ChargeTree->Fill();
        }
    }

  return true;
}


bool MyWCSimHitPlotter::Finalise(){
    outfile = new TFile("outfile.root","RECREATE");
    TimeTree->Write();
    ChargeTree->Write();
    outfile->Close();

    std::cout<<"Total number of photons: "<<photons<<std::endl;
    std::cout<<"Total number of electrons: "<<electrons<<std::endl;
    std::cout<<"Total number of muons: "<<muons<<std::endl;
    
    delete outfile;
    delete TimeTree;
    delete ChargeTree;

  return true;
}
