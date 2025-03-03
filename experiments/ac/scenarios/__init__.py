
from __future__ import annotations

import copy
import logging
import warnings
import time
from abc import ABC
import numpy as np
import pandas as pd
from pathlib import Path
from ConfigSpace import (
    Categorical,
    Configuration,
    ConfigurationSpace,
    EqualsCondition,
    Float,
    InCondition,
    Integer,
)
# from smac.scenario.scenario import Scenario
from smac.scenario import Scenario
from ConfigSpace.read_and_write import pcs, pcs_new
import ConfigSpace
from subprocess import PIPE, Popen
import json

from .base import ExpScenario
from .MIP import (MIP_CPLEX_REGIONS200_cutoff,
                  MIP_CPLEX_REGIONS200_runtime,
                  MIP_test,
                  MIP_CPLEX_REGIONS200_CONT_cutoff,
                  MIP_CPLEX_REGIONS200_CONT_runtime)
from .SAT import (SAT_CMS_QUEENS_runtime_memory,
                  SAT_test,
                  SAT_CMS_QUEENS_runtime_solved,
                  SAT_ganak_DQMR_runtime_memory)
from .ML.SVM import (ML_SVM_MNIST_precision_recall,
                     ML_SVM_MNIST_precision_recall_fittime,
                     ML_SVM_MNIST_precision_recall_fittime_memory)
from .ML.RF import (ML_RF_STUDENTS_precision_recall,
                    ML_RF_STUDENTS_accuracy_size,
                    ML_RF_STUDENTS_precision_recall_size)
from .EA import EA_OO_MMMOOP

from sklearn.datasets import load_digits
from sklearn.model_selection import StratifiedKFold, cross_val_score
from sklearn.neural_network import MLPClassifier


class MLP:

    def __init__(self):
        self.digits = load_digits()
        self.objectives = ["accuracy", "time"]

    @property
    def configspace(self) -> ConfigurationSpace:
        cs = ConfigurationSpace()

        n_layer = Integer("n_layer", (1, 5), default=1)
        n_neurons = Integer("n_neurons", (8, 256), log=True, default=10)
        activation = Categorical("activation", ["logistic", "tanh", "relu"], default="tanh")
        solver = Categorical("solver", ["lbfgs", "sgd", "adam"], default="adam")
        batch_size = Integer("batch_size", (30, 300), default=200)
        learning_rate = Categorical("learning_rate", ["constant", "invscaling", "adaptive"], default="constant")
        learning_rate_init = Float("learning_rate_init", (0.0001, 1.0), default=0.001, log=True)

        cs.add_hyperparameters([n_layer, n_neurons, activation, solver, batch_size, learning_rate, learning_rate_init])

        use_lr = EqualsCondition(child=learning_rate, parent=solver, value="sgd")
        use_lr_init = InCondition(child=learning_rate_init, parent=solver, values=["sgd", "adam"])
        use_batch_size = InCondition(child=batch_size, parent=solver, values=["sgd", "adam"])

        # We can also add multiple conditions on hyperparameters at once:
        cs.add_conditions([use_lr, use_batch_size, use_lr_init])

        return cs

    def train(self, config: Configuration, instance: str = "i0", seed: int = 0, budget: int = 10) -> dict[str, float]:
        seed = int(seed)
        lr = config["learning_rate"] if config["learning_rate"] else "constant"
        lr_init = config["learning_rate_init"] if config["learning_rate_init"] else 0.001
        batch_size = config["batch_size"] if config["batch_size"] else 200

        start_time = time.time()

        with warnings.catch_warnings():
            warnings.filterwarnings("ignore")

            classifier = MLPClassifier(
                hidden_layer_sizes=[config["n_neurons"]] * config["n_layer"],
                solver=config["solver"],
                batch_size=batch_size,
                activation=config["activation"],
                learning_rate=lr,
                learning_rate_init=lr_init,
                max_iter=int(np.ceil(budget)),
                random_state=seed,
            )

            # Returns the 5-fold cross validation accuracy
            cv = StratifiedKFold(n_splits=5, random_state=seed, shuffle=True)  # to make CV splits consistent
            score = cross_val_score(classifier, self.digits.data, self.digits.target, cv=cv, error_score="raise")

        performances = {
            "accuracy": 1 - np.mean(score),
            "time": time.time() - start_time,
        }
        return {k: performances[k] for k in self.objectives}

def test_mlp_scenario():

    mlp = MLP()

    scenario = ExpScenario(
        name="test",
        objectives=[
            {"name": "accuracy", "cost_for_crash": 1.0},
            {"name": "time", "cost_for_crash": 1000.0},
        ],
        ta=mlp.train,
        cs=mlp.configspace,
        instances=[f"i{i}" for i in range(10)],
        instances_test=[f"i{i+10}" for i in range(10)],
        instances_val=[f"i{i+20}" for i in range(10)],
        instance_features={f"i{i}": [j for j in np.random.random(10)] for i in range(10)},
        deterministic=True,
        cutoff=1000,
        runcount_limit=25,

    )

    return scenario
