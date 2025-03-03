import copy

from ConfigSpace import (
    Categorical,
    Configuration,
    ConfigurationSpace,
    EqualsCondition,
    Float,
    InCondition,
    Integer,
)
try:
    from .base import MLExpScenario
except:
    pass
import time
import sys
import re
import os
import json
import warnings
from pathlib import Path

import numpy as np
import pandas as pd

# from genericWrapper4AC.generic_wrapper import AbstractWrapper

from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import StratifiedKFold, train_test_split
from sklearn.metrics import accuracy_score, precision_score, recall_score, f1_score
from sklearn.datasets import load_digits, load_iris

class DataSet():

    def __init__(self):
        self.data = None
        self.target = None

    def load_from_csv(self, filepath: Path):
        df = pd.read_csv(filepath, delimiter=";")
        attributes = list(df.columns)[:-1]
        target = list(df.columns)[-1]

        self.data = df[attributes].to_numpy()
        self.target = df[target].to_numpy()
        self.target = self.target == "Dropout" #TODO do this in dataset

        return self

class RandomForest:
    def __init__(self, dataset):
        self.objectives = ["precision", "recall"]
        self.dataset = dataset
    @property
    def configspace(self) -> ConfigurationSpace:
        cs = ConfigurationSpace()

        n_estimators = Integer("n_estimators", (1, 500), default=100)
        criterion = Categorical("criterion", ["gini", "entropy", "log_loss"], default="gini")
        max_depth_type = Categorical("max_depth_type", ["None", "int"], default="None")
        max_depth = Integer("max_depth", (1, 100), default=10)

        use_max_depth = EqualsCondition(child=max_depth, parent=max_depth_type, value="int")

        min_samples_split = Integer("min_samples_split", (1, 100), default=2)
        min_samples_leaf = Integer("min_samples_leaf", (1, 100), default=1)
        min_weight_fraction_leaf = Float("min_weight_fraction_leaf", (0.0, 0.5), default=0.0)

        max_features_type = Categorical("max_features_type", ["special", "float"], default="special")
        max_features_special = Categorical("max_features_special", ["sqrt", "log2", "None"], default="sqrt")
        max_features_float = Float("max_features_float", (0.0, 1.0), default=0.5)  # TODO max val to max features in dataset

        use_max_features_special = EqualsCondition(child=max_features_special, parent=max_features_type, value="special")
        use_max_features_float = EqualsCondition(child=max_features_float, parent=max_features_type, value="float")

        max_leaf_nodes_type = Categorical("max_leaf_nodes_type", ["None", "int"], default="None")
        max_leaf_nodes = Integer("max_leaf_nodes", (1, 1000), default=100)
        use_max_leaf_nodes = EqualsCondition(child=max_leaf_nodes, parent=max_leaf_nodes_type, value="int")

        min_impurity_decrease = Float("min_impurity_decrease", (0.0, 0.5), default=0.0)

        bootstrap = Categorical("bootstrap", [True, False], default=True)
        oob_score = Categorical("oob_score", [True, False], default=True)

        use_oob_score = EqualsCondition(child=oob_score, parent=bootstrap, value=True)

        max_samples_type = Categorical("max_samples_type", ["None", "float"], default="None")
        max_samples = Float("max_samples", (0.05, 0.95), default=0.8)

        use_max_samples = EqualsCondition(child=max_samples, parent=max_samples_type, value="float")
        use_max_samples2 = EqualsCondition(child=max_samples_type, parent=bootstrap, value=True)

        cs.add_hyperparameters([n_estimators, criterion, max_depth_type, max_depth,
                               min_samples_split, min_samples_leaf, min_weight_fraction_leaf,
                               max_features_type, max_features_special, max_features_float,
                               max_leaf_nodes_type, max_leaf_nodes, min_impurity_decrease,
                               bootstrap, oob_score, max_samples_type, max_samples])

        cs.add_conditions([use_max_depth, use_max_features_float, use_max_features_special,
                           use_max_leaf_nodes, use_oob_score, use_max_samples, use_max_samples2])

        return cs

    def train(self,
              config: Configuration,
              instance: str = "0",
              seed: int = 0,
              budget: int = 10) -> dict[str, float]:
        seed = int(seed)
        rng = np.random.RandomState(seed)

        max_depth = None if config["max_depth_type"] == "None" else config["max_depth"]
        max_features = config["max_features_float"] if config["max_features_type"] == "float" else config["max_features_special"]
        if max_features == "None":
            max_features = None
        max_leaf_nodes = None if config["max_leaf_nodes_type"] == "None" else config["max_leaf_nodes"]

        keys = ["n_estimators", "criterion", "min_samples_split", "min_samples_leaf",
                "min_weight_fraction_leaf", "min_impurity_decrease", "bootstrap"]
        kwargs = {k: config[k] for k in keys}
        if config["bootstrap"] == True:
            kwargs["oob_score"] = config["bootstrap"]
            max_samples = None if config["max_samples_type"] == "None" else config["max_samples"]
            kwargs["max_samples"] = max_samples


        start_time = time.time()

        with warnings.catch_warnings():
            warnings.filterwarnings("ignore")
        ###
        classifier = RandomForestClassifier(
            max_depth=max_depth,
            max_features=max_features,
            max_leaf_nodes=max_leaf_nodes,
            **kwargs
        )

        instance = int(instance)
        dataset = self.dataset
        X_train, X_test, y_train, y_test = train_test_split(dataset.data, dataset.target,
                                                            test_size=0.2, shuffle=True,
                                                            random_state=seed)

        if instance == -1:
            # test
            pass
        else:
            # Returns the 10-fold cross validation accuracy
            X_train_base = copy.copy(X_train)
            y_train_base = copy.copy(y_train)
            cv = StratifiedKFold(n_splits=10, random_state=seed, shuffle=True)  # to make CV splits consistent
            train_id, test_id = list(cv.split(X_train_base, y_train_base))[int(instance)]
            X_train = X_train_base[train_id, :]
            X_test = X_train_base[test_id, :]
            y_train = y_train_base[train_id]
            y_test = y_train_base[test_id]

        classifier.fit(X_train, y_train)

        y_pred = classifier.predict(X_test)

        model_size = 0
        for decision_tree in classifier.estimators_:
            model_size += decision_tree.tree_.node_count

        averaging = "binary"

        performances = {
            "accuracy": 1 - accuracy_score(y_test, y_pred),
            "precision": 1 - precision_score(y_test, y_pred, average=averaging),
            "recall": 1 - recall_score(y_test, y_pred, average=averaging),
            "f1": 1 - f1_score(y_test, y_pred, average=averaging),
            "time": time.time() - start_time,
            "size": model_size,
        }
        ###
        return {k: performances[k] for k in self.objectives}


def _ML_RF_(objectives, dataset: str, basedir: Path = Path(".")):
    ta = ["python3", str(Path(__file__)),
          "--dataset", dataset]

    for obj in objectives:
        ta += ["--obj", obj[0]]

    random_forest = RandomForest(dataset)

    scenario = MLExpScenario("ML_RF_STUDENTS_"+"_".join([obj[0] for obj in objectives]),
                             objectives=[{"name": obj, "cost_for_crash": cost} for obj, cost in objectives],
                             ta=" ".join(ta),
                             cs=random_forest.configspace,
                             instances=[f"{i}" for i in range(10)],
                             instances_val=[f"{i}" for i in range(10)],
                             instances_test=["-1"],
                             deterministic=True,
                             cutoff=3600,
                             runcount_limit=500,
                             basedir=basedir,
                           )

    return scenario

def ML_RF_STUDENTS_precision_recall(basedir: Path = Path(".")):
    objectives = [
        ("precision", 100),
        ("recall", 100),
    ]

    dataset = str(basedir.joinpath("instances/ml/data/students/predict_student_dropout.csv"))

    return _ML_RF_(objectives, dataset, basedir)

def ML_RF_STUDENTS_accuracy_size(basedir: Path = Path(".")):
    objectives = [
        ("accuracy", 100),
        ("size", 2**32),
    ]

    dataset = str(basedir.joinpath("instances/ml/data/students/predict_student_dropout.csv"))

    return _ML_RF_(objectives, dataset, basedir)

def ML_RF_STUDENTS_precision_recall_size(basedir: Path = Path(".")):
    objectives = [
        ("precision", 100),
        ("recall", 100),
        ("size", 2 ** 32),
    ]

    dataset = str(
        basedir.joinpath("instances/ml/data/students/predict_student_dropout.csv"))

    return _ML_RF_(objectives, dataset, basedir)


if __name__ == "__main__":
    # Wrapper for RandomForest to run MO-ParamILS
    import argparse
    import json

    parser = argparse.ArgumentParser()
    parser.add_argument("--dataset", required=True, type=Path)
    parser.add_argument("--instance", required=True, type=str)
    parser.add_argument("--seed", required=True, type=int)
    parser.add_argument("--cutoff", required=False, type=int)
    parser.add_argument("--config", required=True, action="append", nargs="+", default=[])
    parser.add_argument("--obj", required=True, action="append", nargs="+", default=[], choices=["precision", "recall", "accuracy", "f1score", "time", "size"])

    if "--config" in sys.argv:
        old_style = False
        args = parser.parse_args()
        config = []
        for c in args.config:
            config += c
        assert len(config) % 2 == 0

        dataset = args.dataset
        instance = args.instance
        seed = args.seed
        obj = []
        for o in args.obj:
            obj += o
        config = {k: v for k, v in zip(config[::2], config[1::2])}
    else:
        # ParamILS Style
        old_style = True
        args = sys.argv
        i = 1
        state = 0
        obj = []
        while True:
            if re.match("--", args[i]):
                key = args[i][2:]
                if key == "dataset":
                    dataset = args[i+1]
                if key == "obj":
                    obj.append(args[i+1])
                i += 2
                continue
            break
        # <instance name> <instance-specific
        # information> <cutoff time> <cutoff length> <seed>
        instance, _, _, _, seed = args[i:i+5]
        seed = max(min(int(seed), 2**32-1), 0)

        i += 5
        config = {k[1:]: v for k, v in zip(args[i::2], args[i+1::2])}

    dataset = DataSet().load_from_csv(dataset)
    rf = RandomForest(dataset)
    rf.objectives = obj

    print(f"{config=}")

    for k, v in config.items():
        if v == "True":
            config[k] = True
        elif v == "False":
            config[k] = False
        elif re.fullmatch("-?\d+", v):
            config[k] = int(v)
        elif re.fullmatch("-?[\d\.]+", v):
            config[k] = float(v)


    # Fix constraints
    if config["max_depth_type"] != "int" and "max_depth" in config:
        del config["max_depth"]

    if config["max_features_type"] == "float" and "max_features_special" in config:
        del config["max_features_special"]
    elif config["max_features_type"] == "special" and "max_features_float" in config:
        del config["max_features_float"]

    if config["max_leaf_nodes_type"] != "int" and "max_leaf_nodes" in config:
        del config["max_leaf_nodes"]

    if config["bootstrap"] == False:
        if "oob_score" in config:
            del config["oob_score"]
        if "max_samples_type" in config:
            del config["max_samples_type"]
        if "max_samples" in config:
            del config["max_samples"]

    if "max_samples_type" in config:
        if config["max_samples_type"] != "float" and "max_samples" in config:
            del config["max_samples"]

    config = Configuration(rf.configspace, config)
    status = "SUCCESS"
    start_time = time.time()
    try:
        result = rf.train(config, instance, seed)
    except:
        status = "CRASHED"
        result = {k: 100 for k in obj}
    run_time = time.time() - start_time

    res = {
        "status": status,
        "cost": list(result.values()),
        "runtime": 1.0,
        "mics": ""
    }
    if not old_style:
        print(f"Result of this algorithm run: {json.dumps(res)}\n")


    #Result: <status>, <runtime>, <quality>, <seed>
    if len(rf.objectives) > 1:
        cost_str = "[{}]".format(", ".join([str(float(c)) for _, c in result.items()]))
    else:
        cost_str = f"{list(result.values())[0]}"
    sys.stdout.write(f"Result: {status}, {run_time}, {cost_str}, {seed}\n")

    #f"Result for SMAC3v2: status={self.data.status};cost={cost_str};runtime={self.data.time};additional_info={self.data.additional}")
    if len(rf.objectives) > 1:
        cost_str = ",".join([str(float(c)) for _, c in result.items()])
    else:
        cost_str = f"{list(result.values())[0]}"

    sys.stdout.write(f"Result for SMAC3v2: status={status};cost={cost_str};runtime={run_time}\n")
