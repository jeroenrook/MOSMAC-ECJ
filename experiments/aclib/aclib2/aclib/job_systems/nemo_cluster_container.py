import logging
import datetime
import time
import math
import os
from subprocess import Popen

from aclib.job_systems.nemo_cluster import NemoCluster

__author__ = "Marius Lindauer"
__version__ = "0.0.1"
__license__ = "BSD"


class NemoClusterContainer(NemoCluster):

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

module load tools/singularity/2.6

# prevent from reading user site-packages
export PYTHONUSERBASE="TEST" 
export PYTHONPATH=

# ensure to copy TMPDIR into singularity
export SINGULARITYENV_TMPDIR=$TMPDIR

cd $PBS_O_WORKDIR

{misc}

export # show all environment variables
ulimit -a # show all env limits

export PATH=.:$PATH
#ulimit -v unlimited #disable virtual memory limit
        
# for validation with DB and workers
unlink /tmp/aclib2
ln -s $TMPDIR /tmp/aclib2
mkdir /tmp/aclib2/validate/
ln -s /aclib2/instances/ /tmp/aclib2/validate/instances
ln -s /aclib2/target_algorithms/ /tmp/aclib2/validate/target_algorithms
        
cp {container_path} $TMPDIR

cd $TMPDIR

{subdir_create}

ls -lart .

{call}

cd $TMPDIR

{subdir_copy}

"""

        self.if_temp = """if [ "$MOAB_JOBARRAYINDEX" -eq "{idx}" ]
then
  {cmd_}
fi

"""

    def run(self, exp_dir: str, cmds: list, cores_per_job: int=1,
            calls_per_job: int=1,
            job_cutoff: int=172800, startup_fl: str=None,
            exp_seed_dirs:str=[],
            aclib_root:str=None,
            container_path:str="/home/fr/fr_fr/fr_tl1023/aclib/test_container/aclib.simp"):
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
            exp_seed_dirs: str
                list of all specific experimentation directories
            aclib_root: str
                root directory of aclib installation 
                (not within singularity)
            container_path: str 
                path to singularity container with installed AClib and Python
                (incl. all Python packages)
        '''
        if cores_per_job < calls_per_job:
            raise ValueError("number of cores per job have to be at least calls_per_job")
        
        rep_dict = self._get_base_rep_dict(exp_dir=exp_dir, cmds=cmds, 
                                           cores_per_job=cores_per_job, job_cutoff=job_cutoff,
                                           startup_fl=startup_fl)

        call, n_jobs = self._build_call(cmds=cmds, calls_per_job=calls_per_job)
        
        st = datetime.datetime.fromtimestamp(time.time()).strftime('%Y-%m-%d_%H:%M:%S')
        call_fn = "call%s.sh" %(st)
        with open(call_fn, "w") as fp:
            fp.write("#!/bin/bash\n\n")
            fp.write(call)
            
        if not exp_seed_dirs:
            rep_dict["subdir_create"] = ""
        else:
            subdir_str = ""
            for dir_ in exp_seed_dirs:
                subdir_str += "mkdir -p ${TMPDIR}/%s \n" %(dir_)
                subdir_str += "ln -s /aclib2/instances/ ${TMPDIR}/%s/instances \n" %(dir_)
                subdir_str += "ln -s /aclib2/target_algorithms/ ${TMPDIR}/%s/target_algorithms \n" %(dir_)
                subdir_str += "ln -s /aclib2/scenarios/ ${TMPDIR}/%s/scenarios \n" %(dir_)
                subdir_str += "ln -s %s/configurators/ ${TMPDIR}/%s/configurators \n" %(aclib_root, dir_)
            rep_dict["subdir_create"] = subdir_str
            
        if not exp_seed_dirs:
            rep_dict["subdir_copy"] = ""
        else:
            subdir_copy = ""
            for dir_ in exp_seed_dirs:
                subdir_copy += "cp -fr \"${TMPDIR}/%s\"  \"${PBS_O_WORKDIR}/%s\" \n" %(dir_, os.path.split(dir_)[0])
            rep_dict["subdir_copy"] = subdir_copy
        
        rep_dict["container_path"] = container_path
        
        # TODO: check whether this messes the startup scripts up
        rep_dict["call"] = "singularity exec $TMPDIR/aclib.simp bash \"${PBS_O_WORKDIR}/%s\"" %(call_fn)
        job = self.template.format(**rep_dict)
        self.logger.debug(job)
        
        self._start_job(job=job,exp_dir=exp_dir, n_jobs=n_jobs)