#!/usr/bin/env python2.7
# encoding: utf-8

'''
dimetheusWrapper -- ACLib Target algorithm warpper for SAT solver dimetheus

@author:     Marius Lindauer, Chris Fawcett, Alex Fréchette, Frank Hutter, Yasha Pushak
@copyright:  2014 AClib. All rights reserved.
@license:    GPL
@contact:    lindauer@informatik.uni-freiburg.de, fawcettc@cs.ubc.ca, afrechet@cs.ubc.ca, fh@informatik.uni-freiburg.de, ypushak@cs.ubc.ca
'''

from genericWrapper4AC.generic_wrapper import AbstractWrapper
from genericWrapper4AC.domain_specific.satwrapper import SatWrapper

import random
import string
import os

class Cadical_Wrapper(SatWrapper):
    '''
        Simple wrapper for a SAT solver (dimetheus)
    '''

    def __init__(self):
        SatWrapper.__init__(self)


    def get_command_line_args(self, runargs, config):
        '''
        Returns the command line call string to execute the target algorithm (here: cadical).
        Args:
            runargs: a map of several optional arguments for the execution of the target algorithm.
                    {
                      "instance": <instance>,
                      "specifics" : <extra data associated with the instance>,
                      "cutoff" : <runtime cutoff>,
                      "runlength" : <runlength cutoff>,
                      "seed" : <seed>
                    }
            config: a mapping from parameter name to parameter value
        Returns:
            A command call list to execute the target algorithm.
        '''
      
        binary_path =  "./target_algorithms/sat/cadical-sc17/cadical-sc17-proof/bin/cadical"

        paramString = ''

        
        for name, value in config.items():
            #The parameter names have ' -' prepended to them, so we keep this.
            #CaDiCaL can only parse scientific notation done with a lower case e..
            try:
                value = str(float(value))  # converts from notation 1e9 to 1000000000.0
                # value = value.replace('E','e')
                if (float(value) == int(float(value))):
                    value = str(int(float(value)))  # Does nothing, we just want to keep it as an integer if it can be one and only reformat floats.
            except:
                pass

            paramString += ' -' + name + '=' + value
           
        cmd = binary_path + paramString + ' ' + runargs['instance']
        
        return cmd

if __name__ == "__main__":
    wrapper = Cadical_Wrapper()
    wrapper.main()    
