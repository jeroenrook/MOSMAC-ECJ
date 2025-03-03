import logging
import os
import glob

from aclib.configurators.base_configurator import BaseConfigurator
from aclib.scenario_modifiers.discretize import discretize_pcs

__author__ = "Marius Lindauer"
__version__ = "0.0.1"
__license__ = "BSD"


class ParamILS(BaseConfigurator):

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

        self.logger = logging.getLogger("ParamILS")

        self.traj_file_regex = "paramils_out/*traj_*.csv"

        #self._bin = os.path.abspath(
        #    "%s/configurators/paramils%s/param_ils_2_3_run.rb" % (aclib_root, suffix_dir))

        self._bin = "./configurators/paramils%s/param_ils_2_3_run.rb" % (suffix_dir)

    def get_call(self,
                 scenario_fn: str,
                 seed: int=1,
                 ac_args: list=None,
                 exp_dir: str=".",
                 cores: int = 1):
        '''
            returns call to AC procedure for a given scenario and seed
            Assumption: PCS was discretized

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
        
        cmd = "%s -scenariofile %s -numRun %d -validN 0 1> log-%d.txt 2>&1" % (
            self._bin, scenario_fn, seed, seed)
        if ac_args:
            cmd += " " + " ".join(ac_args)

        return cmd


