#!/usr/bin/env python3.5
# encoding: utf-8

import logging
import glob
import json
from argparse import ArgumentParser, ArgumentDefaultsHelpFormatter

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
from aclib.configurators.base_configurator import BaseConfigurator

from aclib.job_systems.meta_cluster import MetaCluster
from aclib.job_systems.slurm_cluster import MetaSLURMCluster
from aclib.job_systems.local_system import LocalSystem
from aclib.job_systems.nemo_cluster import NemoCluster
from aclib.job_systems.nemo_cluster_container import NemoClusterContainer

from aclib.scenario_reader.scenario_reader import InputReader
from aclib.configurators.gga import GGA


__author__ = "Marius Lindauer and Katharina Eggensperger"
__version__ = "0.0.1"
__license__ = "BSD"


class AClibVal(object):

    def __init__(self):
        '''
            Constructor
        '''
        logging.basicConfig(level=logging.DEBUG)
        self.logger = logging.getLogger("AClibVal")

        self.aclib_root = os.path.abspath(
            os.path.split(os.path.split(__file__)[0])[0])

        self.validate_call = "configurators/validate/smac-validate" \
            " --scenarioFile {scenarioFile}" \
            " --numRun {seed}" \
            " --num-validation-runs {val_runs}"\
            " --useScenarioOutDir true" \
            " --max-timestamp {wallclock-limit}" \
            " --algo-exec-dir {exec_dir}"
            
        self.use_local_scenario = False # in the optimal case this should never be set to True

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
                               choices=["SMAC2", "PARAMILS", "ROAR", "SMAC3", "IRACE2", "IRACE3",
                                        "GGA", "GGA++", "GGA_def", "GGA++_def"
                                        ], help="algorithm configuration procedure")

        adv_group = parser.add_argument_group("Advanced")
        adv_group.add_argument(
            "--suffix", default="", help="suffix of AC procedure directory")
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
        exec_group.add_argument("--startup", dest="startup", type=str,
                                default=None,
                                help="File with commands to execute before "
                                    "starting commands. Ignored when '--env "
                                    "local'")
        exec_group.add_argument(
            "--job_cutoff", dest="job_cutoff", type=int, default=172800,
            help="Expected overall runtime in sec to choose queue")
        exec_group.add_argument(
            "--aclib_root", dest="aclib_root", type=str, default=None,
            help="Overwrite AClib root; useful for container experiments")

        val_group = parser.add_argument_group("Validation")
        val_group.add_argument(
            "--set", choices=["TRAIN", "TEST", "TRAIN+TEST"], required=True, help="validation of which instance set")
        val_group.add_argument(
            "--confs", choices=["DEF", "INC", "DEF+INC", "TIME", "RANDOM"], required=True, help="what to validate")
        val_group.add_argument(
            "--n_rand", type=int, help="number of random confiurations; requires --confs RANDOM")
        val_group.add_argument(
            "--pool", help="database pool to use worker instead of local validation")
        val_group.add_argument(
            "--num_validation_runs", type=int, default=1, help="input of the same name for smac-validate (num-validation-runs)")
        val_group.add_argument(
            "--validate_all", default=False, action="store_true", help="validate all incumbents in trajectory -- overwrites --confs")

        return parser.parse_known_args()

    def main(self):
        '''
            main method
        '''

        args_, ac_args = self.get_args()

        exp_dir = "%s/%s" % (args_.scenario, args_.configurator)

        if args_.suffix:
            exp_dir += args_.suffix

        if ac_args:
            exp_dir += "_" + "_".join(ac_args).replace("/","_")
            
        if args_.name_map_json:
            with open(args_.name_map_json) as fp:
                name_mapping = json.load(fp)
                exp_dir = exp_dir[len(args_.scenario)+1:] # rm scenario name
                exp_dir = name_mapping.get(exp_dir, exp_dir)
                exp_dir = "%s/%s" %(args_.scenario, exp_dir) #re-add scenario name
            
        if "--no_conds" in ac_args:
            self.use_local_scenario = True

        bac = ACInterface(aclib_root=self.aclib_root, suffix_dir=args_.suffix)
        ac = bac.get_AC(ac_name=args_.configurator)

        val_cmds = []

        for seed in range(1, args_.number_of_runs + 1):
            exp_seed_dir = exp_dir + "/run-%d" % (seed)
            self.check_exp_dir(path=exp_seed_dir)
            if args_.pool:
                exec_dir = self.check_exec_dir(scenario=args_.scenario, 
                                               aclib_root=args_.aclib_root if args_.aclib_root else self.aclib_root)
            else:
                exec_dir = "./"
            val_cmds.extend(self.create_validation_calls(scenario=args_.scenario,
                                                         seed=seed,
                                                         path=exp_seed_dir,
                                                         inst_set=args_.set,
                                                         mode=args_.confs,
                                                         ac=ac,
                                                         exec_dir=exec_dir,
                                                         max_timestamp=-1.0,
                                                         min_timestamp=0.0,
                                                         mult_factor=2.0,
                                                         pool=args_.pool,
                                                         val_runs=args_.num_validation_runs,
                                                         validate_all=args_.validate_all,
                                                         n_rand=args_.n_rand))

        self.submit(cmds=val_cmds, system=args_.env,
                    exp_dir=exp_dir, cores_per_job=args_.cores_per_job,
                    startup=args_.startup, job_cutoff=args_.job_cutoff)

    def check_exec_dir(self, scenario, aclib_root:str):
        '''
        verifies that validate_exec_dir exists, otherwise creates it

            Arguments
            ---------
            scenario: str
                scenario name
            aclib_root: str
                 root directory of aclib to link against
        '''
        
        exec_dir = os.path.abspath(os.path.join(scenario, "validate_exec_dir"))
        if os.path.isdir(exec_dir):
            self.logger.info("validate_exec_dir already exists, assuming "
                             "correct symlinks")
        else:
            self.logger.info("Create validate_exec_dir")
            # Create directory
            os.mkdir(exec_dir)
            # Now create symlinks
            os.symlink(os.path.join(aclib_root, "configurators"),
                       os.path.join(exec_dir, "configurators"))
            os.symlink(os.path.join(aclib_root, "scenarios"),
                       os.path.join(exec_dir, "scenarios"))
            os.symlink(os.path.join(aclib_root, "target_algorithms"),
                       os.path.join(exec_dir, "target_algorithms"))
            os.symlink(os.path.join(aclib_root, "instances"),
                       os.path.join(exec_dir, "instances"))
        return exec_dir

    def check_exp_dir(self, path: str):
        '''
            verifies that experimental directory exists

            Arguments
            ---------
            path: str
                path to experimental dir
        '''

        if not os.path.exists(path):
            self.logger.error("Does not exist: %s" % (path))
            sys.exit(1)

    def create_validation_calls(self, scenario: str,
                                seed: int, path: str,
                                inst_set: str,
                                mode: str,
                                ac: BaseConfigurator,
                                exec_dir: str,
                                max_timestamp: float = -1.0,
                                min_timestamp: float = 0.0,
                                mult_factor: float = 2.0,
                                pool: str=None,
                                val_runs: int=1,
                                validate_all:bool=False,
                                n_rand:int=0):
        '''
            generates validation command line calls for a given experimental directory

            Arguments
            ---------
            scenario: str
                AC scenario filename
            seed: int
                random seed of AC procedure
            path: str
                path to experimental directory
            inst_set: str
                set of instances (TRAIN, TEST)
            mode: str:
                validation mode (DEF, INC, TIME)
            ac: BaseConfigurator
                used AC procedure (to get location of trajectory file)
            exec_dir: str
                path to dir with symlinks where validation will be executed
            max_timestamp: float
                maximal timestamp for time validation
            min_timestamp: float
                minimal timestamp for time validation
            mult_factor: float
                multiplication factor of timestamps for time validation
            pool:str
                database pool
            val_runs:int
                number of validation runs (i.e., number of seeds)
            validate_all:bool
                validate all incumbents in trajectory

            Returns
            -------
            cmd_list: list
                list of validation calls
        '''

        if self.use_local_scenario:
            try:  # TODO: remove this later
                self.logger.warning("Using local scenario.txt, which could result in noisy validation results using the DB workers because all runs are validated independently.")
                scenario_fn = glob.glob(
                    os.path.join(path, "scenario.txt"))[0]
                if scenario_fn:
                    scenio_fn_rel =  "scenario.txt"
            except IndexError: 
                self.logger.error("Expected to find local scenario file, but have not found it at: %s" %(os.path.join(path,"scenario.txt")))
                sys.exit()
        else:
            scenario_fn = glob.glob(
                    os.path.join(self.aclib_root, "scenarios/*/%s/scenario.txt" % (scenario)))[0]
            scenio_fn_rel = "./scenarios/*/%s/scenario.txt" % (scenario)
                
        in_reader = InputReader()
        scen_dict = in_reader.read_scenario_file(scenario_fn)
        wallclock_time = scen_dict.get("wallclock-limit")
        if not wallclock_time:
            self.logger.warn("Have not found wallclock-limit in scenario file")
            wallclock_time = 2 * 32

        if type(ac).__name__ in ["IRACE2", "IRACE3"]:
            ac.generate_traj_file(exp_dir=path, id_number=seed)

        if not "RANDOM" in mode:
            traj_fn = glob.glob(os.path.join(path, ac.traj_file_regex), recursive=True)[0]

            if type(ac).__name__ in ["GGA","GGAPP", "IRACE2", "IRACE3"]:
                ac.add_def2traj(exp_dir=path,
                                 traj_fn=traj_fn,
                                 scenario_fn=scenario_fn)

            self.truncate_traj_file(
                traj_fn=traj_fn,
                wallclock_limit=int(wallclock_time))

            traj_fn = "." + traj_fn.replace(path, "")

        cmds = []
        main_cmd = "cd \"%s\" && %s" % (path, self.validate_call.format(**{"scenarioFile": scenio_fn_rel,
                                                                     "seed": seed,
                                                                     # TODO
                                                                     "val_runs": val_runs,
                                                                     "wallclock-limit": wallclock_time,
                                                                     "exec_dir": exec_dir
                                                                     }))
        
        if validate_all:
            main_cmd += " --validate-all true"

        if pool:
            mysqlTaeDefaultsFile = os.path.expanduser(
                os.path.join("~", ".aeatk", "mysqldbtae.opt"))
            if not os.path.isfile(mysqlTaeDefaultsFile):
                raise IOError(
                    "%s not found, please create it to use worker" % mysqlTaeDefaultsFile)

            mysql_args = ["--tae", "MYSQLDB", "--mysqldbtae-pool", pool,
                          "--wait-for-persistent-run-completion", "false",
                          "--mysqlTaeDefaultsFile", mysqlTaeDefaultsFile,
                          "--output-file-suffix", "worker"]
            mysql_args = " ".join(mysql_args)
        else:
            mysql_args = ""

        if "TRAIN" in inst_set and "DEF" in mode:
            cmd = main_cmd + " --includeDefaultAsFirstRandom true --random-configurations 1 " \
                "--outputDirectory ./validate-def-train --validate-test-instances false %s " \
                "1> log-val-train-def.txt 2>&1" % (mysql_args)
            cmds.append(cmd)
        if "TRAIN" in inst_set and "INC" in mode:
            cmd = main_cmd + \
                " --outputDirectory ./validate-inc-train --validate-test-instances false "\
                "--trajectoryFile \"%s\" %s 1> log-val-train-inc.txt 2>&1" % (
                    traj_fn, mysql_args)
            cmds.append(cmd)
        if "TEST" in inst_set and "INC" in mode:
            cmd = main_cmd + \
                " --outputDirectory ./validate-inc-test "\
                "--trajectoryFile \"%s\" %s 1> log-val-test-inc.txt 2>&1" % (
                    traj_fn, mysql_args)
            cmds.append(cmd)
        if "TEST" in inst_set and "DEF" in mode:
            cmd = main_cmd + " --includeDefaultAsFirstRandom true --random-configurations 1 " \
                "--outputDirectory ./validate-def-test %s "\
                "1> log-val-test-def.txt 2>&1" % (mysql_args)
            cmds.append(cmd)
        if "TEST" in inst_set and "TIME" in mode:
            cmd = main_cmd + " --min-timestamp %f --mult-factor %f " \
                "--outputDirectory ./validate-time-test --validateOnlyLastIncumbent false " \
                "--trajectoryFile \"%s\" %s 1> log-val-test-time.txt 2>&1" % (
                    min_timestamp, mult_factor, traj_fn, mysql_args)
            cmds.append(cmd)
        if "TRAIN" in inst_set and "TIME" in mode:
            cmd = main_cmd + " --min-timestamp %f --mult-factor %f " \
                "--outputDirectory ./validate-time-train --validateOnlyLastIncumbent false "\
                "--validate-test-instances false "\
                "--trajectoryFile \"%s\" %s 1> log-val-train-time.txt 2>&1" % (
                    min_timestamp, mult_factor, traj_fn, mysql_args)
            cmds.append(cmd)
        if "TRAIN" in inst_set and "RANDOM" in mode:
            cmd = main_cmd + " --outputDirectory ./validate-rand-train "\
                "--validate-test-instances false "\
                "--random-configurations %d --includeDefaultAsFirstRandom true "\
                "%s 1> log-val-rand-time.txt 2>&1" % (
                    n_rand, mysql_args)
            cmds.append(cmd)
        if "TEST" in inst_set and "RANDOM" in mode:
            cmd = main_cmd + " --outputDirectory ./validate-rand-test " \
                                 "--validate-test-instances true " \
                                 "--random-configurations %d --includeDefaultAsFirstRandom true " \
                                 "%s 1> log-val-rand-test.txt 2>&1" % (
                        n_rand, mysql_args)
            cmds.append(cmd)


        return cmds

    def submit(self, cmds: list, system: str, exp_dir: str, cores_per_job: int,
               startup: str=None, job_cutoff: int=172800):
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
                    job_cutoff=job_cutoff,
                    startup_fl=startup)
        elif system == "local":
            env = LocalSystem()
            env.run(exp_dir=exp_dir, cmds=cmds,
                    cores_per_job=cores_per_job, job_cutoff=job_cutoff)
        elif system == "nemo":
            env = NemoCluster()
            env.run(exp_dir=exp_dir, cmds=cmds,
                    cores_per_job=cores_per_job, job_cutoff=job_cutoff)
        elif system == "nemoc":
            env = NemoClusterContainer()
            env.run(exp_dir=exp_dir, cmds=cmds,
                    cores_per_job=cores_per_job,
                    job_cutoff=job_cutoff)

    def truncate_traj_file(self, traj_fn: str, wallclock_limit: int):
        '''
            truncates the trajectory file such that the last entry has at most used wallclock_limit seconds

            Arguments
            ---------
            traj_fn: str
                trajectory file name
            wallclock_limit: int
                wallclock time limit (according to scenario file) 
        '''

        with open(traj_fn) as fp:
            lines = fp.readlines()

        with open(traj_fn, "w") as fp:
            for line in lines:
                try:
                    wallclock_time = float(line.split(",")[2])
                    if wallclock_time <= wallclock_limit:
                        fp.write(line)
                except (ValueError, IndexError):
                    fp.write(line)  # probably the header


if __name__ == "__main__":
    aclib = AClibVal()
    aclib.main()
