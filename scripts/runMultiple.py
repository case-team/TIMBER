import os
import glob
from addSys import *

def print_and_do(cmd):
    print(cmd)
    os.system(cmd)

def get_year(dname):
    ul_idx = dname.find("UL")
    year = int(dname[ul_idx+2:ul_idx+4])
    return year

def get_file_list(fname):
    return glob.glob(fname + "/*/*/*/*.root")
    

def run_sys(opts):
    cmd = "python addSys.py " 
    for key,val in opts.__dict__.items():
        if (type(val) == str or type(val) == int): cmd += "--%s %s " % (key, val)
        elif(type(val) == bool and val): cmd += "--%s " % key
        elif(type(val) == list):
            f_temp = open(key + "_temp.txt", "w")
            for l in val:
                f_temp.write(l + "\n")
            f_temp.close()
            cmd += "--%s %s" % (key, key + "_temp.txt")
    print("Running: " + cmd)
    os.system(cmd)



f_inputs = "/eos/cms/store/group/phys_b2g/CASE/PFNanos/2017/"
outdir = "/eos/cms/store/group/phys_b2g/CASE/TIMBER_files/2017/"
year = "2017"

os.system("ls %s > temp.txt" % f_inputs)

f_dataset = open("temp.txt", "r")

datasets = f_dataset.read().splitlines()
#print(datasets)

for dname in datasets:
    #dset_tag = d[1:].replace("/","_").replace("MINIAODSIM", "")
    #if("Qstar" in dname): continue


    file_list = get_file_list(f_inputs + dname)


    parser = get_opts()
    opts = parser.parse_args([])

    opts.inputFile = file_list
    #opts.year = get_year(dname)
    opts.year = year
    opts.outputFile = outdir + dname + "_TIMBER.root"
    if('RSGraviton' in dname or "Qstar" in dname) : opts.no_pdf = True
    if(not os.path.exists(opts.outputFile)):
        print("\n Running %s output \n" % opts.outputFile)
        run_sys(opts)
    else:
        print("Skipping, %s already exists" % opts.outputFile)

    
    



