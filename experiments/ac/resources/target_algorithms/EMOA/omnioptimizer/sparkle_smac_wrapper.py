#!/usr/bin/env python3
# -*- coding: UTF-8 -*-

import os
import sys
import time
import tempfile
import json
import argparse
import json
from pathlib import Path
import subprocess

from utils import *

# Exemplary manual call
# ./sparkle_smac_wrapper_old.py ../../instances/DTLZ2 dummy 3 10 123 -mu 30

if __name__ == "__main__":
    # assert (len(sys.argv) >= 6)
    #
    # # Argument parsing
    # instance = sys.argv[1]
    # specifics = sys.argv[2] # not used
    # cutoff_time = int(float(sys.argv[3]) + 1)
    # run_length = int(sys.argv[4]) # not used
    # seed = int(sys.argv[5])
    # params = sys.argv[6:]
    # print(params)

    parser = argparse.ArgumentParser()
    parser.add_argument("--instance", required=True, type=str)
    parser.add_argument("--seed", required=True, type=int)
    parser.add_argument("--cutoff", required=False, type=int)
    parser.add_argument("--config", required=True, action="append", nargs="+", default=[])
    parser.add_argument("--obj", required=False, action="append", nargs="+", default=[],
                        choices=["HV", "SP"])

    print(f"{sys.argv}")

    if "--config" in sys.argv:
        print("New style")
        old_style = False
        args = parser.parse_args()
        config = []
        for c in args.config:
            config += c
        assert len(config) % 2 == 0

        instance = args.instance
        seed = args.seed
        obj = []
        for o in args.obj:
            obj += o
        config = {k: v for k, v in zip(config[::2], config[1::2])}
    else:
        # ParamILS Style
        print("Detected ParamILS Style")
        old_style = True
        args = sys.argv
        i = 1
        state = 0
        obj = []
        while True:
            if re.match("--", args[i]):
                key = args[i][2:]
                if key == "obj":
                    obj.append(args[i + 1])
                i += 2
                continue
            break
        # <instance name> <instance-specific
        # information> <cutoff time> <cutoff length> <seed>
        instance, _, _, _, seed = args[i:i + 5]
        seed = max(min(int(seed), 2 ** 32 - 1), 0)
        i += 5
        config = {k[1:]: v for k, v in zip(args[i::2], args[i + 1::2])}

    # print(f"{obj=}")
    # print(f"{config=}")

    # Constants
    solver_binary = r'./algorithm.r'

    # Build command
    paramstring = " ".join(f"--{k} {v}" for k, v in config.items())


    ## Create temporary file to write results to
    _, result_path = tempfile.mkstemp(text=True)

    command = f"{solver_binary} --instance {instance} --seed {seed} --budget 20000 --output {result_path} {paramstring}"
    print(command)

    # get output
    ta_dir = Path(__file__).parent
    os.chdir(ta_dir)

    start_time = time.time()
    #  output_list = os.popen(command).readlines()
    subprocess.run(command.split(" "), stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    end_time = time.time()
    run_time = end_time - start_time #Wallclock time

    #Result: <status>, <runtime>, <quality>, <seed>
    # objective_names = ["POP_HV_NORM", "POP_SP"]
    # objectives = ["HV", "SP"]
    objective_mapping = {
        "HV": "POP_HV_NORM",
        "SP": "POP_SP",
    }

    try:
        with open(result_path, "r") as fh:
            run_results = json.loads(fh.read())

        result = {k: run_results[objective_mapping[k]] for k in obj}
        status = "SUCCESS"
    except:
        status = "CRASHED"
        result = {k: 0 for k in obj}

    Path(result_path).unlink()

    # parse output
    # measures = parse_solution_set(output_list)
    res = {
        "status": status,
        "cost": list(result.values()),
        "runtime": run_time,
        "mics": ""
    }
    if not old_style:
        print(f"Result of this algorithm run: {json.dumps(res)}\n")

    if len(obj) > 1:
        cost_str = "[{}]".format(", ".join([str(float(c)) for _, c in result.items()]))
    else:
        cost_str = f"{list(result.values())[0]}"
    sys.stdout.write(f"Result: {status}, {run_time}, {cost_str}, {seed}\n")


    #f"Result for SMAC3v2: status={self.data.status};cost={cost_str};runtime={self.data.time};additional_info={self.data.additional}")
    if len(obj) > 1:
        cost_str = ",".join([str(float(c)) for _, c in result.items()])
    else:
        cost_str = f"{list(result.values())[0]}"

    sys.stdout.write(f"Result for SMAC3v2: status={status};cost={cost_str};runtime={run_time}\n")

    # result_line = "Result for SMAC: {}, {}, 0, {}, {}".format(status, run_time, measures[target], seed)
    # print(result_line)
