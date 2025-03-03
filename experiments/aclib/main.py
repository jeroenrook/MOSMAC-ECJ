#!/usr/bin/env python
import os
import sys
import pandas as pd
import numpy as np
import itertools
import pickle
from smac.configspace import Configuration
from smac.scenario.scenario import Scenario
from smac.utils.constants import MAXINT
from smac.initial_design.latin_hypercube_design import LHDesign
from smac.facade.smac_ac_facade import SMAC4AC
from smac.facade.smac_moac_facade import SMAC4MOAC
from smac.facade.smac_hpo_facade import SMAC4HPO
from smac.epm.random_forest.rf_mo import MultiObjectiveRandomForest
from ConfigSpace.read_and_write import pcs
from smac.tae.execute_ta_run_aclib import ExecuteTARunAClib
from smac.multi_objective.parego import ParEGO
from smac.runhistory.runhistory2epm import RunHistory2EPM4ScaledCost

#TODO: check if legacy


class ExecuteTARunAClibMO(ExecuteTARunAClib):
    def run(
            self,
            config: Configuration,
            instance: str,
            cutoff: float = None,
            seed: int = 12345,
            budget: float = None,
            instance_specific: str = "0",
    ):
        status, cost, runtime, results = super().run(config,
                                                     instance,
                                                     cutoff,
                                                     seed,
                                                     budget,
                                                     instance_specific
                                                     )
        print("ExecuteTARunAClibMO RESULT:", status, {"runtime": runtime, "cost": cost}, runtime, results)
        #return status, cost, runtime, results
        return status, [np.log(runtime), cost], runtime, results


def mosmac(**kwargs) -> SMAC4AC:
    kwargs["model"] = kwargs.get("model", MultiObjectiveRandomForest)
    intensifier_kwargs = {
        "minimum_population_size": 2,
        "maximum_population_size": 8,
    }
    kwargs["intensifier_kwargs"] = kwargs.get("intensifier_kwargs", intensifier_kwargs)
    smac = SMAC4MOAC(**kwargs)

    return smac


def parego(**kwargs) -> SMAC4AC:
    kwargs["multi_objective_algorithm"] = kwargs.get("multi_objective_algorithm", ParEGO)
    multi_objective_kwargs = {
        "rho": 0.05,
    }
    kwargs["multi_objective_kwargs"] = kwargs.get("multi_objective_kwargs", multi_objective_kwargs)
    kwargs["runhistory2epm"] = kwargs.get("runhistory2epm", RunHistory2EPM4ScaledCost)
    smac = SMAC4HPO(**kwargs)

    return smac

def smac(**kwargs) -> SMAC4AC:
    smac = SMAC4AC(**kwargs)

    return smac

def run_job(input):
    c, i, s = input
    to = f"../results/{c.__name__}_{i.split('/')[-1]}_{s}.pickle"
    run(c, i, s, to)
    print(to)
    return "Done"

def run(smac_class, seed, wallclock, writefile):
    pwd = os.getcwd()
    if pwd.split("/")[-1] != "aclib2":
        os.chdir("aclib2")
    print(os.getcwd())

    with open("./target_algorithms/mip/cplex12.6/params.pcs", "r") as fh:
        cs = pcs.read(fh)

    import pandas as pd

    df = pd.read_csv("./instances/mip/sets/Regions200/features.txt", delimiter=", ", engine="python")
    # features_dict = df.set_index("INSTANCE_ID").T.to_dict()
    feature_dict = {}
    for instance in df["INSTANCE_ID"].unique():
        features = df[df["INSTANCE_ID"] == instance].iloc[0]
        feature_dict[instance] = list(features.to_numpy()[1:])

    train_insts = []
    with open("./instances/mip/sets/Regions200/training.txt") as fh:
        for instance in fh:
            train_insts.append([instance.strip()])

    # test_insts = []
    # with open("./instances/mip/sets/Regions200/test.txt") as fh:
    #     for instance in fh:
    #         test_insts.append([instance.strip()])

    scenario = Scenario(
        {
            "run_obj": "quality",  # we optimize quality (alternatively runtime)
            "wallclock_limit": wallclock,
            "cs": cs,  # configuration space
            "deterministic": True,
            "multi_objectives": ["runtime", "quality"],
            # You can define individual crash costs for each objective
            "cutoff_time": 11,
            "cost_for_crash": [float(np.log(20)), float(MAXINT)],
            "output_dir": f"/scratch/rook/mosmac/aclib/output/mip_cplex_region200/{wallclock}_{smac_class.__name__}_{seed}/",
            "train_insts": train_insts,
            # "test_insts": test_insts,
            "feature_dict": feature_dict,
            "algo": "target_algorithms/mip/cplex12.6/wrapper.py --runsolver-path ~/Configurators/sparkle/Components/runsolver/src/runsolver",
        }
    )

    smac_kwargs = {
        "tae_runner": ExecuteTARunAClibMO,
        "tae_runner_kwargs": {
            "ta": ["python3",
                   "target_algorithms/mip/cplex12.6/wrapper.py",
                   "--obj-file", "/home/rook/projects/mosmac/experiments/aclib/aclib2/instances/mip/sets/Regions200/solubility.txt",
                   "--runsolver-path", "~/Configurators/sparkle/Components/runsolver/src/runsolver",
                   "--mem-limit", "2048"],
            "par_factor": 2,
            "multi_objectives": ["runtime", "quality"],
            "cost_for_crash": [float(np.log(20)), float(MAXINT)],
        },
        "scenario": scenario,
        "rng": np.random.RandomState(seed),
        "initial_design": LHDesign,
        "initial_design_kwargs": {
            "init_budget": 6,  # 3*d
        },
    }

    smac_instance = smac_class(**smac_kwargs)
    smac_instance.optimize()

    result = {
        "configurator": smac_class.__name__,
        "instance": instance,
        "seed": seed,
        "wallclock": wallclock,
        "stats": smac_instance.stats,
        "runhistory": smac_instance.runhistory,
        "trajectory": smac_instance.trajectory,
    }
    print(os.getcwd())
    os.chdir(pwd)
    with open(writefile, "wb") as f:
        pickle.dump(result, f, protocol=5)

def run_job(input):
    c, s, w = input
    print(c, s, w)
    to = f"results/{w}_{c.__name__}_{s}.pickle"
    run(c, s, w, to)
    print(to)
    return "Done"

if __name__ == "__main__":
    # test = tae("development/resources/instances/DTLZ1")
    # print(test([0.25, 0.25]))
    #run(mosmac, "development/resources/instances/DTLZ2", 14, "test/")

    configurators = [mosmac, parego]  # parego
    seeds = list(range(5))
    wallclock = [600, 7200, 24*3600]

    configurators = [mosmac]
    seeds = list(range(1))
    wallclock = [600]

    runs = list(itertools.product(configurators, seeds, wallclock))
    print(f"jobs: {len(runs)}")
    njobs = len(runs)
    args = sys.argv
    procid = 1
    procs = 1
    if len(args) == 3:
        procid = int(args[1])
        procs = int(args[2])

    chunksize = (njobs // procs)
    offset = 1 if procid-1 < njobs % procs else 0
    jobstart = chunksize * (procid-1) + min(procid-1, njobs % procs)
    print(f"proc {procid} out of {procs} doing jobs {jobstart} to {jobstart+chunksize+offset}.")
    for i in range(jobstart, jobstart+chunksize+offset):
        if 0 <= i < njobs:
            print(i, runs[i])
            run_job(runs[i])
