#include "../include/SJBtag_SF.h"

SJBtag_SF::SJBtag_SF(int year, std::string tagger, std::string op_string) {

    std::string meas_type = "comb";

    auto sys_uncs = sys_uncs_norm;

    if (op_string == "loose") {
        operating_point = BTagEntry::OP_LOOSE;
    } else if (op_string == "medium") {
        operating_point = BTagEntry::OP_MEDIUM;
    } else if (op_string == "tight") {
        operating_point = BTagEntry::OP_TIGHT;
    } else if( op_string == "shape"){
        operating_point = BTagEntry::OP_RESHAPING;
        meas_type = "iterativefit";
        sys_uncs = sys_uncs_shape_budsg;
    }
    else {
        throw std::runtime_error("Operating point type not supported!");
    }

    
    if (year == 16) {
        csv_file = std::string(std::getenv("TIMBERPATH"))+"TIMBER/data/OfficialSFs/DeepCSV_2016LegacySF_V1.csv";
    } else if (year == 17) {
        csv_file = std::string(std::getenv("TIMBERPATH"))+"TIMBER/data/OfficialSFs/subjet_DeepCSV_94XSF_V4_B_F.csv";
    } else if (year == 18) {
        csv_file = std::string(std::getenv("TIMBERPATH"))+"TIMBER/data/OfficialSFs/DeepCSV_102XSF_V1.csv";
    }


    // setup calibration + reader
    printf("Loading b SF's \n");
    calib = BTagCalibration(tagger, csv_file);
    b_reader = BTagCalibrationReader(operating_point,  // operating point
                                    "central",             // central sys type
                                    sys_uncs);      // other sys types

    b_reader.load(calib,                // calibration instance
                  BTagEntry::FLAV_B,    // btag flavour
                  meas_type);               // measurement type

    if( operating_point == BTagEntry::OP_RESHAPING){
    printf("Loading c SF's \n");
        //load other flavor calibrations as well
        c_reader = BTagCalibrationReader(operating_point,  // operating point
                                        "central",             // central sys type
                                        sys_uncs_shape_c);      // other sys types

        c_reader.load(calib,                // calibration instance
                      BTagEntry::FLAV_C,    // btag flavour
                      meas_type);               // measurement type

        printf("Loading udsg SF's \n");
        udsg_reader = BTagCalibrationReader(operating_point,  // operating point
                                        "central",             // central sys type
                                        sys_uncs_shape_budsg);      // other sys types

        udsg_reader.load(calib,                // calibration instance
                      BTagEntry::FLAV_UDSG,    // btag flavour
                      meas_type);               // measurement type
    }

}; 

/*
RVec<float> SJBtag_SF::eval(float pt, float eta) {
    RVec<float> jet_scalefactor(3);

    float nom, up, down;
    nom = b_reader.eval_auto_bounds("central", BTagEntry::FLAV_B, eta, pt);

    if( operating_point != BTagEntry::OP_RESHAPING){
        float up = b_reader.eval_auto_bounds("up", BTagEntry::FLAV_B, eta, pt);
        float down = b_reader.eval_auto_bounds("down", BTagEntry::FLAV_B, eta, pt);
    }
    else{ // for now
        up = nom;
        down = nom;
    }

    jet_scalefactor[0] = nom;
    jet_scalefactor[1] = up;
    jet_scalefactor[2] = down;

    return jet_scalefactor;
};
*/



RVec<float> SJBtag_SF::eval(
        unsigned int fatjet_idx,
        unsigned int nFatJets,
    ROOT::VecOps::RVec<unsigned int> sj_idx1_col,
    ROOT::VecOps::RVec<unsigned int> sj_idx2_col,
    ROOT::VecOps::RVec<float> subjet_btag,
    ROOT::VecOps::RVec<float> subjet_pt,
    ROOT::VecOps::RVec<float> subjet_eta,
    ROOT::VecOps::RVec<int> subjet_flavour) {

    RVec<float> jet_scalefactor(3, 1.);


    //return if this fatjet doesn't exist
    if(fatjet_idx >= nFatJets) return jet_scalefactor;


    //else look up subjets idxs of this fatjet
    
    int sj_idx1 = sj_idx1_col[fatjet_idx];
    int sj_idx2 = sj_idx2_col[fatjet_idx];


    //no subjets
    if(sj_idx1 < 0) return jet_scalefactor;


    int sj_idx = -1;

    //want subjet with higher btag score
    if(sj_idx2 < 0 || (subjet_btag[sj_idx1] > subjet_btag[sj_idx2])) sj_idx  = sj_idx1;
    else    sj_idx = sj_idx2;


    //work around b/c it doesn't like casting...
    const int b_flav = 5;
    const int c_flav = 4;
    BTagEntry::JetFlavor flav;
    
    
    BTagCalibrationReader reader;
    std::vector<std::string> sys_list;
    if(subjet_flavour[sj_idx] == b_flav){
        reader = b_reader;
        flav = BTagEntry::FLAV_B;
        sys_list = sys_uncs_shape_budsg;
    }
    else if(subjet_flavour[sj_idx] == c_flav){
        reader = c_reader;
        flav = BTagEntry::FLAV_C;
        sys_list = sys_uncs_shape_c;
    }
    else{
        reader = udsg_reader;
        flav = BTagEntry::FLAV_UDSG;
        sys_list = sys_uncs_shape_budsg;
    }
        

    float nom = reader.eval_auto_bounds("central", flav, std::fabs(subjet_eta[sj_idx]), subjet_pt[sj_idx], subjet_btag[sj_idx]);
    float up(0.), down(0.);

    
    //envelop of all systematics (simplest procedure possible for now...)
    for (std::string & sys: sys_list){
        float diff = reader.eval_auto_bounds(sys, flav, std::fabs(subjet_eta[sj_idx]), subjet_pt[sj_idx], subjet_btag[sj_idx]) - nom;

        if(diff > 0) up += diff * diff;
        else down += diff * diff;
    }
    up = nom + std::pow(up, 0.5);
    down = nom - std::pow(down, 0.5);



    jet_scalefactor[0] = nom;
    jet_scalefactor[1] = up;
    jet_scalefactor[2] = down;
    

    return jet_scalefactor;
};
