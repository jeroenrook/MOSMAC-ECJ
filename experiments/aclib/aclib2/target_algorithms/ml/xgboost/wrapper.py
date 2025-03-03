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

class XGBoostWrapper(AbstractWrapper):
    '''
        Simple wrapper for LPG
    '''
    def __init__(self):
        AbstractWrapper.__init__(self)
        self.__script_dir = os.path.abspath(os.path.split(__file__)[0])

        self.parser.add_argument("--dataset", dest="dataset", default=None, help="dataset to use")

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

        cmd = ["python","\"%s/xgboost_run.py\"" % (self.__script_dir), runargs["instance"], os.path.abspath(self.args.dataset), str(runargs["seed"])]

        for key, value in config.items():
            cmd.extend([key, value])

        return " ".join(cmd)

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
        data = str(filepointer.read())
        resultMap = {}

        accResult = re.search("Error:[ ]+(%s)" % (self.float_regex()), data)

        exit_code = out_args["exit_code"]

        if accResult:
            resultMap['status'] = 'SUCCESS'
            resultMap['quality'] = accResult.group(1)

        return resultMap

if __name__ == "__main__":
    wrapper = XGBoostWrapper()
    wrapper.main()
