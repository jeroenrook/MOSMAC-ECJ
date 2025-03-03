import logging
import datetime
import time
import math
import os
from subprocess import Popen

from aclib.job_systems.base_system import BaseSystem

__author__ = "Marius Lindauer"
__version__ = "0.0.1"
__license__ = "BSD"


class MetaSLURMCluster(BaseSystem):

    def __init__(self):
        '''
            Constructor
        '''
        super(MetaSLURMCluster, self).__init__()
        self.logger = logging.getLogger("MetaSLURMCluster")

        self.template = """#!/bin/bash
#SBATCH --mem {memlimit:d}MB # memory pool for all cores (4GB)
#SBATCH -t 0:00:{timelimit:d} # time (D-HH:MM)
#SBATCH -o {out_}/slurm-%A_%a.out # STDOUT  (the folder log has to be created prior to running or this won't work)
#SBATCH -e {err_}/slurm-%A_%a.err # STDERR  (the folder log has to be created prior to running or this won't work)
#SBATCH -J {job_name} # sets the job name. If not specified, the file name will be used as job name
#SBATCH -a 1-{n_jobs}
#SBATCH -c {n_cores}
# Print some information about the job to STDOUT

{misc}

echo "Here's what we know from the SGE environment"
echo SHELL=$SHELL
echo HOME=$HOME
echo USER=$USER
echo JOB_ID=$JOB_ID
echo JOB_NAME=$JOB_NAME
echo HOSTNAME=$HOSTNAME
echo SLURM_TASK_ID=$SLURM_ARRAY_TASK_ID
echo Here we are: `pwd`

{call}

echo Finished at $(date)
"""

        self.if_temp = """if [ "$SLURM_ARRAY_TASK_ID" -eq "{idx}" ]
then
  {cmd_}
fi

"""

    def run(self, exp_dir: str, cmds: list, cores_per_job: int=1,
            calls_per_job: int=1,
            job_cutoff: int=172800, startup_fl: str=None):
        '''
            runs the given command line calls (e.g., locally or by submitting jobs)

            Arguments
            ---------
            exp_dir: str
                execution directory of job
            cmds: list
                command line calls for AC experiments
            cores_per_job: int
                number of cores per job
            calls_per_job: int
                number of cmd calls per job
            job_cutoff: int
                runtime cutoff per job
            startup_fl: str
                file with commands to execute before starting jobs
        '''
        if cores_per_job < calls_per_job:
            raise ValueError("number of cores per job have to be at least calls_per_job")

        rep_dict = {"memlimit": int(3900 * cores_per_job),
                    "timelimit": int(job_cutoff),
                    "out_": exp_dir,
                    "err_": exp_dir,
                    "misc": "",
                    "job_name": exp_dir.replace("/","_"),
                    "n_cores": cores_per_job,
                    "n_jobs": None,
                    "call": None}

        if startup_fl is not None:
            if not os.path.isfile(startup_fl):
                raise ValueError("Given startup file does not exist: %s" % startup_fl)
            with open(startup_fl, 'r') as fl:
                rep_dict["misc"] = "".join(fl.readlines())
        call, n_jobs = self._build_call(cmds=cmds, calls_per_job=calls_per_job)

        rep_dict["n_jobs"] = n_jobs
        rep_dict["call"] = call
        job = self.template.format(**rep_dict)
        self.logger.debug(job)

        self._start_job(job=job, n_jobs=n_jobs)

    def _build_call(self, cmds, calls_per_job):
        call = ""
        job_runs = 0
        n_jobs = 0
        while True:
            cmd_ = cmds[job_runs:job_runs + calls_per_job]
            cmd_ = " & \n".join(cmd_) + " &\n"
            cmd_ += "for job in `jobs -p`; do wait $job; done"
            call += self.if_temp.format(**{"idx": n_jobs + 1, "cmd_": cmd_})
            job_runs += calls_per_job
            n_jobs += 1
            if job_runs >= len(cmds):
                break
        return call, n_jobs

    def _start_job(self, job, n_jobs):

        st = datetime.datetime.fromtimestamp(time.time()).strftime('%Y-%m-%d_%H:%M:%S')
        job_fn = "job%s.slurm" % st
        with open(job_fn, "w") as fp:
            fp.write(job)

        cmd = "sbatch -p ml_cpu-ivy %s" % (job_fn)

        self.logger.info(cmd)
        p = Popen(cmd, shell=True)
        p.communicate()
