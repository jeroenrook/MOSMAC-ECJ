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

class LPGWrapper(AbstractWrapper):
    '''
        Simple wrapper for LPG
    '''
    def __init__(self):
        AbstractWrapper.__init__(self)
        self.__script_dir = os.path.abspath(os.path.split(__file__)[0])

        self.parser.add_argument("--solution-validator", dest="solution_validator", default=None, help="binary of the VAL plan validator")
        self.parser.add_argument("--problem-enc", dest="problem_enc", default=None, help="problem encoding / domain")

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

        self.__solution_file_path = "%s/planner_output" % (tmpdir)

        self.__instance_abs_path = os.path.abspath(runargs["instance"])
        self.__domain_abs_path = self.args.problem_enc

        cmd = ["\"%s/src/lpg\"" % (self.__script_dir), "-speed", "-seed", str(runargs["seed"]), "-o", "\"%s\"" %(self.__domain_abs_path), "-f", "\"%s\"" %(self.__instance_abs_path), "-out", "\"%s\"" %(self.__solution_file_path)]

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

        matchResult = re.search("Solution found:", data)
        costResult = re.search("Plan quality:[ ]+(%s)" % (self.float_regex()), data)
        restartExceededResult = re.search("Total restart limit reached, exiting.", data)

        exit_code = out_args["exit_code"]

        if matchResult and os.path.isfile(self.__solution_file_path):
            # check for validator, validate if present
            resultMap['status'] = 'SUCCESS'
            resultMap['quality'] = costResult.group(1)

            if (self.args.solution_validator):
                validator_abspath = os.path.abspath(self.args.solution_validator)

                cmd = ["\"%s\"" %(validator_abspath), "-S", "\"%s\"" %(self.__domain_abs_path), "\"%s\"" %(self.__instance_abs_path), "\"%s\"" %(self.__solution_file_path)]
                cmd = " ".join(cmd)
                io = Popen(cmd, stdout=PIPE, shell=True, universal_newlines=True)
                out_,err_ = io.communicate()

                failedResult = re.search('failed', out_)
                planCostResult = re.search("(%s)" % (self.float_regex()), out_)

                if (planCostResult and not failedResult):
                    resultMap['quality'] = planCostResult.group(1)
                    resultMap['misc'] = "plan verified by VAL"
                else:
                    resultMap['status'] = 'CRASHED'
                    resultMap['quality'] = "-1"
                    resultMap['misc'] = "produced plan was ruled invalid by VAL"
        elif exit_code == 0 and restartExceededResult:
            resultMap['status'] = 'TIMEOUT'
            resultMap['quality'] = "-1"
            resultMap['misc'] = "restart limit was exceeded"

        return resultMap

if __name__ == "__main__":
    wrapper = LPGWrapper()
    wrapper.main()
