import numpy as np


def sample_runs(runs: dict, n_samples: int, bootstrapping: bool, n_iterations: int):

    new_data = {}
    run_data = list(runs.values())
    for i in range(n_iterations):
        sample_idx = np.random.choice(list(runs.keys()),
                                      size=min(len(runs), n_samples),
                                      replace=bootstrapping)
        best_run = None
        best_value = 2**31
        for s in sample_idx:
            try:
                perf = runs[s]["train"]["traj"]["Test Set Performance"].values[-1]
            except KeyError:
                raise KeyError("Cannot sample runs without training performance")
            if perf < best_value:
                best_value = perf
                best_run = runs[s]
        new_data[i+1] = best_run 
        
    return new_data