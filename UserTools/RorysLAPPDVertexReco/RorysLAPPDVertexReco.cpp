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

  return true;
}


bool RorysLAPPDVertexReco::Execute(){
    // get the reco digits
    std::vector<RecoDigit>* DigitList = nullptr;
    m_data->Stores.at("RecoEvent")->Get("RecoDigit", DigitList);


    // get information about the mc hits
    std::map<unsigned long,std::vector<MCHit>>* fMCPMTHits=nullptr;             ///< PMT hits
    std::map<unsigned long,std::vector<MCLAPPDHit>>* fMCLAPPDHits=nullptr;   ///< LAPPD hits
    m_data->Stores.at("ANNIEEvent")->Get("MCHits",fMCPMTHits);
    m_data->Stores.at("ANNIEEvent")->Get("MCLAPPDHits",fMCLAPPDHits);
    //int number_of_pmt_hits = 0;
    for (auto&& pair : *fMCPMTHits) {
        auto& hits = pair.second;
        number_of_pmt_hits += hits.size();
    }
    //int number_of_lappd_hits = 0;
    for (auto&& pair : *fMCLAPPDHits) {
        auto& hits = pair.second;
        number_of_lappd_hits += hits.size();
    }
    std::cout<<"number of pmt hits: "<<number_of_pmt_hits<<std::endl;
    std::cout<<"number of lappd hits: "<<number_of_lappd_hits<<std::endl;



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
                break;
            case 999:
                std::cout<<"not a pmt or lappd digit"<<std::endl;
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
            std::cout<<"deleting LAPPD reco digit"<<std::endl;
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
    /*for (auto& reco_digit : *DigitList) {
        int DigitType = reco_digit.GetDigitType();
        if (DigitType != 1) {
            continue; // ignore non-lappd reco digits
        }
        int region = reco_digit.GetRegion();
        Position pos = reco_digit.GetPosition();
        double calT = reco_digit.GetCalTime();
        double calQ = reco_digit.GetCalCharge();
        int DetectorID = reco_digit.GetDetectorID();
        // create new reco "pmt" digit
        RecoDigit fake_reco_digit(region, pos, calT, calQ, DigitType, DetectorID);
        fake_reco_digit.SetDigitType(RecoDigit::PMT8inch);
        int newDigitType = fake_reco_digit.GetDigitType();
        FakePmtHits.push_back(fake_reco_digit);
    }*/
    return FakePmtHits;
}
