import abc
import os
import shutil

from ConfigSpace.read_and_write import pcs

__author__ = "Marius Lindauer"
__version__ = "0.0.1"
__license__ = "BSD"


class BaseConfigurator(object):

    def __init__(self, aclib_root: str, suffix_dir: str=""):
        '''
            Constructor

            Arguments
            ---------
            aclib_root: str
                root directory to AClib
            suffix_dir : str
                suffix of AC procedure directory
        '''

        self.traj_file_regex = None
        self.suffix_dir = suffix_dir

    @abc.abstractmethod
    def get_call(self,
                 scenario_fn: str,
                 seed: int=1,
                 ac_args: list=None,
                 exp_dir: str=".",
                 cores: int = 1):
        '''
            returns call to AC procedure for a given scenario and seed

            Arguments
            ---------
            scenario_fn:str
                scenario file name
            seed: int
                random seed
            ac_args: list
                list of further arguments for AC procedure
            exp_dir: str
                experimental directory
            cores: int
                number of available cores

            Returns
            -------
                commandline_call: str
        '''
        
    def add_def2traj(self, exp_dir, traj_fn, scenario_fn):
        '''
            add default configuration as first entry in trajectory file
            
            ATTENTION: old file will be overwritten
            
            Arguments
            ---------
            exp_dir : str
                experimental directory
            traj_fn: str
                trajectory file name
            scenario_fn:
                scenario file name (absolute path)
        '''
        
        with open(scenario_fn) as fp:
            for line in fp:
                if self.__startwith_list(line, ["pcs-file", "param-file", "paramFile", "paramfile"]):
                    pcs_fn = line.split("=")[1].strip(" ").replace("\n","")
                    with open(os.path.join(exp_dir,pcs_fn)) as pcs_fp:
                        pcs_str = pcs_fp.readlines()
                        cs = pcs.read(pcs_str)
                        def_confg = cs.get_default_configuration()
                        
        with open(traj_fn) as fp:
            traj = fp.readlines()
            
        shutil.copy(traj_fn, traj_fn+".back")
            
        with open(traj_fn, "w") as fp:
            fp.write(traj[0])
            # "CPU Time Used","Estimated Training Performance","Wallclock Time","Incumbent ID","Automatic Configurator (CPU) Time","Configuration..."
            fp.write("0.0,9999999999,0.0,0,0.0")
            for param in def_confg:
                if def_confg.get(param) is not None:
                    fp.write(", %s='%s'" %(param, def_confg.get(param)))
            fp.write("\n")
            for line in traj[1:]:
                fp.write(line)
                
    def __startwith_list(self, str_, list):
        hit = False
        for l in list:
            hit = hit or str_.startswith(l)
        return hit
