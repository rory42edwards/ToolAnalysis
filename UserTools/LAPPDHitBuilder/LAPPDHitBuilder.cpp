#include "LAPPDHitBuilder.h"

LAPPDHitBuilder::LAPPDHitBuilder():Tool(){}


bool LAPPDHitBuilder::Initialise(std::string configfile, DataModel &data){

  /////////////////// Useful header ///////////////////////
  if(configfile!="") m_variables.Initialise(configfile); // loading config file
  //m_variables.Print();

  m_data= &data; //assigning transient data pointer
  /////////////////////////////////////////////////////////////////

  m_variables.Get("LAPPDHitBuilderVerbosity",LAPPDHitBuilderVerbosity);

  LAPPDHitsPulses = new BoostStore(false,2);

  return true;
}


bool LAPPDHitBuilder::Execute(){
  std::map<unsigned long, std::vector<LAPPDPulse>> lappdpulses;
  std::map<unsigned long, std::vector<LAPPDHit>> lappdhits;

  m_data->Stores["ANNIEEvent"]->Get("CFDRecoLAPPDPulses",lappdpulses);
  //std::cout<<"lappdpulses size: "<<lappdpulses.size()<<std::endl;
  m_data->Stores["ANNIEEvent"]->Get("Clusters",lappdhits);
  //std::cout<<"lappdhits size: "<<lappdhits.size()<<std::endl;

  for (auto& channelpulses : lappdpulses){
    //std::cout<<"channelno: "<<channelpulses.first<<std::endl;
    std::vector<LAPPDPulse> pulses = channelpulses.second;
    for (auto& pulse : pulses){
      //pulse.Print();
    }
  }

  for (auto& channelhits : lappdhits){
    std::cout<<"channelno: "<<channelhits.first<<std::endl;
    std::vector<LAPPDHit> hits = channelhits.second;
    // need to associate hits with a position, need to set channel numbers here to overall ANNIE channel numbers which are associated with a position (mid point of a stripline??? centre of lappd + local position?)
    for (auto& hit : hits){
      std::vector<double> Position = hit.GetPosition();
      std::cout<<"Position size: "<<Position.size()<<std::endl;
      if(Position.size()>0) std::cout<<"Hit position (x,y,z): ("<<Position.at(0)<<","<<Position.at(1)<<","<<Position.at(2)<<")"<<std::endl;
      //hit.Print();
    }
  }

  this->SetAndSaveEvent();

  return true;
}


bool LAPPDHitBuilder::Finalise(){

  LAPPDHitsPulses->Close();
  LAPPDHitsPulses->Delete();
  delete LAPPDHitsPulses;

  return true;
}

void LAPPDHitBuilder::SetAndSaveEvent(){
  // Declare all event variables
  uint32_t RunNumber, SubrunNumber, EventNumber, TriggerWord, LocalEventNumber;
  uint64_t RunStartTime, EventTimeTank, EventTimeMRD, EventTimeLAPPD, CTCTimestamp, LAPPDOffset;
  int PartNumber, RunType, TriggerExtended;
  std::map<std::string, bool> DataStreams;
  std::string MRDTriggerType;
  std::map<std::string, int> MRDLoopbackTDC;
  TriggerClass TriggerData;
  BeamStatus BeamStatus;
  std::map<unsigned long, std::vector<Hit>>* TDCData;
  std::map<unsigned long, std::vector<Hit>> *Hits, *AuxHits;
  std::map<unsigned long, std::vector<int>> RawAcqSize;
  std::map<unsigned long, std::vector<std::vector<ADCPulse>>> RecoADCData, RecoAuxADCData;

  std::map<unsigned long, std::vector<Hit>> *NewTDCData = new std::map<unsigned long, std::vector<Hit>>;
  std::map<unsigned long, std::vector<Hit>> *NewHits = new  std::map<unsigned long, std::vector<Hit>>;
  std::map<unsigned long, std::vector<Hit>> *NewAuxHits = new std::map<unsigned long, std::vector<Hit>>;

  std::map<unsigned long, std::vector<LAPPDPulse>> lappdpulses;
  std::map<unsigned long, std::vector<LAPPDHit>> lappdhits;

  // Get and Set all variables in the event to the new booststore
  m_data->Stores["ANNIEEvent"]->Get("AuxHits",AuxHits);
  for (auto&& entry : (*AuxHits)){
      NewAuxHits->emplace(entry.first,entry.second);
      }
  LAPPDHitsPulses->Set("AuxHits",NewAuxHits,true);

  m_data->Stores["ANNIEEvent"]->Get("BeamStatus",BeamStatus);  LAPPDHitsPulses->Set("BeamStatus",BeamStatus);
  m_data->Stores["ANNIEEvent"]->Get("CTCTimestamp",CTCTimestamp);  LAPPDHitsPulses->Set("CTCTimestamp",CTCTimestamp);
  m_data->Stores["ANNIEEvent"]->Get("DataStreams",DataStreams);  LAPPDHitsPulses->Set("DataStreams",DataStreams);
  m_data->Stores["ANNIEEvent"]->Get("EventNumber",EventNumber);  LAPPDHitsPulses->Set("EventNumber",EventNumber);
  m_data->Stores["ANNIEEvent"]->Get("EventTimeLAPPD",EventTimeLAPPD);  LAPPDHitsPulses->Set("EventTimeLAPPD",EventTimeLAPPD);
  m_data->Stores["ANNIEEvent"]->Get("EventTimeMRD",EventTimeMRD);  LAPPDHitsPulses->Set("EventTimeMRD",EventTimeMRD);
  m_data->Stores["ANNIEEvent"]->Get("EventTimeTank",EventTimeTank);  LAPPDHitsPulses->Set("EventTimeTank",EventTimeTank);
  m_data->Stores["ANNIEEvent"]->Get("Hits",Hits);
  for (auto&& entry : (*Hits)){
      NewHits->emplace(entry.first,entry.second);
      }
  LAPPDHitsPulses->Set("Hits",NewHits,true);
  
  m_data->Stores["ANNIEEvent"]->Get("LAPPDOffset",LAPPDOffset);  LAPPDHitsPulses->Set("LAPPDOffset",LAPPDOffset);
  m_data->Stores["ANNIEEvent"]->Get("LocalEventNumber",LocalEventNumber);  LAPPDHitsPulses->Set("LocalEventNumber",LocalEventNumber);
  m_data->Stores["ANNIEEvent"]->Get("MRDLoopbackTDC",MRDLoopbackTDC);  LAPPDHitsPulses->Set("MRDLoopbackTDC",MRDLoopbackTDC);
  m_data->Stores["ANNIEEvent"]->Get("MRDTriggerType",MRDTriggerType);  LAPPDHitsPulses->Set("MRDTriggerType",MRDTriggerType);
  m_data->Stores["ANNIEEvent"]->Get("PartNumber",PartNumber);  LAPPDHitsPulses->Set("PartNumber",PartNumber);
  m_data->Stores["ANNIEEvent"]->Get("RawAcqSize",RawAcqSize);  LAPPDHitsPulses->Set("RawAcqSize",RawAcqSize);
  m_data->Stores["ANNIEEvent"]->Get("RecoADCData",RecoADCData);  LAPPDHitsPulses->Set("RecoADCData",RecoADCData);
  m_data->Stores["ANNIEEvent"]->Get("RecoAuxADCData",RecoAuxADCData);  LAPPDHitsPulses->Set("RecoAuxADCData",RecoAuxADCData);
  m_data->Stores["ANNIEEvent"]->Get("RunNumber",RunNumber);  LAPPDHitsPulses->Set("RunNumber",RunNumber);
  m_data->Stores["ANNIEEvent"]->Get("RunStartTime",RunStartTime);  LAPPDHitsPulses->Set("RunStartTime",RunStartTime);
  m_data->Stores["ANNIEEvent"]->Get("RunType",RunType);    LAPPDHitsPulses->Set("RunType",RunType);
  m_data->Stores["ANNIEEvent"]->Get("SubrunNumber",SubrunNumber);  LAPPDHitsPulses->Set("SubrunNumber",SubrunNumber);
  m_data->Stores["ANNIEEvent"]->Get("TDCData",TDCData);
  for (auto&& entry : (*TDCData)){
      NewTDCData->emplace(entry.first,entry.second);
      }
  
  LAPPDHitsPulses->Set("TDCData",NewTDCData,true);
  m_data->Stores["ANNIEEvent"]->Get("TriggerData",TriggerData);  LAPPDHitsPulses->Set("TriggerData",TriggerData);
  m_data->Stores["ANNIEEvent"]->Get("TriggerExtended",TriggerExtended);  LAPPDHitsPulses->Set("TriggerExtended",TriggerExtended);
  m_data->Stores["ANNIEEvent"]->Get("TriggerWord",TriggerWord);  LAPPDHitsPulses->Set("TriggerWord",TriggerWord);
  m_data->Stores["ANNIEEvent"]->Get("CFDRecoLAPPDPulses",lappdpulses); LAPPDHitsPulses->Set("CFDRecoLAPPDPulses",lappdpulses);
  m_data->Stores["ANNIEEvent"]->Get("Clusters",lappdhits); LAPPDHitsPulses->Set("LAPPDHits",lappdhits);

  std::string Filename = "LAPPDHitsPulses";

  if (LAPPDHitBuilderVerbosity>0) std::cout<<"Filename is "<<Filename<<std::endl;
  LAPPDHitsPulses->Save(Filename);
  LAPPDHitsPulses->Delete();
  return;
}