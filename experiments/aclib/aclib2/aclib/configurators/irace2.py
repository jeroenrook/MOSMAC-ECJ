import logging
import os
import glob
from subprocess import Popen

from aclib.configurators.base_configurator import BaseConfigurator
from aclib.converters.scenario2aclib import ScenarioAClibConverter

__author__ = "Marius Lindauer"
__version__ = "0.0.1"
__license__ = "BSD"


class IRACE2(BaseConfigurator):

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

        self.logger = logging.getLogger("IRACE2")

        self.traj_file_regex = "traj_irace_wc.csv"

        #self._bin = os.path.abspath(
        #    "%s/configurators/irace_2.0%s/run_irace.sh" % (aclib_root, suffix_dir))
        self._bin = "./configurators/irace_2.0%s/run_irace.sh" % (suffix_dir)
        self.aclib_root = aclib_root
        
        self._converter_script = os.path.abspath("%s/configurators/irace_2.0%s/convert_to_irace_scenario.pl" % (aclib_root, suffix_dir))
        self._traj_script = os.path.abspath("%s/configurators/irace_2.0%s/get_traj.py" % (aclib_root, suffix_dir))
        self._defaut_script = os.path.abspath("%s/configurators/irace_3.3%s/create_default_file.py" % (aclib_root, suffix_dir))

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

        cwd = os.getcwd()
        os.chdir(exp_dir)
                
        new_scenario_file = "scenario.txt"

        sac = ScenarioAClibConverter(fn=scenario_fn, out_fn=new_scenario_file)
        sac.convert()
        
        cmd = "%s %s" %(self._converter_script, os.path.split(new_scenario_file)[1])
        self.logger.info("Call: %s" %(cmd))
        p = Popen(cmd, shell=True)
        p.communicate()

        cmd = "python %s %s > paramfile.default.irace" %(self._defaut_script, sac.pcs_file)
        self.logger.info("Call: %s" % (cmd))
        p = Popen(cmd, shell=True)
        p.communicate()

        os.chdir(cwd)

        cmd = "\"%s\" scenario.irace %d 1> log-%d.txt 2>&1" % (
            self._bin, cores, seed)
        if ac_args:
            raise ValueError("Not Supported by Irace")
        return cmd
    
    def generate_traj_file(self, exp_dir, id_number):
        '''
            calls python configurators/irace2/get_traj.py -f log-*.txt
        '''
        
        cwd = os.getcwd()
        os.chdir(exp_dir)
        cmd = "python %s -f log-%d.txt" %(self._traj_script, id_number)
        self.logger.info("Call (%s): %s" %(exp_dir, cmd))
        p = Popen(cmd, shell=True)
        p.communicate()
        os.chdir(cwd)
