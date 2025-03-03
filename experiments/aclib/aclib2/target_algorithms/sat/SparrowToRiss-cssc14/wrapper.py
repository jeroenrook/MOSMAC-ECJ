from genericWrapper4AC.generic_wrapper import AbstractWrapper
from genericWrapper4AC.domain_specific.satwrapper import SatWrapper

class SparrowToRiss_Wrapper(SatWrapper):
    
    def __init__(self):
        SatWrapper.__init__(self)
        
    def get_command_line_args(self, runargs, config):
        '''
        Returns the command line call string to execute the target algorithm (here: Riss).
        Args:
                runargs: a map of several optional arguments for the execution of the target algorithm.
                                {
                                    "instance": <instance>,
                                    "specifics" : <extra data associated with the instance>,
                                    "cutoff" : <runtime cutoff>,
                                    "runlength" : <runlength cutoff>,
                                    "seed" : <seed>
                                    "tmp" : directory for temporary files (only available if --temp-file-dir-algo was used)
                                }
                config: a mapping from parameter name to parameter value
        Returns:
                A command call list to execute the target algorithm.
        '''

#        print "full runargs: " + str(runargs)
    
    
    # single parts of the command line:
        binary_path = "target_algorithms/sat/SparrowToRiss-cssc14/SparrowToRiss.sh" 
        instance = " %s" %(runargs["instance"])
        
        flips = 500000000 # default value
        if '-strFlips' in runargs.keys():
            flips = " %d" %(runargs['-strFlips'])
        seconds = 150
        if '-strSseconds' in runargs.keys():
            seconds = " %s" %(runargs["-strSseconds"])

        seed = " %s" %(runargs["seed"])
        tmpDir = " " + runargs["tmp"]
        # first part of the command:        
        cmd = "%s %s %s %s %s %s" %(binary_path, instance, seed, tmpDir, flips, seconds )           

        # collect remaining parameters
        sparrowParams=" "        
        rissParams=" -config=CSSC2014 "
                
        for name, value in config.items():
            if name[:8]    == "-SPARROW": # sparrow parameter
                if value == "yes":
                    sparrowParams += " %s" %(name[8:])
# sparrow has no "disable"
#                elif value == "no":
#                    sparrowParams += " -no%s" %(name[8:])
                else:
                    sparrowParams += " %s=%s" %(name[8:], value)
            else:                      # riss parameter
                if value == "yes":
                    rissParams += " %s" %(name)
                elif value == "no":
                    rissParams += " -no%s" %(name)
                else:
                    rissParams += " %s=%s" %(name, value)
        # construct the final command for the script: 
        cmd += " \"%s\"  \"%s\"" %( sparrowParams, rissParams )
        
        # remember instance and cmd to verify the result later on
        self._instance = runargs["instance"] 
        self._cmd = cmd
        
        return cmd
        
if __name__ == "__main__":
    wrapper = SparrowToRiss_Wrapper()
    wrapper.main()  
