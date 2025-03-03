#!/usr/bin/env python3
from __future__ import annotations

import pathlib
from abc import ABC
import itertools
import argparse
import os
import pickle
import subprocess
import yaml
from pathlib import Path
import logging
import time
import tempfile
import copy
import uuid
import pandas as pd
import logging
import shutil

#local imports

import configurators
import scenarios

from ConfigSpace import Configuration

class Action(ABC):

    def __init__(self,
                 name: str,
                 fn: callable,
                 callback: callable = None,
                 dependency: list | str = None,
                 reduce: list = None,
                 reduce_fn: callable = None,
                 expand: list = None,
                 expand_fn: callable = None,
                 chunk_size: int = 1,
                 ):
        self.name = name
        self.fn = fn
        self.callback = callback
        self.dependency = dependency if dependency is not None else []
        if not isinstance(self.dependency, list):
            self.dependency = [self.dependency]
        self.reduce = reduce if reduce is not None else []
        self.reduce_fn = reduce_fn
        self.expand = expand if expand is not None else []
        self.expand_fn = expand_fn
        self.chunk_size = chunk_size


class Experiment(ABC):

    def __init__(self, experiment_space: dict, action_space: dict, pipelines: dict = None):
        self.experiment_space = experiment_space
        self.action_space = action_space
        self.pipelines = pipelines

        self.args = None

        self.main()

    # MAIN
    def main(self):
        self.args = self._parse_arguments()

        if self.args.modus is None:
            raise ValueError("Unknown experimental action!")

        modus = self.args.modus
        if modus == "launch":
            self.launch()
        elif modus == "run":
            self.run()
        elif modus == "pipeline":
            self.pipeline()

    def _parse_arguments(self):
        parser = argparse.ArgumentParser(
            prog="Experiment Controller",
            description="Generate, launch, validate and analyse experiments"
        )

        parser.add_argument("-c", "--config", default="config.yaml")

        parser.add_argument("-d", "--expdir",
                            required=False,
                            default=None,
                            help="Base directory to store the experimental scripts in",
                            type=Path)

        parser.add_argument("-t", "--targetdir",
                            required=False,
                            default=None,
                            help="Base directory to store results of an experiment in",
                            type=Path)

        parser.add_argument("--sbatch", required=False, nargs=2, action="append", dest="sbatch_args", default=[])

        parser.add_argument("--sbatch-array-limit", required=False, type=int, dest="sbatch_array_limit")

        parser.add_argument("-n", "--name",
                            default=None,
                            required=False,
                            help="Name of the experiment",
                            )

        parser.add_argument("-a", "--action",
                            default=list(self.action_space.keys())[0],
                            choices=self.action_space.keys(),
                            help="Available actions",
                            dest="action")

        parser.set_defaults(dummy=True)
        parser.add_argument("--dummy", action="store_false")

        subparsers = parser.add_subparsers(
            help="The experiment modus: [launch, run]",
            dest="modus"
        )

        launch_parser = subparsers.add_parser("launch")
        run_parser = subparsers.add_parser("run")

        launch_parser.add_argument("-r", "--replace",
                                help="Replace existing results. TODO implement")

        launch_parser.set_defaults(repair=False)
        launch_parser.add_argument("-f", "--repair",
                                   help="Repair failed results", action="store_true")

        launch_parser.set_defaults(runlocal=False)
        launch_parser.add_argument("--local", action="store_true", dest="runlocal")

        for action_key, action in self.action_space.items():
            if len(action.expand) > 0:
                print(action)
                for argument in action.expand:
                    run_parser.add_argument(f"--{self.get_action_argument_name(action_key, argument)}",
                                            required=False,  # Check after parsing if is needed
                                            default=None,
                                            )

        # validation specific arguments
        run_parser.add_argument("--instance", type=int, default=-1, dest="validation_instance_id", required=False)
        run_parser.add_argument("--configuration", type=int, default=-1, dest="validation_config_id", required=False)


        for key, options in self.experiment_space.items():
            option_names = [str(o) for o in options]
            launch_parser.add_argument(f"--{key}", required=False, action="append", nargs="+", default=[], choices=option_names)
            run_parser.add_argument(f"--{key}", required=False, choices=option_names) #TODO check required args for action

        args = parser.parse_args()
        action = self.action_space[args.action]

        if args.modus == "launch":
            for key, options in self.experiment_space.items():
                if not hasattr(args, key) or len(getattr(args, key)) == 0:
                    # All options
                    option_names = [str(o) for o in options]
                    setattr(args, key, option_names)
                elif isinstance(getattr(args, key), list):
                    option_names = []
                    for opt in getattr(args, key):
                        if isinstance(opt, str):
                            option_names.append(opt)
                        elif isinstance(opt, list):
                            option_names += opt
                    setattr(args, key, option_names)

        print(args)

        # Configuration file
        if hasattr(args, "config"):
            config_file = Path(args.config)
            if config_file.exists():
                config = yaml.safe_load(config_file.read_text())
                for key, value in config.items():
                    if key == "sbatch":
                        print(config["sbatch"])
                        sbatchkeys = [a[0] for a in args.sbatch_args]
                        for sbatchkey, sbatchval in value.items():
                            if sbatchkey not in sbatchkeys:
                                logging.info(f"Read sbatch value for {sbatchkey} from config file.")
                                args.sbatch_args.append((sbatchkey, sbatchval))
                    else:
                        if not hasattr(args, key) or getattr(args, key) is None:
                            logging.info(f"Read value for {key} from config file.")
                            setattr(args, key, value)

        # Set defaults if they are not set by now
        if args.expdir is None:
            args.expdir = "./output"
        args.expdir = Path(args.expdir)
        if args.targetdir is None:
            args.targetdir = "./results"
        args.targetdir = Path(args.targetdir)

        # Expand name with experiment settings
        basename = "" if args.name is None else args.name
        nameargs = [basename, args.modus, args.action]
        for key in self.experiment_space.keys():
            if key in action.reduce:
                continue
            if args.modus == "launch":
                if len(getattr(args, key)) == 1:
                    nameargs.append(getattr(args, key)[0])
                elif len(getattr(args, key)) == len(self.experiment_space[key]):
                    nameargs.append("all")
                else:
                    nameargs.append("multiple")
            elif args.modus == "run":
                nameargs.append(getattr(args, key))
        args.name = "_".join(nameargs)

        # Check if action arguments are met
        if args.modus == "run" and len(action.expand) > 0:
            for argument in action.expand:
                if not hasattr(args, self.get_action_argument_name(args.action, argument)):
                    raise ValueError(f"action argument '{self.get_action_argument_name(args.action, argument)}' missing. Aborting run!")

        print(f"{args=}")

        return args

    # HELPER FUNCTIONS
    def get_action(self, action=None) -> Action:
        action = self.args.action if action is None else action
        return self.action_space[action]

    def _get_dir(self, directory: Path, action: str = None):
        action = self.args.action if action is None else action
        return directory.joinpath(f"{action}/")

    def get_expdir(self, action: str = None):
        return self._get_dir(self.args.expdir, action)

    def get_targetdir(self, action: str = None):
        return self._get_dir(self.args.targetdir, action)

    def get_action_argument_name(self, action: str, argument: str):
        return f"{action}_{argument}"

    def get_exp_run_name(self, **kwargs):
        # Make sure each option is in the kwargs
        exp_name = []
        for key, item in kwargs.items():
            if key not in self.experiment_space.keys():
                raise ValueError(f"{key} not in experiment space!")
            exp_name.append(str(item).replace("_", "-"))
        exp_name = "_".join(exp_name)
        return exp_name

    def get_actual_experiment(self, experiment_string: dict) -> dict:
        """ Returns the experiment space objects from their names """
        experiment = {}
        for key, string_val in experiment_string.items():
            option_names = [str(o) for o in self.experiment_space[key]]
            value_index = option_names.index(string_val)
            experiment[key] = self.experiment_space[key][value_index]
        return experiment

    def save_result(self, result: dict, experiment: dict, action: str = None, **expand_arguments):
        action = action if action is not None else self.args.action
        result = {
            "experiment": {k: str(v) for k, v in experiment.items()},
            "run_result": result,
            "action": action,
            "timestamp": time.time()
        }

        if len(expand_arguments) == 0:
            # Save in pickle
            result_file = self.get_targetdir(action).joinpath(f"{self.get_exp_run_name(**experiment)}.pickle")
        else:
            result["expand_arguments"] = expand_arguments
            # Directory
            result_dir = self.get_targetdir(action).joinpath(f"{self.get_exp_run_name(**experiment)}/")
            result_dir.mkdir(parents=True, exist_ok=True)
            filename = "_".join([str(v) for v in expand_arguments.values()])
            filename = f"{filename}.pickle"
            result_file = result_dir.joinpath(filename)

        with open(result_file, "wb") as fh:
            pickle.dump(result, fh, protocol=5)


    def get_result(self, experiment: dict, action: str):
        action = self.get_action(action)
        exp_name = self.get_exp_run_name(**experiment)

        if len(action.expand) == 0:
            # Get pickle
            result_file = self.get_targetdir(action.name).joinpath(f"{exp_name}.pickle")
            if not result_file.exists():
                message = f"File '{result_file}' does not exist. Please launch the action {action.name} on {exp_name} first."
                raise FileExistsError(message)
            with open(result_file, "rb") as fh:
                result = pickle.load(fh)

            # Legacy
            if "run_result" not in result.keys():
                result = {
                    "experiment": {k: str(v) for k, v in experiment.items()},
                    "run_result": result,
                    "action": action,
                }
            return result
        else:
            result_dir = self.get_targetdir(action.name).joinpath(f"{exp_name}/")
            if not result_dir.exists():
                message = f"Directory '{result_dir}' does not exist. Please launch the action {action.name} on {exp_name} first."
                raise FileExistsError(message)
            result = None
            for result_file in result_dir.iterdir():
                with open(result_file, "rb") as fh:
                    res = pickle.load(fh)

                if result is None:
                    result = copy.copy(res)
                    result["expand_arguments"] = list(result["expand_arguments"].keys())
                    result["run_result"] = {}

                result["run_result"][tuple(res["expand_arguments"].items())] = res["run_result"]

            # TODO check if all expand options are in
            # for expand_arguments in action.expand_fn(experiment, self):
            #     result_file = result_dir.joinpath("_".join([str(v) for v in expand_arguments.values()]))

            return result

    # Actions
    def launch(self):
        """Launches the execution of a series of experiments"""
        args = self.args
        print("Launch experiment!!")

        combinations = []
        experiment_space_keys = []
        reduce_combinations = {}
        for key in self.experiment_space.keys():
            # if "reduce" in self.get_action().keys():
            if len(self.get_action().reduce) > 0:
                if key in self.get_action().reduce:
                    reduce_combinations[key] = getattr(args, f"{key}")
                    continue
            combinations.append(getattr(args, key))
            experiment_space_keys.append(key)

        print(combinations)
        print(reduce_combinations)

        action = self.get_action()

        possible_runs = 0
        actual_runs = 0

        # Argument lines
        run_arguments = []
        for run in itertools.product(*combinations):
            run_args = " ".join([f"--{k} {v}" for k, v in zip(experiment_space_keys, run)])
            # TODO Add inquiry data and expand on the combinations
            # if "inquiry" in self.action_space[args.action].keys():

            # TODO Add constraints to the experimental space
            experiment = {k: v for k, v in zip(experiment_space_keys, run)}
            experiment = self.get_actual_experiment(experiment)

            # Clear previous results
            print(f"{run=}")
            if len(action.expand) > 0:
                # Remove dir if exists
                # TODO remove only expand field
                result_dir = self.get_targetdir().joinpath(f"{self.get_exp_run_name(**experiment)}/")
                print(f"> {result_dir=}")
                kwargs_runs = action.expand_fn(experiment, self)
                for kwarg_run in kwargs_runs:
                    possible_runs += 1

                    if result_dir.exists():
                        filename = "_".join([str(v) for v in kwarg_run])
                        filename = f"{filename}.pickle"
                        result_file = result_dir.joinpath(filename)
                        if args.repair and result_file.exists():
                            # If there is no check only queue missing files
                            # print(f"Result file exists for {result_file}. Skipping!")
                            continue

                        if args.dummy and result_file.exists():
                            print(f"Removing {result_file}")
                            result_file.unlink(missing_ok=True)

                    kwarg_args = " ".join([f"--{self.get_action_argument_name(args.action, k)} {v}" for k, v in
                                           zip(action.expand, kwarg_run)])
                    run_arguments.append(f"{run_args} {kwarg_args}")
                    actual_runs += 1
            else:
                possible_runs += 1
                # Remove file if exists
                result_file = self.get_targetdir().joinpath(f"{self.get_exp_run_name(**experiment)}.pickle")
                if result_file.exists():
                    if args.repair:
                        # print(f"Result file exists for {result_file}. Skipping!")
                        continue
                    if args.dummy:
                        print(f"Removing {result_file}")
                        result_file.unlink()
                run_arguments.append(run_args)
                actual_runs += 1

            # # Clear previous results OLD
            # if len(action.expand) > 0:
            #     # Remove dir if exists
            #     result_dir = self.get_targetdir().joinpath(f"{self.get_exp_run_name(**experiment)}/")
            #     if result_dir.exists():
            #         print(f"Removing {result_dir}")
            #         # result_dir.rmdir()
            #         shutil.rmtree(result_dir)
            # else:
            #     # Remove file if exists
            #     result_file = self.get_targetdir().joinpath(f"{self.get_exp_run_name(**experiment)}.pickle")
            #     if result_file.exists():
            #         print(f"Removing {result_file}")
            #             result_file.unlink()
            #
            #
            # if len(action.expand) > 0:
            #     experiment = self.get_actual_experiment(experiment)
            #     # TODO make nicer:
            #     #experiment_name = self.get_exp_run_name(**experiment)
            #     #setattr(args, "experiment_name", experiment_name)
            #     #
            #     # if len(reduce_combinations) == 0:
            #     #     experiment = self.get_actual_experiment(experiment)
            #     # else:
            #     #     experiments = []
            #     #     for reduce_runs in itertools.product(*reduce_combinations.values()):
            #     #         for rkey, rval in zip(reduce_combinations.keys(), reduce_runs):
            #     #             experiment[rkey] = rval
            #     #
            #     #         experiments.append(self.get_actual_experiment(experiment))
            #     #     experiment = experiments
            #
            #     kwargs_runs = action.expand_fn(experiment, self)
            #
            #     for kwarg_run in kwargs_runs:
            #         kwarg_args = " ".join([f"--{self.get_action_argument_name(args.action, k)} {v}" for k, v in zip(action.expand, kwarg_run)])
            #         run_arguments.append(f"{run_args} {kwarg_args}")
            # else:
            #     run_arguments.append(run_args)


        print(f"Total number of independent runs={len(run_arguments)}")

        # Create dest dir
        exp_dir_path = Path(args.expdir).joinpath(args.action)
        try:
            exp_dir_path.mkdir(parents=True, exist_ok=True)
        except OSError:
            print("Cannot create exp dir")

        # Create target dir
        target_dir_path = Path(args.targetdir).joinpath(args.action)
        try:
            target_dir_path.mkdir(parents=True, exist_ok=True)
        except OSError:
            print("Cannot create exp dir")


        chunksize = 10000
        chunkid = 0
        while chunkid*chunksize < len(run_arguments):

            # Create file
            launch_name = f"launch_{args.name}.sh"
            launch_name = f"launch_{uuid.uuid4().hex}.sh"

            chunk_start = chunkid * chunksize
            chunk_end = min(chunk_start + chunksize-1, len(run_arguments)-1)
            print(chunk_start, chunk_end)

            # SLURM lines
            sbatch_args = []
            sbatch_args.append(f"--job-name=exp_{args.name}_{chunkid}")
            for sbatchkey, sbatchval in args.sbatch_args:
                sbatch_args.append(f"--{sbatchkey}={sbatchval}")
            logoutput = exp_dir_path.joinpath(f"{args.name}_%a.out")
            sbatch_args.append(f"--output={logoutput}")
            sbatch_args.append(f"--array=0-{chunk_end - chunk_start}")
            if hasattr(args, "sbatch_array_limit"):
                sbatch_args[-1] += f"%{args.sbatch_array_limit}"

            # Pulling it all together
            script = ["#!/usr/bin/bash"]
            script += [f"#SBATCH {line}" for line in sbatch_args]
            script.append("")
            script.append("experiment=( \"" + "\" \\\n\"".join(run_arguments[chunk_start:chunk_end+1]) + "\" )")
            script.append("")
            pass_args_on = f"--name {args.name} --expdir {args.expdir} --targetdir {args.targetdir} --config {args.config}"
            script.append(f"python3 ./run_experiment.py --action {args.action} {pass_args_on} run ${{experiment[$SLURM_ARRAY_TASK_ID]}}")

            #print("\n".join(script))
            chunkid += 1

            # TODO save jobids for pipelines
            if args.dummy:
                print(launch_name)
                # Write to file
                with open(launch_name, "w") as fh:
                    fh.write("\n".join(script)+"\n")

                os.chmod(launch_name, 0o755)

                subprocess.run(["sbatch", launch_name])
            else:
                print("\n".join(script))
                print(script[-1][:-35] + run_arguments[-1])

            print(f"There were {possible_runs} possible runs of which {actual_runs} were queued.")

    def run(self):
        """ Runs one specific experiment """
        args = self.args
        action = self.get_action()

        experiment = {k: getattr(args, f"{k}") for k in self.experiment_space.keys() if k not in action.reduce}
        experiment_name = self.get_exp_run_name(**experiment)
        setattr(self.args, "experiment_name", experiment_name)

        # Retrieve experiment
        experiment = self.get_actual_experiment(experiment)

        # TODO retrieve dependency
        # previous_results = {}
        # for dependency in action.dependency:
        #     if len(action.reduce) == 0:
        #         # No reduce
        #
        #     else:
        #         # Reduce

        # Attach action specific arguments
        action_arguments = {}
        action_data = self.action_space[args.action]
        # if "kwargs" in action_data.keys():
        if len(action.expand) > 0:
            for argument in action.expand:
                argument_name = self.get_action_argument_name(action.name, argument)
                if not hasattr(args, argument_name):
                    raise ValueError(f"Expected the argument '{argument_name}'.")
                action_arguments[argument] = getattr(args, argument_name)

        result = action.fn(experiment, self, **action_arguments)

        self.save_result(result, experiment, **action_arguments)

        if action.callback is not None:
            action.callback(experiment, self)

    def pipeline(self):
        args = self.args
        raise NotImplementedError


if __name__ == "__main__":
    aclib_basedir = Path("/home/rook/projects/mosmac/experiments/aclib/aclib2/")
    # aclib_basedir = Path("/Users/jeroen/Documents/Work/Projects/SMSEMOAAS/experiments/aclib/aclib2/")

    scenario_list = []
    #ML
    scenario_list.append(scenarios.ML_RF_STUDENTS_precision_recall(aclib_basedir))
    scenario_list.append(scenarios.ML_RF_STUDENTS_accuracy_size(aclib_basedir))
    scenario_list.append(scenarios.ML_RF_STUDENTS_precision_recall_size(aclib_basedir))

    scenario_list.append(scenarios.EA_OO_MMMOOP())
    scenario_list.append(scenarios.SAT_CMS_QUEENS_runtime_memory(aclib_basedir))
    scenario_list.append(scenarios.SAT_CMS_QUEENS_runtime_solved(aclib_basedir))
    scenario_list.append(scenarios.SAT_ganak_DQMR_runtime_memory())
    # Test
    scenario_list.append(scenarios.SAT_test(aclib_basedir))
    scenario_list.append(scenarios.MIP_CPLEX_REGIONS200_cutoff(aclib_basedir))
    scenario_list.append(scenarios.MIP_CPLEX_REGIONS200_runtime(aclib_basedir))
    scenario_list.append(scenarios.MIP_CPLEX_REGIONS200_CONT_cutoff(aclib_basedir))
    scenario_list.append(scenarios.MIP_CPLEX_REGIONS200_CONT_runtime(aclib_basedir))


    #MO configurators
    configurator_list = []
    configurator_list.append(configurators.Default())
    configurator_list.append(configurators.MOSMACPHVI())
    configurator_list.append(configurators.ParEGO())
    configurator_list.append(configurators.MOParamILS())
    configurator_list.append(configurators.SMAC())  # SO baseline
    configurator_list.append(configurators.RandomSearch())
    configurator_list.append(configurators.RandomSearch25())

    seeds = list(range(20))

    exp_space = {"scenarios": scenario_list,
                 "configurators": configurator_list,
                 "seeds": seeds,
                 }

    actions = {}

    def configure_action(experiment, expclass: Experiment):
        """ Configure a scenario with a seed """

        args = expclass.args
        # set outdir
        output_dir = expclass.get_expdir().joinpath(f"{args.experiment_name}/")
        output_dir.mkdir(parents=True, exist_ok=True)
        print(f"{output_dir=}")
        experiment["scenarios"].outputdir = output_dir
        experiment["scenarios"].make_scenario(seed=experiment["seeds"])

        pwd = os.getcwd()
        os.chdir(str(output_dir))  # Move to execdir to handle logfiles created in wrappers better...

        result = experiment["configurators"].run(experiment["scenarios"], experiment["seeds"])

        os.chdir(pwd)

        # result_file = expclass.get_targetdir().joinpath(f"{args.experiment_name}.pickle")
        # with open(result_file, "wb") as fh:
        #     pickle.dump(result, fh, protocol=5)

        return result

    def configure_callback(experiment, expclass: Experiment):
        # TODO make more elegant...
        args = expclass.args

        exp_dir_path = args.expdir  # "/".join(exp_dir_path.split("/")[:-1])
        target_dir_path = args.targetdir  # "/".join(target_dir_path.split("/")[:-1])
        pass_args_on = f"./run_experiment.py --name {args.name} --expdir {exp_dir_path} --targetdir {target_dir_path} " \
                       f"--config {args.config} --action validate " \
                       f"--sbatch mem 3gb " \
                       f"launch " \
                       f"--scenarios {experiment['scenarios']} " \
                       f"--configurators {experiment['configurators']} " \
                       f"--seeds {experiment['seeds']}"

        # subprocess.run(pass_args_on.split(" "))

        pass_args_on = f"./run_experiment.py --name {args.name} --expdir {exp_dir_path} --targetdir {target_dir_path} " \
                       f"--config {args.config} --action test " \
                       f"--sbatch mem 3gb " \
                       f"launch " \
                       f"--scenarios {experiment['scenarios']} " \
                       f"--configurators {experiment['configurators']} " \
                       f"--seeds {experiment['seeds']}"

        # subprocess.run(pass_args_on.split(" "))


    # actions["configure"] = {"callback": configure_action}
    actions["configure"] = Action("configure",
                                  configure_action,
                                  callback=configure_callback,)

    # Expand functions
    def benchmark_expand_fn(experiment, expclass: Experiment, instance_set: str) -> iter:
        """ Retrieves the extra kwargs and add them to the launch """
        args = expclass.args

        # get configuration results pickle
        # configuration_pickle = expclass.get_expdir(action="configure").joinpath(f"{expclass.get_exp_run_name(**experiment)}.pickle")
        # if not configuration_pickle.exists():
        #     raise ValueError(
        #         f"Configuration results for {args.experiment_name} do not exist! Please configure first or wait until configuration has finished!")
        #
        # with open(configuration_pickle, "rb") as fh:
        #     result = pickle.load(fh)

        full_result = expclass.get_result(experiment, "configure")
        result = full_result["run_result"]

        if "incumbent" in result:
            incumbent = result["incumbent"]
        else:
            incumbent = result["trajectory"][-1].incumbent
        if not isinstance(incumbent, list):
            incumbent = [incumbent]
        incumbent = list(range(len(incumbent)))

        # get scenario instance
        # scenario = experiment["scenarios"]
        # instance_set = getattr(scenario, instance_set)
        # instances = scenario.get_instance_list(instance_set) if isinstance(instance_set, str) else instance_set
        # instances = list(range(len(instances)))
        seeds = [0]

        return itertools.product(incumbent, seeds)

    def validate_expand_fn(experiment, expclass: Experiment) -> iter:
        return benchmark_expand_fn(experiment, expclass, "instances_val")

    def test_expand_fn(experiment, expclass: Experiment) -> iter:
        return benchmark_expand_fn(experiment, expclass, "instances_test",)


    def benchmark_trajectory_expand_fn(experiment, expclass: Experiment, instance_set: str) -> iter:
        if str(experiment["configurators"]) == "default":
            return []

        full_result = expclass.get_result(experiment, "configure")
        result = full_result["run_result"]
        if isinstance(result["trajectory"], list):
            print("SMAC RUN DETECTED!")
            print(f"{experiment=}")
            # SMAC
            incumbent_config_ids = set()
            for trid, tr in enumerate(result["trajectory"]):
                config_ids = set(itertools.chain(*[i["config_ids"] for i in tr["trajectory"]]))
                last_incumbent = set(tr["trajectory"][-1]["config_ids"])
                print(f"{last_incumbent=}")
                config_ids = config_ids.difference(last_incumbent)
                config_ids = [f"{trid}_{cid}" for cid in config_ids]
                print(f" > {trid}: {config_ids}")
                incumbent_config_ids = incumbent_config_ids.union(config_ids)
        else:
            incumbent_config_ids = set(itertools.chain(*[i["config_ids"] for i in result["trajectory"]["trajectory"]]))
            last_incumbent = set(result["trajectory"]["trajectory"][-1]["config_ids"])
            incumbent_config_ids = incumbent_config_ids.difference(last_incumbent)

        seeds = [0]

        return itertools.product(incumbent_config_ids, seeds)

    def validate_trajectory_expand_fn(experiment, expclass: Experiment) -> iter:
        return benchmark_trajectory_expand_fn(experiment, expclass, "instances_val")

    def test_trajectory_expand_fn(experiment, expclass: Experiment) -> iter:
        return benchmark_trajectory_expand_fn(experiment, expclass, "instances_test",)

    # Run instances
    def run_action(experiment, expclass: Experiment, configuration, seed, instance_set):
        """ Run the target algorithm with a configuration with an instance """

        # get configuration results pickle
        full_result = expclass.get_result(experiment, action="configure")
        result = full_result["run_result"]

        if "incumbent" in result:
            incumbent = result["incumbent"]
        else:
            incumbent = result["trajectory"][-1].incumbent

        if not isinstance(incumbent, list):
            incumbent = [incumbent]
        configuration = incumbent[int(configuration)]

        scenario = experiment["scenarios"]
        return run_configuration(scenario, configuration, seed, instance_set)


    def validate_action(*args, **kwargs):
        return run_action(*args, **kwargs, instance_set="instances_val")


    def test_action(*args, **kwargs):
        return run_action(*args, **kwargs, instance_set="instances_test")

    def run_trajectory_action(experiment, expclass: Experiment, configuration, seed, instance_set):
        print(experiment)
        # get configuration results pickle
        full_result = expclass.get_result(experiment, action="configure")
        result = full_result["run_result"]

        scenario = experiment["scenarios"]

        if isinstance(result["runhistory"], list):
            # SMAC
            print("SMAC!")
            trid, configid = [int(i) for i in configuration.split("_")]
            configuration = result["runhistory"][trid].ids_config[configid]

            nscenario = copy.deepcopy(scenario)
            sub_scenario, constants = list(nscenario.so_procedure(seed=seed))[trid]
            print(f"{constants=}")

            # Extend configspace
            for param, value in constants:
                sub_scenario.scenario.configspace.add_hyperparameter(param)
            cs = sub_scenario.scenario.configspace

            # Extend configuration
            configuration = configuration.get_dictionary()
            for param, value in constants:
                configuration[param.name] = value

            # Cast to Configuration object
            configuration = Configuration(cs, configuration)

        else:
            configuration = result["runhistory"].ids_config[int(configuration)]

        return run_configuration(scenario, configuration, seed, instance_set)

    def validate_trajectory_action(*args, **kwargs):
        return run_trajectory_action(*args, **kwargs, instance_set="instances_val")

    def test_trajectory_action(*args, **kwargs):
        return run_trajectory_action(*args, **kwargs, instance_set="instances_test")

    def run_configuration(scenario: Scenario, configuration, seed, instance_set):
        instance_set = getattr(scenario, instance_set)
        instances = scenario.get_instance_list(instance_set) if isinstance(instance_set, str) else instance_set
        # instances = list(range(len(instances)))
        run_results = []
        for iid, instance in enumerate(instances):
            print(iid, instance, configuration, seed)

            # instance_set = getattr(scenario, instance_set)
            # instances = scenario.get_instance_list(instance_set) if isinstance(instance_set, str) else instance_set
            # instance = instances[int(instance)]

            pwd = os.getcwd()
            tempdir = tempfile.mkdtemp()
            print(f"{tempdir=}")
            os.chdir(tempdir)
            run_result = scenario.run_configuration(configuration, instance, seed, scenario.cutoff)
            os.chdir(pwd)
            os.system(f"rm -r {tempdir}")

            print(f"{run_result=}")
            run_results.append(run_result)

        result = {
            "seed": seed,
            "configuration": configuration,
            "run_result": run_results
        }

        return result



    actions["validate"] = Action("validate",
                                 validate_action,
                                 expand=["configuration", "seed"],
                                 expand_fn=validate_expand_fn)

    actions["test"] = Action("test",
                             test_action,
                             expand=["configuration", "seed"],
                             expand_fn=test_expand_fn)

    actions["validatetraj"] = Action("validatetraj",
                                     validate_trajectory_action,
                                     expand=["configuration", "seed"],
                                     expand_fn=validate_trajectory_expand_fn)

    actions["testtraj"] = Action("testtraj",
                                     test_trajectory_action,
                                     expand=["configuration", "seed"],
                                     expand_fn=test_trajectory_expand_fn)

    ## Get Performance TAbles
    def collect_performance(base_exp, expclass: Experiment):
        def make_df(results):
            #Check if it is old or new procedure
            if isinstance(list(results["run_result"].values())[0]["run_result"], dict):
                print("Instances are in ran in parallel")
                df = []
                for expand_args, result in results["run_result"].items():
                    res = {"action": results["action"]}
                    res.update(results["experiment"])
                    res.update({k: v for k, v in expand_args})
                    res.update(result["run_result"])
                    del res["cost"]
                    res.update(result["run_result"]["cost"])

                    df.append(res)
            else:
                print("Instances are in ran in sequence")
                df = []
                for expand_args, result in results["run_result"].items():
                    for iid, instance_result in enumerate(result["run_result"]):
                        res = {"action": results["action"]}
                        res.update(results["experiment"])
                        res.update({k: v for k, v in expand_args})
                        res.update(instance_result)
                        res["instance"] = iid
                        # print(f"{res=}")
                        if "cost" in res:
                            del res["cost"]
                        if "cost" not in instance_result:
                            continue
                        # print(f"{instance_result=}")
                        res.update(instance_result["cost"])

                        df.append(res)

            df = pd.DataFrame(df)

            return df

        df = None

        # reduce_options = [expclass.experiment_space[dim] for dim in expclass.action_space[args.action]]
        reduce_options = [expclass.experiment_space[dim] for dim in ["configurators", "seeds"]]
        all_experiments = []
        for configurator, seed in itertools.product(*reduce_options):
            # if str(configurator) == "default" and seed != 0:
            #     continue
            print(configurator, seed)
            experiment = copy.copy(base_exp)
            experiment["configurators"] = configurator
            experiment["seeds"] = seed

            # get run results
            for a in ["validate", "test"]:
                print(f"Getting the {a} results from {configurator}-{seed}")
                try:
                    validate_result = expclass.get_result(experiment, action=a)
                except:
                    print("No results yet!")
                    continue
                val_df = make_df(validate_result)

                if df is None:
                    df = val_df
                else:
                    df = pd.concat([df, val_df])

                print(f"Df has {len(df)} rows now")

        print(df.head())
        print(f"{len(df)} rows")
        return df

        # val_performances = val_df.groupby(["configurators", "seeds", "configuration"])[objectives].mean()
        # test_performances = test_df.groupby(["configurators", "seeds", "configuration"])[objectives].mean()
        #
        # print(val_performances)


    actions["collect"] = Action("collect",
                                collect_performance,
                                reduce=["configurators", "seeds"],
                                reduce_fn=lambda x: x)

    def collect_performance_traj(base_exp, expclass: Experiment):
        def make_df(results):
            # print(f"{type(results)=}")
            # print(f"{results.keys()=}")
            # print(f"{results=}")
            #Check if it is old or new procedure
            if isinstance(list(results["run_result"].values())[0]["run_result"], dict):
                print("Instances are ran in parallel")
                df = []
                for expand_args, result in results["run_result"].items():
                    res = {"action": results["action"]}
                    res.update(results["experiment"])
                    res.update({k: v for k, v in expand_args})
                    res.update(result["run_result"])
                    del res["cost"]
                    res.update(result["run_result"]["cost"])

                    df.append(res)
            else:
                print("Instances are ran in sequence")
                df = []
                for expand_args, result in results["run_result"].items():
                    for iid, instance_result in enumerate(result["run_result"]):
                        res = {"action": results["action"]}
                        res.update(results["experiment"])
                        res.update({k: v for k, v in expand_args})
                        res.update(instance_result)
                        res["instance"] = iid
                        del res["cost"]
                        res.update(instance_result["cost"])

                        df.append(res)

            df = pd.DataFrame(df)

            return df

        df = None

        reduce_options = [expclass.experiment_space[dim] for dim in ["configurators", "seeds"]]
        for configurator, seed in itertools.product(*reduce_options):
            print(configurator, seed)
            experiment = copy.copy(base_exp)
            experiment["configurators"] = configurator
            experiment["seeds"] = seed

            # get run results
            for a in ["validatetraj", "testtraj"]:
                try:
                    validate_result = expclass.get_result(experiment, action=a)
                except:
                    print("No results yet!")
                    continue
                val_df = make_df(validate_result)

                if df is None:
                    df = val_df
                else:
                    df = pd.concat([df, val_df])

                print(f"Df has {len(df)} rows now")

        print(df.head())
        print(f"{len(df)} rows")
        return df

    actions["collecttraj"] = Action("collecttraj",
                                    collect_performance_traj,
                                    reduce=["configurators", "seeds"],
                                    reduce_fn=lambda x: x)

    def get_trajectory(experiment, expclass: Experiment):
        args = expclass.args
        output_dir = expclass.get_expdir() / "../configure"
        output_dir = output_dir.resolve().joinpath(f"{args.experiment_name}/")
        print(f"Search in '{output_dir}'")
        if experiment['configurators'].long_name == "MO-ParamILS":
            output_dir /= f"paramils-output/{args.experiment_name}"
            if not output_dir.exists():
                olddir = Path(str(output_dir).replace("ac6", "ac5"))
                if olddir.exists():
                    print(f"Changed to old scratch dir: f{olddir}")
                    output_dir = olddir
                else:
                    print(f"Cannot find results for {experiment}!")
                    return

            trajectory, runhistory = configurators._reconstruct_moparamils_trajectory(output_dir)
            result = {
                "configurator": experiment['configurators'].long_name,
                "instance": experiment["scenarios"].name,
                "seed": experiment["seeds"],
                "stats": None,
                "runhistory": runhistory,
                "trajectory": trajectory,
                "incumbent": [runhistory.ids_config[id] for id in trajectory["trajectory"][-1]["config_ids"]],
            }
            expclass.save_result(result, experiment, action="configure")

    actions["trajectory"] = Action("trajectory",
                                   get_trajectory)

    Experiment(exp_space, actions)


