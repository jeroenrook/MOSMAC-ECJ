import logging
import os
from subprocess import Popen

from aclib.configurators.irace2 import IRACE2
from aclib.converters.scenario2aclib import ScenarioAClibConverter

__author__ = "Marius Lindauer"
__version__ = "0.0.1"
__license__ = "BSD"


class IRACE3(IRACE2):

    def __init__(self, aclib_root: str, suffix_dir: str=""):
        '''
            Constructor

            Arguments
            ---------
            aclib_root: str
                root directory to AClib
            suffix_dir : str
                suffix of AC procedure directory
        '''

        self.logger = logging.getLogger("IRACE3")

        self.traj_file_regex = "traj_irace_wc.csv"

        self._bin = "./configurators/irace_3.3%s/run_irace.sh" % (suffix_dir)
        self.aclib_root = aclib_root
        
        self._converter_script = os.path.abspath("%s/configurators/irace_3.3%s/convert_to_irace_scenario.pl" % (aclib_root, suffix_dir))
        self._traj_script = os.path.abspath("%s/configurators/irace_3.3%s/get_traj.py" % (aclib_root, suffix_dir))
        self._defaut_script = os.path.abspath("%s/configurators/irace_3.3%s/create_default_file.py" % (aclib_root, suffix_dir))