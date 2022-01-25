#include "../include/SJBtag_SF.h"

SJBtag_SF::SJBtag_SF(int year, std::string tagger, std::string op_string) {
    if (op_string == "loose") {
        operating_point = BTagEntry::OP_LOOSE;
    } else if (op_string == "medium") {
        operating_point = BTagEntry::OP_MEDIUM;
    } else if (op_string == "tight") {
        operating_point = BTagEntry::OP_TIGHT;
    } else if( op_string == "reshaping"){
        operating_point = BTagEntry::OP_RESHAPING;
    }
    else {
        throw "Operating point type not supported!";
    }

    
    if (year == 16) {
        csv_file = std::string(std::getenv("TIMBERPATH"))+"TIMBER/data/OfficialSFs/DeepCSV_2016LegacySF_V1.csv";
    } else if (year == 17) {
        csv_file = std::string(std::getenv("TIMBERPATH"))+"TIMBER/data/OfficialSFs/subjet_DeepCSV_94XSF_V4_B_F.csv";
    } else if (year == 18) {
        csv_file = std::string(std::getenv("TIMBERPATH"))+"TIMBER/data/OfficialSFs/DeepCSV_102XSF_V1.csv";
    }

    // setup calibration + reader
    calib = BTagCalibration(tagger, csv_file);
    b_reader = BTagCalibrationReader(operating_point,  // operating point
                                    "central",             // central sys type
                                    {"up", "down"});      // other sys types

    b_reader.load(calib,                // calibration instance
                  BTagEntry::FLAV_B,    // btag flavour
                  "comb");               // measurement type

    if( operating_point == BTagEntry::OP_RESHAPING){
        //load other flavor calibrations as well
        c_reader = BTagCalibrationReader(operating_point,  // operating point
                                        "central",             // central sys type
                                        {"up", "down"});      // other sys types

        c_reader.load(calib,                // calibration instance
                      BTagEntry::FLAV_C,    // btag flavour
                      "comb");               // measurement type

        udsg_reader = BTagCalibrationReader(operating_point,  // operating point
                                        "central",             // central sys type
                                        {"up", "down"});      // other sys types

        udsg_reader.load(calib,                // calibration instance
                      BTagEntry::FLAV_UDSG,    // btag flavour
                      "comb");               // measurement type
    }

}; 

RVec<float> SJBtag_SF::eval(float pt, float eta) {
    RVec<float> jet_scalefactor(3);

    float nom = b_reader.eval_auto_bounds("central", BTagEntry::FLAV_B, eta, pt);
    float up = b_reader.eval_auto_bounds("up", BTagEntry::FLAV_B, eta, pt);
    float down = b_reader.eval_auto_bounds("down", BTagEntry::FLAV_B, eta, pt);

    jet_scalefactor[0] = nom;
    jet_scalefactor[1] = up;
    jet_scalefactor[2] = down;

    return jet_scalefactor;
};



/*
RVec<float> SJBtag_SF::eval(
    int sj_idx1, 
    int sj_idx2, 
    ROOT::VecOps::RVec<float> subjet_btag,
    ROOT::VecOps::RVec<float> subjet_pt,
    ROOT::VecOps::RVec<float> subjet_eta,
    ROOT::VecOps::RVec<BTagEntry::JetFlavor> subjet_flavour) {


    unsigned int sj_idx = -1;

    if(sj_idx2 < 0 || (subjet_btag[sj_idx1] > subjet_btag[sj_idx2])) sj_idx  = sj_idx1;
    else    sj_idx = sj_idx2;

    BTagCalibrationReader reader;
    if(subjet_flavour[sj_idx] == BTagEntry::FLAV_B) reader = b_reader;
    else if(subjet_flavour[sj_idx] == BTagEntry::FLAV_C) reader = c_reader;
    else if(subjet_flavour[sj_idx] == BTagEntry::FLAV_UDSG) reader = udsg_reader;
    else throw "Unknown hadron flavor";
        
    RVec<float> jet_scalefactor(3);

    float nom = reader.eval_auto_bounds("central", subjet_flavour[sj_idx], subjet_eta[sj_idx], subjet_pt[sj_idx]);
    float up = reader.eval_auto_bounds("up", subjet_flavour[sj_idx], subjet_eta[sj_idx], subjet_pt[sj_idx]);
    float down = reader.eval_auto_bounds("down", subjet_flavour[sj_idx], subjet_eta[sj_idx], subjet_pt[sj_idx]);

    jet_scalefactor[0] = nom;
    jet_scalefactor[1] = up;
    jet_scalefactor[2] = down;

    return jet_scalefactor;
};
*/
