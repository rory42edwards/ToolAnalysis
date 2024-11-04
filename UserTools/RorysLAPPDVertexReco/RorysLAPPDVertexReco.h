#ifndef RorysLAPPDVertexReco_H
#define RorysLAPPDVertexReco_H

#include <string>
#include <iostream>
#include <vector>

#include "Tool.h"
#include "ANNIEGeometry.h"
#include "Detector.h"
#include "TRandom3.h"
#include "Vec3.h"


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
  void ReadLAPPDIDFile();
  std::map<int, std::vector<MCLAPPDHit>> SortHitsToQuadrants(const std::vector<MCLAPPDHit>& LAPPDHits);
  std::vector<Vec3> CalculateQuadrantCentres(const Vec3& LAPPDCentre, const Vec3& LAPPDDirection);
  bool BuildMCLAPPDRecoDigits(std::vector<RecoDigit>* DigitList, std::map<unsigned long, std::vector<MCLAPPDHit>>& newLAPPDHits);


 private:
    //Shifts needed for simulation package in use (in cm)
    //Defaults to values needed for WCSim MC data
    double xshift = 0.0;
    double yshift = 14.46469;
    double zshift = -168.1;
    int number_of_pmt_digits = 0;
    int number_of_lappd_digits = 0;
    int number_of_all_digits = 0;
    int number_of_pmt_hits = 0;
    int number_of_lappd_hits = 0;
    std::vector<RecoDigit>* FakeDigitList = nullptr;
    bool AddFakeLAPPDDigits = true;
    int verbosity = 0;
    std::string fLAPPDIDFile="none";
    std::vector<int> fLAPPDId;
    std::map<unsigned long,int> detectorkey_to_lappdid;
    int v_error=0;
    int v_warning=1;
    int v_message=2;
    int v_debug=3;
    std::string logmessage;
    Geometry* fGeometry = nullptr;
    TRandom3 frand;
  //std::map<unsigned long,std::vector<MCHit>>* fMCPMTHits=nullptr;             ///< PMT hits
  //std::map<unsigned long,std::vector<MCLAPPDHit>>* fMCLAPPDHits=nullptr;   ///< LAPPD hits





};


#endif
