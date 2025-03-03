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
import json

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

        self.parser.add_argument("--obj", action="append", default=["cost"], dest="obj")

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

        cmd = ["python", "\"%s/svm_run.py\"" % (self.__script_dir), runargs["instance"], os.path.abspath(self.args.dataset), str(runargs["seed"])]
        print(f"{config=}")
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
        print(data)
        resultMap = {}

        accResult = re.search("Error:[ ]+(%s)" % (self.float_regex()), data)

        exit_code = out_args["exit_code"]

        resultMap["cost"] = 1.0 if len(self.args.obj) == 1 else {}


        if accResult:
            resultMap['status'] = 'SUCCESS'

            if len(self.args.obj) == 1:
                resultMap["cost"] = accResult.group(1)
            else:
                for score in ["accuracy", "precision", "recall", "f1"]:
                    m = re.search(f"{score}_score ({self.float_regex()})", data)
                    cost = float(m.group(1)) if m else 1.0
                    resultMap["cost"][score] = cost

                m = re.search(f"cache_size ({self.float_regex()})", data)
                cost = float(m.group(1)) if m else 2048.0
                resultMap["cost"]["memory"] = cost

                m = re.search(f"fittime ({self.float_regex()})", data)
                cost = float(m.group(1)) if m else 7200.0
                resultMap["cost"]["fittime"] = cost

        return resultMap

    def print_result_string(self):
        '''
            print result in old ParamILS format
            and also in new AClib format
              if it new call string format was used
        '''

        # ensure a minimal runtime of 0.0005
        self.data.time = max(0.0005, self.data.time)

        if self.args.overwrite_cost_runtime:
            self.data.cost = self.data.time

        print(self.args.obj)

        if len(self.args.obj) == 1:
            cost = float(self.data.cost)
            cost_str = f"{cost}"
        elif len(self.args.obj) == 2:
            cost = self.data.cost.get(self.args.obj[1], None)
            if cost is not None:
                cost = float(cost)
            cost_str = f"{cost}"
        else:
            self.data.cost["runtime"] = self.data.time  # add to cost metric
            cost = [self.data.cost.get(obj, None) for obj in self.args.obj[1:]]
            cost_str = "[{}]".format(", ".join([str(c) for c in cost]))

        if self.data.new_format:
            aclib2_out_dict = {"status": str(self.data.status),
                               "cost": cost,
                               "runtime": float(self.data.time),
                               "misc": str(self.data.additional)
                               }
            print("Result of this algorithm run: %s" %
                  (json.dumps(aclib2_out_dict)))

        sys.stdout.write(f"Result: {self.data.status}, {self.data.time:.4f}, {cost_str}, {self.data.seed}")
        #sys.stdout.write(f"Result for ParamILS: {self.data.status}, {self.data.time:.4f}, {self.data.runlength}, {cost_str}, {self.data.seed}")

        if self.data.additional != "":
            sys.stdout.write(", %s" % (self.data.additional))
        sys.stdout.write("\n")
        sys.stdout.flush()

        #Result for SMAC3v2..
        cost_str = ",".join([str(c) for c in cost]) if isinstance(cost, list) else cost
        sys.stdout.write(f"Result for SMAC3v2: status={self.data.status};cost={cost_str};runtime={self.data.time};additional_info={self.data.additional}")

if __name__ == "__main__":
    wrapper = XGBoostWrapper()
    wrapper.main()
