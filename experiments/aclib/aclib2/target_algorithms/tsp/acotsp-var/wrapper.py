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
        
        self.parser.add_argument("--opt_fn", dest="opt_fn", default=None, help="file with lower bounds")

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
        cmd = "target_algorithms/tsp/acotsp-var/src/acotsp --tries 1 --quiet --mmas --localsearch 1 --alpha 1  -i %s " %(runargs["instance"])

        for key, value in config.items():
            if key == "-algorithm":
                cmd += "--%s " %(value)
            else:
                key = key.replace("start-","")
                cmd += "-%s %s " %(key, value)

        cmd += "--time %d --seed %d" %(runargs["cutoff"], runargs["seed"])
        
        # remember instance name and cutoff
        self._instance_fn = runargs["instance"]
        self._cutoff = runargs["cutoff"]

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
        
        norm_bin="target_algorithms/tsp/acotsp-var/mo-tools/nondominated"
        hv_bin="target_algorithms/tsp/acotsp-var/hv/hv"
        
        with open(self.args.opt_fn) as fp:
            lower_bound = float(re.search("%s (\d+.\d+)" %(os.path.split(self._instance_fn)[1]), fp.read()).group(1))
            print(lower_bound)
            upper_bound = lower_bound + (lower_bound * 0.15)
            print(upper_bound)
        
        resultMap = {}
        filepointer.seek(0)
        data = str(filepointer.read())
        costResult = re.search("try (\d+), Best (\d+), found at iteration (\d+), found at time (\d+.\d+)", 
                               data)
        
        if costResult:
            qual = int(costResult.group(2)) 
            time = float(costResult.group(4))
            
            norm_cmd = "echo \"%d %d\" | " \
                        "%s --force-bound --filter --upper-bound \"%d %d\" --lower-bound \"0 %d\" --quiet -n \"0.0 0.9\" | " \
                        "%s --quiet -r \"1.0 1.0\"" %(
                time, qual, norm_bin, self._cutoff, upper_bound, lower_bound, hv_bin)
            print(norm_cmd)
            p = Popen(norm_cmd, shell=True, stdout=PIPE)
            (out_, err_) = p.communicate()
            print(out_)
            
            resultMap["quality"] = -1 * float(out_)
            resultMap["status"] = "SUCCESS"

        return resultMap

if __name__ == "__main__":
    wrapper = ACOTSPWrapper()
    wrapper.main()
