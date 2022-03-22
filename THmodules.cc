#include "ROOT/RVec.hxx"
#include "TIMBER/Framework/include/common.h"

using namespace ROOT::VecOps;

RVec<int> PickDijets(RVec<float> pt, RVec<float> eta, RVec<float> phi, RVec<float> mass, RVec<int> jetId) {
    int jet0Idx = -1;
    int jet1Idx = -1;
    //lower pt cut so still have jets for JES variations
    float pt_cut = 50;
    for (int ijet = 0; ijet < pt.size(); ijet++) {
            if (pt[ijet] > pt_cut && std::abs(eta[ijet]) < 2.5 && (jetId[ijet] & 2) == 2) {
                if (jet0Idx == -1 || pt[ijet] > pt[jet0Idx]) {
                    jet1Idx = jet0Idx;
                    jet0Idx = ijet;
                } else if (jet1Idx == -1 || pt[ijet] > pt[jet1Idx]){
                        jet1Idx = ijet;
                }
        }       
    }

    //mass order
    if(jet0Idx >= 0 && jet1Idx >= 0){
        if(mass[jet1Idx] > mass[jet0Idx]){
            int temp = jet1Idx;
            jet1Idx = jet0Idx;
            jet0Idx = temp;
        }
    }
    return {jet0Idx,jet1Idx};
}

int myPrint(RVec<float> FatJet_JES_nom, RVec<float> FatJet_JER_nom, 
        RVec<float> FatJet_JES_up, RVec<float> FatJet_JER_up, RVec<float> FatJet_JES_down, RVec<float> FatJet_JER_down,
        RVec<float> FatJet_pt, RVec<float> FatJet_pt_corr){
    std::cout << "JES_nom " <<  FatJet_JES_nom << std::endl;
    std::cout << "JER_nom " <<  FatJet_JER_nom << std::endl;
    std::cout << "JES_up " <<  FatJet_JES_up << std::endl;
    std::cout << "JER_up " <<  FatJet_JER_up << std::endl;
    std::cout << "JES_down " <<  FatJet_JER_down << std::endl;
    std::cout << "JER_down " <<  FatJet_JER_down << std::endl;
    std::cout << "pt " <<  FatJet_pt << std::endl;
    std::cout << "pt_corr " <<  FatJet_pt_corr << std::endl;

    return 1;
}

std::vector<int> PickTop(RVec<float> mass, RVec<float> tagScore, RVec<int> idxs, std::pair<float,float> massCut, float scoreCut, bool invertScore=false) {
    if (idxs.size()>2) {
        std::cout << "PickTop -- WARNING: You have input more than two indices. Only two accepted. Assuming first two indices.";
    }
    std::vector<int> out(2);
    float WP = scoreCut;

    int idx0 = idxs[0];
    int idx1 = idxs[1];
    bool isTop0, isTop1;
    if (!invertScore) {
        isTop0 = (mass[idx0] > massCut.first) && (mass[idx0] < massCut.second) && (tagScore[idx0] > WP);
        isTop1 = (mass[idx1] > massCut.first) && (mass[idx1] < massCut.second) && (tagScore[idx1] > WP);
    } else {
        isTop0 = (mass[idx0] > massCut.first) && (mass[idx0] < massCut.second) && (tagScore[idx0] < WP);
        isTop1 = (mass[idx1] > massCut.first) && (mass[idx1] < massCut.second) && (tagScore[idx1] < WP);
    }
    
    if (isTop0 && isTop1) {
        if (tagScore[idx0] > tagScore[idx1]) {
            out[0] = idx0;
            out[1] = idx1;
        } else {
            out[0] = idx1;
            out[1] = idx0;
        }
    } else if (isTop0) {
        out[0] = idx0;
        out[1] = idx1;
    } else if (isTop1) {
        out[0] = idx1;
        out[1] = idx0;
    } else {
        out[0] = -1;
        out[1] = -1;
    }
    return out;
}

bool MatchToGen(int pdgID, ROOT::Math::PtEtaPhiMVector jet,
                RVec<ROOT::Math::PtEtaPhiMVector> GenPart_vect,
                RVec<int> GenPart_pdgId) {
    bool found = false;
    for (int igp = 0; igp<GenPart_vect.size(); igp++) {
        if (abs(GenPart_pdgId[igp]) == pdgID) {
            if (hardware::DeltaR(jet,GenPart_vect[igp]) < 0.8) {
                found = true;
                break;
            }
        }
    }
    return found;
}

std::vector<int> PickTopGenMatch(RVec<ROOT::Math::PtEtaPhiMVector> Dijet_vect,
                                 RVec<ROOT::Math::PtEtaPhiMVector> GenPart_vect,
                                 RVec<int> GenPart_pdgId) {
    if (Dijet_vect.size()>2) {
        std::cout << "PickTopGenMatch -- WARNING: You have input more than two indices. Only two accepted. Assuming first two indices.";
    }
    int tIdx = -1;
    int hIdx = -1;
    for (int ijet = 0; ijet < 2; ijet++) {
        if (MatchToGen(6, Dijet_vect[ijet], GenPart_vect, GenPart_pdgId)) {
            tIdx = ijet;
        } else if (MatchToGen(25, Dijet_vect[ijet], GenPart_vect, GenPart_pdgId)) {
            hIdx = ijet;
        }
    }
    return {tIdx,hIdx};
}
