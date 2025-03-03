import logging
import os
import glob
import shutil
import subprocess

from aclib.configurators.gga import GGA

__author__ = "Marius Lindauer"
__version__ = "0.0.1"
__license__ = "BSD"


class GGAPP(GGA):

    def __init__(self, aclib_root: str, suffix_dir: str="", ie_def:bool=False):
        '''
            Constructor

            Arguments
            ---------
            aclib_root: str
                root directory to AClib
            suffix_dir : str
                suffix of AC procedure directory
        '''

        self.logger = logging.getLogger("GGA++")

        self.traj_file_regex = "traj.csv"

        self._bin = os.path.abspath(
            "%s/configurators/gga++%s/dgga-static" % (aclib_root, suffix_dir))
        self._bin = "./configurators/gga++%s/dgga-static" % (suffix_dir)
        
        self.additional_options = ["--os", "regtree"]

        self.ie_def = ie_def
