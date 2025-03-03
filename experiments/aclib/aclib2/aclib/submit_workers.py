#!/usr/bin/env python3.5
# encoding: utf-8

import logging
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


__author__ = "Marius Lindauer and Katharina Eggensperger"
__version__ = "0.0.1"
__license__ = "BSD"


class AClibVal(object):

    def __init__(self):
        '''
            Constructor
        '''
        logging.basicConfig(level=logging.DEBUG)
        self.logger = logging.getLogger("AClibWorkers")

        self.aclib_root = os.path.abspath(
            os.path.split(os.path.split(__file__)[0])[0])

        self.worker_cmd_str = os.path.join(self.aclib_root, "configurators/validate/lib/mysql-worker ") + \
            "--pool {pool} --delay-between-requests {delay} " \
            "--job-id {ID} --min-cutoff-death-time {min_cutoff_death} " \
            "--log-output-dir ./workers/ --idle-time-limit {idle_time} " \
            "--mysqlTaeDefaultsFile {mysql_fn} --time-limit {time_limit}"

    def get_args(self):
        '''
            parses command line argument 
        '''

        parser = ArgumentParser(
            # version=__version__,
            formatter_class=ArgumentDefaultsHelpFormatter)
        req_group = parser.add_argument_group("Required")

        req_group.add_argument(
            "-n", "--number_of_workers", required=True, type=int, help="number of workers to run")
        req_group.add_argument(
            "--pool", required=True, help="database pool to use worker instead of local validation")
        req_group.add_argument(
            "--time_limit", type=int, required=True, help="Amount of time to work for, you should set this to the time limit of the job, otherwise the database may have jobs that are stuck.")

        opt_group = parser.add_argument_group("Optional")
        opt_group.add_argument(
            "--env", choices=["meta", "nemo", "nemoc"], default="meta", help="environment to execute runs")
        opt_group.add_argument("--delay_between_requests", type=int, default=10,
                               help="Minimum amount of time (in seconds) required between fetching runs from the MySQL DB")
        opt_group.add_argument("--min_cutoff_death_time", type=int, default=600,
                               help="Amount of time to wait after discovering all jobs exceed the remaining time",
                               )
        opt_group.add_argument("--idle_time_limit", type=int, default=30,
                               help="Amount of time to not have a task before shutting down",
                               )
        opt_group.add_argument("--cores_per_job", type=int, default=1,
                               help="number of CPU cores per job",
                               )
        opt_group.add_argument("--startup", dest="startup", type=str,
                               default=None,
                               help="File with commands to execute before "
                                    "starting commands. Ignored when '--env "
                                    "local'")
        return parser.parse_known_args()

    def main(self):
        '''
            main method
        '''

        if not os.path.isdir("workers"):
            os.mkdir("workers")

        args_, ac_args = self.get_args()

        mysqlTaeDefaultsFile = os.path.expanduser(os.path.join(
            "~", ".aeatk", "mysqldbtae.opt"))  # file that contains default settings for MySQL
        if not os.path.isfile(mysqlTaeDefaultsFile):
            raise IOError("%s not found, please create it" %
                          mysqlTaeDefaultsFile)

        cmds = [self.worker_cmd_str.format(**{
            "ID": idx,
            "pool": args_.pool,
            "delay": args_.delay_between_requests,
            "min_cutoff_death": args_.min_cutoff_death_time,
            "idle_time": args_.idle_time_limit,
            "mysql_fn": mysqlTaeDefaultsFile,
            "time_limit": args_.time_limit}) for idx in range(1, args_.number_of_workers + 1)]

        self.submit(cmds=cmds, system=args_.env, exp_dir="workers",
                    cores_per_job=args_.cores_per_job,
                    job_cutoff= (args_.time_limit + args_.idle_time_limit) * 1.1,
                    startup=args_.startup)

    def submit(self, cmds: list, system: str, exp_dir: str, cores_per_job: int,
               job_cutoff: int, startup: str=None):
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
            job_cutoff: int
                running time cutoff
        '''

        if system == "meta":
            """
            env = MetaCluster()
            env.run(exp_dir=exp_dir, cmds=cmds,
                    cores_per_job=cores_per_job, job_cutoff=job_cutoff,
                    startup_fl=startup)
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
                    cores_per_job=cores_per_job, job_cutoff=job_cutoff,
                    startup_fl=startup)
        elif system == "nemoc":
            env = NemoClusterContainer()
            env.run(exp_dir=exp_dir, cmds=cmds,
                    cores_per_job=cores_per_job,
                    job_cutoff=job_cutoff,
                    startup_fl=startup)


if __name__ == "__main__":
    aclib = AClibVal()
    aclib.main()
