#!/bin/python

'''
    pcs2aclib.py - converts the old pcs format of SMAC to the new common AClib format

    @author: Marius Lindauer
'''

import sys
import os
import shutil
import re
import argparse
import tempfile

class PCSAClibConverter(object):
    
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
        self.CONT_REGEX = re.compile("^[ ]*(?P<name>[^ ]+)[ ]*\[[ ]*(?P<range_start>%s)[ ]*,[ ]*(?P<range_end>%s)\][ ]*\[(?P<default>%s)\](?P<misc>.*)$" %(num_regex,num_regex, num_regex))
        self.CAT_REGEX = re.compile("^[ ]*(?P<name>[^ ]+)[ ]*{(?P<values>.+)}[ ]*\[(?P<default>[^#]*)\](?P<misc>.*)$")
        self.COND_REGEX = re.compile("^[ ]*(?P<cond>[^ ]+)[ ]*\|[ ]*(?P<head>[^ ]+)[ ]*in[ ]*{(?P<values>.+)}(?P<misc>.*)$")
        self.FORBIDDEN_REGEX = re.compile("^[ ]*{(?P<values>.+)}(?P<misc>.*)*$")
        
        self.param2conditionals = {}
        self.forbiddens = []
        
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

                cont_match = self.CONT_REGEX.match(line) 
                cat_match = self.CAT_REGEX.match(line)
                cond_match = self.COND_REGEX.match(line)
                forbidden_match = self.FORBIDDEN_REGEX.match(line) 

                if cont_match:
                    self._convert_cont_param(cont_match, comment)
                elif cat_match:
                    self._convert_cat_param(cat_match, comment)
                elif cond_match:
                    self._save_cond(cond_match)
                elif forbidden_match: # no conversion for forbidden clauses necessary
                    self.forbiddens.append(line+comment)
                else:
                    self._write(line+comment)
                    
        self.print_cond()
        
        # ensure that forbiddens are printed at the end
        for line in self.forbiddens:
            self._write(line) 
        
        if self.out_fp:
            self.out_fp.close()
        
    def _convert_cont_param(self, match, comment):
        '''
            param [min,max][def]l -> param r [min,max][def]log
            param [min,max][def]il -> param i [min,max][def]log
        '''
        if "i" in match.group("misc"):
            type = "integer"
        else:
            type = "real"
        if "l" in match.group("misc"):
            log = "log"
        else:
            log = ""
            
        str = "%s %s [%s,%s][%s] %s" %(match.group("name"), type, match.group("range_start"), match.group("range_end"), match.group("default"), log)
        self._write(str + " " + comment)
        
    def _convert_cat_param(self, match, comment):
        '''
            param {v1,v2,v3}[def] -> param c {v1,v2,v3}[def]
        '''
        str = "%s categorical {%s}[%s]" %(match.group("name"), match.group("values"), match.group("default"))
        self._write(str + " " + comment)

    def _save_cond(self, match):
        '''
            child | parent_1 in {v1,v2,v3}
            child | parent_2 in {v1,v2,v3}
            ->
            child | parent_1 in {v1,v2,v3} && parent_2 in {v1,v2,v3}
        '''
        
        if self.param2conditionals.get(match.group("cond")):
            self.param2conditionals[match.group("cond")] += " && %s in {%s}" %(match.group("head"), match.group("values"))
        else:
            self.param2conditionals[match.group("cond")] = "%s in {%s}" %(match.group("head"), match.group("values"))
        
    def print_cond(self):
        for child, cond in self.param2conditionals.items():
            str = "%s | %s" %(child, cond)
            self._write(str)

    def _write(self, str):
        if not self.out_fp:
            print(str)
        else:
            self.out_fp.write(str+"\n")

if __name__ == '__main__':
    
    argparse = argparse.ArgumentParser()
    argparse.add_argument("-f","--file", required=True, help="pcs file in SMAC format")
    argparse.add_argument("-o","--out", default=None, help="output file of new pcs format; if not set, using stdout")
    
    args = argparse.parse_args()
    
    converter = PCSAClibConverter(args.file, args.out)
    converter.convert()
    
    