#!/usr/bin/env python2.7
# encoding: utf-8

'''
EAXWrapper -- ACLib Target algorithm warpper for TSP solver EAX

@author:     Marius Lindauer, Chris Fawcett, Alex Fréchette, Frank Hutter, Yasha Pushak
@copyright:  2014 AClib. All rights reserved.
@license:    GPL
@contact:    lindauer@informatik.uni-freiburg.de, fawcettc@cs.ubc.ca, afrechet@cs.ubc.ca, fh@informatik.uni-freiburg.de, ypushak@cs.ubc.ca

'''

from genericWrapper4AC.generic_wrapper import AbstractWrapper

import os
import re

class EAXWrapper(AbstractWrapper):
    '''
        Simple wrapper for a TSP solver (EAX)
    '''

    def __init__(self):
        self.opt = 0    
        super(EAXWrapper,self).__init__()
        self.parser.add_argument("--sol-file", dest="solution_quality_file", default=None, help="File with \"<instance> - {optimum|bestknown} - <solution quality>\" ")



    def get_command_line_args(self, runargs, config):
        '''
        Returns the command line call string to execute the target algorithm (here: EAX).
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
      
        binary_path = "./target_algorithms/tsp/eax/source/jikken"
        tmpdir = os.path.abspath(runargs["tmp"])

        if(not self.args.solution_quality_file):
            raise ValueError("Solution quality file is required.")
        elif(not os.path.isfile(self.args.solution_quality_file)):
            raise ValueError("Solution quality file not found.")

        #Get the size and problem instance being run
        inst = runargs["instance"].split("/")
        size = int(inst[len(inst)-1].split("-")[0])
        num = int(inst[len(inst)-1].split("-")[1].split(".")[0])
        #Get the optimal solution quality of the run
        with open(self.args.solution_quality_file) as f:
            for line in f:
                if line.split(" - ")[0] == str(size) + '-' + str(num):
                    self.opt = int(line.split(" - ")[2])
                    break
 
        #Begin creating the command line call: binary, # of runs, output file
        cmd = "%s 1 %s/DATA-%s.txt " %(binary_path, tmpdir, str(size) + '-' + str(num) )                
 
        for name, value in config.items():
            if(name == '-Npop'):
                Npop = value
            elif(name == '-Nch'):
                Nch = value
            else:
                raise ValueError('Unknown parameter name: ' + name + ', with value: ' + str(value))
        
        cmd += Npop + ' ' + Nch
        
        #Problem instance, optimal solution quality, cutoff time, random number seed
        cmd += " %s %s %s %s" %(runargs["instance"], self.opt, runargs["cutoff"], runargs["seed"])
        
        return cmd

    def process_results(self, filepointer, out_args):
        '''
        Parse a results file to extract the run's status (SUCCESS/CRASHED/etc) and other optional results.
    
        Args:
            filepointer: a pointer to the file containing the solver execution standard out.
            out_args : a map with {"exit_code" : exit code of target algorithm} 
        Returns:
            A map containing the standard AClib run results. The current standard result map as of AClib 2.06 is:
            {
                "status" : <"SUCCESS"/"SAT"/"UNSAT"/"TIMEOUT"/"CRASHED"/"ABORT">,
                "runtime" : <runtime of target algrithm>,
                "quality" : <a domain specific measure of the quality of the solution [optional]>,
                "misc" : <a (comma-less) string that will be associated with the run [optional]>
            }
            ATTENTION: The return values will overwrite the measured results of the runsolver (if runsolver was used). 
        '''  
        data = str(filepointer.read())
        self._watcher_file.seek(0)
        watcherData = str(self._watcher_file.read())
        self._watcher_file.seek(0)
        resultMap = {}

        resultMap['misc'] = 'Number of Restarts: ' + str(data.count('hh: restarting'))
        
        if (re.search('best value found', data) and re.search(str(self.opt),data)):
            resultMap['status'] = 'SUCCESS'
            #resultMap['runtime'] = numbers[4]
        elif (re.search('larger than cut-off time',data) or re.search('Maximum CPU time exceeded', watcherData)):
            resultMap['status'] = 'TIMEOUT'
        elif(re.search('exception', data) or re.search('Aborted', data)):
            resultMap['status'] = 'CRASHED'
            #resultMap['runtime'] = 0
        elif(re.search('Maximum VSize exceeded', watcherData)):
            resultMap['status'] = 'CRASHED'
            resultMap['misc'] =  ' - Exceeded memory limit'
        else:
            resultMap['status'] = 'CRASHED'
           
        
        return resultMap

if __name__ == "__main__":
    wrapper = EAXWrapper()
    wrapper.main()    
