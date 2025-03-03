import os
import re
import subprocess
import time
import copy
from abc import ABC, abstractmethod
from types import SimpleNamespace
import numpy as np
import pandas as pd
from pathlib import Path

from ConfigSpace import Configuration
from smac.facade.algorithm_configuration_facade import AlgorithmConfigurationFacade as ACFacade
from smac.facade.multi_objective_facade import MultiObjectiveFacade as MOFacade
from smac.random_design import ProbabilityRandomDesign
from smac.acquisition.function.expected_hypervolume import PHVI
from smac.initial_design.random_design import RandomInitialDesign
from smac.runner.aclib_runner import ACLibRunner
from smac.main.config_selector import ConfigSelector
from smac.intensifier.mixins import intermediate_update, intermediate_decision, update_incumbent
from smac.intensifier.intensifier import Intensifier

import pcsparser

class ExecuteTARunAClibMO:
    def run(
            self,
            config: Configuration,
            instance: str,
            cutoff: float = None,
            seed: int = 12345,
            budget: float = None,
            instance_specific: str = "0",
    ):
        status, cost, runtime, results = (0, 0, 0, 0)
        # super().run(config,
        #                                              instance,
        #                                              cutoff,
        #                                              seed,
        #                                              budget,
        #                                              instance_specific
        #                                              )
        print("ExecuteTARunAClibMO RESULT:", status, cost, runtime, results)
        return status, cost, runtime, results


class AbstractConfigurator(ABC):

    def __init__(self, **kwargs):
        self.kwargs = kwargs
        self.long_name = "Undefined"

    def __str__(self):
        return self.long_name

    def _include_class_kwargs(self, kwargs) -> dict:
        if isinstance(self.kwargs, dict):
            for k, v in self.kwargs.items():
                kwargs[k] = v  # kwargs.get(k, v)  # Not replacing
        return kwargs

    def run(self, scenario, seed):
        """
        Run the configuration run

        Returns
        -------

        """
        pass


class AbstractSMACConfigurator(AbstractConfigurator):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)

    @abstractmethod
    def configurator_class(self, **kwargs) -> ACFacade:
        raise NotImplementedError

    def _include_shared_components(self, kwargs: dict) -> dict:
        kwargs["config_selector"] = ConfigSelector(scenario=kwargs["scenario"],
                                                   retrain_after=None,
                                                   retrain_wallclock_ratio=0.5)

        return kwargs

    def run(self, scenario, seed):
        # init
        if not hasattr(scenario, "scenario"):
            scenario.make_scenario(seed=seed)  # outdir?

        print(scenario.ta)
        if not isinstance(scenario.ta, str) and not isinstance(scenario.ta, list):
            ta = scenario.ta
        else:
            ta = ACLibRunner(
                target_function=scenario.ta,
                scenario=scenario.scenario,
                required_arguments=["instance", "seed"]
            )

        smac_kwargs = {
            "target_function": ta,
            "scenario": scenario.scenario,
            "overwrite": True,
            "logging_level": 0,
            # "tae_runner_kwargs": {
            #     "ta": scenario.ta.split(" "),
            #     "par_factor": 2,
            #     "multi_objectives": [obj["name"] for obj in scenario.objectives], #["runtime", "quality"],
            #     "cost_for_crash": [obj["cost_for_crash"] for obj in scenario.objectives], #[float(np.log(20)), float(MAXINT)],
            # },
            # "initial_design": LHDesign,
            # "initial_design_kwargs": {
            #     "init_budget": min(64, 3*len(scenario.scenario.cs)),  # 3*d
            # },
        }

        smac_kwargs = self._include_class_kwargs(smac_kwargs)

        smac = self.configurator_class(**smac_kwargs)

        # run
        incumbent = smac.optimize()

        # parse
        result = {
            "configurator": self.long_name,
            "instance": scenario.name,
            "seed": seed,
            # "stats": smac.stats,
            "runhistory": smac.runhistory,
            "trajectory": smac.intensifier.get_save_data(),
            "incumbent": incumbent
        }

        return result


class Default(AbstractConfigurator):

    def __init__(self, **kwargs):
        """ Dummy configurator that uses the default parameters."""
        super().__init__(**kwargs)
        self.long_name = "default"

    def run(self, scenario, seed):
        # parse
        incumbent = scenario.default_procedure()

        result = {
            "configurator": self.long_name,
            "instance": scenario.name,
            "seed": seed,
            "stats": None,
            "runhistory": SimpleNamespace(ids_config={k+1: c for k, c in enumerate(incumbent)}),
            "trajectory": [{"config_ids": [k+1 for k in range(len(incumbent))], "costs": None, "trail":None, "walltime":0}],
            "incumbent": incumbent,
        }
        print(f"{result=}")
        return result


class RandomSearch(AbstractSMACConfigurator):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.long_name = "RandomSearch"

    def configurator_class(self, **kwargs):
        kwargs = self._include_shared_components(kwargs)
        kwargs["random_design"] = ProbabilityRandomDesign(probability=1)
        kwargs["config_selector"] = ConfigSelector(scenario=kwargs["scenario"],
                                                   retrain_after=np.inf)

        class NewIntensifier(intermediate_decision.DoublingNComparison,
                             intermediate_update.NoComparison,
                             Intensifier):
            pass

        kwargs["intensifier"] = NewIntensifier(scenario=kwargs["scenario"],
                                               max_config_calls=2000,
                                               max_incumbents=10)

        kwargs["initial_design"] = RandomInitialDesign(scenario=kwargs["scenario"],
                                                       n_configs=1)

        return MOFacade(**kwargs)

class RandomSearch25(AbstractSMACConfigurator):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.long_name = "RandomSearch25"

    def configurator_class(self, **kwargs):
        kwargs = self._include_shared_components(kwargs)
        kwargs["random_design"] = ProbabilityRandomDesign(probability=1)
        kwargs["config_selector"] = ConfigSelector(scenario=kwargs["scenario"],
                                                   retrain_after=np.inf)

        class NewIntensifier(intermediate_decision.Never,
                             intermediate_update.NoComparison,
                             Intensifier):
            pass

        kwargs["intensifier"] = NewIntensifier(scenario=kwargs["scenario"],
                                               min_config_calls=25,
                                               max_config_calls=25,
                                               max_incumbents=10)

        kwargs["initial_design"] = RandomInitialDesign(scenario=kwargs["scenario"],
                                                       n_configs=1)

        return MOFacade(**kwargs)



class RandomSearchSI(AbstractSMACConfigurator):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.long_name = "RandomSearchSI"

    def configurator_class(self, **kwargs):
        kwargs = self._include_shared_components(kwargs)
        kwargs["random_design"] = ProbabilityRandomDesign(probability=1)
        kwargs["config_selector"] = ConfigSelector(scenario=kwargs["scenario"],
                                                   retrain_after=np.inf)

        # Use the same intensifier as MO-SMAC
        # class NewIntensifier(intermediate_decision.DoublingNComparison,
        #                      intermediate_update.NoComparison,
        #                      Intensifier):
        #     pass

        # kwargs["intensifier"] = NewIntensifier(scenario=kwargs["scenario"],
        #                                        max_config_calls=2000,
        #                                        max_incumbents=10)

        return MOFacade(**kwargs)


class SMAC(AbstractSMACConfigurator):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.long_name = "SMAC"

    def configurator_class(self, **kwargs):
        kwargs = self._include_shared_components(kwargs)

        class NewIntensifier(intermediate_decision.DoublingNComparison,
                             intermediate_update.FullIncumbentComparison,
                             Intensifier):
            pass

        kwargs["intensifier"] = NewIntensifier(scenario=kwargs["scenario"],
                                               max_config_calls=2000,
                                               max_incumbents=10)

        return ACFacade(**kwargs)

    def run(self, scenario, seed):
        # Get rules for SO optimization
        smacruns = []
        incumbent = []

        for count, (sub_scenario, constants) in enumerate(scenario.so_procedure(seed=seed)):
            print(f"Subscenario {count}")
            print(sub_scenario.ta)

            if not isinstance(sub_scenario.ta, str) and not isinstance(sub_scenario.ta, list):
                ta = sub_scenario.ta
            else:
                ta = ACLibRunner(
                    target_function=sub_scenario.ta,
                    scenario=sub_scenario.scenario,
                    required_arguments=["instance", "seed"]
                )

            smac_kwargs = {
                "target_function": ta,
                # "tae_runner_kwargs": {
                #     "ta": sub_scenario.ta.split(" "),
                #     "par_factor": 2,
                #     # "multi_objectives": [obj["name"] for obj in sub_scenario.objectives], #["runtime", "quality"],
                #     # "cost_for_crash": [obj["cost_for_crash"] for obj in sub_scenario.objectives], #[float(np.log(20)), float(MAXINT)],
                # },
                "scenario": sub_scenario.scenario,
                "overwrite": True,
                "logging_level": 0,
            }

            smac_kwargs = self._include_class_kwargs(smac_kwargs)

            smac = self.configurator_class(**smac_kwargs)

            # run
            inc = smac.optimize()

            smacruns.append(smac)
            # add constants

            # Extend configspace
            for param, value in constants:
                sub_scenario.scenario.configspace.add_hyperparameter(param)
            cs = sub_scenario.scenario.configspace

            # Extend configuration
            inc = inc.get_dictionary()
            for param, value in constants:
                inc[param.name] = value

            # Cast to Configuration object
            inc = Configuration(cs, inc)

            incumbent.append(inc)

        # parse
        result = {
            "configurator": self.long_name,
            "instance": scenario.name,
            "seed": seed,
            # "stats": [smac.stats for smac in smacruns],
            "runhistory": [smac.runhistory for smac in smacruns],
            "trajectory": [smac.intensifier.get_save_data() for smac in smacruns],
            "incumbent": incumbent,
        }

        return result


class MOSMAC(AbstractSMACConfigurator):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.long_name = "MO-SMAC"

    def configurator_class(self, **kwargs):
        kwargs = self._include_shared_components(kwargs)

        return MOFacade(**kwargs)

    # def run(self, scenario, seed):
    #     super(MOSMAC, self).run(scenario, seed)


class MOSMACPHVI(AbstractSMACConfigurator):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.long_name = "MO-SMAC-PHVI"

    def configurator_class(self, **kwargs):
        kwargs = self._include_shared_components(kwargs)
        kwargs["acquisition_function"] = PHVI()

        return MOFacade(**kwargs)

    # def run(self, scenario, seed):
    #     super(MOSMAC, self).run(scenario, seed)


class ParEGO(AbstractSMACConfigurator):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.long_name = "ParEGO"

    def configurator_class(self, **kwargs):
        kwargs = self._include_shared_components(kwargs)
        return ACFacade(**kwargs)  # Is nativally ParEGO now


class MOParamILS(AbstractConfigurator):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.long_name = "MO-ParamILS"

    def run(self, scenario, seed):
        print(">>>>>> CONFIGURATION <<<<<<")
        print(f"{os.getcwd()=}")
        print(os.listdir(os.getcwd()))

        # Discretise PCS
        parser = pcsparser.SMACParser()
        pcspath = Path(os.getcwd()).joinpath("configspace.pcs")
        parser.load(pcspath)
        print(f"{parser.check_validity()}")
        parser.export("paramils", pcspath)
        os.system(f"cat {pcspath}")

        # Create scenario file for ParamILS
        scenariopath = Path(os.getcwd()).joinpath("scenario.txt")
        newscenario = {}
        #TODO fix new mapping. include None mappinf
        change_keys = {"train_inst_fn": "instances",
                       "feature_fn": "feature_file",
                       "ta_run_limit": "runcount-limit",
                       "initial_incumbent": "initial-incumbent",
                       "cutoff": "cutoff_time",
                       "ta": "algo"}
        with open(scenariopath, "r") as fh:
            for line in fh:
                line = line.strip()
                setting = line.split(" = ")
                key = setting[0]
                key = change_keys[key] if key in change_keys else key
                if key is None:
                    continue
                newscenario[key] = " = ".join(setting[1:])

        for key in ["pcs_fn", "ta_run_limit", "always_race_default", "save_results_instantly", "cost_for_crash",
                    "multi_objectives", "par_factor", "overall_obj"]:
            if key in newscenario:
                del newscenario[key]

        for key in ["wallclock_limit", "runcount-limit"]:
            if key in newscenario:
                if newscenario[key] == "inf":
                    continue
                newscenario[key] = round(float(newscenario[key]))

        newscenario["pcs-file"] = "./configspace.pcs"
        newscenario["MO"] = "TRUE"
        newscenario["validation"] = "false"
        newscenario["overall_obj"] = "mean"

        print(newscenario)
        with open(scenariopath, "w") as fh:
            for key, value in newscenario.items():
                if value != "inf":
                    fh.write(f"{key} = {value}\n")

        os.system(f"cat {scenariopath}")

        #Remove old output files
        os.system("rm -rf paramils-output/")

        # Run from commandline
        moparamils_exc = "/home/rook/projects/mosmac/experiments/ac/resources/configurators/moparamils2/"
        os.system(f"cp {moparamils_exc}/paramils .")
        # os.system(f"cp -r {moparamils_exc}/src/ .")
        # os.system(f"cp -r {moparamils_exc}/bin/ .")
        os.system(f"cp -r {moparamils_exc}/lib/ .")
        print(moparamils_exc)
        os.system("pwd")
        #os.system(f"./paramils --MO TRUE --scenario scenario.txt")
        #os.system(f"./paramils -h")
        with open("output.txt", "w") as fh:
            s = subprocess.call(["./paramils", "--scenario", "scenario.txt"], stdout=fh, stderr=fh)

        # Parse results to dict
        expdirname = Path(os.getcwd()).name
        outputdir = f"paramils-output/{expdirname}/"

        configurations = []

        trajectory, runhistory = _reconstruct_moparamils_trajectory(outputdir)


        # for filename in Path(outputdir).iterdir():
        #     #print(filename)
        #     if re.match("configs\d+.txt", filename.name):
        #         with open(filename) as fh:
        #             for config in fh:
        #                 configurations.append(_parse_paramils_config(config))

        result = {
            "configurator": self.long_name,
            "instance": scenario.name,
            "seed": seed,
            "stats": None,
            "runhistory": runhistory,
            "trajectory": trajectory,
            "incumbent": [runhistory.ids_config[id] for id in trajectory["trajectory"][-1]["config_ids"]],
        }

        return result


def _parse_paramils_config(config: str) -> dict:
    config = config.strip().split()
    configuration = {}
    for i in range(0, len(config), 2):
        configuration[config[i][1:]] = config[i + 1][1:-1]
    return configuration


def _reconstruct_moparamils_trajectory(outputdir: Path):
    """
    Based on the MO-ParamILS logs, reconstruct the trajactory of the configuration run
    Returns
    -------
    """
    trajectory = {}
    configs = {}
    
    for filename in Path(outputdir).iterdir():
        print(filename.name)
        if re.match("log-run\d+.txt", filename.name):
            print("FOUND LOG")
            with open(filename, "r") as fh:
                active_key = None
                start_time = None
                day_offset = 0
                last_time = 0

                for ln, line in enumerate(fh.readlines()):
                    # print(f"{ln:4} {line.strip()}")
                    # Change incumbent
                    match = re.search("New incumbent\! \#(\d+)", line)
                    if match:
                        configid = int(match.group(1))
                        # print(f"{configid=}")
                        if len(trajectory) == 0:
                            active_key = int(last_time)
                            trajectory[active_key] = [configid]
                        else:
                            new_inc = copy.copy(trajectory[active_key]) + [configid]
                            active_key = int(last_time)
                            trajectory[active_key] = new_inc

                    # Remove from incumbent
                    match = re.search("No more incumbent: \#(\d+)", line)
                    if match:
                        configid = int(match.group(1))
                        # print(f"{configid=}")
                        trajectory[active_key].remove(configid)

                    # Timestamp
                    match = re.search("^(\d{2}):(\d{2}):(\d{2})\.(\d{3})", line)
                    if match:
                        # print(match.group(0))
                        timestamp = 3600 * int(match.group(1)) + 60 * int(match.group(2)) + int(
                            match.group(3)) + int(match.group(4)) / 1000
                        # print(f"{timestamp=}")
                        if start_time is None:
                            start_time = timestamp
                        last_time = (86400 * day_offset + timestamp) - start_time
                        if last_time < 0:
                            day_offset += 1
                            last_time = (86400 * day_offset + timestamp) - start_time

            del active_key, start_time, day_offset
            
        if re.match("detailed-traj-run-\d+.csv", filename.name):
            print("FOUND TRAJECTORY")
            with open(filename, "r") as fh:
                keys = []
                df = []
                for i, line in enumerate(fh.readlines()):
                    row = {}
                    for j, field in enumerate(re.split("\",\s?\"", line.strip())):
                        field = field.replace("\"", "")
                        if i == 0:
                            keys.append(field)
                        else:
                            row[keys[j]] = field

                    if i > 0:
                        df.append(row)

                df = pd.DataFrame(df)
                for id in df["Incumbent ID"].unique():
                    configstring = df[df["Incumbent ID"] == id].iloc[0]["Full Configuration"]
                    configuration = {}
                    for parameter in configstring.split(","):
                        parameter = parameter.strip().split("=")
                        if len(parameter) == 2:
                            configuration[parameter[0]] = parameter[1][1:-1]
                    configs[int(id)] = configuration  # _parse_paramils_config(configstring)
                    
    #Check if logs match
    trajectory_configs = set()
    for _, inc in trajectory.items():
        trajectory_configs = trajectory_configs.union(inc)
    return_trajectory = []
    for wtime, inc in trajectory.items():
        return_trajectory.append({"config_ids": inc, "costs": None, "trail": None, "walltime": wtime})

    return_trajectory_data = {
        "incumbent_ids": return_trajectory[-1]["config_ids"],
        "rejected_config_ids": [i for i in range(1, max(list(configs.keys()))+1) if i not in configs],  # approximately
        "incumbents_changed": len(return_trajectory),
        "trajectory": return_trajectory,
        "state": None,
    }

    runhistory = SimpleNamespace(
        ids_config=configs
    )

    return return_trajectory_data, runhistory

