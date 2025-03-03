import sys
import re
import os

from subprocess import Popen, PIPE

from genericWrapper4AC.generic_wrapper import AbstractWrapper

class ACOTSPWrapper(AbstractWrapper):
    '''
        Simple wrapper for ACOTSPWrapper
    '''
    def __init__(self):
        AbstractWrapper.__init__(self)
        self.__script_dir = os.path.abspath(os.path.split(__file__)[0])

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
        cmd = "target_algorithms/tsp/acotsp/ACOTSP-1.03-tuning/acotsp --tries 1 --quiet -i %s " %(runargs["instance"])

        for key, value in config.items():
            if key == "-algorithm":
                cmd += "--%s " %(value)
            else:
                cmd += "-%s %s " %(key, value)

        cmd += "--time %d --seed %d" %(runargs["cutoff"], runargs["seed"])

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
                "status" : <"SAT"/"UNSAT"/"TIMEOUT"/"CRASHED"/"ABORT">,
                "runtime" : <runtime of target algrithm>,
                "quality" : <a domain specific measure of the quality of the solution [optional]>,
                "misc" : <a (comma-less) string that will be associated with the run [optional]>
            }
            ATTENTION: The return values will overwrite the measured results of the runsolver (if runsolver was used). 
        '''
        resultMap = {}
        filepointer.seek(0)
        data = str(filepointer.read())
        costResult = re.search("try (\d+), Best (\d+), found at iteration (\d+), found at time (\d+.\d+)", 
                               data)
        
        if costResult:
            resultMap["quality"] = str(int(costResult.group(2)) / 1000) 
            resultMap["status"] = "SUCCESS"

        return resultMap

if __name__ == "__main__":
    wrapper = ACOTSPWrapper()
    wrapper.main()
