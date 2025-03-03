import os
from subprocess import Popen
import logging

__author__ = "Marius Lindauer"
__version__ = "0.0.1"
__license__ = "BSD"

class LocalSystem(object):
    
    def __init__(self):
        '''
            Constructor
        '''
        
        self.template = ""
        self.logger = logging.getLogger("LocalSystem")
        
    def run(self, exp_dir: str, cmds:list, cores_per_job:int=1, job_cutoff:int=172800):
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
        '''
        
        #write cmds to disk
        cmd_fn = os.path.join(exp_dir, "cmds.txt")
        with open(cmd_fn, "w") as fp:
            for cmd in cmds:
                fp.write("'%s'\n" %(cmd))
            
        cmd = "cat \"%s\" | xargs -n 1 -L 1 -P %d bash -c" %(cmd_fn, cores_per_job)
        self.logger.info(cmd)
        p = Popen(cmd, shell=True)
        p.communicate()
