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


class NemoCluster(BaseSystem):

    def __init__(self):
        '''
            Constructor
        '''
        self.logger = logging.getLogger("NemoCluster")

        self.template = """
#!/bin/bash
#MSUB -N {job_name}
#MSUB -o {out_}
#MSUB -e {err_}
#MSUB -r n
#MSUB -l walltime=00:{job_cutoff_min}:00
#MSUB -l nodes=1:ppn={cores_per_job}
#MSUB -l pmem=6gb
#MSUB -m n 

cd $PBS_O_WORKDIR

{misc}

export # show all environment variables
ulimit -a # show all env limits

export PATH=.:$PATH
#ulimit -v unlimited #disable virtual memory limit
        
{call}"""

        self.if_temp = """if [ "$MOAB_JOBARRAYINDEX" -eq "{idx}" ]
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
        
        rep_dict = self._get_base_rep_dict(exp_dir=exp_dir, cmds=cmds, 
                                           cores_per_job=cores_per_job, job_cutoff=job_cutoff,
                                           startup_fl=startup_fl)

        call, n_jobs = self._build_call(cmds=cmds, calls_per_job=calls_per_job)
        
        rep_dict["call"] = call
        job = self.template.format(**rep_dict)
        self.logger.debug(job)
        
        self._start_job(job=job,exp_dir=exp_dir, n_jobs=n_jobs)
            
    def _get_base_rep_dict(self, exp_dir, cmds, cores_per_job, job_cutoff, startup_fl):
        rep_dict = {"out_": exp_dir,
                    "err_": exp_dir,
                    "misc": "",
                    "job_name": exp_dir.replace("/","_"),
                    "n_runs" : len(cmds),
                    "cores_per_job" : cores_per_job,
                    "job_cutoff_min" : int(math.ceil(job_cutoff/60.)),
                    "call": None}
        
        if startup_fl is not None:
            if not os.path.isfile(startup_fl):
                raise ValueError("Given startup file does not exist: %s" %
                                     startup_fl)
            with open(startup_fl, 'r') as fl:
                rep_dict["misc"] = "".join(fl.readlines())
        
        return rep_dict
        
    def _build_call(self, cmds, calls_per_job ):
        call = ""
        job_runs = 0
        n_jobs = 0
        while True:
            cmd_ = cmds[job_runs:job_runs+calls_per_job]
            cmd_ = " & \n".join(cmd_)+" &\n"
            cmd_ += "for job in `jobs -p`; do wait $job; done"
            call += self.if_temp.format(**{"idx": n_jobs+1, "cmd_": cmd_})
            job_runs += calls_per_job
            n_jobs += 1
            if job_runs >= len(cmds):
                break
        return call, n_jobs
        
    def _start_job(self, job, exp_dir, n_jobs):
        
        st = datetime.datetime.fromtimestamp(time.time()).strftime('%Y-%m-%d_%H:%M:%S')
        job_fn = "job%s.sge" %(st)
        with open(job_fn, "w") as fp:
            fp.write(job)

        cmd = "msub %s -t \"%s\".[1-%d]" %(job_fn, exp_dir.replace("/","_"), n_jobs) 

        self.logger.info(cmd)            
        p = Popen(cmd, shell=True)
        p.communicate()