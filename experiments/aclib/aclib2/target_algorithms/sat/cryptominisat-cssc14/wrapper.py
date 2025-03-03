from genericWrapper4AC.generic_wrapper import AbstractWrapper
from genericWrapper4AC.domain_specific.satwrapper import SatWrapper
import re
import sys
import json

class CM_Wrapper(SatWrapper):
    
    def __init__(self):
        SatWrapper.__init__(self)

        self.parser.add_argument("--obj", action="append", default=["PAR10"], dest="obj")
    
    def get_command_line_args(self, runargs, config):
        '''
        @contact:    lindauer@informatik.uni-freiburg.de, fh@informatik.uni-freiburg.de
        Returns the command line call string to execute the target algorithm (here: glucose).
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
        solver_binary = "/home/rook/projects/mosmac/experiments/aclib/aclib2/target_algorithms/sat/cryptominisat-cssc14/cryptominisat"
    
        # Construct the call string to glucose.
        #cmd = "%s %s -rnd-seed=%d -cpu-lim=%d" % (solver_binary, runargs["instance"], runargs["seed"], runargs["cutoff"])
        cmd = "%s -r %d" % (solver_binary, runargs["seed"])

        dashes = "-" if list(config.keys())[0][0] == "-" else "--"
        for name, value in config.items():
            cmd += " %s%s %s" % (dashes, name,  value)
        
        cmd += " %s" %(runargs["instance"])
    
        # remember instance and cmd to verify the result later on
        self._instance = runargs["instance"] 
        self._cmd = cmd
    
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
        resultMap = super().process_results(filepointer, out_args)

        filepointer.seek(0)
        data = str(filepointer.read())
        # print(data)
        print(f"{self.args.obj=}")

        if "memory" in self.args.obj:
            try:
                m = re.search("c Mem used[\s:]+(\d+)\s*MB", data)
                print(f"{m=}")
                cost = float(m.group(1))
                print(f"{cost=}")
                setattr(self.data, "memory_used", cost)
            except:
                pass

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

        print(self.data.cost)

        if len(self.args.obj) == 1:
            cost = float(self.data.cost)
            cost_str = f"{cost}"
        else:
            self.data.cost = {}
            self.data.cost["runtime"] = self.data.time  # add to cost metric
            self.data.cost["PAR10"] = self.data.cost["runtime"]
            if self.data.cost["PAR10"] >= self.data.cutoff or self.data.status == "TIMEOUT":
                self.data.cost["PAR10"] = 10 * self.data.cutoff  # add to cost metric
            self.data.cost["memory"] = self.data.memory_used
            self.data.cost["solved"] = 0 if self.data.status == "SUCCESS" else 1  # we want to minimise to solved=0, not-solved=1

            if len(self.args.obj) == 2:
                cost = self.data.cost.get(self.args.obj[1], None)
                if cost is not None:
                    cost = float(cost)
                cost_str = f"{cost}"
            else:
                cost = [self.data.cost.get(obj, None) for obj in self.args.obj[1:]]
                cost_str = "[{}]".format(", ".join([str(c) for c in cost]))

        print(f"{self.data.cost=}")
        print(f"{cost=}")
        print(f"{cost_str=}")

        if self.data.new_format:
            aclib2_out_dict = {"status": str(self.data.status),
                               "cost": cost,
                               "runtime": float(self.data.time),
                               "misc": str(self.data.additional)
                               }
            print("Result of this algorithm run: %s" %
                  (json.dumps(aclib2_out_dict)))

        sys.stdout.write(f"Result: {self.data.status}, {self.data.time:.4f}, {cost_str}, {self.data.seed}")
        # sys.stdout.write(f"Result for ParamILS: {self.data.status}, {self.data.time:.4f}, {self.data.runlength}, {cost_str}, {self.data.seed}")

        if self.data.additional != "":
            sys.stdout.write(", %s" % (self.data.additional))
        sys.stdout.write("\n")
        sys.stdout.flush()

        # Result for SMAC3v2..
        cost_str = ",".join([str(c) for c in cost]) if isinstance(cost, list) else cost
        sys.stdout.write(
            f"Result for SMAC3v2: status={self.data.status};cost={cost_str};runtime={self.data.time};additional_info={self.data.additional.replace(';','')}")

    def read_runsolver_output(self):
        super().read_runsolver_output()
        '''
            reads self._watcher_file, 
            extracts runtime
            and returns if memout or timeout found
        '''

        self.logger.debug("Reading runsolver output from %s" %
                          (self._watcher_file.name))
        try:
            self._watcher_file.seek(0, 0)  # Reset read pointer
            data = str(self._watcher_file.read().decode("utf8"))
        except:
            # due to the common, rare runsolver bug,
            # the watcher file can be corrupted and can failed to be read
            self.data.exit_code = 0
            self.logger.warn(
                "Failed to read runsolver's watcher file---trust own wc-time measurment")
            return

        m = re.search("Max\. memory \(cumulated for all children\) \(KiB\): (\d+)", data)
        memory_used = 2**32
        if m:
            memory_used = int(m.group(1)) / 1024 #MB

        setattr(self.data, "memory_used", memory_used)

if __name__ == "__main__":
    wrapper = CM_Wrapper()
    wrapper.main()    
