import re

from genericWrapper4AC.generic_wrapper import AbstractWrapper
from genericWrapper4AC.domain_specific.satwrapper import SatWrapper
import json
import sys
import os

class Clasp_Wrapper(SatWrapper):
    
    def __init__(self):
        SatWrapper.__init__(self)

        self.parser.add_argument("--obj", action="append", default=["PAR10"], dest="obj")

    def get_command_line_args(self, runargs, config):
        '''
        @contact:    lindauer@informatik.uni-freiburg.de, fh@informatik.uni-freiburg.de
        Returns the command line call string to execute the target algorithm (here: Spear).
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
        solver_binary = "/home/rook/projects/mosmac/experiments/aclib/aclib2/target_algorithms/sat/clasp-cssc14/clasp"
        cmd = "%s %s --seed %d " %(solver_binary, runargs["instance"], runargs["seed"], )       
        
        params = []
        for name, value in config.items():
            params.append(name)
            params.append(value)
        thread_to_params, thread_to_solver, params_to_tags = self.parse_parameters(params)
        
        for t, p_list in thread_to_params.items():
             for p in p_list:
                 cmd += " "+p
                 
        # remember instance and cmd to verify the result later on
        self._instance = runargs["instance"] 
        self._cmd = cmd
        
        return cmd
    
    def parse_parameters(self, params, prefix="--", separator="="):
        ''' parse parameters; syntax: -@[Thread]:{[component]:|S|F}*[param] [value]#
            globals parameters at thread-id 0
            Args:
                parameter : parameter list
                prefix: of build parameters, e.g. "--" (can be overwritten by parameter file)
                separator: of build parameters, e.g. "=" for --t=8 or " " for --t 8
            Returns:
                thread_to_params:    thread-id to list of parameters (0 -> global parameters)
                thread_to_solver:    thread-id to solver binary
                params_to_tags:      parameter name to smac like tags 
        '''     
        params_to_tags = {}
        thread_to_params = {}
        thread_to_solver = {}
        parallel_thread_list = []
        threadIndex = 0
        while params:
            localParam = {}
            local_prefix = prefix
            local_separator = separator
            value = ""
            option = ""
            backupparams = []
            while(params != []):
                head = params.pop(0)
                value = params.pop(0).replace("'","")
                if not "solver" in head: 
                    value = value.replace("__",",")
                value_is_int = False
                try:
                    value = int(value)
                    value = str(value)
                    value_is_int = True
                except ValueError:
                    value_is_int = False
                if not value_is_int:
                    try:
                        value = round(float(value),4) # some solvers have problems with E notation, e.g. 6,37E8
                        value = str(value)
                    except ValueError:
                        pass
                if (head.startswith("-@"+str(threadIndex))):
                    option = head[4:]
                    optionSplits = option.split(":")
                    parameterName = optionSplits.pop() # last item in list = parameterName
                    if (parameterName == "solver"): # special filter for solver binary path
                        thread_to_solver[threadIndex] = value
                        continue
                    if (parameterName == "prefix"):
                        local_prefix = value
                        continue
                    if (parameterName == "separator"):
                        if value == "W": # parameter with only whitespaces is not allowed by SMAC
                            value = " "
                        local_separator = value
                        continue
                    if len(optionSplits) > 0:  # composed multi parameter
                        skip = False
                        flag = False
                        component = -1
                        if (params_to_tags.get(parameterName) == None):
                            params_to_tags[parameterName] = set()
                        for opt in optionSplits:
                            if (opt == "F"): # Flag
                                flag = True
                                continue
                            if (opt == "S"): # skip
                                skip = True
                                continue
                            try:
                                component = int(opt)
                            except ValueError:
                                params_to_tags[parameterName].add(opt)
                        if (skip == True):
                            continue
                        if (flag == True and value == "no"):
                            continue # won't be passed to solver
                        if (flag == True and value == "yes"):
                            value = ""
                        if (component != -1):
                            if (localParam.get(parameterName) == None):
                                localParam[parameterName] = {}
                            localParam[parameterName][component] = value
                        else:
                            localParam[parameterName] = value
                        tags = params_to_tags[parameterName]
                        if "ITERATIVE" in tags:
                            parallel_thread_list.append(threadIndex)
                    else:
                        localParam[option] = value
                else:
                    backupparams.append(head) # keep unhandled
                    backupparams.append(value)
            params = backupparams
            if not (len(localParam) == 0 and threadIndex > 0):
                thread_to_params[threadIndex] = self.__dic_to_flat_list(localParam, local_prefix, local_separator)
            threadIndex += 1
        #print(thread_to_params)
        self.__join_multi_threading_solver(thread_to_params, thread_to_solver, params_to_tags, parallel_thread_list)
        return thread_to_params, thread_to_solver, params_to_tags
    
    def __join_multi_threading_solver(self, thread_to_params, thread_to_solver, params_to_tags, parallel_thread_list):
        '''
            join all multi threading solvers and increase their thread argument
            Args:
                thread_to_params:    thread-id to list of parameters (0 -> global parameters)
                thread_to_solver:    thread-id to solver binary
                params_to_tags:      parameter name to smac like tags
                parallel_thread_list: list of threadindexes of parallel solvers 
        '''
        handled_solvers = []
        for parallel_thread in parallel_thread_list:
            try:    
                solver = thread_to_solver[parallel_thread]
            except KeyError:
                continue
            params = thread_to_params[parallel_thread]
            dup_dict = dict(map(lambda t, x: (t,x==solver), thread_to_solver.items()))
            dups = 0
            index = 1
            # remove all duplicates
            for thread, dup in dup_dict.items():
                if dup == True:
                    dups += 1
                    thread_to_solver.pop(thread)
                    thread_to_params.pop(thread)
                    index += 1
                    
            # modify configuration, resp. threading parameter
            for par_index in range(0, len(params), 2): # pair (parameter_name, value)
                parameter_name = params[par_index]
                value = params[par_index+1]
                tags_minus = params_to_tags.get(parameter_name[1:]) # parameter has already prefix attached :-(
                tags_minus_minus = params_to_tags.get("--"+parameter_name[2:])
                if (tags_minus and "ITERATIVE" in tags_minus) or (tags_minus_minus and "ITERATIVE" in tags_minus_minus): 
                    #TODO: if a parameter name is used more than once (can happen by using several solver),
                    #      the parameter tag mapping is not unique -> not proper handled right now
                    value = dups # assumption: number of threads is an integer
                    params[par_index+1] = str(value)
            handled_solvers.append(solver)
            thread_to_solver[parallel_thread] = solver
            thread_to_params[parallel_thread] = params
        
    def __sorted_dict_values(self, adict):
        keys = list(adict.keys())
        keys.sort()
        return map(adict.get, keys)
    
    def __dic_to_flat_list(self, adict, prefix, separator):
        '''
            convert dictionary to flat parameter list
            Args:
                adict: dictionary : head -> value
                prefix: prefix of parameter head, e.g. "--"
        '''
        allP = []
        for k,v in adict.items():
            if (type(v) is dict):
                vsort = self.__sorted_dict_values(v)
                sortedvalues = ",".join(vsort)
                if separator == " ":
                    allP.append(prefix+k)
                    allP.append(sortedvalues)
                else:
                    allP.append(prefix+k+separator+sortedvalues )
            else:
                if (type(v) is list):
                    allP.extend(v)
                else:
                    if (v == ""):   # flag handling
                        allP.append(prefix+k)
                    else:
                        if separator == " ":
                            allP.append(prefix+k)
                            allP.append(v)
                        else:
                            allP.append(prefix+k+separator+v)
                            
        return allP


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
            self.data.cost["memory"] = self.data.memory_used / 1024

            if len(self.args.obj) == 2:
                cost = self.data.cost.get(self.args.obj[1], None)
                if cost is not None:
                    cost = float(cost)
                cost_str = f"{cost}"
            else:
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
        # sys.stdout.write(f"Result for ParamILS: {self.data.status}, {self.data.time:.4f}, {self.data.runlength}, {cost_str}, {self.data.seed}")

        if self.data.additional != "":
            sys.stdout.write(", %s" % (self.data.additional))
        sys.stdout.write("\n")
        sys.stdout.flush()

        # Result for SMAC3v2..
        cost_str = ",".join([str(c) for c in cost])
        sys.stdout.write(
            f"Result for SMAC3v2: status={self.data.status};cost={cost_str};runtime={self.data.time};additional_info={self.data.additional}")

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
            memory_used = int(m.group(1))

        setattr(self.data, "memory_used", memory_used)

if __name__ == "__main__":
    wrapper = Clasp_Wrapper()
    wrapper.main()    
