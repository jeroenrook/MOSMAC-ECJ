#!/usr/bin/env python

__author__ = "Marius Lindauer and Katharina Eggensperger"
__version__ = "0.0.1"
__license__ = "BSD"

# hack to avoid installing of Aclib
import sys
import os
import inspect
cmd_folder = os.path.realpath(
    os.path.abspath(os.path.split(inspect.getfile(inspect.currentframe()))[0]))
cmd_folder = os.path.realpath(os.path.join(cmd_folder, ".."))
if cmd_folder not in sys.path:
    sys.path.insert(0, cmd_folder)

from argparse import ArgumentParser, ArgumentDefaultsHelpFormatter
import sys
import json

import matplotlib
matplotlib.use('Agg')
import numpy as np

from plottingscripts.plotting import plot_methods
from plottingscripts.utils import helper


from aclib.stats.read_traj_files import read_traj_files


def main():
    prog = "python plot_performance.py"
    description = "Plot a median trace with quantiles for multiple " \
                  "experiments. For each <WhatIsThis> that does not contain " \
                  "the string 'GGA' bootstrap samples are drawn and test of " \
                  "best train is plotted."

    parser = ArgumentParser(description=description, prog=prog,
                            formatter_class=ArgumentDefaultsHelpFormatter)

    # General Options
    parser.add_argument("-s", "--scenario",
                        required=True, help="scenario name")

    parser.add_argument("--logx", action="store_true", dest="logx",
                        default=False, help="Plot x on log scale")
    parser.add_argument("--logy", action="store_true", dest="logy",
                        default=False, help="Plot y on log scale")
    parser.add_argument("--ymax", dest="ymax", type=float,
                        default=None, help="Maximum of the y-axis")
    parser.add_argument("--ymin", dest="ymin", type=float,
                        default=None, help="Minimum of the y-axis")
    parser.add_argument("--xmax", dest="xmax", type=float,
                        default=None, help="Maximum of the x-axis")
    parser.add_argument("--xmin", dest="xmin", type=float,
                        default=None, help="Minimum of the x-axis")
    parser.add_argument("-S", "--save", dest="save",
                        default=None,
                        help="Save plot to a specific destination? Otherwise "
                             "will be saved as ./<scenario>_perf_over_time.pdf")
    parser.add_argument("-t", "--title", dest="title",
                        default="", help="Optional supertitle for plot")
    parser.add_argument("--maxvalue", dest="maxvalue", type=float,
                        default=sys.maxsize,
                        help="Replace all values higher than this?")
    parser.add_argument("--subset_ac", dest="subset_ac", nargs="*",
                        default=[],
                        help="Specify a subset of algorithm configurators to be"
                             " used -- regex supported")
    parser.add_argument("--config_json", required=True,
                        help="A json file with the following entries for each configurator: \"legendlabel\", \"parallel\", \"bootstrap\"")
    parser.add_argument("--rm_tos_at", default=None, type=float,
                        help="Remove all common timeouts at given cutoff ")
    parser.add_argument("--no_legend", default=False, action="store_true", 
                        help="Don't plot legend")
    parser.add_argument("-b", "--bootstrap", dest="bootstrap", default="10x8",
                        help="nxm; For each non-GGA experiment draw n times m "
                             "trajectories and plot best of train")
    parser.add_argument("--best_of", default="train", dest="bestof",
                        choices=("train", "est_train", "test"),
                        help="Plot testperformance wrt to "
                             "train/test/est_train performance")
    parser.add_argument("--seed", default=1, type=int, dest="seed",
                        help="Seed for reproducibility."
                             "Will be used for Bootstrap sampling")
    parser.add_argument("-v", "--verbose", dest="verbose", action="store_true",
                        default=False,
                        help="print number of runs on plot")
    
    args = parser.parse_args()
    
    # config format
    # "configurator" : {
    #                    "legendlabel": "short",
    #                    "parallel: True|False, [Default:False]
    #                    "bootstrap": True|False [Default:True]
    #                    }
     
    # Always plot test data, but also read est train data
    args.test = True
    args.est_train = False
    args.train = False

    performance, new_time_list, name_list = read_traj_files(args)
    
    with open(args.config_json) as fp:
        config = json.load(fp)
        for ac in name_list:
            if config.get(ac) is None:
                config[ac] = {"legendlabel": ac,
                              "parallel": False,
                              "bootstrap": True}
            else:
                if config[ac].get("parallel") is None:
                    config[ac]["parallel"] = False
                if config[ac].get("bootstrap") is None:
                    config[ac]["bootstrap"] = True
                    
    if args.bestof == "train":
        args.train = True
        args.test = False
        trn_performance, _, _ = read_traj_files(args)
    elif args.bestof == "est_train":
        args.est_train = True
        args.test = False
        trn_performance, _, _ = read_traj_files(args)
    elif args.bestof == "test":
        trn_performance = performance

    # Calc bootstrap samples
    bootstrap_repetitions, bootstrap_samples = [int(i) for i
                                                in args.bootstrap.split("x")]
    print("[Sampling] Do %d repetition with %d samples each" %
          (bootstrap_repetitions, bootstrap_samples))

    # sort name_list and adjust other lists accordingly
    old_name_list = name_list[:]
    name_list = sorted(name_list)
    sorted_perf = []
    sorted_times = []
    for name in name_list:
        old_indx = old_name_list.index(name)

        p = performance[old_indx]
        t = new_time_list[old_indx]

        if config[name]["parallel"]:
            trn_p = trn_performance[old_indx]
            # draw bootstrap samples
            if config[name]["bootstrap"]:
                print("Sample %s (with bootstrapping)" % name)
            else:
                print("Sample %s (without bootstrapping)" % name)
            new_performance = np.zeros([bootstrap_repetitions,
                                        p.shape[1]])
            for timestep in range(p.shape[1]):
                # for each timestep
                for i in range(bootstrap_repetitions):
                    # get best run on train
                    sample_idx = np.random.choice(
                                        range(0,len(p)), 
                                        size=min(p,bootstrap_samples), 
                                        replace=config[name]["bootstrap"])
                    best_train = np.argmin(trn_p[sample_idx, timestep])
                    # and use this as new performance for pseudorun i at time t
                    new_performance[i, timestep] = (p[sample_idx, timestep])[best_train]
            sorted_perf.append(new_performance)
            sorted_times.append(t)
        else:
            print("Do NOT bootstrap %s" % name)
            sorted_perf.append(p)
            sorted_times.append(t)

    name_list = [config.get(x, x).get("legendlabel",x) for x in name_list]

    #print([s.shape for s in sorted_perf])
    #print([s.shape for s in sorted_times])
    fig = plot_methods.\
        plot_optimization_trace_mult_exp(time_list=sorted_times,
                                         performance_list=sorted_perf,
                                         title=args.title,
                                         name_list=name_list,
                                         logx=args.logx, logy=args.logy,
                                         y_min=args.ymin, y_max=args.ymax,
                                         x_min=args.xmin, x_max=args.xmax,
                                         agglomeration="median",
                                         properties={'labelfontsize': 22,
                                                     'legendsize': 18,
                                                     'ticklabelsize':22},
                                         legend=not args.no_legend,
                                         ylabel="PAR10 [sec]")
    fig.tight_layout()

    if args.save is None:
        save = "%s_bootstrapped_perf_over_time.pdf" % args.scenario
    else:
        save = args.save
    fig.savefig(save)
    print("Saved figure as %s" % save)

if __name__ == "__main__":
    main()