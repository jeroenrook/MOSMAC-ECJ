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

from argparse import ArgumentParser
import sys
import json

import matplotlib
matplotlib.use('Agg')

from plottingscripts.plotting import plot_methods

from aclib.stats.read_traj_files import read_traj_files


def main():
    prog = "python plot_performance.py"
    description = "Plot a median trace with quantiles for multiple experiments"

    parser = ArgumentParser(description=description, prog=prog)

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
    parser.add_argument("--agglomeration", dest="agglomeration", type=str,
                        default="median", choices=("median", "mean"),
                        help="Plot mean or median")
    parser.add_argument("--subset_ac", dest="subset_ac", nargs="*",
                        default=[],
                        help="Specify a subset of algorithm configurators to be"
                             " used -- regex supported")
    parser.add_argument("--name_map_json", default=None, 
                        help="A json file with a mapping from AC names to names"
                             " to be used in plots")
    parser.add_argument("--rm_tos_at", default=None, type=float,
                         help="Remove all common timeouts at given cutoff ")
    parser.add_argument("--no_legend", default=False, action="store_true", 
                         help="Don't plot legend")
    parser.add_argument("--step_func", default=False, action="store_true", 
                         help="Plot as step function")


    parser.add_argument("-v", "--verbose", dest="verbose", action="store_true",
                        default=False,
                        help="print number of runs on plot")
    group = parser.add_mutually_exclusive_group()
    group.add_argument(
        '--train', dest="train",  default=False, action='store_true')
    group.add_argument(
        '--est_train', dest="est_train",  default=False, action='store_true')
    group.add_argument(
        '--test', dest="test", default=False, action='store_true')
    
    args = parser.parse_args()
    
    if not(args.test or args.train or args.est_train):
        args.test = True

    performance, new_time_list, name_list = read_traj_files(args)

    # sort name_list and adjust other lists accordingly
    old_name_list = name_list[:]
    name_list = sorted(name_list)
    sorted_perf = []
    sorted_times = []
    for name in name_list:
        old_indx = old_name_list.index(name)
        sorted_perf.append(performance[old_indx])
        sorted_times.append(new_time_list[old_indx])

    if args.name_map_json:
        with open(args.name_map_json) as fp:
            name_mapping = json.load(fp)
        name_list = [name_mapping.get(x, x) for x in name_list]

    fig = plot_methods.\
        plot_optimization_trace_mult_exp(time_list=sorted_times,
                                         performance_list=sorted_perf,
                                         title=args.title,
                                         name_list=name_list,
                                         logx=args.logx, logy=args.logy,
                                         y_min=args.ymin, y_max=args.ymax,
                                         x_min=args.xmin, x_max=args.xmax,
                                         agglomeration=args.agglomeration,
                                         properties={'labelfontsize': 22,
                                                     'legendsize': 18,
                                                     'ticklabelsize':22},
                                         legend=not args.no_legend,
                                         step=args.step_func,
                                         ylabel="PAR10 [sec]")
    fig.tight_layout()

    if args.save is None:
        save = "%s_perf_over_time.pdf" % args.scenario
    else:
        save = args.save
    fig.savefig(save)
    print("Saved figure as %s" % save)

if __name__ == "__main__":
    main()