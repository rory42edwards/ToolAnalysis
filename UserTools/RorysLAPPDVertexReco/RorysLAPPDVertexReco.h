#ifndef RorysLAPPDVertexReco_H
#define RorysLAPPDVertexReco_H

#include <string>
#include <iostream>

#include "Tool.h"


/**
 * \class RorysLAPPDVertexReco
 *
 * This is a blank template for a Tool used by the script to generate a new custom tool. Please fill out the description and author information.
*
* $Author: B.Richards $
* $Date: 2019/05/28 10:44:00 $
* Contact: b.richards@qmul.ac.uk
*/
class RorysLAPPDVertexReco: public Tool {


 public:

  RorysLAPPDVertexReco(); ///< Simple constructor
  bool Initialise(std::string configfile,DataModel &data); ///< Initialise Function for setting up Tool resources. @param configfile The path and name of the dynamic configuration file to read in. @param data A reference to the transient data class used to pass information between Tools.
  bool Execute(); ///< Execute function used to perform Tool purpose.
  bool Finalise(); ///< Finalise function used to clean up resources.
  std::vector<RecoDigit> ExtractLAPPDDigits(std::vector<RecoDigit>* DigitList);


 private:
    int number_of_pmt_digits = 0;
    int number_of_lappd_digits = 0;
    int number_of_all_digits = 0;
    int number_of_pmt_hits = 0;
    int number_of_lappd_hits = 0;
    std::vector<RecoDigit>* FakeDigitList;
    bool AddFakeLAPPDDigits = true;
    int verbosity = 0;

  //std::map<unsigned long,std::vector<MCHit>>* fMCPMTHits=nullptr;             ///< PMT hits
  //std::map<unsigned long,std::vector<MCLAPPDHit>>* fMCLAPPDHits=nullptr;   ///< LAPPD hits





};


#endif
