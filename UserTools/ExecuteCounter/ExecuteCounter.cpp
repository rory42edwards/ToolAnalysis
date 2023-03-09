#include "ExecuteCounter.h"

ExecuteCounter::ExecuteCounter():Tool(){}


bool ExecuteCounter::Initialise(std::string configfile, DataModel &data){

  /////////////////// Useful header ///////////////////////
  if(configfile!="") m_variables.Initialise(configfile); // loading config file
  //m_variables.Print();

  m_data= &data; //assigning transient data pointer
  /////////////////////////////////////////////////////////////////
  counter = 0;

  return true;
}


bool ExecuteCounter::Execute(){
  counter ++;

  return true;
}


bool ExecuteCounter::Finalise(){
  std::cout<<"ExecuteCounter: "<<counter<<" execute steps completed"<<std::endl;

  return true;
}
