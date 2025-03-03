#!/bin/python
# encoding: utf-8

from argparse import ArgumentParser
import os
import sys
import re
import math
import numpy
import shutil

__author__ = "Marius Lindauer"
__version__ = "0.0.1"
__license__ = "BSD"

def discretize_pcs_scenario(scenario_fn: str, exp_dir: str):
    '''
        copy scenario file and replace pcs file

        Arguments
        ---------
        scenario_fn: str
            scenario file name
        exp_dir: str
            experimental directory

        Returns
        -------
        fn : str
            new scenario file name
    '''
    pcs_fn = None
    out_fn = "scenario.txt"
    new_scenario_fn = os.path.join(exp_dir, out_fn)
    shutil.copy(os.path.join(exp_dir, scenario_fn), os.path.join(exp_dir, "scenario.back"))
    with open(new_scenario_fn, "w") as out_fp:
        with open(os.path.join(exp_dir, "scenario.back")) as fp:
            for line in fp:
                line = line.replace("\n", "").strip(" ")
                if line.startswith("pcs-file") or \
                        line.startswith("param-file") or \
                        line.startswith("paramFile") or \
                        line.startswith("paramfile"):

                    pcs_fn = line.split("=")[1].strip(" ")
                    out_fp.write("paramfile=disc_pcs.txt\n")
                    discretize_pcs(pcs_fn=os.path.join(exp_dir,pcs_fn), granularity=7, fn_out=os.path.join(exp_dir, "disc_pcs.txt"))

                else:
                    out_fp.write(line + "\n")

    if pcs_fn is None:
        sys.stderr.write(
            "PCS file not found in scenario file (%s)" % (os.path.join(exp_dir, scenario_fn)))
        sys.exit(44)

    return out_fn

def discretize_pcs(pcs_fn:str, granularity:int, fn_out:str=None):
    '''
        read and discretize configuration space (only SMAC 2.08 pcs format supported)
        
        Arguments
        ---------
        pcs_fn: str
            file name of input pcs file
        granularity: int
            number of values of each discretized parameter (maybe plus default value)
        fn_out: str
            file name of output file
    '''
    

    if fn_out is None:
        fp_out = None
    else:
        fp_out = open(fn_out, "w")
    num_regex = "[+\-]?(?:0|[1-9]\d*)(?:\.\d*)?(?:[eE][+\-]?\d+)?"
    REGEX = re.compile(
        "^[ ]*(?P<name>[^ ]+)[ ]*\[[ ]*(?P<range_start>%s)[ ]*,[ ]*(?P<range_end>%s)\][ ]*\[(?P<default>%s)\](?P<misc>.*)$" % (num_regex, num_regex, num_regex))

    with open(pcs_fn, "r") as fp:
        for line in fp:
            line = line.strip("\n")
            if line.find("#") > -1:
                line = line[:line.find("#")]  # remove comments
            match = REGEX.match(line)
            if line == "":
                continue
            if match:
                range_start = float(match.group("range_start"))
                range_end = float(match.group("range_end"))
                default = match.group("default")

                # logarithm scale
                if "l" in match.group("misc"):
                    range_start = math.log(range_start)
                    range_end = math.log(range_end)

                dist = range_end - range_start
                step_size = dist / (granularity - 1)
                discretized_values = numpy.arange(
                    range_start, range_end + math.pow(0.1, 10), step_size)

                # prevents rounding issues
                discretized_values = map(
                    lambda x: round(x, 6), discretized_values)

                if "l" in match.group("misc"):
                    discretized_values = map(math.exp, discretized_values)

                if "i" in match.group("misc"):
                    discretized_values = map(
                        int, map(round, discretized_values))

                discretized_values = set(list(discretized_values))
                discretized_values.add(float(match.group("default")))

                # prevents rounding issues
                discretized_values = map(
                    lambda x: round(x, 8), discretized_values)
                discretized_values = set(discretized_values)
                discretized_values = sorted(list(discretized_values))
                # print(discretized_values)

                discretized_values[0] = float(match.group("range_start"))
                discretized_values[-1] = float(match.group("range_end"))

                # +0 prevents -0.0000
                if "i" in match.group("misc"):
                    discretized_values = map(
                        int, map(round, discretized_values))
                    discretized_values = ",".join(
                        ["%d" % (v+0) for v in discretized_values])
                else:
                    discretized_values = ",".join(
                        ["%.8f" % (v+0) for v in discretized_values])
                    default = "%.8f" % (float(default))

                if fp_out is None:
                    print(
                        "%s {%s}[%s]" % (match.group("name"), discretized_values, default))
                else:
                    fp_out.write(
                        "%s {%s}[%s]\n" % (match.group("name"), discretized_values, default))
            else:
                if fp_out is None:
                    print(line)
                else:
                    fp_out.write(line + "\n")

    if fp_out is not None:
        fp_out.flush()
        fp_out.close()

if __name__ == "__main__":

    parser = ArgumentParser()
    parser.add_argument("pcs", metavar="pcs file", nargs=1,
                        help="the parameter configuration space file")
    parser.add_argument("-n", "--n", dest="n", type=int, default=7,
                        help="at most number of discretized values per continous parameter (at least 3!); plus default if not already included")

    args = parser.parse_args()

    discretize_pcs(pcs_fn=args.pcs[0], granulariy=args.n)
