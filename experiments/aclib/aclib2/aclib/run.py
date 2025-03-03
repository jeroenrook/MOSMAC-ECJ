#!/usr/bin/env python3.5
# encoding: utf-8

import logging
from collections import OrderedDict
import re
from argparse import ArgumentParser, ArgumentDefaultsHelpFormatter, FileType

# hack to avoid installing of Aclib
import sys
import os
import glob
import inspect
cmd_folder = os.path.realpath(
    os.path.abspath(os.path.split(inspect.getfile(inspect.currentframe()))[0]))
cmd_folder = os.path.realpath(os.path.join(cmd_folder, ".."))
if cmd_folder not in sys.path:
    sys.path.insert(0, cmd_folder)

import json

from aclib.configurators.ac_interface import ACInterface
from aclib.scenario_modifiers.no_conds import no_conds_scenario
from aclib.scenario_modifiers.discretize import discretize_pcs_scenario
from aclib.scenario_modifiers.features import rm_features, enum_features

from aclib.job_systems.meta_cluster import MetaCluster
from aclib.job_systems.slurm_cluster import MetaSLURMCluster
from aclib.job_systems.nemo_cluster import NemoCluster
from aclib.job_systems.nemo_cluster_container import NemoClusterContainer
from aclib.job_systems.local_system import LocalSystem

from aclib.scenario_reader.scenario_reader import InputReader

__author__ = "Marius Lindauer and Katharina Eggensperger"
__version__ = "0.0.1"
__license__ = "BSD"


class AClibRun(object):

    def __init__(self):
        '''
            Constructor
        '''
        logging.basicConfig(level=logging.DEBUG)
        self.logger = logging.getLogger("AClibRun")

        self.aclib_root = os.path.abspath(
            os.path.split(os.path.split(__file__)[0])[0])

    def get_args(self):
        '''
            parses command line argument 
        '''

        parser = ArgumentParser(
            # version=__version__,
            formatter_class=ArgumentDefaultsHelpFormatter)
        req_group = parser.add_argument_group("Required")

        req_group.add_argument("-s", "--scenario", required=True,
                               help="Scenario name; see \"/scenarios/<domain>/<scenario name>\"")
        req_group.add_argument("-c", "--configurator", required=True,
                               choices=["SMAC2", "SMAC3", "PARAMILS", 
                                        "GGA", "GGA++", "GGA_def", "GGA++_def",
                                        "ROAR", "IRACE2", "IRACE3"], help="algorithm configuration procedure")

        adv_group = parser.add_argument_group("Advanced")
        adv_group.add_argument(
            "--suffix", default="", help="suffix of AC procedure directory")
        adv_group.add_argument(
            "--ac_cores", type=int, default=1, help="number of AC procedure CPU cores (only applicable for GGA at the moment)")
        adv_group.add_argument("--name_map_json", default=None, 
                        help="A json file with a mapping from AC names to names"
                             " as a replacement for directory names")

        exec_group = parser.add_argument_group("Execution")
        exec_group.add_argument(
            "--env", choices=["local", "meta", "nemo", "nemoc"], default="local", help="environment to execute runs")
        exec_group.add_argument(
            "-n", "--number_of_runs", type=int, default=1, help="number of AC runs in parallel")
        exec_group.add_argument(
            "--cores_per_job", type=int, default=1, help="number of CPU cores per job")
        exec_group.add_argument(
            "--calls_per_job", type=int, default=1, help="number of cmd calls per job; should be larger than <cores_per_job>")
        exec_group.add_argument(
            "--init_run_number", type=int, default=1, help="initial ID of run (also random seed for AC)")
        exec_group.add_argument(
            "--startup", dest="startup", type=str, default=None,
            help="File with commands to execute before running commands. "
                 "Ignored when '--env local'")
        exec_group.add_argument(
            "--aclib_root", dest="aclib_root", type=str, default=None,
            help="Overwrite AClib root; useful for container experiments")
        
        mod_group = parser.add_argument_group("Scenario Modifiers")
        mod_group.add_argument("--no_conds", default=False, action="store_true", 
                               help="removes all conditional constraints from pcs")
        mod_group.add_argument("--discretize_pcs", default=False, action="store_true", 
                               help="discretize pcs (activated by default for PARAMILS")
        mod_group.add_argument("--no_features", default=False, action="store_true", 
                               help="Removes reference to feature file in scenario file (priority over --enum_features)")
        mod_group.add_argument("--enum_features", default=False, action="store_true", 
                               help="Replaces features with a simple feature enumerating the instances")

        return parser.parse_known_args()

    def main(self):
        '''
            main method
        '''

        args_, ac_args = self.get_args()

        exp_dir = "%s/%s" % (args_.scenario, args_.configurator)

        if args_.suffix:
            exp_dir += args_.suffix
        if args_.no_conds:
            exp_dir += "_--no_conds"
        if args_.discretize_pcs:
            exp_dir += "_--discretize_pcs"
        if args_.no_features:
            exp_dir += "_--no_features"
        if args_.enum_features:
            exp_dir += "_--enum_features"

        if ac_args:
            exp_dir += "_" + "_".join(ac_args).replace("/","_")
            
        if args_.name_map_json:
            with open(args_.name_map_json) as fp:
                name_mapping = json.load(fp, object_pairs_hook=OrderedDict)
                exp_dir = exp_dir[len(args_.scenario)+1:] # rm scenario name
                for name_regex in name_mapping:
                    if re.match(name_regex, exp_dir):
                        exp_dir = name_mapping.get(name_regex)
                        break
                exp_dir = "%s/%s" %(args_.scenario, exp_dir) #re-add scenario name

        bac = ACInterface(aclib_root=self.aclib_root, suffix_dir=args_.suffix)
        ac = bac.get_AC(ac_name=args_.configurator)

        scenario_orig_fn = glob.glob(
            os.path.join(self.aclib_root, "scenarios/*/%s/scenario.txt" % (args_.scenario)))[0]
        
        ac_cmds = []
        exp_seed_dirs = []
        for seed in range(args_.init_run_number, args_.number_of_runs + args_.init_run_number):
            exp_seed_dir = exp_dir + "/run-%d" % (seed)
            exp_seed_dirs.append(exp_seed_dir)
            self._create_exp_dir(path=exp_seed_dir, 
                                 aclib_root=args_.aclib_root if args_.aclib_root else self.aclib_root)

            scenario_fn = self.modify_scenario(args_=args_, scenario_orig_fn=scenario_orig_fn, exp_dir=exp_seed_dir)

            ac_cmd = ac.get_call(
                scenario_fn=scenario_fn, seed=seed, ac_args=ac_args, exp_dir=exp_seed_dir, cores=args_.ac_cores)
            self.logger.debug(ac_cmd)
            ac_cmds.append("cd \"%s\" && %s" %
                           (exp_seed_dir, ac_cmd))
            
        # get job running time cutoff
        in_reader = InputReader()
        scen_dict = in_reader.read_scenario_file(scenario_orig_fn)
        job_cutoff = scen_dict.get("wallclock-limit")
        if job_cutoff is None:
            job_cutoff = scen_dict.get("tunerTimeout")
        if job_cutoff is None:
            job_cutoff = 172800
        job_cutoff = float(job_cutoff) * 1.1 + 10*60 # add some slack

        self.submit(cmds=ac_cmds, system=args_.env,
                    exp_dir=exp_dir, 
                    cores_per_job=args_.cores_per_job,
                    calls_per_job=args_.calls_per_job,
                    startup_fl=args_.startup,
                    job_cutoff=job_cutoff,
                    exp_seed_dirs=exp_seed_dirs)
        
    def modify_scenario(self, args_, scenario_orig_fn:str, exp_dir:str):
        '''
            modifies scenario accordingly to command line args
            
            Arguments
            ---------
            args_: Namedtuple
                with fields "no_conds", "discretize_pcs", "no_features", "enum_features"
            scenario_orig_fn: str
                original scenario file in AClib root
            exp_dir: str
                experiment directory
            
            Returns
            -------
            s: str
                new scenario file living in exp_dir
        '''
        
        scenario_fn = scenario_orig_fn
        if args_.no_conds:
            scenario_fn = no_conds_scenario(scenario_fn=scenario_fn, exp_dir=exp_dir)
        if args_.discretize_pcs or args_.configurator == "PARAMILS":
            scenario_fn = discretize_pcs_scenario(scenario_fn=scenario_fn, exp_dir=exp_dir)
        if args_.no_features:
            scenario_fn = rm_features(scenario_fn=scenario_fn, exp_dir=exp_dir)
        elif args_.enum_features:
            scenario_fn = enum_features(scenario_fn=scenario_fn, exp_dir=exp_dir)
            
        # in case scenario file was not modified
        scenario_fn_rel = scenario_fn.replace(self.aclib_root+"/","")
        
        return scenario_fn

    def _create_exp_dir(self, path: str, aclib_root:str):
        '''
            creates an directory with symlinks to AClib 

            Arguments
            ---------
            path : str
                directory path for experiments
            aclib_root: str
                AClib root directory; linked against it
        '''
        os.makedirs(path, exist_ok=True)

        cwd = os.getcwd()
        os.chdir(path)

        scenarios_path = os.path.join(aclib_root, "scenarios")
        algorithms_path = os.path.join(aclib_root, "target_algorithms")
        instances_path = os.path.join(aclib_root, "instances")
        configurators_path = os.path.join(aclib_root, "configurators")

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

    def submit(self, cmds: list, system: str, exp_dir: str, 
               cores_per_job: int,
               calls_per_job: int,
               startup_fl: str=None, 
               job_cutoff: int=172800,
               exp_seed_dirs:str=[]):
        '''
            submits/runs list of command line calls

            Arguments
            ---------
            cmds: list
                command line calls
            system: str
                system to run command on
            exp_dir: str
                experiment directory
            cores_per_job: int
                number of cores per job
            calls_per_job: int
                number of cmd calls per job
            startup_fl: str
                file with commands to execute before starting jobs
            job_cutoff: int
                maximal number of seconds to run the job (not available on all systems)
            exp_seed_dirs: str
                list of all specific experimentation directories
                (only used in nemo_cluster_container)
        '''

        if system == "meta":
            """
            # Does not longer exist
            env = MetaCluster()
            env.run(exp_dir=exp_dir, cmds=cmds,
                    cores_per_job=cores_per_job,
                    startup_fl=startup_fl)
            """
            env = MetaSLURMCluster()
            env.run(exp_dir=exp_dir, cmds=cmds,
                    cores_per_job=cores_per_job,
                    calls_per_job=calls_per_job,
                    job_cutoff=job_cutoff,
                    startup_fl=startup_fl)
        elif system == "local":
            env = LocalSystem()
            env.run(exp_dir=exp_dir, cmds=cmds,
                    cores_per_job=cores_per_job)
        elif system == "nemo":
            env = NemoCluster()
            env.run(exp_dir=exp_dir, cmds=cmds,
                    cores_per_job=cores_per_job,
                    calls_per_job=calls_per_job, 
                    job_cutoff=job_cutoff,
                    startup_fl=startup_fl)
        elif system == "nemoc":
            env = NemoClusterContainer()
            env.run(exp_dir=exp_dir, cmds=cmds,
                    cores_per_job=cores_per_job,
                    calls_per_job=calls_per_job, 
                    job_cutoff=job_cutoff,
                    startup_fl=startup_fl,
                    exp_seed_dirs=exp_seed_dirs,
                    aclib_root=self.aclib_root)

if __name__ == "__main__":
    aclib = AClibRun()
    aclib.main()
