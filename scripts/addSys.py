from TIMBER.Analyzer import *
from TIMBER.Tools import AutoJME as JM
from TIMBER.Tools import AutoPU as p
from TIMBER.Tools.Common import CompileCpp
import argparse
import ROOT,sys

sys.path.append('../../')
sys.path.append('../')
#input_file = 'some_nano_files.txt' # or 'file.root'

def get_opts():

    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--inputFile", default = "in.root", help = "Input file name")
    parser.add_argument("-o", "--outputFile", default = "out.root", help = "Output file name")
    parser.add_argument("-y", "--year",  default = "2017", help = "Year of sample (2016, 2016APV,2017,2018)")
    parser.add_argument("--ttbar", default = False, action = 'store_true', help = "TTbar sample (add top pt reweighting)")
    parser.add_argument("--no_pdf", default = False, action = 'store_true', help = "Don't include pdf weight (some signals don't have it?)")
    return parser


def JMEvariationStr(variation):
    base_calibs = ['FatJet_JES_nom','FatJet_JER_nom', 'FatJet_JMS_nom', 'FatJet_JMR_nom']
    variationType = variation.split('_')[0]
    pt_calib_vect = '{'
    mass_calib_vect = '{'
    for c in base_calibs:
        if 'JM' in c:
            mass_calib_vect+='%s,'%('FatJet_'+variation if variationType in c else c)
        elif 'JE' in c:
            pt_calib_vect+='%s,'%('FatJet_'+variation if variationType in c else c)
            mass_calib_vect+='%s,'%('FatJet_'+variation if variationType in c else c)
    pt_calib_vect = pt_calib_vect[:-1]+'}'
    mass_calib_vect = mass_calib_vect[:-1]+'}'
    return pt_calib_vect, mass_calib_vect



def addSys(options):
    pancakes = False

    if(".txt" in options.inputFile):
        f_ = open(options.inputFile, "r")
        options.inputFile = f_.read().splitlines()

    print(options.inputFile)

    columns_to_save = []

    a = analyzer(options.inputFile)

    CompileCpp('../THmodules.cc')

    year_int = int(options.year[2:4])

    if(not options.no_pdf):
        a.AddCorrection( Correction('Pdfweight','TIMBER/Framework/include/PDFweight_uncert.h',[a.lhaid],corrtype='uncert'))
    else:
        a.Define('Pdfweight__nom', '1.')
        a.Define('Pdfweight__down', '1.')
        a.Define('Pdfweight__up', '1.')
        a.Define('nLHEScaleWeight', '0')
    if year_int == 16 or year_int == 17:
        a.AddCorrection( Correction("Prefire","TIMBER/Framework/include/Prefire_weight.h",[year_int],corrtype='weight'))
        columns_to_save.extend(['Prefire__nom','Prefire__up','Prefire__down'])


    a.Define('FatJet_vect','hardware::TLvector(FatJet_pt, FatJet_eta, FatJet_phi, FatJet_msoftdrop)')
    a.Define('DijetIdxs','PickDijets(FatJet_pt, FatJet_eta, FatJet_phi, FatJet_msoftdrop, FatJet_jetId)')
    a.Define('DijetIdx1','DijetIdxs[0]')
    a.Define('DijetIdx2','DijetIdxs[1]')
    columns_to_save.extend(["DijetIdx1", "DijetIdx2"])



# Add btagging correction
    lead_sjbt_corr = Correction('lead_sjbtag_corr', 'TIMBER/Framework/include/SJBtag_SF.h', [year_int,"DeepCSV","shape"], corrtype='weight')
    sublead_sjbt_corr = lead_sjbt_corr.Clone('sublead_sjbtag_corr')

    hadron_flavor = 'SubJet_hadronFlavour'

    #if(pancakes): #based on older version of nanoAOD, this info isn't saved, just use jet one for now (definitely wrong)
        #hadron_flavor = 'Jet_hadronFlavour'

    a.AddCorrection(lead_sjbt_corr, evalArgs={'fatjet_idx': 'DijetIdx1', 'nFatJets' : 'nFatJet', 'sj_idx1_col': 'FatJet_subJetIdx1', 'sj_idx2_col' : 'FatJet_subJetIdx2', 
                                        'subjet_btag' : 'SubJet_btagDeepB', 'subjet_pt': 'SubJet_pt', 'subjet_eta': 'SubJet_eta', 'subjet_flavour' : hadron_flavor})
    a.AddCorrection(sublead_sjbt_corr, evalArgs={'fatjet_idx': 'DijetIdx2', 'nFatJets' : 'nFatJet', 'sj_idx1_col': 'FatJet_subJetIdx1', 'sj_idx2_col' : 'FatJet_subJetIdx2', 
                                         'subjet_btag' : 'SubJet_btagDeepB', 'subjet_pt': 'SubJet_pt', 'subjet_eta': 'SubJet_eta', 'subjet_flavour' : hadron_flavor})
    columns_to_save.extend(['lead_sjbtag_corr__nom', 'lead_sjbtag_corr__up',  'lead_sjbtag_corr__down',
                            'sublead_sjbtag_corr__nom', 'sublead_sjbtag_corr__up',  'sublead_sjbtag_corr__down'])



    if options.ttbar:

        a.Define('GenParticle_vect','hardware::TLvector(GenPart_pt, GenPart_eta, GenPart_phi, GenPart_mass)')
        a.AddCorrection(
            Correction('TptReweight','TIMBER/Framework/include/TopPt_weight.h',corrtype='weight'),
            evalArgs={
                "jet0_idx":"DijetIdxs[0]",
                "jet1_idx":"DijetIdxs[1]",
                "FatJet_vect": "FatJet_vect",
                'GenPart_vect':'GenParticle_vect',
                'scale': 0.5,
            }
       )
        columns_to_save.extend(['TptReweight__nom','TptReweight__up', 'TptReweight__down',])

    #JME Corrections
    a = JM.AutoJME(a, 'FatJet', options.year) #, 'D')
    mass_base_calibs = "{FatJet_JES_nom, FatJet_JER_nom, FatJet_JMS_nom, FatJet_JMR_nom}"
    pt_base_calibs = "{FatJet_JES_nom, FatJet_JER_nom}"

    variations = ['JES_up', 'JER_up', 'JMS_up', 'JMR_up',
                  'JES_down', 'JER_down', 'JMS_down', 'JMR_down']



    a.Define('FatJet_pt_corr', 'hardware::MultiHadamardProduct(FatJet_pt,%s)'% pt_base_calibs) 
    a.Define('FatJet_msoftdrop_corr', 'hardware::MultiHadamardProduct(FatJet_msoftdrop,%s)'% mass_base_calibs) 
    #save as individual columns b/c issues reading vectors...
    var = "corr"
    a.Define('FatJet1_pt_corr', '(DijetIdx1 >= 0) ? FatJet_pt_%s [DijetIdx1] : 1' % var) 
    a.Define('FatJet1_msoftdrop_corr', '(DijetIdx1 >= 0) ? FatJet_msoftdrop_%s [DijetIdx1] : 1' % var) 
    a.Define('FatJet2_pt_corr', '(DijetIdx2 >= 0) ? FatJet_pt_%s [DijetIdx2] : 1' % var) 
    a.Define('FatJet2_msoftdrop_corr', '(DijetIdx2 >= 0) ? FatJet_msoftdrop_%s [DijetIdx2] : 1' % var) 

    columns_to_save.extend(['FatJet1_pt_corr', 'FatJet1_msoftdrop_corr',  'FatJet2_pt_corr', 'FatJet2_msoftdrop_corr'])

    for var in variations:
        pt_calibs, m_calibs = JMEvariationStr(var)
        a.Define('FatJet_pt_'+ var, 'hardware::MultiHadamardProduct(FatJet_pt,%s)'% pt_calibs) 
        a.Define('FatJet_msoftdrop_'+ var, 'hardware::MultiHadamardProduct(FatJet_msoftdrop,%s)'% m_calibs) 

        #save as individual columns b/c issues reading vectors...
        a.Define('FatJet1_pt_'+ var, '(DijetIdx1 >= 0) ? FatJet_pt_%s [DijetIdx1] : 1' % var) 
        a.Define('FatJet1_msoftdrop_'+ var, '(DijetIdx1 >= 0) ? FatJet_msoftdrop_%s [DijetIdx1] : 1' % var) 
        a.Define('FatJet2_pt_'+ var, '(DijetIdx2 >= 0) ? FatJet_pt_%s [DijetIdx2] : 1' % var) 
        a.Define('FatJet2_msoftdrop_'+ var, '(DijetIdx2 >= 0) ? FatJet_msoftdrop_%s [DijetIdx2] : 1' % var) 

        columns_to_save.extend(['FatJet1_pt_'+ var, 'FatJet1_msoftdrop_'+ var, 'FatJet2_pt_'+ var, 'FatJet2_msoftdrop_'+ var ])
            


    a = p.AutoPU(a, options.year)

    #a.Define("Foo", "myPrint(FatJet_JES_nom, FatJet_JER_up, FatJet_JES_up, FatJet_JER_up, FatJet_JES_down, FatJet_JER_down, FatJet_pt, FatJet_pt_corr)")



    #save all the columns
    for c in a.GetColumnNames():
        if c.split('_')[0] in a.GetCollectionNames():
            continue
        elif c.endswith('s') and c[:-1] in a.GetCollectionNames():
            continue
        else:
            columns_to_save.append(c)

    for c in a.GetCollectionNames():
        if('Dijet_' in c ): continue
        columns_to_save.append(c+'_[^_].*')

    columns_to_save.extend(['Pileup__nom','Pileup__up','Pileup__down','Pdfweight__nom','Pdfweight__up','Pdfweight__down'])

    a.Snapshot(columns_to_save, options.outputFile, 'Events', openOption = 'RECREATE')

if(__name__ == "__main__"):
    parser = get_opts()
    options = parser.parse_args()
    addSys(options)
