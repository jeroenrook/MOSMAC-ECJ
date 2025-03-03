import logging
import os

from aclib.configurators.base_configurator import BaseConfigurator

__author__ = "Marius Lindauer"
__version__ = "0.0.1"
__license__ = "BSD"


class SMAC2(BaseConfigurator):

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

        self.logger = logging.getLogger("SMAC2")

        self.traj_file_regex = "smac-output/aclib/traj-run*.txt"

        #self._bin = os.path.abspath(
        #    "%s/configurators/smac%s/smac" % (aclib_root, suffix_dir))
        self._bin = "./configurators/smac%s/smac" % (suffix_dir)

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

        cmd = "\"%s\" --scenarioFile %s --seed %d --validation false --rungroup aclib 1> log-%d.txt 2>&1" % (
            self._bin, scenario_fn, seed, seed)
        if ac_args:
            cmd += " " + " ".join(ac_args)

        return cmd
