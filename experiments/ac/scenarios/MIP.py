from pathlib import Path
import copy
import numpy as np
import ConfigSpace

from .base import ExpScenario

### MIP scenarios
class MIPExpScenario(ExpScenario):

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.cutoffs = [1, 2, 3, 5, 10]

    def so_procedure(self, output_dir: Path = Path("./output/"), seed: int = 1):
        """ Each scenario has its own way of handling the SO case"""
        seed = int(seed)

        # Change each
        cutoffs = self.cutoffs

        # Remove objectives from the scenario to become SO
        new_ta = []
        i = 0
        self.ta = self.ta.split(" ")
        while i < len(self.ta):
            if self.ta[i] == "--obj":
                # skip and next one too
                i += 2
            else:
                new_ta.append(self.ta[i])
                i += 1
        self.ta = new_ta

        for cutoff in cutoffs:
            sub_scenario = copy.deepcopy(self)

            #Update wallclock limit
            sub_scenario.wallclock_limit = (cutoff / np.sum(cutoffs)) * self.wallclock_limit

            sub_scenario.ta.append("--internal-cutoff")
            sub_scenario.ta.append(f"{cutoff}")
            sub_scenario.ta = " ".join(sub_scenario.ta)

            sub_scenario.cs = str(sub_scenario.basedir.joinpath("target_algorithms/mip/cplex12.6/params.pcs"))

            sub_scenario.outputdir = sub_scenario.outputdir.joinpath(f"{cutoff}/")
            sub_scenario.outputdir.mkdir(parents=True, exist_ok=True)
            sub_scenario.objectives = {"name": "gap", "cost_for_crash": float(2**32)}
            sub_scenario.make_scenario(skip_args=["multi_objectives", "cost_for_crash"], seed=seed)

            yield sub_scenario, [(ConfigSpace.Integer("timelimit", (1, 10)), cutoff)]

    def default_procedure(self):
        default_config = self.scenario.configspace.get_default_configuration().get_dictionary()
        configs = []
        for c in self.cutoffs:
            conf = copy.copy(default_config)
            conf["timelimit"] = c
            configs.append(conf)

        return configs # All the default configurations


def _MIP_CPLEX_REGIONS200_base(basedir: Path, objective: str, limit: int, trainingset: str, validationset: str, testset: str, name_spec: str):
    ta = ["python3", str(basedir.joinpath("target_algorithms/mip/cplex12.6/wrapper.py")),
          "--obj", objective,
          "--obj", "gap",
          "--obj-file", str(basedir.joinpath("instances/mip/sets/Regions200/solubility.txt")),
          "--runsolver-path", "~/Configurators/sparkle/Components/runsolver/src/runsolver",
          "--mem-limit", "2048"]

    scenario = MIPExpScenario(f"MIP_CPLEX_REGIONS200_{name_spec}",
                              objectives=[
                                  {"name": objective, "cost_for_crash": float(2 ** 32)},
                                  {"name": "gap", "cost_for_crash": float(2 ** 32)},
                              ],
                              ta=" ".join(ta),
                              cs=str(basedir.joinpath("target_algorithms/mip/cplex12.6/params_cutoff.pcs")),
                              instances=str(basedir.joinpath(f"instances/mip/sets/Regions200/{trainingset}.txt")),
                              instances_val=str(basedir.joinpath(f"instances/mip/sets/Regions200/{validationset}.txt")),
                              instances_test=str(basedir.joinpath(f"instances/mip/sets/Regions200/{testset}.txt")),
                              instance_features=str(basedir.joinpath("instances/mip/sets/Regions200/features.txt")),
                              deterministic=False,
                              cutoff=20,
                              wallclock_limit=limit,
                              basedir=basedir
                              )

    return scenario

def MIP_CPLEX_REGIONS200_cutoff(basedir: Path = Path(".")):
    return _MIP_CPLEX_REGIONS200_base(basedir, "cutoff", 3600 * 24, "training", "validation", "test", "cutoff")

def MIP_CPLEX_REGIONS200_runtime(basedir: Path = Path(".")):
    return _MIP_CPLEX_REGIONS200_base(basedir, "runtime", 3600*24, "training", "validation",  "test", "runtime")

def MIP_test(basedir: Path = Path(".")):
    exp = _MIP_CPLEX_REGIONS200_base(basedir, "cutoff", 120, "training50", "training10", "training50", "test")
    exp.name = "MIP_test"
    return exp

#MIP with alternative cutoff space
def _MIP_CPLEX_REGIONS200_CONT_base(basedir: Path, objective: str, limit: int, trainingset: str, validationset: str, testset: str, name_spec: str):
    ta = ["python3", str(basedir.joinpath("target_algorithms/mip/cplex12.6/wrapper.py")),
          "--obj", objective,
          "--obj", "gap",
          "--obj-file", str(basedir.joinpath("instances/mip/sets/Regions200/solubility.txt")),
          "--runsolver-path", "~/Configurators/sparkle/Components/runsolver/src/runsolver",
          "--mem-limit", "2048"]

    scenario = MIPExpScenario(f"MIP_CPLEX_REGIONS200_CONT_{name_spec}",
                              objectives=[
                                  {"name": objective, "cost_for_crash": float(2 ** 32)},
                                  {"name": "gap", "cost_for_crash": float(2 ** 32)},
                              ],
                              ta=" ".join(ta),
                              cs=str(basedir.joinpath("target_algorithms/mip/cplex12.6/params_cutoff_cont.pcs")),
                              instances=str(basedir.joinpath(f"instances/mip/sets/Regions200/{trainingset}.txt")),
                              instances_val=str(basedir.joinpath(f"instances/mip/sets/Regions200/{validationset}.txt")),
                              instances_test=str(basedir.joinpath(f"instances/mip/sets/Regions200/{testset}.txt")),
                              instance_features=str(basedir.joinpath("instances/mip/sets/Regions200/features.txt")),
                              deterministic=False,
                              cutoff=120,
                              wallclock_limit=limit,
                              basedir=basedir
                              )
    scenario.cutoffs = [1, 2, 3, 5, 10]

    return scenario


def MIP_CPLEX_REGIONS200_CONT_cutoff(basedir: Path = Path(".")):
    return _MIP_CPLEX_REGIONS200_CONT_base(basedir, "cutoff", 3600 * 24, "training", "validation", "test", "cutoff")


def MIP_CPLEX_REGIONS200_CONT_runtime(basedir: Path = Path(".")):
    return _MIP_CPLEX_REGIONS200_CONT_base(basedir, "runtime", 3600 * 24, "training", "validation",  "test", "runtime")

#TODO MIP CORLAT