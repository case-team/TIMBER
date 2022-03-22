#include "../include/TopPt_weight.h"

TopPt_weight::TopPt_weight(){};

std::vector<float> TopPt_weight::matchingGenPt(
        RVec<int> GenPart_pdgId, RVec<int> GenPart_statusFlags, RVec<ROOT::Math::PtEtaPhiMVector> GenPart_vect,
        ROOT::Math::PtEtaPhiMVector jet0, ROOT::Math::PtEtaPhiMVector jet1){

    float genTPt = -1.;
    float genTbarPt = -1.;

    // For all gen particles
    for (size_t i = 0; i < GenPart_pdgId.size(); i++){
        if (GenPart_statusFlags[i] & (1 << 13)) {
            if (GenPart_pdgId[i] == -6) { 
                if ((hardware::DeltaR(GenPart_vect[i],jet0) < 0.8) || (hardware::DeltaR(GenPart_vect[i],jet1) < 0.8)) {
                    genTbarPt = GenPart_vect[i].Pt();
                }
            } else if (GenPart_pdgId[i] == 6) { 
                if ((hardware::DeltaR(GenPart_vect[i],jet0) < 0.8) || (hardware::DeltaR(GenPart_vect[i],jet1) < 0.8)) {
                    genTPt = GenPart_vect[i].Pt();
                }
            }
        }
    }
    return {genTPt,genTbarPt};
}

RVec<float> TopPt_weight::eval(
        RVec<int> GenPart_pdgId, RVec<int> GenPart_statusFlags, RVec<ROOT::Math::PtEtaPhiMVector> GenPart_vect, RVec<ROOT::Math::PtEtaPhiMVector> FatJet_vect,
        int jet0_idx, int jet1_idx, float scale){

    if(jet0_idx <0 || jet1_idx < 0) return {1.0, 1.0, 1.0};

    auto jet0 = FatJet_vect[jet0_idx];
    auto jet1 = FatJet_vect[jet1_idx];

    std::vector<float> matched = matchingGenPt(GenPart_pdgId, GenPart_statusFlags,
                                          GenPart_vect, jet0, jet1);
    //https://twiki.cern.ch/twiki/bin/viewauth/CMS/TopPtReweighting
    //Max correction at 500 GeV
    float genTPt = std::min(matched[0], 500.f);
    float genTbarPt = std::min(matched[1], 500.f);

    float wTPt = 1.0;
    float wTPt_up = 1.0;
    float wTPt_down = 1.0;
    if (genTPt > 0){ 
        wTPt = exp(0.0615 - 0.0005*genTPt);
        wTPt_up = exp((1+scale)*0.0615 - 0.0005*genTPt);
        wTPt_down = exp((1-scale)*0.0615 - 0.0005*genTPt);
    }

    float wTbarPt = 1.0;
    float wTbarPt_up = 1.0;
    float wTbarPt_down = 1.0;
    if (genTbarPt > 0){
        wTbarPt = exp(0.0615 - 0.0005*genTbarPt);
        wTbarPt_up = exp((1+scale)*0.0615 - 0.0005*genTbarPt);
        wTbarPt_down = exp((1-scale)*0.0615 - 0.0005*genTbarPt);
    }

    return {sqrt(wTPt*wTbarPt),sqrt(wTPt_up*wTbarPt_up),sqrt(wTPt_down*wTbarPt_down)};
}
