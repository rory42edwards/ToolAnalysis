#include "LAPPDLikelihoodFunction.h"
#include <cmath>
#include <stdexcept>
#include <utility>

LAPPDLikelihoodFunction::LAPPDLikelihoodFunction():Tool(){}


bool LAPPDLikelihoodFunction::Initialise(std::string configfile, DataModel &data){

  /////////////////// Useful header ///////////////////////
  if(configfile!="") m_variables.Initialise(configfile); // loading config file
  //m_variables.Print();

  m_data= &data; //assigning transient data pointer
  /////////////////////////////////////////////////////////////////

  m_variables.Get("NoiseModel", noisemodel);
  m_variables.Get("verbosity", verbosity);
  m_variables.Get("isData", isData);
  m_variables.Get("isMC", isMC);
  auto get_geometry= m_data->Stores.at("ANNIEEvent")->Header->Get("AnnieGeometry",fGeometry);

  outfile = new TFile("waveforms.root","RECREATE");
  outfile->mkdir("data");
  outfile->mkdir("model");
  outfile->mkdir("MCHits");

  likelihoodtree = new TTree("LAPPDLikelihoodTree", "LAPPD Likelihood Tree");
  /*likelihoodtree->Branch("muonx",&muonx,"muonx/D");
  likelihoodtree->Branch("muony",&muony,"muony/D");
  likelihoodtree->Branch("muonz",&muonz,"muonz/D");
  likelihoodtree->Branch("loglikelihood",&loglikelihood,"loglikelihood/D");*/
  likelihoodtree->Branch("evtno",&evtno,"evtno/I");
  likelihoodtree->Branch("muonx",&muonxvec);
  likelihoodtree->Branch("muony",&muonyvec);
  likelihoodtree->Branch("muonz",&muonzvec);
  likelihoodtree->Branch("loglikelihood",&loglikelihoodvec);
  likelihoodtree->Branch("hitTime",&hitTimevec);
  likelihoodtree->Branch("hitCharge",&hitChargevec);
  //timingtree = new TTree("HitTimingTree", "Hit Timing Tree");
  //timingtree->Branch("hitTime",&hitTime,"hitTime/D");
  //timingtree->Branch("hitCharge",&hitCharge,"hitCharge/D");



  return true;
}


bool LAPPDLikelihoodFunction::Execute(){
    evtno++;
    muonxvec.clear();
    muonyvec.clear();
    muonzvec.clear();
    loglikelihoodvec.clear();
    hitTimevec.clear();
    hitChargevec.clear();
    if (evtno > 2) return true;
    std::vector<Waveform<double>> LAPPDData;
    if (isData) {
        m_data->Stores.at("ANNIEEvent")->Get("LAPPDData", LAPPDData);
    }
    else if (isMC) {
        bool got_mc = m_data->Stores.at("ANNIEEvent")->Get("MCLAPPDHits",MCLAPPDHits);
        if (!got_mc) return true;
    }
    else {
        Log("LAPPDLikelihoodFunction Tool: ERROR deciding if this is MC or data! Please set.",v_error,verbosity);
        return false;
    }

    std::unique_ptr<NoiseModel> noise;
    switch (noisemodel){
        case 0:
            noise = std::make_unique<GaussianNoise>(0.4);
            break;
        default:
            throw std::invalid_argument("Unknown noise model!");
    }

    // we can calculate the log likelihood for an LAPPD over a grid of muon positions
    // right now, pick an event where an lappd has many hits
    int nlappdhits = 0;
    unsigned long lappdid = 0;
    std::pair<unsigned long, std::vector<MCLAPPDHit>> pair = GetLAPPDWithMostHits(*MCLAPPDHits);
    lappdid = pair.first;
    std::vector<MCLAPPDHit> hitvec = pair.second; 
    if (verbosity>v_debug) std::cout<<"LAPPD ID: "<<lappdid<<" has "<<nlappdhits<<" hits"<<std::endl;
    //if (nlappdhits < 10) return true;

    // plot the true MCLAPPDHits
    outfile->cd("MCHits");
    TString mchitsubdirname = "evtno"+std::to_string(evtno);
    outfile->mkdir("MCHits/"+mchitsubdirname);
    outfile->cd("MCHits/"+mchitsubdirname);
    TGraph* mchitgraph = DrawMCLAPPDHits(hitvec);
    TCanvas* hitcanvas = new TCanvas("canvas", "canvas", 800, 600);
    mchitgraph->Draw("AP");
    mchitgraph->SetMarkerStyle(20);
    hitcanvas->Update();
    hitcanvas->Write();
    delete hitcanvas;
    delete mchitgraph;

    double xmin=-1; double xmax=1; double ymin=-1; double ymax=1; double zmin=-1; double zmax=1; 
    std::vector<Position> test_positions = GeneratePositionGrid(xmin, xmax, ymin, ymax, zmin, zmax);
    std::vector<Waveform<double>> dummydata = GenerateDummyData(hitvec);
    std::vector<Waveform<double>> dummymodel;
    size_t nmodels = 1;
    std::map<unsigned long, Position> LAPPDPositions = GetLAPPDPositions(*MCLAPPDHits, *fGeometry);
    Position lappdpos = LAPPDPositions.at(lappdid);
    size_t counter = 0;
    for (auto& position : test_positions) {
        double muonx = position.X(); double muony = position.Y(); double muonz = position.Z();
        Direction direction;
        std::vector<Waveform<double>> SignalModel = GenerateSignalModel(position, direction, lappdpos);
        double loglikelihood = CalculateLAPPDLogLikelihood(dummydata, SignalModel, noise);
        std::cout<<"Log likelihood for this position: "<<loglikelihood<<std::endl;
        //likelihoodtree->Fill();
        muonxvec.push_back(muonx);
        muonyvec.push_back(muony);
        muonzvec.push_back(muonz);
        loglikelihoodvec.push_back(loglikelihood);
        if (counter < nmodels) dummymodel = SignalModel;
        counter++;
    }
    
    // plot both the data waveforms and 1-2 examples of model waveforms
    outfile->cd("data");
    TString datasubdirname = "data_evtno_"+std::to_string(evtno);
    outfile->mkdir("data/"+datasubdirname);
    outfile->cd("data/"+datasubdirname);
    for (size_t stripno=0; stripno<dummydata.size(); stripno++) {
        std::string type = "data";
        TH1D* wavhist = PlotStripWaveform(dummydata.at(stripno), type, stripno);
        wavhist->Write();
        delete wavhist;
    }
    outfile->cd("model");
    TString modelsubdirname = "model_evtno_"+std::to_string(evtno);
    outfile->mkdir("model/"+modelsubdirname);
    outfile->cd("model/"+modelsubdirname);
    for (size_t stripno=0; stripno<dummymodel.size(); stripno++) {
        std::string type = "model";
        TH1D* wavhist = PlotStripWaveform(dummymodel.at(stripno), type, stripno);
        wavhist->Write();
        delete wavhist;
    }

  likelihoodtree->Fill();
  return true;
}


bool LAPPDLikelihoodFunction::Finalise(){
    outfile->cd();
    outfile->Write();
    outfile->Close();
    delete outfile;

  return true;
}


double LAPPDLikelihoodFunction::CalculateMicrostripLogLikelihood(Waveform<double>& observed, Waveform<double>& model, NoiseModel& NoiseModel, double& weight) {
    double loglikelihood = 0.0;

    for (size_t i=0; i<observed.GetSamples()->size(); i++) {
        double residual = observed.GetSamples()->at(i) - model.GetSamples()->at(i);
        //std::cout<<"residual: "<<residual<<std::endl;
        loglikelihood += weight*NoiseModel.LogLikelihood(residual);
    }

    //std::cout<<"Log likelihood for this strip: "<<loglikelihood<<std::endl;
    loglikelihood /= observed.GetSamples()->size(); // get average log-likelihood per sample
    return loglikelihood;
}


double LAPPDLikelihoodFunction::CalculateLAPPDLogLikelihood(std::vector<Waveform<double>>& LAPPDData, std::vector<Waveform<double>>& model, std::unique_ptr<NoiseModel>& NoiseModel) {
    double total_loglikelihood = 0.0;
    double alpha = 0.5; // hard coded for now, this is the relative importance of timing and amplitude weights

    std::vector<double> amplitude_weights = CalculateAmplitudeWeights(LAPPDData);
    std::vector<double> timing_weights = CalculateTimingWeights(LAPPDData);
    for (size_t i=0; i<LAPPDData.size(); i++) {
        Waveform<double>& MicrostripData = LAPPDData.at(i);
        Waveform<double>& SignalModel = model.at(i);
        double amplitude_weight = amplitude_weights.at(i);
        double timing_weight = timing_weights.at(i);
        double weight = alpha*amplitude_weight + (1-alpha)*timing_weight;
        weight = 1;
        total_loglikelihood += CalculateMicrostripLogLikelihood(MicrostripData, SignalModel, *NoiseModel, weight);
    }
    return total_loglikelihood;
}

std::vector<double> LAPPDLikelihoodFunction::CalculateAmplitudeWeights(std::vector<Waveform<double>>& Signal){
    double max_amp_sum = 0.0;
    std::vector<double> amplitude_weights;

    for (auto& StripData : Signal) {
        double waveform_max_amp = *std::max_element(StripData.GetSamples()->begin(), StripData.GetSamples()->end());
        max_amp_sum += waveform_max_amp;
        amplitude_weights.push_back(waveform_max_amp);
    }

    // normalise the values to the sum of maxima across all strips
    for (auto& value : amplitude_weights) {
        value = value / max_amp_sum;
    }
    return amplitude_weights;
}

std::vector<double> LAPPDLikelihoodFunction::CalculateTimingWeights(std::vector<Waveform<double>>& Signal){
    // currently unimplemented
    double earliest_time = 0.0;
    std::vector<double> timing_weights;

    for (auto& StripData : Signal) {
        double strip_earliest_time = 0.0;
        timing_weights.push_back(strip_earliest_time);
    }
    
    for (auto& value : timing_weights) {
        value = 1 / (value - earliest_time);
    }

    return timing_weights;
}

std::vector<Waveform<double>> LAPPDLikelihoodFunction::GenerateSignalModel(Position&muonpos, Direction& muondir, Position& lappdpos) {
    double muontheta = muondir.GetTheta(); double muonphi = muondir.GetPhi();
    double muonx = muonpos.X(); double muony= muonpos.Y(); double muonz = muonpos.Z();
    double lappdx = lappdpos.X(); double lappdy = lappdpos.Y(); double lappdz = lappdpos.Z();
    std::vector<Waveform<double>> signalmodel;
    // generate a gaussian waveform for each strip
    size_t nstrips = 28;
    unsigned int num_samples = 256;
    double time_start = 0;
    double time_end = 25.6; // nsec
    double sigma = 3.0; // pulse width
    for (size_t stripno=0; stripno<nstrips; stripno++) {
        double amplitude = 1/(1 + std::exp(lappdz - muonz)); // roughly so that a muon closer to the lappd in the z direction gives signals with a higher amplitude
        double time = time_end / 2.0;
        std::vector<double> wav = GenerateGaussianWaveform(num_samples, amplitude, time, sigma, time_start, time_end);
        Waveform<double> my_wav;
        my_wav.SetSamples(wav);
        signalmodel.push_back(my_wav);
    }
    return signalmodel;
}

std::vector<Waveform<double>> LAPPDLikelihoodFunction::GenerateDummyData(std::vector<MCLAPPDHit>& MCLAPPDHitVector) {
    std::vector<Waveform<double>> dummydata;
    std::vector<std::vector<MCLAPPDHit>> dummyhits(28);

    if (verbosity > v_debug) std::cout<<"MCLAPPDHitVector size: "<<MCLAPPDHitVector.size()<<std::endl;
    // loop through hits
    for (auto& hit : MCLAPPDHitVector) {
        std::vector<double> local_position = hit.GetLocalPosition();
        double transverse_pos = local_position.at(1);
        double time = hit.GetTime();
        hitTimevec.push_back(time);
        double charge = hit.GetCharge();
        hitChargevec.push_back(charge);

        int microstrip_index = GetMicrostripIndex(transverse_pos);
        dummyhits.at(microstrip_index).push_back(hit);
    }
    // loop through lappds
    /*
    size_t counter = 1;
    for (auto&& lappd : MCLAPPDHits) {
        if (counter > 1) break;
        std::vector<MCLAPPDHit> lappdhits = lappd.second;
        // loop through hits
        for (auto& hit : lappdhits) {
            std::vector<double> local_position = hit.GetLocalPosition();
            double transverse_pos = local_position.at(1);
            double time = hit.GetTime();
            double charge = hit.GetCharge();

            int microstrip_index = GetMicrostripIndex(transverse_pos);
            dummyhits.at(microstrip_index).push_back(hit);
        }
        counter++;
    }
    */

    // generate a gaussian waveform for each strip
    unsigned int num_samples = 256;
    double time_start = 0;
    double time_end = 25.6; // nsec
    double sigma = 3.0; // pulse width
    for (auto& hitvec : dummyhits) {
        double amplitude = 0.0;
        double total_time = 0.0;
        std::vector<double> hittimes;
        for (auto& hit : hitvec) {
            amplitude += hit.GetCharge();
            total_time += hit.GetTime();
            hittimes.push_back(hit.GetTime());
        }
        std::vector<double> wav;
        if (hitvec.empty()) {
            wav = std::vector<double>(num_samples, 0.0); // empty waveform if no hits
        }
        else {
            double mean = (total_time / hitvec.size());// - *std::min_element(hittimes.begin(), hittimes.end());
            wav = GenerateGaussianWaveform(num_samples, amplitude, mean, sigma, time_start, time_end);
        }

        Waveform<double> my_wav;
        my_wav.SetSamples(wav);
        dummydata.push_back(my_wav);
    }

    return dummydata;
}

std::vector<Position> LAPPDLikelihoodFunction::GeneratePositionGrid(double& xmin, double& xmax, double& ymin, double& ymax, double& zmin, double& zmax) {
    std::vector<Position> position_grid;
    double step = 0.1;
    double step_x = step; double step_y = step; double step_z = step;
    for (double x=xmin; x<xmax; x+=step_x) {
        for (double y=ymin; y<ymax; y+=step_y) {
            for (double z=zmin; z<zmax; z+=step_z) {
                Position pos(x,y,z);
                position_grid.push_back(pos);
            }
        }
    }
    return position_grid;
}

std::vector<double> LAPPDLikelihoodFunction::GenerateGaussianWaveform(unsigned int& num_samples, double& amplitude, double& mean, double& sigma, double& time_start, double& time_end) {
    std::vector<double> wav(num_samples);
    double time_step = (time_end-time_start) / (num_samples-1);
    // loop over samples
    for (size_t i=0; i<num_samples; i++){
        double t = time_start + i*time_step; // time at each sample
        wav.at(i) = amplitude * std::exp(-std::pow(t-mean, 2) / (2*std::pow(sigma, 2)));
    }
    return wav;
}

int LAPPDLikelihoodFunction::GetMicrostripIndex(double& transverse_pos) {
    int index = -1;
    double MaxTransverse = 0.2;
    double strip_width = MaxTransverse/28.0;

    index = static_cast<int>((transverse_pos+0.1)/strip_width);
    if (index > 27) index = 27;
    if (index < 0) index = 0;
    return index;
}

TH1D* LAPPDLikelihoodFunction::PlotStripWaveform(Waveform<double>& waveform, std::string& type, size_t& stripno) {
    TString hname = type + "_wav_" + std::to_string(stripno);

    int nbins = waveform.GetSamples()->size();
    double starttime=0;
    double endtime=starttime + (double)nbins;

    TH1D* hwav = new TH1D(hname,hname,nbins,starttime,endtime);
    for (size_t i=0; i<nbins; i++){
        hwav->SetBinContent(i+1,waveform.GetSamples()->at(i));
    }

    return hwav;
}

std::map<unsigned long, Position> LAPPDLikelihoodFunction::GetLAPPDPositions(std::map<unsigned long, std::vector<MCLAPPDHit>>& MCLAPPDHits, Geometry& fGeometry) {
    std::map<unsigned long, Position> LAPPDPositions;
    for (auto&& lappd : MCLAPPDHits) {
        unsigned long chankey = lappd.first;
        Detector* det = fGeometry.ChannelToDetector(chankey);
        Position lappdpos = det->GetDetectorPosition();
        LAPPDPositions[chankey] = lappdpos;
    }
    return LAPPDPositions;
}

TGraph* LAPPDLikelihoodFunction::DrawMCLAPPDHits(std::vector<MCLAPPDHit>& MCLAPPDHitVector) {
    std::vector<double> transverse_positions;
    std::vector<double> parallel_positions;
    for (auto& hit : MCLAPPDHitVector) {
        std::vector<double> localpos = hit.GetLocalPosition();
        double transverse_pos = localpos.at(1);
        double parallel_pos = localpos.at(0);
        transverse_positions.push_back(transverse_pos);
        parallel_positions.push_back(parallel_pos);
    }
    TGraph* lappdhitgraph = new TGraph(MCLAPPDHitVector.size(), &transverse_positions[0], &parallel_positions[0]);
    lappdhitgraph->SetTitle("True LAPPD Positions of MC Hits");
    lappdhitgraph->GetXaxis()->SetTitle("Transverse hit positions / m");
    lappdhitgraph->GetYaxis()->SetTitle("Parallel hit positions / m");
    return lappdhitgraph;
}

std::pair<unsigned long, std::vector<MCLAPPDHit>> LAPPDLikelihoodFunction::GetLAPPDWithMostHits(std::map<unsigned long, std::vector<MCLAPPDHit>>& MCLAPPDHits) {
    if (MCLAPPDHits.empty()) return {};

    // find the vector with the most hits
    auto maxElementIt = std::max_element(MCLAPPDHits.begin(), MCLAPPDHits.end(),
            [](const auto& a, const auto& b) {
            return a.second.size() < b.second.size();
            });
    auto pair = std::make_pair(maxElementIt->first, maxElementIt->second);
    return pair;
}
