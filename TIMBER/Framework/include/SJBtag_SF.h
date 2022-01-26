#ifndef _TIMBER_SJBTAG_SF
#define _TIMBER_SJBTAG_SF
// without CMSSW / standalone:
#include "../ext/BTagCalibrationStandalone.h"
#include <ROOT/RVec.hxx>

using namespace ROOT::VecOps;

/**
 * @class SJBtag_SF
 * @brief C++ class. Subjet b tagging scale factor lookup.
 */
class SJBtag_SF {
    private:
        std::string csv_file;
        BTagEntry::OperatingPoint operating_point;
        BTagCalibration calib;
        BTagCalibrationReader b_reader, c_reader, udsg_reader;

        //names of systematics for different types
        std::vector<std::string> sys_uncs_norm {"up", "down"};
        std::vector<std::string> sys_uncs_shape_budsg {"up_jes", "up_lf", "up_hf", "up_hfstats1", "up_hfstats2", "up_lfstats1", "up_lfstats2",
                                                       "down_jes", "down_lf", "down_hf", "down_hfstats1", "down_hfstats2", "down_lfstats1", "down_lfstats2"};
        std::vector<std::string> sys_uncs_shape_c {"up_cferr1", "up_cferr2", "down_cferr1", "down_cferr2"};

    public:
        /**
         * @brief Construct a new subjet b tag scale factor lookup object
         * 
         * @param year 16, 17, or 18.
         * @param tagger Ex. DeepCSV. See TIMBER/data/OfficialSFs/ for others.
         * @param op_string "loose", "medium", "tight"
         */
        SJBtag_SF(int year, std::string tagger, std::string op_string);
        ~SJBtag_SF(){};
        /**
         * @brief Per-event evaluation function
         * 
         * @param pt \f$p_{T}\f$ of subjet
         * @param eta \f$\eta\f$ of subjet
         * @return RVec<float> Nominal, up, down scale factor values.
         */
        //RVec<float> eval(float pt, float eta);
        RVec<float> eval( unsigned int fatjet_idx, unsigned int nFatJets, ROOT::VecOps::RVec<unsigned int> sj_idx1_col, ROOT::VecOps::RVec<unsigned int> sj_idx2_col,
                ROOT::VecOps::RVec<float> subjet_btag, ROOT::VecOps::RVec<float> subjet_pt, ROOT::VecOps::RVec<float> subjet_eta, ROOT::VecOps::RVec<int> subjet_flavour);
};
#endif
