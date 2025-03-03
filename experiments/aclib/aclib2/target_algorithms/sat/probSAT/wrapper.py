from genericWrapper4AC.generic_wrapper import AbstractWrapper
from genericWrapper4AC.domain_specific.satwrapper import SatWrapper

class ProbSAT_Wrapper(SatWrapper):
    
    def __init__(self):
        SatWrapper.__init__(self)
        
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
        
        binary_path = "target_algorithms/sat/probSAT/probSAT"
        cmd = "%s -a" %(binary_path)       
        for name, value in config.items():
            cmd += " -%s %s" %(name,  value)
        cmd += " %s %s" %(runargs["instance"], runargs["seed"])
    
        # remember instance and cmd to verify the result later on
        self._instance = runargs["instance"] 
        self._cmd = cmd
    
        return cmd

if __name__ == "__main__":
    wrapper = ProbSAT_Wrapper()
    wrapper.main()    
