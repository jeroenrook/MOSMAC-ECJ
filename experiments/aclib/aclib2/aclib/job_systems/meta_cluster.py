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


class MetaCluster(BaseSystem):

    def __init__(self):
        '''
            Constructor
        '''
        self.logger = logging.getLogger("MetaCluster")


        self.template = """#!/bin/bash
#$ -S /bin/bash
#$ -o {out_} -e {err_}
#$ -cwd
#$ -m n
#$ -N {job_name}
#$ -t 1-{n_runs}
{misc}

echo "Here's what we know from the SGE environment"
echo SHELL=$SHELL
echo HOME=$HOME
echo USER=$USER
echo JOB_ID=$JOB_ID
echo JOB_NAME=$JOB_NAME
echo HOSTNAME=$HOSTNAME
echo SGE_TASK_ID=$SGE_TASK_ID
echo Here we are: `pwd`

{call}"""

        self.if_temp = """if [ "$SGE_TASK_ID" -eq "{idx}" ]
then
  {cmd_}
fi

"""

    def run(self, exp_dir: str, cmds: list, cores_per_job: int=1,
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
            job_cutoff: int
                runtime cutoff per job
            startup_fl: str
                file with commands to execute before starting jobs
        '''
        rep_dict = {"out_": exp_dir,
                    "err_": exp_dir,
                    "misc": "",
                    "job_name": exp_dir.replace("/","_"),
                    "n_runs" : len(cmds),
                    "call": None}

        if startup_fl is not None:
            if not os.path.isfile(startup_fl):
                raise ValueError("Given startup file does not exist: %s" %
                                     startup_fl)
            with open(startup_fl, 'r') as fl:
                rep_dict["misc"] = "".join(fl.readlines())

        call = ""
        for idx, cmd_ in enumerate(cmds):
            call += self.if_temp.format(**{"idx": idx+1, "cmd_": cmd_})
            
        
        rep_dict["call"] = call
        job = self.template.format(**rep_dict)
        self.logger.debug(job)
            
        st = datetime.datetime.fromtimestamp(time.time()).strftime('%Y-%m-%d_%H:%M:%S')
        job_fn = "job%s.sge" %(st)
        with open(job_fn, "w") as fp:
            fp.write(job)



        if cores_per_job > 1:
            cmd = "qsub -l own -q aad_pe.q -pe pe_aad %d %s" %(cores_per_job, job_fn)    
        elif job_cutoff > 5000:
            cmd = "qsub -l own -q aad_core.q %s -l lr=1" %(job_fn)
        else:
            cmd = "qsub -q meta_core.q %s" %(job_fn)
        
        

        self.logger.info(cmd)            
        p = Popen(cmd, shell=True)
        p.communicate()
        
