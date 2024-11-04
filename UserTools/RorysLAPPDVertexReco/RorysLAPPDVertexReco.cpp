#include "RorysLAPPDVertexReco.h"

RorysLAPPDVertexReco::RorysLAPPDVertexReco():Tool(){}


bool RorysLAPPDVertexReco::Initialise(std::string configfile, DataModel &data){

  /////////////////// Useful header ///////////////////////
  if(configfile!="") m_variables.Initialise(configfile); // loading config file
  //m_variables.Print();

  m_data= &data; //assigning transient data pointer
  /////////////////////////////////////////////////////////////////
  ///
  m_variables.Get("verbosity", verbosity);
  m_variables.Get("AddFakeLAPPDDigits", AddFakeLAPPDDigits);
  m_variables.Get("LAPPDIDFile", fLAPPDIDFile);

  auto get_geometry= m_data->Stores.at("ANNIEEvent")->Header->Get("AnnieGeometry",fGeometry);
  if(!get_geometry){
    Log("RorysLAPPDVertexReco Tool: Error retrieving Geometry from ANNIEEvent!",v_error,verbosity); 
    return false; 
  }

  m_data->CStore.Get("detectorkey_to_lappdid",detectorkey_to_lappdid);
  return true;
}


bool RorysLAPPDVertexReco::Execute(){
    ReadLAPPDIDFile();
    // get the reco digits
    std::vector<RecoDigit>* DigitList = nullptr;
    m_data->Stores.at("RecoEvent")->Get("RecoDigit", DigitList);


    // get information about the mc hits
    std::map<unsigned long,std::vector<MCHit>>* fMCPMTHits=nullptr;             ///< PMT hits
    std::map<unsigned long,std::vector<MCLAPPDHit>>* fMCLAPPDHits=nullptr;   ///< LAPPD hits
    m_data->Stores.at("ANNIEEvent")->Get("MCHits",fMCPMTHits);
    m_data->Stores.at("ANNIEEvent")->Get("MCLAPPDHits",fMCLAPPDHits);
    bool get_mclappdhits = m_data->Stores.at("ANNIEEvent")->Get("MCLAPPDHits",fMCLAPPDHits);
    if (!get_mclappdhits) return true;
    //int number_of_pmt_hits = 0;
    for (auto&& pair : *fMCPMTHits) {
        auto& hits = pair.second;
        number_of_pmt_hits += hits.size();
    }
    //int number_of_lappd_hits = 0;
    if (verbosity>0) std::cout<<"RORYS LAPPD VERTEX STUFF: NUMBER OF LAPPDS: "<<fMCLAPPDHits->size()<<std::endl;
    Detector* det=nullptr;
    std::map<unsigned long, std::pair<std::vector<Vec3>,std::map<int, std::vector<MCLAPPDHit>>>> LAPPDQuadrantMap;
    //std::map<unsigned long, std::vector<Vec3>> QuadrantLocations;
    for (auto&& pair : *fMCLAPPDHits) {
        auto& chankey = pair.first;
        det = fGeometry->ChannelToDetector(chankey);
        int detkey = det->GetDetectorID();
        int LAPPDId = detectorkey_to_lappdid.at(detkey);
        bool isSelectedLAPPD=false;
        for (auto& selectedId : fLAPPDId) {
            if (LAPPDId == selectedId) isSelectedLAPPD=true;
        }
        if(!isSelectedLAPPD) continue;
        if (verbosity>0) std::cout<<"LAPPDId: "<<LAPPDId<<" has hits and is within selected LAPPDs"<<std::endl;
        Position LAPPDPos = det->GetDetectorPosition();
        if (verbosity>0) std::cout<<"LAPPD #"<<LAPPDId<<std::endl;
        //det->Print();
        Direction LAPPDDir = det->GetDetectorDirection();
        Vec3 VecLAPPDPos(LAPPDPos.X(), LAPPDPos.Y(), LAPPDPos.Z());
        Vec3 VecLAPPDDir(LAPPDDir.X(), LAPPDDir.Y(), LAPPDDir.Z());
        std::vector<Vec3> QuadrantCentres = CalculateQuadrantCentres(VecLAPPDPos, VecLAPPDDir);
        int centreno = 1;
        for (auto& centre : QuadrantCentres) {
            if (verbosity>0) std::cout<<"Quadrant #"<<centreno<<" coords: ";
            //centre.Print();
            ++centreno;
        }
        //QuadrantLocations[LAPPDId] = QuadrantCentres;
        auto& hits = pair.second;
        number_of_lappd_hits += hits.size();
        for (auto& hit : hits) {
            std::vector<double> localpos = hit.GetLocalPosition();
            //std::cout<<"Transverse position of hit: "<<localpos.at(1)<<std::endl;
        }
        std::map<int, std::vector<MCLAPPDHit>> SingleQuadrantMap = SortHitsToQuadrants(hits);
        for (auto&& pair : SingleQuadrantMap){
            if (verbosity>0) std::cout<<"Quadrant: "<<pair.first<<std::endl;
            if (verbosity>0) std::cout<<"Number of hits: "<<pair.second.size()<<std::endl<<std::endl;
        }
        LAPPDQuadrantMap[LAPPDId] = std::pair<std::vector<Vec3>,std::map<int, std::vector<MCLAPPDHit>>> (QuadrantCentres, SingleQuadrantMap);
    }

    if (verbosity>0) std::cout<<"Number of LAPPDs with hits: "<<LAPPDQuadrantMap.size()<<std::endl;
    std::map<unsigned long, std::vector<MCLAPPDHit>> newLAPPDHits;
    // loop over each lappd
    for (auto&& lappd_entry : LAPPDQuadrantMap) {
        unsigned long lappdid = lappd_entry.first;
        std::vector<Vec3> quadrant_centres = lappd_entry.second.first;
        std::map<int, std::vector<MCLAPPDHit>> quadrant_hits = lappd_entry.second.second;
        // loop over each quadrant
        std::vector<MCLAPPDHit> QuadHits;
        for (auto&& quad_entry : quadrant_hits) {
            int quad_id = quad_entry.first;
            std::vector<MCLAPPDHit> hits = quad_entry.second;

            Vec3 quad_centre = quadrant_centres[quad_id-1];
            MCLAPPDHit QuadHit;
            QuadHit.SetPosition(quad_centre.GetPositionAsVector());
            double quad_charge = 0.0;
            double quad_time = 0;
            for (auto& hit : hits) {
                quad_charge += hit.GetCharge();
                // need to do something with the times... average time?
                quad_time += hit.GetTime();
            }
            quad_time = quad_time/hits.size();
            QuadHit.SetCharge(quad_charge);
            QuadHit.SetTime(quad_time);
            QuadHits.push_back(QuadHit);
        }
        newLAPPDHits[lappdid] = QuadHits;
    }

    // now got all the hits with positions of the quadrants
    // now we need to create new recodigits, with the positions of the quadrants, the mean time of the hits in the quadrant, and the total charge of the hits in the quadrant
    /*
    for (auto&& pair : newLAPPDHits) {
        if (verbosity>0) std::cout<<"newlappdhits: lappdid: "<<pair.first<<std::endl;
        for (auto& hit : pair.second) {
            if (verbosity>0) std::cout<<"hit position: "<<std::endl;
            std::vector<double> hitpos = hit.GetPosition();
            for (auto& pos : hitpos) {
                if (verbosity>0) std::cout<<pos<<std::endl;
            }
        }
    }
    */
    bool builtmcrecodigits = BuildMCLAPPDRecoDigits(DigitList, newLAPPDHits);

    if (verbosity > 0) std::cout<<"number of pmt hits: "<<number_of_pmt_hits<<std::endl;
    if (verbosity > 0) std::cout<<"number of lappd hits: "<<number_of_lappd_hits<<std::endl;



    // get information about the digits
    //int number_of_pmt_digits = 0;
    //int number_of_lappd_digits = 0;
    //int number_of_all_digits = 0;
    for (auto digit : *DigitList){
        int DigitType = digit.GetDigitType();
        switch (DigitType) {
            case 0:
                number_of_pmt_digits++;
                break;

            case 1:
                number_of_lappd_digits++;
                if (verbosity>1) std::cout<<"LAPPD detector ID: "<<digit.GetDetectorID()<<std::endl;
                break;
            case 999:
                if (verbosity > 0) std::cout<<"not a pmt or lappd digit"<<std::endl;
                number_of_all_digits++;
                break;
        }
    }
    //std::cout<<"number of pmt digits: "<<number_of_pmt_digits<<std::endl;
    //std::cout<<"number of lappd digits: "<<number_of_lappd_digits<<std::endl;


    if (!AddFakeLAPPDDigits){
        for (unsigned int i=0; i<DigitList->size(); i++) {
            int DigitType = DigitList->at(i).GetDigitType();
            if (DigitType != 1) {
                continue; // ignore non-lappd reco digits
            }
            if (verbosity > 0) std::cout<<"deleting LAPPD reco digit"<<std::endl;
            DigitList->erase(DigitList->begin() + i); // remove the lappd entry from the list
        }
        return true;
    }
    //std::cout<<"DigitList size before adding fake stuff: "<<DigitList->size()<<std::endl;
    std::vector<RecoDigit> RecoFakePmtDigits = ExtractLAPPDDigits(DigitList);
    double fake_lappd_hit_counter = 0;
    double pmt_hit_counter = 0;


    for (auto digit : RecoFakePmtDigits) {
        fake_lappd_hit_counter++;
        DigitList->push_back(digit);
    }

    pmt_hit_counter = DigitList->size() - fake_lappd_hit_counter;


    //std::cout<<"DigitList size after adding fake stuff: "<<DigitList->size()<<std::endl;
    //std::cout<<"Number of fake lappd hits added to DigitList: "<<fake_lappd_hit_counter<<std::endl;
    //std::cout<<"Size of RecoFakePmtDigits (should be similar to above): "<<RecoFakePmtDigits.size()<<std::endl;
    // replace the stores entry with the new, fake digits
    //m_data->Stores.at("RecoEvent")->Delete();
    //m_data->Stores.at("RecoEvent")->Set("RecoDigit", FakeDigitList, true);

    //delete FakeDigitList;
    //delete DigitList;
    m_data->Stores.at("RecoEvent")->Set("NumLAPPDDigits",fake_lappd_hit_counter);
    m_data->Stores.at("RecoEvent")->Set("NumPMTDigits",pmt_hit_counter);
    //m_data->Stores.at("RecoEvent")->Set("RecoDigit", DigitList);

  return true;
}


bool RorysLAPPDVertexReco::Finalise(){
    std::cout<<"number of pmt digits: "<<number_of_pmt_digits<<std::endl;
    std::cout<<"number of lappd digits: "<<number_of_lappd_digits<<std::endl;
    std::cout<<"number of other digits: "<<number_of_all_digits<<std::endl;
    std::cout<<"number of pmt hits: "<<number_of_pmt_hits<<std::endl;
    std::cout<<"number of lappd hits: "<<number_of_lappd_hits<<std::endl;

  return true;
}

std::vector<RecoDigit> RorysLAPPDVertexReco::ExtractLAPPDDigits(std::vector<RecoDigit>* DigitList){
    std::vector<RecoDigit> FakePmtHits;
    for (unsigned int i=0; i<DigitList->size(); i++) {
        int DigitType = DigitList->at(i).GetDigitType();
        if (DigitType != 1) {
            continue; // ignore non-lappd reco digits
        }
        int region = DigitList->at(i).GetRegion();
        Position pos = DigitList->at(i).GetPosition();
        double calT = DigitList->at(i).GetCalTime();
        double calQ = DigitList->at(i).GetCalCharge();
        int DetectorID = DigitList->at(i).GetDetectorID();
        // create new reco "pmt" digit
        RecoDigit fake_reco_digit(region, pos, calT, calQ, DigitType, DetectorID);
        fake_reco_digit.SetDigitType(RecoDigit::PMT8inch);
        int newDigitType = fake_reco_digit.GetDigitType();
        FakePmtHits.push_back(fake_reco_digit);
        DigitList->erase(DigitList->begin() + i); // remove the lappd entry from the list
    }
    return FakePmtHits;
}

void RorysLAPPDVertexReco::ReadLAPPDIDFile() {
  std::string line;
  ifstream myfile(fLAPPDIDFile);
  if (myfile.is_open()){
    while(getline(myfile,line)){
      if(verbosity>0){
        std::cout << "RorysLAPPDVertexReco tool: Loading hits from LAPPD ID " << line << std::endl;
      }
      int thisID = std::atoi(line.c_str());
      fLAPPDId.push_back(thisID);
    }
  } else {
    Log("Unable to open given LAPPD ID File. Using all LAPPDs",v_error,verbosity);
  }
}

std::map<int, std::vector<MCLAPPDHit>> RorysLAPPDVertexReco::SortHitsToQuadrants(const std::vector<MCLAPPDHit>& LAPPDHits) {
    std::map<int, std::vector<MCLAPPDHit>> QuadrantMap;
    for (const auto& hit : LAPPDHits) {
        std::vector<double> LocalPosition = hit.GetLocalPosition();
        double x = LocalPosition.at(0);
        double y = LocalPosition.at(1);
        if (x>0 && y>0) {
            QuadrantMap[1].push_back(hit);
        }
        else if (x<0 && y>0) {
            QuadrantMap[2].push_back(hit);
        }
        else if (x<0 && y<0) {
            QuadrantMap[3].push_back(hit);
        }
        else if (x>0 && y<0) {
            QuadrantMap[4].push_back(hit);
        }
    }
    return QuadrantMap;
}

std::vector<Vec3> RorysLAPPDVertexReco::CalculateQuadrantCentres(const Vec3& LAPPDCentre, const Vec3& LAPPDDirection) {
    std::vector<Vec3> LocalQuadrantCentres = {
        Vec3(5.0, 5.0, 0.0),    // quadrant 1
        Vec3(-5.0, 5.0, 0.0),   // quadrant 2
        Vec3(-5.0, -5.0, 0.0),  // quadrant 3
        Vec3(5.0, -5.0, 0.0),   // quadrant 4
    };

    Vec3 GlobalUp(0, 1, 0);

    Vec3 local_to_global_x = GlobalUp.cross(LAPPDDirection).normalise();
    Vec3 local_to_global_y = LAPPDDirection.cross(local_to_global_x).normalise();

    std::vector<Vec3> GlobalQuadrantCentres;
    for (auto& LocalCentre : LocalQuadrantCentres) {
        Vec3 GlobalPosition = LAPPDCentre + local_to_global_x*LocalCentre.x + local_to_global_y*LocalCentre.y;
        GlobalQuadrantCentres.push_back(GlobalPosition);
    }
    return GlobalQuadrantCentres;
}

bool RorysLAPPDVertexReco::BuildMCLAPPDRecoDigits(std::vector<RecoDigit>* DigitList, std::map<unsigned long, std::vector<MCLAPPDHit>>& newLAPPDHits) {
	std::string name = "DigitBuilder::BuildMCLAPPDRecoDigit(): ";
	Log(name + " Build LAPPD reconstructed digits",v_message,verbosity);
	int region = -999;
	double calT = 0;
	double calQ = 0;
	int digitType = -999;
	Detector* det=nullptr;
	Position  pos_sim, pos_reco;
  // repeat for LAPPD hits
    Log("RorysLAPPDVertexReco Tool: Num LAPPD Digits = "+to_string(newLAPPDHits.size()),v_message,verbosity);
    // iterate over the map of sensors with a measurement
    for(std::pair<unsigned long,std::vector<MCLAPPDHit>>&& apair : newLAPPDHits){
        /*unsigned long chankey = apair.first;
        det = fGeometry->ChannelToDetector(chankey);
        if(det==nullptr){
            Log("RorysLAPPDVertexReco Tool: LAPPD Detector not found! ",v_message,verbosity);
            continue;
        }
        int detkey = det->GetDetectorID();
        int LAPPDId = detectorkey_to_lappdid.at(detkey);*/
        int LAPPDId = apair.first;
  //Check if LAPPD is in selected LAPPDs
  bool isSelectedLAPPD = false;
  /*
  for(int i=0;i<int(fLAPPDId.size());i++){
          if(LAPPDId == fLAPPDId.at(i)) isSelectedLAPPD=true;
  }
  if(!isSelectedLAPPD && fLAPPDId.size()>0) continue;
  */
        std::vector<MCLAPPDHit>& hits = apair.second;
        for(MCLAPPDHit& ahit : hits){
            //if(v_message<verbosity) ahit.Print(); // << VERY verbose
            // an LAPPDHit has adds (global x-y-z) position, (in-tile x-y) local position
            // and time psecs
            // convert the WCSim coordinates to the ANNIEreco coordinates
            // convert the unit from m to cm
            pos_reco.SetX(ahit.GetPosition().at(0)*100.+xshift); //cm
            pos_reco.SetY(ahit.GetPosition().at(1)*100.+yshift); //cm
            pos_reco.SetZ(ahit.GetPosition().at(2)*100.+zshift); //cm
            calT = ahit.GetTime();  // 
            calT = frand.Gaus(calT, 0.1); // time is smeared with 100 ps time resolution. Harded-coded for now.
            calQ = ahit.GetCharge();
  if (verbosity>4) { 
    std::cout << "LAPPD position (X<Y<Z): " << 
            to_string(pos_reco.X()) << "," << to_string(pos_reco.Y()) <<
            "," << to_string(pos_reco.Z()) << std::endl;
    std::cout << "LAPPD Charge,Time: " << to_string(calQ) << "," <<
            to_string(calT) << std::endl;
  }
            // I found the charge is 0 for all the hits. In order to test the code, 
            // here I just set the charge to 1. We should come back to this later. (Jingbo Wang)
            calQ = 1.;
            digitType = RecoDigit::lappd_v0;
            RecoDigit recoDigit(region, pos_reco, calT, calQ, digitType,LAPPDId);
            //if(v_message<verbosity) recoDigit.Print();
          //make some cuts here. It will be moved to the Hitcleaning tool
          if(calT>40 || calT<-10) continue; // cut off delayed hits
          DigitList->push_back(recoDigit);
        }
    } // end loop over MCLAPPDHits
	return true;
}
