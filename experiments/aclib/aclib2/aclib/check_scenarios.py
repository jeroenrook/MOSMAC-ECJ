#!/usr/bin/env python3.5
# encoding: utf-8

import logging
from subprocess import Popen
from argparse import ArgumentParser, ArgumentDefaultsHelpFormatter, FileType

# hack to avoid installing of Aclib
import sys
import os
import inspect
cmd_folder = os.path.realpath(
    os.path.abspath(os.path.split(inspect.getfile(inspect.currentframe()))[0]))
cmd_folder = os.path.realpath(os.path.join(cmd_folder, ".."))
if cmd_folder not in sys.path:
    sys.path.insert(0, cmd_folder)

from aclib.configurators.ac_interface import ACInterface

from aclib.job_systems.meta_cluster import MetaCluster
from aclib.job_systems.local_system import LocalSystem

__author__ = "Marius Lindauer and Katharina Eggensperger"
__version__ = "0.0.1"
__license__ = "BSD"


class AClibCheck(object):

    def __init__(self):
        '''
            Constructor
        '''
        logging.basicConfig(level=logging.DEBUG)
        self.logger = logging.getLogger("AClibCheck")

        self.aclib_root = os.path.abspath(
            os.path.split(os.path.split(__file__)[0])[0])
        
        self.checker_scenario = "configurators/validate/util/verify-scenario"
        self.checker_algo = "configurators/validate/util/algo-test"

    def get_args(self):
        '''
            parses command line argument 
        '''

        parser = ArgumentParser(
            # version=__version__,
            formatter_class=ArgumentDefaultsHelpFormatter)
        req_group = parser.add_argument_group("Required")
        
        return parser.parse_known_args()

    def _create_exp_dir(self, path: str):
        '''
            creates an directory with symlinks to AClib 

            Arguments
            ---------
            path : str
                directory path for experiments
        '''
        os.makedirs(path, exist_ok=True)

        cwd = os.getcwd()
        os.chdir(path)

        scenarios_path = os.path.join(self.aclib_root, "scenarios")
        algorithms_path = os.path.join(self.aclib_root, "target_algorithms")
        instances_path = os.path.join(self.aclib_root, "instances")
        configurators_path = os.path.join(self.aclib_root, "configurators")

        if not os.path.exists("./scenarios"):
            try:
                os.symlink(scenarios_path, "./scenarios")
            except OSError:
                self.logger.warn(
                    "File was created. Only relevant if NOT using pSMAC")
        else:
            self.logger.warn(
                "./scenarios existed in working directory. Assuming correct symlink...")

        if not os.path.exists("./target_algorithms"):
            try:
                os.symlink(algorithms_path, "./target_algorithms")
            except OSError:
                self.logger.warn(
                    "File was created. Only relevant if NOT using pSMAC")
        else:
            self.logger.warn(
                "./target_algorithms existed in working directory. Assuming correct symlink...")

        if not os.path.exists("./instances"):
            try:
                os.symlink(instances_path, "./instances")
            except OSError:
                self.logger.warn(
                    "[WARNING] File was created. Only relevant if NOT using pSMAC")
        else:
            self.logger.warn(
                "./instances existed in working directory. Assuming correct symlink...")

        if not os.path.exists("./configurators"):
            try:
                os.symlink(configurators_path, "./configurators")
            except OSError:
                self.logger.warn(
                    "File was created. Only relevant if NOT using pSMAC")
        else:
            self.logger.warn(
                "./configurators existed in working directory. Assuming correct symlink...")

        os.chdir(cwd)

    def main(self):
        '''
            main method
        '''

        args_, ac_args = self.get_args()

        self._create_exp_dir(path=".")
        scenario_list = self.get_scenario_files(dn=".")
        self.check_scenario(scenario_list=scenario_list)

    def get_scenario_files(self, dn:str):
        '''
            collect all scenario files in dn
            
            Arguments
            ---------
            dn: str
                directory name
                
            Returns
            -------
            list of scenario files 
        '''
        scenario_list = []
        for root, dirs, files in os.walk(dn, followlinks=True):
            for file_ in files:
                if file_ == "scenario.txt":
                    scenario_list.append(os.path.join(root,file_))

        return scenario_list
    
    def check_scenario(self, scenario_list:list):
        '''
            use self.checker (SMAC's verify-scenario) to check scenarios in ACLib
            
            Arguments
            ---------
            scenario_list: list
                list of scenario file names
            
        '''
        
        for scen in scenario_list:
            print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
            self.logger.info("Check %s" %(scen))
            print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
            self.logger.info("Check Scenario")
            cmd = "%s --scenario %s" %(self.checker_scenario, scen)
            if "bbob" in scen:
                cmd += " --verify-instances false"
            self.logger.info(cmd)
            p = Popen(cmd, shell=True)
            p.communicate()
            
            self.logger.info("Check Algorithm")
            cmd = "%s --scenario %s --instance-selection FIRST" %(self.checker_algo, scen)
            self.logger.info(cmd)
            p = Popen(cmd, shell=True)
            p.communicate()
            
            

if __name__ == "__main__":
    aclib = AClibCheck()
    aclib.main()
