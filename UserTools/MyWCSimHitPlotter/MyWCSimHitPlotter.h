#ifndef MyWCSimHitPlotter_H
#define MyWCSimHitPlotter_H

#include <string>
#include <iostream>

#include "Tool.h"
#include "Hit.h"
#include "Particle.h"
#include "Waveform.h"
#include "LAPPDHit.h"
#include "TriggerClass.h"
#include "TimeClass.h"
#include "BeamStatus.h"
#include "BeamStatusClass.h"
#include "ADCPulse.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TTree.h"
#include "TBranch.h"
#include "TFile.h"
#include "Geometry.h"
#include "Detector.h"


/**
 * \class MyWCSimHitPlotter
 *
 * This is a blank template for a Tool used by the script to generate a new custom tool. Please fill out the description and author information.
*
* $Author: B.Richards $
* $Date: 2019/05/28 10:44:00 $
* Contact: b.richards@qmul.ac.uk
*/
class MyWCSimHitPlotter: public Tool {


 public:

  MyWCSimHitPlotter(); ///< Simple constructor
  bool Initialise(std::string configfile,DataModel &data); ///< Initialise Function for setting up Tool resources. @param configfile The path and name of the dynamic configuration file to read in. @param data A reference to the transient data class used to pass information between Tools.
  bool Execute(); ///< Execute function used to perform Tool purpose.
  bool Finalise(); ///< Finalise function used to clean up resources.


 private:
    std::map<unsigned long,std::vector<MCHit>>* MCHits=nullptr;
    std::vector<MCParticle>* MCParticles=nullptr;
    uint64_t MCEventNum;
    uint16_t MCTriggernum;
    std::string MCFile;
    TH1D* TimeHist = nullptr;
    TCanvas* c1 = nullptr;
    TTree* TimeTree = nullptr;
    TTree* ChargeTree = nullptr;
    double HitTime = 0;
    double EventCharge = 0;
    double EventEnergyPhotons = 0;
    double EventEnergyElectrons = 0;
    double EventEnergyMuons = 0;
    TFile* outfile = nullptr;
    double photons = 0;
    int electrons = 0;
    int muons = 0;
	std::map<unsigned long,std::vector<MCLAPPDHit>>* MCLAPPDHits;
    double LAPPDTime = 0;
    double LAPPDCharge = 0;


    //geometry variables
    Geometry *geom = nullptr;
    Detector det;
    double tank_height;
    double tank_radius;
    int max_num_lappds = 200;              //one is allowed to dream, right?
    int n_tank_pmts;
    int n_mrd_pmts;
    int n_veto_pmts;
    int n_lappds;
    double max_y;
    double min_y;
    double min_mrd_y, max_mrd_y, min_mrd_x, max_mrd_x, min_mrd_z, max_mrd_z;
    double mrd_diffz, mrd_diffy, mrd_diffx;
    std::vector<int> n_particles_ring;
    double detector_version;
    std::string detector_config;
    double tank_center_x;
    double tank_center_y;
    double tank_center_z;
    std::map<int, double> x_pmt, y_pmt, z_pmt;          //143 currently max configuration with additional 2 inch PMTs
    std::map<unsigned long, std::vector<double>> mrd_x, mrd_y, mrd_z;
    std::map<unsigned long, int> mrd_orientation, mrd_half, mrd_side;


};


#endif
