import abc

__author__ = "Marius Lindauer"
__version__ = "0.0.1"
__license__ = "BSD"

class BaseSystem(object):
    
    def __init__(self):
        '''
            Constructor
        '''
        
        self.template = ""
        
    @abc.abstractmethod
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
        
        