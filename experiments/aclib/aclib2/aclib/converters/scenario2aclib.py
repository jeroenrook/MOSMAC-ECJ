
'''
    pcs2aclib.py - converts the old pcs format of SMAC to the new common AClib format

    @author: Marius Lindauer
'''

import sys
import os
import shutil
import argparse
import shlex
import logging
from aclib.converters.pcs2aclib import PCSAClibConverter
from aclib.converters.aclib2pcs import PCSAClibBackConverter

class ScenarioAClibConverter(object):
    ''' converts scenario files to AClib2.0 standard;
    '''
    

    def __init__(self, fn, out_fn):
        ''' Constructor '''
        self.fn = fn
        if out_fn is not None:
            self.out_fp = open(out_fn, "w")
            self.out_dir = os.path.split(out_fn)[0]
        else:
            self.out_fp = None
            self.out_dir = ""
            
        self.instance_files = [] 
        self.pcs_file = None
        
        self.logger = logging.getLogger("ScenarioAClibConverter")
        
        self.convert_dir = {
            "cutoff_time" : "cutoff-time",
            "run_obj" : "run-obj",
            #"overall_obj" : "overall-obj",
            "tunerTimeout": "cpu-limit",
            "wallclock_limit": "wallclock-limit",
            "runcount_limit" : "max-evaluations",
            "instance_file" : "training-instances",
            "test_instance_file" : "test-instances",
            "feature_file": "instance-features",
            "deterministic" : "deterministic"
            }
        
        self.back_convert = {v: k for k, v in self.convert_dir.items()}
        
    def convert(self):
        '''
            convert from old (SMAC) format (self.fn) into new ACLib2 format (self.out_fn) 
        ''' 
    
        with open(self.fn) as fp:
            for line in fp:
                line = line.replace("\n","")
                param, value = list(map(lambda x : x.strip(" "), line.split("=")))
                
                if param == "execdir":
                    continue
                elif param  == "algo": 
                    algo = shlex.split(value)
                    self.write("executable = %s" %algo[0])
                    for a in algo[1:]:
                        self.write("arg = %s" %(a))
                elif line.startswith("paramfile"):
                    self.pcs_file = value
                    pcs_out = os.path.split(self.pcs_file)[1]
                    self.write("paramfile = %s" %(pcs_out))
                elif line.startswith("overall_obj"):
                    val = line.split("=")[1].strip(" ")
                    if val == "mean10":
                        val = "PAR10"
                    self.write("overall-obj = %s" %(val))
                elif param in self.convert_dir:
                    self.write("%s = %s" %(self.convert_dir[param], value))
                else:
                    self.write(line)
                    self.logger.warn("Not found %s" %(line))
        
            
        #for fn in self.instance_files:
        #    self.remove_instance_specifics(fn)
        
        if self.out_fp:
            self.out_fp.close()
            
        pcsconverter = PCSAClibConverter(os.path.join(self.out_dir, self.pcs_file), os.path.join(self.out_dir, pcs_out))
        pcsconverter.convert()            
        
    def write(self, str):
        if not self.out_fp:
            print(str)
        else:
            self.out_fp.write(str+"\n")

    def convert_back(self):
        '''
            convert from new aclib2 format (self.fn) into old (SMAC) format (self.out_fn) 
        '''
        
        with open(fn) as fp:
            for line in fp:
                line = line.replace("\n","")
                param, value = list(map(lambda x : x.strip(" "), line.split("=")))
                
                self.write("execdir = \".\"")
                
                algo_call = []
                if param  in ["algo", "args"]:
                    algo_call.append(value)
                elif line.startswith("paramfile"):
                    self.pcs_file = line.split("=")[1].strip(" ")
                    pcs_out = os.path.split(self.pcs_file)[1]
                    self.write("paramfile = %s" %(pcs_out))
                elif param in self.back_convert:
                    self.write("%s = %s" %(self.back_convert[param], value))
                else:
                    self.write(line)
                    self.logger.warn("Not found %s" %(line))

                self.write("algo = %s" %(" ".join(algo_call)))
                
        if self.out_fp:
            self.out_fp.close()
            
        pcsconverter = PCSAClibBackConverter(os.path.join(self.out_dir, self.pcs_file), os.path.join(self.out_dir, pcs_out))
        pcsconverter.convert()

if __name__ == '__main__':
    
    argparse = argparse.ArgumentParser()
    argparse.add_argument("-f","--file", required=True, help="scenario file in SMAC format")
    argparse.add_argument("-o","--out", default=None, help="output file of new scenario file (if not set, stdout)")
    argparse.add_argument("-b","--back", default=False, action="store_true", help="convert scenario back to SMAC format")
    
    args = argparse.parse_args()
    
    converter = ScenarioAClibConverter(args.file, args.out)
    if not args.back:
        converter.convert()
    else:
        converter.convert_back()
    