#!/bin/python

'''
    pcs2aclib.py - converts the new AClib pcs format into the "old" (SMAC) pcs format

    @author: Marius Lindauer
'''

import sys
import os
import shutil
import re
import argparse
import tempfile

class PCSAClibBackConverter(object):
    
    def __init__(self, pcs_fn, out_fn):
        '''
            PCS Convert
            
            Arguments
            ---------
            pcs_fn: str
                parameter configuration space filename in SMAC (2.08) format
            out_fn: str
                output file name (if not set, used stdout)
        '''
        self.pcs_fn = pcs_fn
        if out_fn:
            self.out_fp = open(out_fn,"w")
        else:
            self.out_fp = None
        
        num_regex = "[+\-]?(?:0|[1-9]\d*)(?:\.\d*)?(?:[eE][+\-]?\d+)?"
        self.CONT_REGEX = re.compile("^[ ]*(?P<name>[^ ]+)[ ]*(?P<type>\w*)[ ]*\[[ ]*(?P<range_start>%s)[ ]*,[ ]*(?P<range_end>%s)\][ ]*\[(?P<default>%s)\](?P<log>.*)$" %(num_regex, num_regex, num_regex))
        self.CAT_REGEX = re.compile("^[ ]*(?P<name>[^ ]+)[ ]*categorical[ ]*{(?P<values>.+)}[ ]*\[(?P<default>[^#]*)\](?P<misc>.*)$")
        self.COND_REGEX = re.compile("^[ ]*(?P<name>[^ ]+)[ ]*\|[ ]*(?P<condition>.*)$")
        self.FORBIDDEN_REGEX = re.compile("^[ ]*{(?P<values>.+)}(?P<misc>.*)*$")
        
        self.param2conditionals = {}
        
    def convert(self):
        '''
            convert self.pcs_fn and output to either self.out_fn or stdout
        '''
        
        with open(self.pcs_fn) as fp:
            for line in fp:
                
                if line.startswith("Conditionals"):
                    continue
                if line.startswith("Forbidden"):
                    continue
                
                line = line.strip("\n")
                #remove comments
                comment = ""
                if line.find("#") > -1:
                    comment = line[line.find("#"):]
                    line = line[:line.find("#")] # remove comments
                    
                line = line.strip(" ")
                
                if line == "":
                    continue

                cont_match = self.CONT_REGEX.match(line) 
                cat_match = self.CAT_REGEX.match(line)
                cond_match = self.COND_REGEX.match(line)
                forbidden_match = self.FORBIDDEN_REGEX.match(line) 

                if cont_match:
                    self._convert_cont_param(cont_match, comment)
                elif cat_match:
                    self._convert_cat_param(cat_match, comment)
                elif cond_match:
                    self._print_cond(cond_match)
                elif forbidden_match: # no conversion for forbidden clauses necessary
                    self._write(line)
                else:
                    raise ValueError("Cannot parse: %s" %(line))
                    
        if self.out_fp:
            self.out_fp.close()
        
    def _convert_cont_param(self, match, comment):
        '''
            param real [min,max][def]log -> param [min,max][def]l  
            param integer [min,max][def]log -> param [min,max][def]il 
        '''
        misc = ""
        if "integer" == match.group("type"):
            misc += "i"
        elif "real" == match.group("type"):
            pass
        else:
            raise ValueError("unknown parameter type: %s" %(match.group("type")))
            
        if "log" in match.group("log"):
            misc += "l" 
            
        str = "%s [%s,%s][%s]%s" %(match.group("name"), match.group("range_start"), match.group("range_end"), match.group("default"), misc)
        self._write(str + " " + comment)
        
    def _convert_cat_param(self, match, comment):
        '''
            param categorical {v1,v2,v3}[def] -> param {v1,v2,v3}[def]
        '''
        str = "%s {%s}[%s]" %(match.group("name"), match.group("values"), match.group("default"))
        self._write(str + " " + comment)

    def _print_cond(self, match):
        '''
            child | parent_1 in {v1,v2,v3} && parent_2 in {v1,v2,v3}
            ->
            child | parent_1 in {v1,v2,v3}
            child | parent_2 in {v1,v2,v3}
        '''
        if "||" in match.group("condition"):
            raise ValueError("|| is not supported in old format.")

        conds = match.group("condition").split("&&")
        for c in conds:
            c = c.strip(" ")
            if " in " not in c:
                raise ValueError("Other conditional operators except \"in\" is not supported in the old format")
            self._write("%s | %s" %(match.group("name"), c))

    def _write(self, str):
        if not self.out_fp:
            print(str)
        else:
            self.out_fp.write(str+"\n")

if __name__ == '__main__':
    
    argparse = argparse.ArgumentParser()
    argparse.add_argument("-f","--file", required=True, help="pcs file in AClib2 format")
    argparse.add_argument("-o","--out", default=None, help="output file for old SMAC format; if not set, using stdout")
    
    args = argparse.parse_args()
    
    converter = PCSAClibBackConverter(args.file, args.out)
    converter.convert()
    
    