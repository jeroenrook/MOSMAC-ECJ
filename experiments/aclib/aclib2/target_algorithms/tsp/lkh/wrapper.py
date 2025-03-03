#!/usr/bin/env python2.7
# encoding: utf-8

'''
LKHWrapper -- ACLib Target algorithm warpper for TSP solver LKH

@author:     Marius Lindauer, Chris Fawcett, Alex Fréchette, Frank Hutter, Yasha Pushak
@copyright:  2014 AClib. All rights reserved.
@license:    GPL
@contact:    lindauer@informatik.uni-freiburg.de, fawcettc@cs.ubc.ca, afrechet@cs.ubc.ca, fh@informatik.uni-freiburg.de, ypushak@cs.ubc.ca
'''

from genericWrapper4AC.generic_wrapper import AbstractWrapper

import string, random
import os
import re

class LKHWrapper(AbstractWrapper):
    '''
        Simple wrapper for a TSP solver (LKH)
    '''

    def __init__(self):
        self.opt = 0
        self.ID = 'r' + self.generateID()    
        super(LKHWrapper,self).__init__()
        self.parser.add_argument("--sol-file", dest="solution_quality_file", default=None, help="File with \"<instance> - {optimum|bestknown} - <solution quality>\" ")



    def generateID(self, size=6, chars=string.ascii_uppercase + string.digits):
        #generate a random ID for identifying SMAC runs
        return ''.join(random.choice(chars) for _ in range(size))


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
        tmpdir = os.path.abspath(runargs["tmp"])       

        #YP: This will correctly handle the "__parent__" hyperparameters and __child_x__ hyperparameters
        #introduced into the
        #Parameter configuration space in order to allow GPS to properly interpret which parameters
        #should be searched numerically and which categorically, particularly in the event that a
        #parameter that is mostly numerical has special meaning encoded in some values (e.g., 0 = off)
        for name, value in config.items():
            if("__parent__" in name):
                if(value == 'on'):
                    #The child is turned on, so we can just remove this parent parameter
                    del config[name]
                else:
                    #The child is set to one of it's "categorical" parameter values. So we
                    #artificially create such a parameter and remove the parent.
                    newName = name.replace('__parent__','')
                    config[name.replace('__parent__','')] = value
                    del config[name]
        #Now we handle the __child_x__ parameters
        for name, value in config.items():
            if("__child_" in name):
                hptext = '__child_' + name.split('__child_')[-1].split('__')[0] + '__'
                #Here, GPS will already have handled for us whether or not this parameter is turned on. Hence, all 
                #We have to do is remove the __child_x__ part of the parameter name. 
                config[name.replace(hptext,'')] = value
                del config[name]



        with open(tmpdir + '/' + self.ID + '.par','w',1) as f_out:

            f_out.write('PROBLEM_FILE = ' + runargs['instance'] + '\n')

            #Get the size and problem instance being run
            inst = runargs["instance"].split("/")
            size = int(inst[len(inst)-1].split("-")[0])
            num = int(inst[len(inst)-1].split("-")[1].split(".")[0])

            if(not self.args.solution_quality_file):
                raise ValueError("Solution quality file is required.")
            elif(not os.path.isfile(self.args.solution_quality_file)):
                raise ValueError("Solution quality file not found.")

            #Get the optimal solution quality of the run
            with open(self.args.solution_quality_file) as f:
                for line in f:
                    if line.split(" - ")[0] == str(size) + '-' + str(num):
                        self.opt = int(line.split(" - ")[2])
                        break

            f_out.write('OPTIMUM = ' + str(self.opt) + '\n')
            
            #Set the random seed
            f_out.write('SEED = ' + str(runargs['seed']) + '\n')
            #Set the time limit
            #Note that this is the time limit assigned to each restart segment,
            #hence it probably won't do much. However, it doesn't hurt to have
            #it in here in case the algorithm is not stopping for an unknown
            #reason. 
            f_out.write('TIME_LIMIT = ' + str(runargs['cutoff']) + '\n')
            #Set the number of runs to a really large number
            f_out.write('RUNS = 999999999\n')
            #This is the default, but I'm including it for clarity:
            f_out.write('STOP_AT_OPTIMUM = YES\n')
            #Write all of the parameters
            for param, value in config.items():
                #Remove the dash from the front of the parameter name
                param = param[1:]
                if(param == 'KICK_WALK'):
                    if(value == 'YES'):
                        #This is a special categorical parameter I put in
                        #The original way to specify the special kick strategy
                        #is with zeros
                        f_out.write('KICK_TYPE = 0\n')
                        f_out.write('KICKS = 0\n')
                elif(param == 'SAME_MOVE_TYPE'):
                    if(value == 'YES'):
                        #Also a categorical parameter I put in
                        f_out.write('SUBSEQUENT_MOVE_TYPE = 0\n')
                else:
                    #Write the parameter and the value to the file
                    f_out.write(param + ' = ' + value + '\n')
        
        cmd = './target_algorithms/tsp/lkh/source/LKH ' + tmpdir + '/' + self.ID + '.par'

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
        #resultMap['misc'] = data.replace(',',';')
        
        if (re.search('Gap.min = 0.0000%', data) and re.search('Cost.min = ' + str(self.opt),data)):
            resultMap['status'] = 'SUCCESS'
            #resultMap['runtime'] = numbers[4]
        elif (re.search('Time limit exceeded',data) or re.search('Maximum CPU time exceeded', watcherData)):
            resultMap['status'] = 'TIMEOUT'
        elif(re.search('Error', data) or re.search('Aborted', data)):
            resultMap['status'] = 'CRASHED'
            #resultMap['runtime'] = 0
        elif(re.search('Maximum VSize exceeded', watcherData)):
            resultMap['status'] = 'CRASHED'
            resultMap['misc'] = 'Exceeded memory limit'
        else:
            resultMap['status'] = 'CRASHED'
       
        return resultMap

if __name__ == "__main__":
    wrapper = LKHWrapper()
    wrapper.main()    
