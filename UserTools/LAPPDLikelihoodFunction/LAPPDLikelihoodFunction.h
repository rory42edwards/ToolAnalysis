#ifndef LAPPDLikelihoodFunction_H
#define LAPPDLikelihoodFunction_H

#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include <cmath>
#include <algorithm>

#include "Tool.h"
#include "NoiseModels.h"
#include "Waveform.h"
#include "Position.h"
#include "Direction.h"
#include "TFile.h"
#include "TH1.h"
#include "TGraph.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TBranch.h"
#include "Detector.h"
#include "Geometry.h"
#include "TString.h"


/**
 * \class LAPPDLikelihoodFunction
 *
 * This is a blank template for a Tool used by the script to generate a new custom tool. Please fill out the description and author information.
*
* $Author: Rory Edwards $
* $Date: 2024/10/21 $
* Contact: rory.edwards@warwick.ac.uk
*/
class LAPPDLikelihoodFunction: public Tool {


 public:

  LAPPDLikelihoodFunction(); ///< Simple constructor
  bool Initialise(std::string configfile,DataModel &data); ///< Initialise Function for setting up Tool resources. @param configfile The path and name of the dynamic configuration file to read in. @param data A reference to the transient data class used to pass information between Tools.
  bool Execute(); ///< Execute function used to perform Tool purpose.
  bool Finalise(); ///< Finalise function used to clean up resources.

  /**
   * Calculates the log-likelihood on a single microstrip.
   * @param observed The observed (data) waveform of a microstrip.
   * @param model The signal model of the waveform.
   * @param NoiseModel The desired noise model (gaussian, etc).
   * @param weight The relative weight applied to this microstrip.
   */
  double CalculateMicrostripLogLikelihood(Waveform<double>& observed, Waveform<double>& model, NoiseModel& NoiseModel, double& weight); // a function to calculate the log likelihood for a single microstrip
                                                                                                                                        //
  /**
   * Calculates the log-likelihood on an LAPPD.
   * @param observed The observed (data) waveforms of an LAPPD.
   * @param model The signal model of the waveforms.
   * @param NoiseModel A smart pointer to the desired noise model (gaussian, etc).
   */
  double CalculateLAPPDLogLikelihood(std::vector<Waveform<double>>& observed, std::vector<Waveform<double>>& model, std::unique_ptr<NoiseModel>& NoiseModel);

  /**
   * Calculates the weights of the strips due to the relative amplitudes of the waveforms.
   * @param Signal The signal waveforms.
   * @return The amplitude weights in a vector.
   */
  std::vector<double> CalculateAmplitudeWeights(std::vector<Waveform<double>>& Signal);

  /**
   * A function to calculate the weights of the strips due to the relative timings of the waveforms.
   * @param Signal The signal waveforms.
   * @return The timing weights in a vector.
   */
  std::vector<double> CalculateTimingWeights(std::vector<Waveform<double>>& Signal);

  /**
   * Produces the signal model for an LAPPD from given muon parameters.
   * @param muonpos the muon position.
   * @param muondir the muon direction.
   * @return A vector of waveforms.
   */
  std::vector<Waveform<double>> GenerateSignalModel(Position& muonpos, Direction& muondir, Position& lappdpos); // just position for now
                                                                                                                //
  /**
   * Produces some dummy data waveforms from monte carlo hits.
   * @param MCLAPPDHitVector a vector of hits on an LAPPD.
   * @return A vector of waveforms.
   */
  std::vector<Waveform<double>> GenerateDummyData(std::vector<MCLAPPDHit>& MCLAPPDHitVector);

  /**
   * Creates a basic grid of positions to simulate muon positions.
   * @param xmin minimum x coordinate.
   * @param xmax maximum x coordinate.
   * @param ymin minimum y coordinate.
   * @param ymax maximum y coordinate.
   * @param zmin minimum z coordinate.
   * @param zmax maximum z coordinate.
   * @return A vector of muon positions in the range specified by the minima and maxima.
   */
  std::vector<Position> GeneratePositionGrid(double& xmin, double& xmax, double& ymin, double& ymax, double& zmin, double& zmax);

  /**
   * Generates a gaussian waveform for an LAPPD channel with given parameters.
   * @param num_samples how many samples should be in the waveform.
   * @param amplitude the amplitude of the resultant waveform.
   * @param mean the mean of the gaussian.
   * @param sigma the standard deviation of the gaussian.
   * @param time_start the start time of the waveform.
   * @param time_end the end time of the waveform.
   * @return A vector of length num_samples with entries calculated from a gaussian formula.
   */
  std::vector<double> GenerateGaussianWaveform(unsigned int& num_samples, double& amplitude, double& mean, double& sigma, double& time_start, double& time_end);

  /**
   * Returns the microstrip index of an LAPPD hit.
   * @param transverse_pos the local position on the LAPPD perpendicular to the microstrip direction.
   * @return An index from 0-27 representing the strip number.
   */
  int GetMicrostripIndex(double& transverse_pos);

  /**
   * Plots the waveform of a microstrip.
   * @param waveform the waveform to plot.
   * @param type either data, or model, ie the type of the waveform.
   * @param stripno the microstrip index.
   * @return A TH1D of the strip's waveform.
   */
  TH1D* PlotStripWaveform(Waveform<double>& waveform, std::string& type, size_t& stripno);

  /**
   * Draws the local positions of the MC hits on an LAPPD.
   * @param MCLAPPDHitVector a vector of the hits to be drawn.
   * @return The drawn local positions of the MC hits.
   */
  TGraph* DrawMCLAPPDHits(std::vector<MCLAPPDHit>& MCLAPPDHitVector);

  /**
   * Gets the global positions of the LAPPDs.
   * @param MCLAPPDHits the map of LAPPD ids to each of their hit vectors.
   * @param Geometry the detector geometry.
   * @return A map of LAPPD ids to their global positions.
   */
  std::map<unsigned long, Position> GetLAPPDPositions(std::map<unsigned long, std::vector<MCLAPPDHit>>& MCLAPPDHits, Geometry&);

  /**
   * Finds the LAPPD with the most hits.
   * @param MCLAPPDHits the map of LAPPD ids to each of their hit vectors.
   * @return The LAPPD id with the greatest number of hits, as well as its hits.
   */
  std::pair<unsigned long, std::vector<MCLAPPDHit>> GetLAPPDWithMostHits(std::map<unsigned long, std::vector<MCLAPPDHit>>& MCLAPPDHits);


 private:
  int evtno = 0;
  unsigned int noisemodel = 0;
  unsigned int verbosity = 0;
  const unsigned int v_error = 0;
  const unsigned int v_debug = 1;
  std::map<unsigned long,std::vector<MCLAPPDHit>>* MCLAPPDHits=nullptr;   ///< LAPPD hits
  bool isData = false;
  bool isMC = false;
  Geometry* fGeometry=nullptr;    ///< ANNIE Geometry
  std::vector<double> hitTimevec;
  std::vector<double> hitChargevec;

  TFile* outfile = nullptr;
  TTree* timingtree = nullptr;
  TTree* likelihoodtree = nullptr;
  std::vector<double> muonxvec, muonyvec, muonzvec, loglikelihoodvec;




};


#endif
