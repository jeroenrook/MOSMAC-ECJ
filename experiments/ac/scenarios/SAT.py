from pathlib import Path
import numpy as np

from .base import ExpScenario

### SAT

def SAT_CLASP_QUEENS_runtime_memory(basedir: Path = Path(".")):
    ta = ["python3", str(basedir.joinpath("target_algorithms/sat/clasp-cssc14/wrapper.py")),
          "--obj", "PAR10",
          "--obj", "memory",
          "--runsolver-path", "~/Configurators/sparkle/Components/runsolver/src/runsolver",
          "--mem-limit", "2048"]

    scenario = ExpScenario("SAT_CLASP_QUEENS_runtime_memory",
                           objectives=[
                               {"name": "PAR10", "cost_for_crash": float(2**32)},
                               {"name": "memory", "cost_for_crash": float(2**32)},
                           ],
                           ta=" ".join(ta),
                           cs=str(basedir.joinpath("target_algorithms/sat/clasp-cssc14/params.pcs")),
                           instances=str(basedir.joinpath("instances/sat/sets/QUEENS-CSSC14/training.txt")),
                           instances_val=str(basedir.joinpath("instances/sat/sets/QUEENS-CSSC14/validation.txt")),
                           instances_test=str(basedir.joinpath("instances/sat/sets/QUEENS-CSSC14/test.txt")),
                           instance_features=str(basedir.joinpath("instances/sat/sets/QUEENS-CSSC14/features.txt")),
                           deterministic=False,
                           cutoff=60,
                           wallclock_limit=86400,
                           basedir=basedir
                           )

    return scenario

def _SAT_CMS_QUEENS_base(basedir: Path, cutoff: int, limit: int, trainingset: str, validationset: str, testset: str, name_spec: str):
    ta = ["python3", str(basedir.joinpath("target_algorithms/sat/cryptominisat-cssc14/wrapper.py")),
          "--obj", "PAR10",
          "--obj", "memory",
          "--runsolver-path", "~/Configurators/sparkle/Components/runsolver/src/runsolver",
          "--mem-limit", "2048"]

    scenario = ExpScenario(f"SAT_CMS_QUEENS_runtime_memory{name_spec}",
                           objectives=[
                               {"name": "PAR10", "cost_for_crash": float(2**32)},
                               {"name": "memory", "cost_for_crash": float(2**32)},
                           ],
                           ta=" ".join(ta),
                           cs=str(basedir.joinpath("target_algorithms/sat/cryptominisat-cssc14/params.pcs")),
                           instances=str(basedir.joinpath(f"instances/sat/sets/QUEENS-CSSC14/{trainingset}.txt")),
                           instances_val=str(basedir.joinpath(f"instances/sat/sets/QUEENS-CSSC14/{validationset}.txt")),
                           instances_test=str(basedir.joinpath(f"instances/sat/sets/QUEENS-CSSC14/{testset}.txt")),
                           instance_features=str(basedir.joinpath("instances/sat/sets/QUEENS-CSSC14/features.txt")),
                           deterministic=False,
                           cutoff=cutoff,
                           wallclock_limit=limit,
                           basedir=basedir
                           )

    return scenario

def SAT_CMS_QUEENS_runtime_memory(basedir: Path = Path(".")):
    return _SAT_CMS_QUEENS_base(basedir, 60, 3600*6, "training", "validation", "test", "")
    # ta = ["python3", str(basedir.joinpath("target_algorithms/sat/cryptominisat-cssc14/wrapper.py")),
    #       "--obj", "PAR10",
    #       "--obj", "memory",
    #       "--runsolver-path", "~/Configurators/sparkle/Components/runsolver/src/runsolver",
    #       "--mem-limit", "2048"]
    #
    # scenario = ExpScenario("SAT_CMS_QUEENS_runtime_memory",
    #                        objectives=[
    #                            {"name": "PAR10", "cost_for_crash": float(2**32)},
    #                            {"name": "memory", "cost_for_crash": float(2**32)},
    #                        ],
    #                        ta=" ".join(ta),
    #                        cs=str(basedir.joinpath("target_algorithms/sat/cryptominisat-cssc14/params.pcs")),
    #                        instances=str(basedir.joinpath("instances/sat/sets/QUEENS-CSSC14/training.txt")),
    #                        instances_val=str(basedir.joinpath("instances/sat/sets/QUEENS-CSSC14/validation.txt")),
    #                        instances_test=str(basedir.joinpath("instances/sat/sets/QUEENS-CSSC14/test.txt")),
    #                        instance_features=str(basedir.joinpath("instances/sat/sets/QUEENS-CSSC14/features.txt")),
    #                        deterministic=False,
    #                        cutoff=300,
    #                        wallclock_limit=86400,
    #                        basedir=basedir
    #                        )

    return scenario

def SAT_CMS_QUEENS_runtime_solved(basedir: Path = Path(".")):
    ta = ["python3", str(basedir.joinpath(
        "target_algorithms/sat/cryptominisat-cssc14/wrapper.py")),
          "--obj", "PAR10",
          "--obj", "solved",
          "--runsolver-path",
          "~/Configurators/sparkle/Components/runsolver/src/runsolver",
          "--mem-limit", "2048"]

    scenario = ExpScenario(f"SAT_CMS_QUEENS_runtime_solved",
                           objectives=[
                               {"name": "PAR10", "cost_for_crash": float(2 ** 32)},
                               {"name": "solved", "cost_for_crash": float(2 ** 32)},
                           ],
                           ta=" ".join(ta),
                           cs=str(basedir.joinpath(
                               "target_algorithms/sat/cryptominisat-cssc14/params.pcs")),
                           instances=str(basedir.joinpath(
                               f"instances/sat/sets/QUEENS-CSSC14/training.txt")),
                           instances_val=str(basedir.joinpath(
                               f"instances/sat/sets/QUEENS-CSSC14/validation.txt")),
                           instances_test=str(basedir.joinpath(
                               f"instances/sat/sets/QUEENS-CSSC14/test.txt")),
                           instance_features=str(basedir.joinpath(
                               "instances/sat/sets/QUEENS-CSSC14/features.txt")),
                           deterministic=False,
                           cutoff=10,  # to have unsolved instances after configuration!
                           wallclock_limit=6*3600,
                           basedir=basedir
                           )

    return scenario

def SAT_test(basedir: Path = Path(".")):
    exp = _SAT_CMS_QUEENS_base(basedir, 6, 120, "training10", "training10", "training50", "_test")
    exp.name = "SAT_test"
    return exp