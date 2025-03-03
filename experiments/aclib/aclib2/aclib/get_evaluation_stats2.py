# hack to avoid installing of Aclib
import sys
import os
import glob
import inspect
cmd_folder = os.path.realpath(
    os.path.abspath(os.path.split(inspect.getfile(inspect.currentframe()))[0]))
cmd_folder = os.path.realpath(os.path.join(cmd_folder, ".."))
if cmd_folder not in sys.path:
    sys.path.insert(0, cmd_folder)

import os
import sys
import json
import tabulate
import copy
import logging
del(tabulate.LATEX_ESCAPE_RULES[u'$'])
del(tabulate.LATEX_ESCAPE_RULES[u'\\'])
del(tabulate.LATEX_ESCAPE_RULES[u'{'])
del(tabulate.LATEX_ESCAPE_RULES[u'}'])
del(tabulate.LATEX_ESCAPE_RULES[u'^'])
from argparse import ArgumentParser, ArgumentDefaultsHelpFormatter
from collections import OrderedDict

import numpy as np
import scipy.stats
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

from plottingscripts.plotting.scatter import plot_scatter_plot
from plottingscripts.plotting import plot_methods

from aclib.stats import read_val, sample_runs, read_traj_files, stattest_per_timestep


def load_config(fn: str):
    '''
        Load config JSON file which should containt the following data:

        {"scenarios": { 
            "scenario_1": 
              { "legend_name" : "label_name_1",
                "cutoff"    : 5,
                "maxy"     : 5 
              },
              "scenario_2": 
              { "legend_name" : "label_name_2",
                "cutoff"    : 10,
                "maxy"     : 8
              }
            },
        "configurators" : {
            "configurator_1": { 
                "legend_name" : "label_ac_1",
                "n_samples": 1,
                "bootstrapping": false,
                "max_run_id": 100,
                "min_run_id": 1
                }
            ...
          }
        }

        Arguments
        ---------
        fn: str
            config json file
    '''

    with open(fn) as fp:
        # lead as OrderedDict
        config = json.load(fp, object_pairs_hook=OrderedDict)

    if config.get("scenarios") is None:
        config["scenarios"] = {}
        for dn in os.listdir("."):
            if dn != "workers" and os.path.isdir(dn):
                config["scenarios"][dn] = dn

    return config


def generate_table(data_dict: dict, config: dict, 
                   compare_to:str=None,
                   add_quartiles: bool=True, alpha=0.05,
                   table_style="latex"):

    scenarios = list(config["scenarios"].keys())

    configurators = []
    for ac in config["configurators"].values():
        if ac["n_samples"] > 1 and ac["bootstrapping"]:
            configurators.append(ac["legend_name"] + "x%d" % (ac["n_samples"]))
        else:
            configurators.append(ac["legend_name"])
    table_data = []
    table_speedup_data = []
    for scen in scenarios:
        median_row = []
        q25_row = []
        q75_row = []
        all_data_row = []
        best_row = []
        stat_sim = []
        stat_better = []

        default_perf = None
        for configurator in config["configurators"]:
            try:
                default_perf = data_dict[scen][configurator][1]["test"]["traj"]["Test Set Performance"].values[0]
                break
            except KeyError:
                pass

        if default_perf is None:
            raise ValueError(
                "Have not found default performance in any of the runs")

        table_row = [config["scenarios"][scen].get("legend_name", scen),
                     "%.2f" % (default_perf)]
        table_speedup_row = [config["scenarios"]
                             [scen].get("legend_name", scen)]
        for configurator, ac_config in config["configurators"].items():
            if not data_dict[scen][configurator]:
                # no data available
                median_row.append(2**32 - 1)
                q25_row.append(2**32 - 1)
                q75_row.append(2**32 - 1)
                all_data_row.append([2**32 - 1])
                stat_sim.append(False)
                stat_better.append(False)
            else:
                if ac_config.get("n_samples", 1) > 1:
                    # samples only at the end point of the trajectories
                    ac_data = sample_runs.sample_runs(runs=data_dict[scen][configurator],
                                                      n_samples=ac_config.get(
                                                          "n_samples"),
                                                      bootstrapping=ac_config.get(
                                                          "bootstrapping", True),
                                                      n_iterations=args.n_sample_iterations)
                else:
                    ac_data = data_dict[scen][configurator]

                perf_values = [
                    run["test"]["traj"]["Test Set Performance"].values[-1] for run in ac_data.values()]
                median_row.append(np.median(perf_values))
                q25_row.append(np.percentile(perf_values, q=25))
                q75_row.append(np.percentile(perf_values, q=75))
                all_data_row.append(perf_values)
                stat_sim.append(False)
                stat_better.append(False)

        best_value = np.min(median_row)
        best_row = best_value == np.array(median_row)
        if compare_to:
            best_idx = configurators.index(compare_to)
        else:
            best_idx = np.argmin(median_row)

        for idx, d in enumerate(all_data_row):
            if np.all(all_data_row[best_idx] == d):
                p = 1.0
            else:
                p = stattest_per_timestep.perm_unpaired_test(
                    x=all_data_row[best_idx], y=d)
            if p > alpha:
                stat_sim[idx] = True
            if p > 1 - alpha: # statistically better
                stat_better[idx] = True

        for idx, median in enumerate(median_row):
            if median == 2**32 - 1:
                value = "--"
            else:
                value = "%.2f" % (median)
                speedup = "%.2f" % (default_perf / median)
                if best_row[idx]:
                    value = "\\underline{%s}" % (value)
                    speedup = "\\underline{%s}" % (speedup)
                if stat_sim[idx] or best_row[idx]:
                    value = "\\mathbf{%s}" % (value)
                if stat_better[idx] and best_idx != idx:
                    value = "%s^*" % (value)
                if add_quartiles:
                    value += " [%.2f;%.2f]" % (q25_row[idx], q75_row[idx])
            value = "$%s$" % (value)
            table_row.append(value)
            table_speedup_row.append(speedup)
        table_data.append(table_row)
        table_speedup_data.append(table_speedup_row)

    headers = ["Set", "Default"] + configurators
    print(tabulate.tabulate(tabular_data=table_data,
                            headers=headers, tablefmt=table_style))

    print()
    print(">>> Speedup Table (TA)")
    sut_headers = ["Set", ] + configurators
    print(tabulate.tabulate(tabular_data=table_speedup_data,
                            headers=sut_headers, tablefmt=table_style))

    return headers, table_data, sut_headers, table_speedup_data


def generate_scatter_plot(data_dict: dict, config: dict):

    for scen in config["scenarios"]:
        for configurator, ac_config in config["configurators"].items():

            if not data_dict[scen][configurator]:
                continue

            if ac_config.get("n_samples", 1) > 1:
                # samples only at the end point of the trajectories
                ac_data = sample_runs.sample_runs(runs=data_dict[scen][configurator],
                                                  n_samples=ac_config.get(
                                                      "n_samples"),
                                                  bootstrapping=ac_config.get(
                                                      "bootstrapping", True),
                                                  n_iterations=args.n_sample_iterations)
            else:
                ac_data = data_dict[scen][configurator]

            perf_values = [run["test"]["traj"]["Test Set Performance"].values[-1]
                           for run in ac_data.values()]

            # sort by perf
            sorted_runs = sorted(ac_data.values(
            ), key=lambda x: x["test"]["traj"]["Test Set Performance"].values[-1])
            median_idx = int(len(sorted_runs) / 2)
            median_run = sorted_runs[median_idx]

            x_def = median_run["test"]["obj_matrix"]["Objective of validation config #1"].values
            y_inc = median_run["test"]["obj_matrix"].values[:, -1]

            figure = plot_scatter_plot(x_data=x_def, y_data=y_inc, labels=["Default [sec]", "Optimized [sec]"],
                                       max_val=config["scenarios"][scen]["cutoff"],
                                       linefactors=[2, 10, 100])
            figure.tight_layout()
            figure.savefig("scatter_%s_%s.png" % (scen, configurator), bbox_inches='tight')
            plt.close(figure)


def generate_overtime_plot(data_dict: dict, config: dict, samples: int,
                           xmin: float=None, xmax: float=None,
                           ymin: float=None, ymax: float=None,
                           ylog: bool=False,
                           ylabel: str="PAR10 [sec]",
                           xlabel: str="Configuration Budget [sec]",
                           agglomeration: str="median",
                           step: bool=False,
                           legend: bool=True):

    ymax_def = ymax

    for scen in config["scenarios"]:

        performance_list, time_list, configurators = \
            get_perf_time_matrix(data_dict=data_dict,
                                 config=config, scen=scen, samples=samples)

        if ymax_def is not None:
            ymax = ymax_def
        elif config["scenarios"][scen].get("ymax", None) is not None:
            ymax = config["scenarios"][scen].get("ymax")
        else:
            ymax = ymax_def

        fig = plot_methods.\
            plot_optimization_trace_mult_exp(time_list=time_list,
                                             performance_list=performance_list,
                                             title=None,
                                             name_list=configurators,
                                             logx=True, logy=ylog,
                                             x_min=xmin,
                                             x_max=xmax,
                                             y_min=ymin,
                                             y_max=ymax,
                                             agglomeration=agglomeration,
                                             properties={'labelfontsize': 22,
                                                         'legendsize': 18,
                                                         'ticklabelsize': 22,
                                                         'markersize': 6},
                                             ylabel=ylabel,
                                             xlabel=xlabel,
                                             step=step,
                                             legend=legend)
        fig.tight_layout()
        fig.savefig("over_time_%s.png" % (scen), bbox_inches='tight')
        plt.close(fig)


def generate_aggregated_overtime_plot(data_dict: dict, config: dict, samples: int,
                                      xmin: float=None, xmax: float=None,
                                      ymin: float=None, ymax: float=None,
                                      ylog: bool=False,
                                      step: bool=False,
                                      legend: bool=True):

    # normalize data
    for scen in config["scenarios"]:

        default_test_perf = None
        default_train_perf = None
        for configurator in config["configurators"]:
            try:
                default_test_perf = data_dict[scen][configurator][1]["test"]["traj"]["Test Set Performance"].values[0]
                break
            except KeyError:
                pass

        for configurator in config["configurators"]:
            try:
                default_train_perf = data_dict[scen][configurator][1]["train"]["traj"]["Test Set Performance"].values[0]
                break
            except KeyError:
                pass

        if default_test_perf is None:
            raise ValueError(
                "Have not found default performance in any of the runs")

        # get stats
        max_t = -np.inf
        best_y_train = np.inf
        best_y_test = np.inf
        for ac, ac_data in data_dict[scen].items():
            for run, run_data in ac_data.items():
                if not run_data:
                    continue
                max_t = max(run_data["test"]["traj"]["Time"].max(), max_t)
                best_y_test = min(
                    run_data["test"]["traj"]["Test Set Performance"].min(), best_y_test)
                try:
                    best_y_train = min(
                        run_data["train"]["traj"]["Test Set Performance"].min(), best_y_train)
                except KeyError:
                    pass

        for ac, ac_data in data_dict[scen].items():
            for run, run_data in ac_data.items():
                if not run_data:
                    continue
                # scale timestamps
                run_data["test"]["traj"]["Time"] /= max_t

                test_perf_vec = run_data["test"]["traj"]["Test Set Performance"]
                if default_test_perf != best_y_test:
                    test_perf_vec = (test_perf_vec - best_y_test) / \
                        (default_test_perf - best_y_test)
                else:
                    test_perf_vec *= 0
                run_data["test"]["traj"]["Test Set Performance"] = test_perf_vec

                try:
                    run_data["train"]["traj"]["Time"] /= max_t

                    train_perf_vec = run_data["train"]["traj"]["Test Set Performance"]
                    if default_test_perf != best_y_test:
                        train_perf_vec = (
                            train_perf_vec - best_y_train) / (default_train_perf - best_y_train)
                    else:
                        train_perf_vec *= 0
                    run_data["train"]["traj"]["Test Set Performance"] = train_perf_vec

                except KeyError:
                    pass

    data_dict_agg = {
        "aggregated_scenario": {}
    }

    for scen in config["scenarios"]:
        data_dict_agg["aggregated_%s" % (scen)] = {}
        for ac, ac_data in data_dict[scen].items():
            data_dict_agg["aggregated_scenario"][ac] = data_dict_agg["aggregated_scenario"].get(ac, {
            })
            for run, run_data in ac_data.items():
                n_runs = len(data_dict_agg["aggregated_scenario"][ac])
                data_dict_agg["aggregated_scenario"][ac][n_runs + 1] = run_data

    config = {"scenarios": {
        "aggregated_scenario":
              {"legend_name": "Aggregated Scenario"
               }
              },
              "configurators": config["configurators"]
              }

    generate_overtime_plot(data_dict=data_dict_agg, config=config, samples=samples,
                           xmin=xmin if xmin is not None else 0.0001,
                           xmax=1,
                           ymin=ymin, ymax=ymax,
                           ylog=ylog,
                           step=step,
                           ylabel="Gap Default-Best",
                           xlabel="Configuration Budget [scaled]",
                           agglomeration="mean",
                           legend=legend)


def multiply_perf(data_dict: dict, mult_factor: float):
    for scen, scen_data in data_dict.items():
        for ac, ac_data in scen_data.items():
            for run, run_data in ac_data.items():
                run_data["test"]["traj"]["Test Set Performance"] *= args.perf_factor
                run_data["test"]["obj_matrix"] *= args.perf_factor
                try:
                    run_data["train"]["traj"]["Test Set Performance"] *= args.perf_factor
                    run_data["train"]["obj_matrix"] *= args.perf_factor
                except KeyError:
                    pass


def use_train(data_dict: dict):
    for scen, scen_data in data_dict.items():
        for ac, ac_data in scen_data.items():
            for run, run_data in ac_data.items():
                try:
                    run_data["test"] = copy.deepcopy(run_data["train"])
                except KeyError:
                    raise IndexError(
                        "Missing training data for %s with %s (run-%s)" % (scen, ac, run))


def get_perf_time_matrix(data_dict: dict, config: dict, scen: str, samples: int):

    performance_list = []
    time_list = []
    configurators = []

    for id_, (configurator, ac_config) in enumerate(config["configurators"].items()):

        if not data_dict[scen].get(configurator):
            continue

        ac_data = data_dict[scen][configurator]
        test_perf_values = np.array(
            [run["test"]["traj"]["Test Set Performance"].values for run in ac_data.values()])

        if test_perf_values.size == 0:
            continue

        time_stamps = np.array(
            [run["test"]["traj"]["Time"].values for run in ac_data.values()])

        test_perf_values, time_stamps = read_traj_files.unify_timestamps(
            performances=[test_perf_values], times_=[time_stamps])
        time_stamps = time_stamps[0]
        test_perf_values = test_perf_values[0]

        if ac_config.get("n_samples", 1) > 1:

            try:
                train_perf_values = np.array(
                    [run["train"]["traj"]["Test Set Performance"].values for run in ac_data.values()])
            except KeyError:
                logging.warn(
                    "No train data for %s on %s available -- sampling not possible -- skip for overtime plot" % (configurator, scen))
                continue

            test_perf_values = sample_parallel_AC(ac_data=ac_data, ac_config=ac_config,
                                                  configurator=configurator, scen=scen,
                                                  train_perf_values=train_perf_values,
                                                  samples=samples,
                                                  time_stamps=time_stamps, test_perf_values=test_perf_values)

        if ac_config["n_samples"] > 1 and ac_config["bootstrapping"]:
            configurators.append(
                ac_config["legend_name"] + "x%d" % (ac_config["n_samples"]))
        else:
            configurators.append(ac_config["legend_name"])

        time_list.append(time_stamps)
        performance_list.append(test_perf_values)

    return performance_list, time_list, configurators


def sample_parallel_AC(ac_data: dict, ac_config: dict,
                       configurator: str, scen: str,
                       train_perf_values: list,
                       samples: int,
                       time_stamps: list, test_perf_values: list):

    time_stamps_train = np.array(
        [run["train"]["traj"]["Time"].values for run in ac_data.values()])
    train_perf_values, time_stamps_train = read_traj_files.unify_timestamps(
        performances=[train_perf_values], times_=[time_stamps_train])
    train_perf_values = train_perf_values[0]

    sampled_test_perf_values = []
    for _ in range(samples):
        time_row = []
        for t_idx in range(len(time_stamps)):
            sample_idx = np.random.choice(range(0, len(test_perf_values[:, t_idx])),
                                          size=min(len(test_perf_values),
                                                   ac_config["n_samples"]),
                                          replace=ac_config["bootstrapping"])
            sampled_train = [train_perf_values[i, t_idx] for i in sample_idx]
            sampled_test = [test_perf_values[i, t_idx] for i in sample_idx]
            best_idx = np.argmin(sampled_train)
            time_row.append(sampled_test[best_idx])
        sampled_test_perf_values.append(time_row)
    return sampled_test_perf_values


def generate_speedup_table(data_dict: dict,
                           config: dict,
                           samples: int,
                           reference: str,
                           table_style: str):

    speed_dict = {}

    for scen in config["scenarios"]:

        speed_dict[scen] = {}

        performance_list, time_list, configurators = \
            get_perf_time_matrix(data_dict=data_dict,
                                 config=config, scen=scen, samples=samples)

        expended_time_list = []
        for tl, pt in zip(time_list, performance_list):
            expended_time_list.append(np.repeat([tl], len(pt), axis=0))

        performance_list, time_list = read_traj_files.unify_timestamps(
            performances=performance_list, times_=expended_time_list)

        #=======================================================================
        # speedup_matrix = stattest_per_timestep.speedups_to_ref(performances=performance_list,
        #                                                        compare_to_indx=configurators.index(
        #                                                            reference),
        #                                                        time_lists=time_list)
        #=======================================================================
        
        speedup_matrix = stattest_per_timestep.bootstrapping_speedups_to_ref(performances=performance_list,
                                                                             compare_to_indx=configurators.index(
                                                                                 reference),
                                                                             time_lists=time_list)


        for i, ac in enumerate(configurators):
            speed_dict[scen][ac] = speedup_matrix[i]

    configurators = []
    for ac in config["configurators"].values():
        if ac["n_samples"] > 1 and ac["bootstrapping"]:
            configurators.append(ac["legend_name"] + "x%d" % (ac["n_samples"]))
        else:
            configurators.append(ac["legend_name"])

    table_data = []
    ac_speedups = {}
    for scen in config["scenarios"]:
        table_data.append([config["scenarios"][scen]["legend_name"]])
        for ac in configurators:
            speedup = speed_dict[scen].get(ac)
            table_data[-1].append("%.1f" %
                                  (speedup) if speedup is not None else "--")
            if speedup is not None:
                ac_speedups[ac] = ac_speedups.get(ac, [])
                ac_speedups[ac].append(speedup)

    table_data.append(["Geo Avg."])
    for ac in configurators:
        if ac_speedups.get(ac):
            table_data[-1].append("%.1f" %
                                  (scipy.stats.mstats.gmean(ac_speedups[ac])))
        else:
            table_data[-1].append("--")

    headers = ["Set"] + configurators
    print(tabulate.tabulate(tabular_data=table_data,
                            headers=headers, tablefmt=table_style))

    return headers, table_data


if __name__ == "__main__":

    parser = ArgumentParser(formatter_class=ArgumentDefaultsHelpFormatter)

    parser.add_argument("--config_json", required=True,
                        help="Config JSON file to specify scenarios and configurators"
                        " to be parsed")
    parser.add_argument("--alpha", dest="alpha", default=0.05, type=float,
                        help="Alpha for stat test")
    parser.add_argument("--n_sample_iterations", default=100, type=int,
                        help="Number of sample iterations")
    parser.add_argument("--table_style", dest="table_style", choices=tabulate.tabulate_formats,
                        default="latex", help="Table Style")
    parser.add_argument("--rm_tos", default=False, action="store_true",
                        help="Remove all common timeouts")

    parser.add_argument("--perf_factor", default=1, type=float,
                        help="multiplicative factor for performance values (e.g., to go to percentage)")

    parser.add_argument("--no_table", default=True, dest="do_table",
                        action="store_false",
                        help="Don't generate table")
    parser.add_argument("--add_quartiles_table", default=False,
                        action="store_true",
                        help="Add 25th and 75th quantile in table")

    parser.add_argument("--compare_to", default=None,
                        help="Creates a performance table with stat tests according to given configurator (has to be part of the legendname in the config file)")

    parser.add_argument("--speedup_to", default=None,
                        help="Creates a table with speedups to given reference configurator (has to be part of the legendname in the config file)")

    parser.add_argument("--no_scatter", default=True, dest="do_scatter",
                        action="store_false",
                        help="Don't generate Scatter plots")

    parser.add_argument("--no_overtime_plot", default=True, dest="do_overtime_plot",
                        action="store_false",
                        help="Don't generate overtime plots")

    parser.add_argument("--no_aggregate_plot", default=True, dest="do_aggregate_plot",
                        action="store_false",
                        help="Don't generate aggregated overtime plots")

    parser.add_argument("--use_train", default=False,
                        action="store_true",
                        help="Use training performance instead of test performance"
                        "(still, test and trainings performance needs to be available)")

    parser.add_argument("-v", "--verbose", dest="verbose",
                        choices=["INFO", "DEBUG"],
                        default="INFO",
                        help="verbosity level")

    plot_parser = parser.add_argument_group(
        "Performance-Over-Time Plots Options")

    plot_parser.add_argument("--xmin", type=float,
                             default=None, help="Minimum of the x-axis")
    plot_parser.add_argument("--xmax", type=float,
                             default=None, help="Maximum of the x-axis")
    plot_parser.add_argument("--ymin", type=float,
                             default=None, help="Minimum of the y-axis")
    plot_parser.add_argument("--ymax", type=float,
                             default=None, help="Maximum of the y-axis")
    plot_parser.add_argument("--ylog", action="store_true",
                             default=False, help="log transformed y scale for over-time plots")
    plot_parser.add_argument("--ylabel", default="PAR10 [sec]",
                             help="ylabel in overtime plots")
    plot_parser.add_argument("--step", action="store_true",
                             default=False, help="Use step function in performance over time plots")
    plot_parser.add_argument("--no_legend", action="store_true",
                             default=False, help="Don't plot a legend")

    np.random.seed(12345)

    args = parser.parse_args()

    logging.basicConfig(level=args.verbose)

    #=========================================================================
    # if not(args.test or args.train or args.est_train):
    #     args.test = True
    #=========================================================================

    config = load_config(fn=args.config_json)

    # read data
    data_dict = {}
    for scen in config["scenarios"]:
        data_dict[scen] = read_val.read_val_files(
            scen_name=scen, config=config)

    if args.perf_factor != 1:
        multiply_perf(data_dict, mult_factor=args.perf_factor)

    if args.use_train:
        use_train(data_dict)

    if args.rm_tos:
        for scen in config["scenarios"]:
            logging.debug(scen)
            read_val.rm_common_tos_test(
                data_dict[scen], cutoff=config["scenarios"][scen]["cutoff"])

    ta_header, ta_table = None, None
    if args.do_table:
        print(">>> Performance Table")
        ta_header, ta_table, sut_header, sut_table = generate_table(data_dict, 
                                                                    config, 
                                                                    compare_to=args.compare_to,
                                                                    add_quartiles=args.add_quartiles_table,
                                                                    alpha=args.alpha, 
                                                                    table_style=args.table_style)

    su_header, su_table = [], []
    if args.speedup_to:
        print("")
        print(">>> Speedup Table (configurator)")
        su_header, su_table = generate_speedup_table(data_dict=data_dict,
                                                     config=config,
                                                     samples=args.n_sample_iterations,
                                                     reference=args.speedup_to,
                                                     table_style=args.table_style)

    if args.do_table:
        print("")
        print(">>> Combined Table")
        if sut_table:
            ta_header += sut_header[1:]
            ta_table = [t1 + t2[1:] for t1, t2 in zip(ta_table, sut_table)]
        if su_table:
            ta_header += su_header[1:]
            ta_table = [t1 + t2[1:] for t1, t2 in zip(ta_table, su_table)]

        print(tabulate.tabulate(tabular_data=ta_table,
                                headers=ta_header, tablefmt=args.table_style))

    if args.do_scatter:
        generate_scatter_plot(data_dict, config)

    if args.do_overtime_plot:
        generate_overtime_plot(data_dict=data_dict,
                               config=config,
                               samples=args.n_sample_iterations,
                               xmin=args.xmin,
                               xmax=args.xmax,
                               ymin=args.ymin,
                               ymax=args.ymax,
                               ylog=args.ylog,
                               ylabel=args.ylabel,
                               step=args.step,
                               legend=not args.no_legend)

    if args.do_aggregate_plot:
        # WARNING transforms data!
        generate_aggregated_overtime_plot(data_dict, config, args.n_sample_iterations,
                                          args.xmin, args.xmax, args.ymin, args.ymax,
                                          args.ylog,
                                          args.step,
                                          legend=not args.no_legend)
