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

from smac.scenario import Scenario
from ConfigSpace.read_and_write import pcs, pcs_new
from subprocess import PIPE, Popen
import json

class ExpScenario(ABC):

    def __init__(self,
                 name: str,
                 objectives: list[dict],
                 ta: str,
                 cs: str | ConfigurationSpace,
                 instances: str | list,
                 instances_val: str | list = None,
                 instances_test: str | list = None,
                 instance_features: str | dict = None,
                 deterministic: bool = True,
                 cutoff: int = None,
                 wallclock_limit=None,
                 runcount_limit=None,
                 basedir: Path = Path("./"),
                 outputdir: Path = Path("./output/")):
        self.name = name
        self.objectives = objectives
        self.ta = ta
        self.cs = cs
        self.instances = instances
        self.instances_val = instances_val
        self.instances_test = instances_test
        self.instance_features = instance_features
        self.deterministic = deterministic
        self.cutoff = cutoff
        self.wallclock_limit = wallclock_limit
        self.runcount_limit = runcount_limit
        self.basedir = basedir
        self.outputdir = outputdir

    def __str__(self):
        return self.name

    def make_scenario(self, output_dir: Path = None, skip_args=None, seed: int = -1):

        output_dir = self.outputdir if output_dir is None else output_dir

        if isinstance(self.objectives, list):
            objectives = [obj["name"] for obj in self.objectives]
            cost_for_crash = [obj["cost_for_crash"] for obj in self.objectives]
        else:
            objectives = self.objectives["name"]
            cost_for_crash = self.objectives["cost_for_crash"]



        scenario_kwargs = {
                # "run_obj": "quality",  # we optimize quality (alternatively runtime)
                "configspace": self.get_cs(),  # configuration space
                "deterministic": self.deterministic,
                "objectives": objectives,
                "crash_cost": cost_for_crash,
                "output_directory": str(output_dir.absolute()),
                # "output_dir_for_this_run": str(output_dir.absolute()),
                "instances": self.get_instance_list(),
                # "algo": self.ta,
                "n_workers": 1,
                "seed": seed,
                "name": f"{self.__str__()}",
            }

        if skip_args is not None:
            for arg in skip_args:
                if arg in scenario_kwargs:
                    del scenario_kwargs[arg]

        if self.wallclock_limit is not None:
            scenario_kwargs["walltime_limit"] = self.wallclock_limit
        if self.runcount_limit is not None:
            scenario_kwargs["n_trials"] = self.runcount_limit
        else:
            scenario_kwargs["n_trials"] = np.inf

        if scenario_kwargs.get("walltime_limit") is None and scenario_kwargs.get("n_trials") is None:
            logging.warning("No limit given by scenario definition! Setting the wallclock limit to 600 seconds.")
            scenario_kwargs["walltime_limit"] = 600

        # if self.instance_features:
        scenario_kwargs["instance_features"] = self.get_feature_dict(only_train_instances=True)

        if self.cutoff is not None:
            scenario_kwargs["trial_walltime_limit"] = self.cutoff

        scenario = Scenario(
            **scenario_kwargs
        )

        #scenario.save()  # write to outdir
        #TODO reimplement old write function to use with paramils
        pcs_file = Path(output_dir).joinpath("configspace.pcs")
        print(f"{pcs_file=}")
        with open(pcs_file, "w") as fh:
            fh.write(pcs_new.write(scenario.configspace))

        instances_file = Path(output_dir).joinpath("training_instances.txt")
        with open(instances_file, "w") as fh:
            for i in self.get_instance_list(self.instances):
                fh.write(f"{i}\n")

        features_file = Path(output_dir).joinpath("features.txt")
        features = self.get_feature_dict(only_train_instances=True)
        feature_size = len(list(features.values())[0])
        with open(features_file, "w") as fh:
            fh.write("Instance, ")
            fh.write(", ".join([f"feature{i}" for i in range(feature_size)]))
            fh.write("\n")
            for inst, feat in features.items():
                fh.write(f"{inst}, {', '.join([f'{f}'for f in feat])}\n")

        scenario_file = Path(output_dir).joinpath("scenario.txt")

        #Mainly for ParamILS
        scenario_file_kwargs = {
            "ta": self.ta,
            "execdir": ".",
            "deterministic": self.deterministic,
            "run_obj": "quality",
            "cutoff_time": self.cutoff,
            "instances": "training_instances.txt",
            "feature_file": "features.txt",
            "initial-incumbent": "DEFAULT",
            "pcs-file": "./configspace.pcs",
            "MO": "TRUE",
            "validation": "false",
            "overall_obj": "mean",
            "wallclock_limit": scenario.walltime_limit,
            "runcount-limit": scenario.n_trials,
        }

        with open(scenario_file, "w") as fh:
            # for key, value in scenario.__dict__.items():
            for key, value in scenario_file_kwargs.items():
                fh.write(f"{key} = {value}\n")

        self.scenario = scenario

    def get_cs(self) -> ConfigurationSpace:
        if isinstance(self.cs, ConfigurationSpace):
            return self.cs

        cs = None
        try:
            with open(self.cs, "r") as fh:
                cs = pcs.read(fh)
        except NotImplementedError as e:
            print(e)
            print("Could not read configuration in old convention. Trying new one")
            with open(self.cs, "r") as fh:
                cs = pcs_new.read(fh)

        return cs

    def get_feature_dict(self, only_train_instances: bool = False) -> dict[str, list[float]]:
        if self.instance_features is None:
            # Use instance id instead
            train_instances = self.get_instance_list()
            feature_dict = {instance: [instance_number] for instance_number, instance in enumerate(train_instances)}
            return feature_dict
        if isinstance(self.instance_features, dict):
            return self.instance_features
        df = pd.read_csv(self.instance_features, delimiter=", ", engine="python")
        train_instances = self.get_instance_list()
        instance_id = df.columns[0]
        feature_dict = {}
        for instance in df[instance_id].unique():
            features = df[df[instance_id] == instance].iloc[0]
            if only_train_instances and self._get_instance(instance) not in train_instances:
                continue
            feature_vector = [float(feat) for feat in features.to_numpy()[1:]]
            feature_dict[self._get_instance(instance)] = feature_vector

        return feature_dict

    def get_instance_list(self, instance_file=None) -> list:
        if isinstance(self.instances, list):
            return self.instances
        instance_file = self.instances if instance_file is None else instance_file
        insts = []
        with open(instance_file) as fh:
            for instance in fh:
                #print(self._get_instance(instance))
                insts.append(self._get_instance(instance))
        return insts

    def _get_instance(self, instance):
        instance_path = self.basedir.joinpath(instance.strip())
        if instance_path.exists():
            return str(instance_path)
        else:
            return instance.strip()

    def run_configuration(self, configuration, instance, seed=0, cutoff=None) -> list:
        """ Run a configuration """
        #TODO callable subclass..
        if not isinstance(self.ta, str) and not isinstance(self.ta, list):
            # It is not a command line but an actual function
            cost = self.ta(configuration, instance, seed)
            results = {
                "status": "SUCCESS",
                "cost": cost,

            }
            return results

        configline = []
        for k, v in configuration.items():
            while k[0] == "-":
                k = k[1:]
            configline.append(f"{k} {v}")
        configline = " ".join(configline)

        if cutoff is None:
            cutoff = 2**32

        print(f"{self.ta=}")
        cmd = f"{self.ta} --instance {instance} --seed {seed} --cutoff {cutoff} --config {configline}"
        print(f"{cmd=}")
        cmd = cmd.split(" ")
        #Run

        p = Popen(cmd, shell=False, stdout=PIPE, stderr=PIPE, universal_newlines=True)
        stdout_, stderr_ = p.communicate()
        print(f"{stdout_=}")

        results = {"status": "CRASHED", "cost": [1234567890]*len(self.objectives)}
        for line in stdout_.split("\n"):
            if line.startswith("Result of this algorithm run:"):
                fields = ":".join(line.split(":")[1:])
                results = json.loads(fields)

        print(f"{results=}")

        if "cost" in results:
            print(f'{results["cost"]=}')
            results["cost"] = {k: v for k, v in zip([o["name"] for o in self.objectives], results["cost"])}

        return results

    def so_procedure(self, output_dir: Path = Path("./output/"), seed: int = -1):
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

        for objective in self.objectives:
            sub_scenario = copy.deepcopy(self)

            # Update wallclock limit
            if sub_scenario.wallclock_limit is not None:
                sub_scenario.wallclock_limit = self.wallclock_limit / len(self.objectives)
            if sub_scenario.runcount_limit is not None:
                sub_scenario.runcount_limit = self.runcount_limit // len(self.objectives)  # int

            sub_scenario.ta.append("--obj")
            sub_scenario.ta.append(f"{objective['name']}")
            sub_scenario.ta = " ".join(sub_scenario.ta)

            sub_scenario.objectives = objective

            sub_scenario.outputdir = sub_scenario.outputdir.joinpath(f"{objective}/")
            sub_scenario.outputdir.mkdir(parents=True, exist_ok=True)
            sub_scenario.make_scenario(skip_args=["multi_objectives", "cost_for_crash"], seed=seed)

            yield sub_scenario, []

    def default_procedure(self):
        # self.scenario.configspace
        default_config = self.scenario.configspace.get_default_configuration().get_dictionary()
        return [default_config]