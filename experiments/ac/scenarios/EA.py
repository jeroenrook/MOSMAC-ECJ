from pathlib import Path
import numpy as np
from .base import ExpScenario

def EA_OO_MMMOOP():
    """
    Omnioptimizer on MMMOO problems
    Returns
    -------

    """
    wrapperdir = Path("resources/target_algorithms/EMOA/omnioptimizer/")
    ta = ["python3", str(wrapperdir.joinpath("sparkle_smac_wrapper.py").absolute().resolve()),
          "--obj", "HV",
          "--obj", "SP"]

    instancepath = Path("resources/instances/EMOA/MMMOOP/").absolute().resolve()
    instances = list(instancepath.iterdir())
    rng = np.random.RandomState(42)
    train_idx = rng.choice(len(instances), size=int(0.8 * len(instances)), replace=False)
    instances_train = [str(instance) for i, instance in enumerate(instances) if i in train_idx]
    instances_test = [str(instance) for i, instance in enumerate(instances) if i not in train_idx]

    objectives = [("HV", 0), ("SP", 0)]

    scenario = ExpScenario("EA_OO_MMMOOP",
                           objectives=[{"name": obj, "cost_for_crash": cost} for obj, cost in objectives],
                           ta=" ".join(ta),
                           cs=str(wrapperdir.joinpath("omnioptimizer-parameters.pcs").absolute().resolve()),
                           instances=instances_train,
                           instances_val=instances_train,
                           instances_test=instances_test,
                           deterministic=False,
                           cutoff=3600,
                           runcount_limit=500,
                           basedir=wrapperdir,
                           )

    return scenario