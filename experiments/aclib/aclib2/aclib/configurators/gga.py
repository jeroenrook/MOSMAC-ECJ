import logging
import os
import glob
import shutil
import subprocess

from aclib.configurators.base_configurator import BaseConfigurator

from ConfigSpace.read_and_write import pcs

__author__ = "Marius Lindauer"
__version__ = "0.0.1"
__license__ = "BSD"


class GGA(BaseConfigurator):

    def __init__(self, aclib_root: str, suffix_dir: str="", ie_def:bool=False):
        '''
            Constructor

            Arguments
            ---------
            aclib_root: str
                root directory to AClib
            suffix_dir : str
                suffix of AC procedure directory
            ie_def: bool
                set ie option to default of 100
        '''

        self.logger = logging.getLogger("GGA")

        self.traj_file_regex = "traj.csv"

        self._bin = "./configurators/gga%s/build/debug/bin/dgga" % (suffix_dir)
        
        self.additional_options = []
        
        self.ie_def = ie_def

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

        # discretize pcs file
        new_scen_fn = "scenario.txt"
        new_pcs_fn = "gga_pcs.txt"

        pcs_fn, instance_fn, algo_exec = self.__rewrite_scen_fn(scen_fn=scenario_fn, out_fn=new_scen_fn, new_pcs_fn=new_pcs_fn)
        with open(instance_fn) as fp:
            num_inst = len(fp.readlines())

        shutil.copy(pcs_fn, new_pcs_fn)

        self.logger.info("Converting PCS file to XML")
        with open(new_pcs_fn + ".xml", "w") as fp:
            cmd = ["python", "./configurators/gga/paramils_convert.py", new_pcs_fn]
            self.logger.info("Calling: %s" % (" ".join(cmd)))
            p = subprocess.Popen(cmd, stdout=fp)
            p.communicate()

        # remove instance specific place-holder from gga xml pcs file
        cmd = "sed 's#$extra0#empty#g' %s -i" % (new_pcs_fn + ".xml")
        self.logger.info("Calling: %s" % (cmd))
        p = subprocess.Popen(cmd, shell=True)
        p.communicate()

        self.logger.info("Build a bash wrapper for GGA")
        with open("run-algo.sh", "w") as fp:
            fp.write("#!/bin/bash\n")
            fp.write(algo_exec + " $*\n")

        if self.ie_def:
            ie = 100
        else:
            ie = num_inst

        scenario_command_line = [self._bin, new_pcs_fn + ".xml", "/dev/null",
                                 "--scen_file", new_scen_fn, "-p", "100", "-g", "100", "--gf", "75",
                                 "--is", "5", "--ie", str(ie), "--pe", "10", "-v", "5",
                                 "-t", str(cores), "--traj_file", self.traj_file_regex, "--seed", str(seed)]
        scenario_command_line.extend(self.additional_options)
        
        scenario_command_line.extend(ac_args)

        os.chdir(cwd)

        return " ".join(scenario_command_line) + " 1> log-run%d.txt 2>&1" %(seed)

    def __rewrite_scen_fn(self, scen_fn: str, out_fn: str, new_pcs_fn: str):
        '''
            rewrites scenario file to be compatible with normal scenario files 
             * replace pcs file
             * replace TA call
             * parses training instance file


            Arguments
            ---------
            scen_fn: str
                scenario file name
            out_fn: str
                new scenario file name
            new_pcs_fn: str
                new file name of pcs

            Returns
            -------
            pcs_fn: str
                file name of original pcs file
            instance_fn: str
                file name of training instance file
            algo_exec: str
                original TA command line call
        '''

        pcs_fn = None
        instance_fn = None
        algo_exec = None

        with open(out_fn, "w") as out_fp:
            with open(scen_fn) as fp:
                for line in fp:
                    line = line.replace("\n", "").strip(" ")
                    if self.__startwith_list(line, ["pcs-file", "param-file", "paramFile", "paramfile"]):
                        pcs_fn = line.split("=")[1].strip(" ")
                        out_fp.write("paramfile = %s\n" % (new_pcs_fn))
                    elif self.__startwith_list(line, ["instances", "instance-file", "instance-dir", "instanceFile", "instance_file", "instance_seed_file"]):
                        instance_fn = line.split("=")[1].strip(" ")
                        out_fp.write(line + "\n")
                    elif self.__startwith_list(line, ["algo-exec", "algoExec", "algo"]):
                        algo_exec = line.split("=")[1].strip(" ")
                        out_fp.write("algo = bash run-algo.sh\n")
                    else:
                        out_fp.write(line + "\n")

        if pcs_fn is None:
            self.logger.error(
                "PCS file not found in scenario file (%s)\n" % (scen_fn))
            sys.exit(44)
        if instance_fn is None:
            self.logger.error(
                "Instance file not found in scenario file (%s)\n" % (scen_fn))
            sys.exit(45)

        return pcs_fn, instance_fn, algo_exec
    
    def __startwith_list(self, str_, list):
        hit = False
        for l in list:
            hit = hit or str_.startswith(l)
        return hit
    
