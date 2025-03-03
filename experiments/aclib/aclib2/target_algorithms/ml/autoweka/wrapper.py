#!/usr/bin/env python2.7
# encoding: utf-8

'''
lpg-wrapper -- Target algorithm wrapper for the version of LPG used in the ICAPS'2013 paper 'Improved Features for Runtime Prediction of Domain-Independent Planners'

@author:     Chris Fawcett
@copyright:  2014 Chris Fawcett. All rights reserved.
@license:    BSD
@contact:    fawcettc@cs.ubc.ca

example call:
python <path to wrapper>/lpg-wrapper.py <instance> "<domain>" <runtime cutoff> <runlength cutoff> <seed>
'''

import sys
import re
import os

from subprocess import Popen, PIPE

from genericWrapper4AC.generic_wrapper import AbstractWrapper

class AutoWekaWrapper(AbstractWrapper):
    '''
        Simple wrapper for LPG
    '''
    def __init__(self):
        AbstractWrapper.__init__(self)
        self.__script_dir = os.path.abspath(os.path.split(__file__)[0])

        self.parser.add_argument("--dataset", dest="dataset", default=None, help="dataset to use")
        self.parser.add_argument("--max_error", dest="max_error", default="2", help="maximal error")
        self.parser.add_argument("--class_index", dest="class_index", default=None, help="column index for class labels")

    def get_command_line_args(self, runargs, config):
        '''
        Returns the command line call string to execute the target algorithm
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

        # hack for irace to remove leading path
        runargs["instance"] = os.path.split(runargs["instance"])[1]
        call = "java -Dautoweka.infinity={5} -Xmx512m -cp target_algorithms/ml/autoweka/autoweka.jar autoweka.smac.SMACWrapper -prop datasetString=testArff=__dummy____COLONESCAPE__:classIndex={0}__COLONESCAPE__:type=trainTestArff__COLONESCAPE__:trainArff=instances/ml/data/{1}/{1}.arff:instanceGenerator=autoweka.instancegenerators.CrossValidation:resultMetric=errorRate -prop initialIncumbent=DEFAULT:acq-func=EI -wrapper {2} 0 {3} 123456789 {4}".format(self.args.class_index, self.args.dataset, runargs["instance"], runargs["cutoff"], runargs["seed"], self.args.max_error)

        for key, value in config.items():
            call += " %s \'%s\'" %(key, value)

        return call

    def process_results(self, filepointer, out_args):
        '''
        Parse a results file to extract the run's status (SUCCESS/CRASHED/etc) and other optional results.

        Args:
            filepointer: a pointer to the file containing the solver execution standard out.
            out_args : a map with {"exit_code" : exit code of target algorithm}
        Returns:
            A map containing the standard AClib run results. The current standard result map as of AClib 2.06 is:
            {
                "status" : <"SAT"/"UNSAT"/"TIMEOUT"/"CRASHED"/"ABORT">,
                "runtime" : <runtime of target algrithm>,
                "quality" : <a domain specific measure of the quality of the solution [optional]>,
                "misc" : <a (comma-less) string that will be associated with the run [optional]>
            }
            ATTENTION: The return values will overwrite the measured results of the runsolver (if runsolver was used). 
        '''
        resultMap = {"quality": self.args.max_error,
                     "status": "TIMEOUT"}
        
        for line in filepointer:
            line = line.decode()
            if line.startswith("Result for ParamILS: SAT"):
                resultMap["quality"] = line.split(",")[1]
                resultMap["status"] = "SAT"

        return resultMap

if __name__ == "__main__":
    wrapper = AutoWekaWrapper()
    wrapper.main()
