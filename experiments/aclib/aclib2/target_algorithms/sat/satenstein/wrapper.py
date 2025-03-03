import traceback

from ConfigSpace.read_and_write import pcs
from ConfigSpace.util import deactivate_inactive_hyperparameters, fix_types

from genericWrapper4AC.generic_wrapper import AbstractWrapper
from genericWrapper4AC.domain_specific.satwrapper import SatWrapper

class Satenstein_Wrapper(SatWrapper):
    
    def __init__(self):
        SatWrapper.__init__(self)
    
    def get_command_line_args(self, runargs, config):
        '''
        @contact:    lindauer@informatik.uni-freiburg.de, fh@informatik.uni-freiburg.de
        Returns the command line call string to execute the target algorithm (here: Satenstein).
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
        solver_binary = "./target_algorithms/sat/satenstein/ubcsat"
        pcs_fn = "./target_algorithms/sat/satenstein/params.pcs"
        
        with open(pcs_fn) as fp:
            pcs_str = fp.readlines()
            cs = pcs.read(pcs_string=pcs_str)
            
        try:
            cleaned_config = {}
            for k,v in config.items():
                cleaned_config[k[1:]] = v # remove leadning "-"
            print(cleaned_config)
            config = fix_types(cleaned_config, cs)
            config = deactivate_inactive_hyperparameters(config, cs)
            print(config)
        except ValueError:
            traceback.print_exc()
            self.logger.info("No full instantiation -- don't deactivate inactive parameters.")
    
        # Construct the call string to glucose.
        cmd = "%s -alg satenstein" % (solver_binary)
    
        for name in config:
            value = config[name]
            if value is None:
                continue
            if name.startswith("DLS"):
                name = name[3:]
            if name.startswith("CON"):
                name = name[3:]
            cmd += " -%s %s" % (name,  value)
        
        cmd += " -inst %s -target 0 -seed %d -r satcomp -cutoff -1 -target 0" %(runargs["instance"], runargs["seed"] )
    
        # remember instance and cmd to verify the result later on
        self._instance = runargs["instance"] 
        self._cmd = cmd
    
        return cmd

if __name__ == "__main__":
    wrapper = Satenstein_Wrapper()
    wrapper.main()    
