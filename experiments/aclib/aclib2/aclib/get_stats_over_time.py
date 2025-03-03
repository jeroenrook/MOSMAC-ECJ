#!/usr/bin/env python

__author__ = "Marius Lindauer and Katharina Eggensperger"
__version__ = "0.0.1"
__license__ = "BSD"

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
#ACLIB_ROOT = cmd_folder
ACLIB_ROOT = "/home/lindauer/git/aclib2-cssc/"

from argparse import ArgumentParser
import glob
import sys
import re
import tabulate
del(tabulate.LATEX_ESCAPE_RULES[u'$'])
del(tabulate.LATEX_ESCAPE_RULES[u'\\'])
del(tabulate.LATEX_ESCAPE_RULES[u'{'])
del(tabulate.LATEX_ESCAPE_RULES[u'}'])
del(tabulate.LATEX_ESCAPE_RULES[u'^'])

import numpy as np

import matplotlib
matplotlib.use('Agg')

from aclib.stats.read_traj_files import read_traj_files
from aclib.stats.stattest_per_timestep import test_per_timestep,get_auc,speedups_to_ref

from smac.scenario.scenario import Scenario

def main():
    prog = "python plot_performance.py"
    description = "Plot a median trace with quantiles for multiple experiments"

    parser = ArgumentParser(description=description, prog=prog)

    # General Options
    parser.add_argument("-s", "--scenario",
                        default=None, help="scenario name")

    parser.add_argument("--logx", action="store_true", dest="logx",
                        default=False, help="Plot x on log scale")
    parser.add_argument("--logy", action="store_true", dest="logy",
                        default=False, help="Plot y on log scale")
    parser.add_argument("--xmin", dest="xmin", type=float,
                        default=None, help="Minimum of the x-axis")
    parser.add_argument("--maxvalue", dest="maxvalue", type=float,
                        default=sys.maxsize,
                        help="Replace all values higher than this?")
    parser.add_argument("--subset_ac", dest="subset_ac", nargs="*",
                        default=[],
                        help="Specify a subset of algorithm configurators to be used -- regex supported")
    parser.add_argument("--ref", dest="ref", default=None, required=True,
                        help="Reference AC")
    parser.add_argument("--alpha", dest="alpha", default=0.05, type=float,
                        help="Alpha for stat test")
    parser.add_argument("--table_style", dest="table_style", choices=tabulate.tabulate_formats,
                        default="pipe", help="Table Style")
    parser.add_argument("--rm_tos_at", default=None, type=float,
                         help="Remove all common timeouts at given cutoff ")
    parser.add_argument("--name_map_json", default=None, 
                        help="A json file with a mapping from AC+scenario names to names to be used in plots. Key words: \"ac_dict\" and \"scen_dict\"")
    
    parser.add_argument("-v", "--verbose", dest="verbose", action="store_true",
                        default=False,
                        help="print number of runs on plot")
    group = parser.add_mutually_exclusive_group()
    group.add_argument(
        '--train', dest="train",  default=False, action='store_true')
    group.add_argument(
        '--est_train', dest="est_train",  default=False, action='store_true')
    group.add_argument(
        '--test', dest="test", default=True, action='store_true')

    args = parser.parse_args()
    
    if not(args.test or args.train or args.est_train):
        args.test = True

    
    if args.name_map_json:
        with open(args.name_map_json) as fp:
            name_mapping = json.load(fp)
        setattr(args, "ac_dict", name_mapping.get("ac_dict",{}))
        setattr(args, "scen_dict", name_mapping.get("scen_dict",{}))
    else:
        setattr(args, "ac_dict", {})
        setattr(args, "scen_dict", {})
    
    if args.scenario:
        single_scen(args=args)
    else:
        all_scens(args=args)
    
def add_mean_at_end(X):
    return np.hstack((X,np.reshape(np.mean(X,axis=1),(X.shape[0],1))))

def add_names(X,name_list):
    return [[ac_name]+x for ac_name,x in zip(name_list,X)]

def all_scens(args):

    data_dict = {}
    for scenario in os.listdir("."):
        if not os.path.isdir(scenario) or os.path.islink(scenario):
            continue
        
        scenario_str = os.path.join(ACLIB_ROOT,"scenarios/*/%s/scenario.txt"%(scenario))
                                    
        scenario_fn = glob.glob(scenario_str)[0]
        scen = Scenario(scenario_fn)
        
        args.scenario = scenario
        print(">>> %s" %(scenario))
        performance, new_time_list, name_list = read_traj_files(args)
        performance, new_time_list = cut_performances(performance, new_time_list, xmin=scen.cutoff)
        
            
        data_dict[scenario] = {"perf": performance, "time_list": new_time_list, "names": name_list}
        
    # get acs which are in all data sets
    ac_list = [d["names"] for d in data_dict.values()]
    acs = set(ac_list[0])
    for ac in ac_list:
        acs = acs.intersection(ac)
    acs = sorted(list(acs))
    scens = sorted(data_dict.keys())
        
    auc_scen_table(scens, data_dict, acs, args)
    median_scen_table(scens, data_dict, acs, args)
    speedup_scen_table(scens, data_dict, acs, args)
    
def median_scen_table(scens, data_dict, acs, args):
    # Median
    median_table = []
    rej_table = []
    for scen in scens:
        data = data_dict[scen]
        if args.ref:
            ref = data["names"].index(args.ref)
            print("Reference: %s (%d)" %(args.ref,ref))
        else:
            ref = None
        
        p_values, medians = test_per_timestep(data["perf"],compare_to_indx=ref)
        one_zero = lambda x : 1 if x <= args.alpha else 0
        rej = np.array([[one_zero(x)  for x in l] for l in p_values])
        final_median = medians[:,-1]
        final_rej = rej[:,-1]
        median_row = [medians[0,0]]
        rej_row = [0]
        for ac in acs:
            indx = data["names"].index(ac)
            median_row.append(final_median[indx])
            rej_row.append(final_rej[indx])
        median_table.append([args.scen_dict.get(scen,scen)] + median_row)
        rej_table.append([args.scen_dict.get(scen,scen)] + rej_row)
    median_table = prettify(array=median_table, sig_one_zero=rej_table)
    
    acs_header = [args.ac_dict.get(ac,ac) for ac in acs]            
    print(">>> Median + MWU Test")
    print(tabulate.tabulate(tabular_data=median_table, headers=["Set","Default"]+acs_header,tablefmt=args.table_style))
    
def auc_scen_table(scens, data_dict, acs, args):
    # AUC
    auc_table = []
    for scen in scens:
        data = data_dict[scen]
        auc_row = []
        auc = get_auc(performances=data["perf"])[0]
        for ac in acs:
            indx = data["names"].index(ac)
            auc_row.append(auc[indx])
        auc_table.append([args.scen_dict.get(scen,scen)] + auc_row)
    auc_table = prettify(array=auc_table)

    acs_header = [args.ac_dict.get(ac,ac) for ac in acs]            
    print(">>> AUC")
    print(tabulate.tabulate(tabular_data=auc_table, headers=["Set"]+acs_header,tablefmt=args.table_style))
    
def speedup_scen_table(scens, data_dict, acs, args):
    # Speedup
    table = []
    for scen in scens:
        data = data_dict[scen]
        row = []
        if args.ref:
            ref = data["names"].index(args.ref)
            print("Reference: %s (%d)" %(args.ref,ref))
        else:
            ref = None
        speedups = speedups_to_ref(performances=data["perf"],
                               compare_to_indx=ref,
                               time_list=data["time_list"][0])[0]
        for ac in acs:
            indx = data["names"].index(ac)
            row.append(speedups[indx])
        table.append([args.scen_dict.get(scen,scen)] + row)
    table = prettify(array=table,min=False, int_=False)

    acs_header = [args.ac_dict.get(ac,ac) for ac in acs]            
    print(">>> Speedup")
    print(tabulate.tabulate(tabular_data=table, headers=["Set"]+acs_header,tablefmt=args.table_style))

def single_scen(args):
    
    ALPHA = args.alpha
    
    performance, new_time_list, name_list = read_traj_files(args)

    if args.xmin:
        performance, new_time_list = cut_performances(performance, new_time_list, args.xmin)
    
    if args.ref:
        ref = name_list.index(args.ref)
        print("Reference: %s (%d)" %(args.ref,ref))
    else:
        ref = None
    
    p_values, medians = test_per_timestep(performance,compare_to_indx=ref)
    
    one_zero = lambda x : 1 if x <= ALPHA else 0
    rej = np.array([[one_zero(x)  for x in l] for l in p_values])

    auc = get_auc(performances=performance)
    speedups = speedups_to_ref(performances=performance,
                               compare_to_indx=ref,
                               time_list=new_time_list[0])
    final_median = medians[:,-1]
    final_rej = rej[:,-1] 
    aggr_stats = np.vstack((final_median,final_rej,auc,speedups)).T
    

    # add means
    p_values = add_mean_at_end(X=p_values)
    medians = add_mean_at_end(X=medians)
    rej = add_mean_at_end(X=rej)

    # convert
    pretty = lambda x: "%.2f" %(x)
    p_values = [[ pretty(x) for x in l] for l in p_values]
    medians  = [[ pretty(x) for x in l] for l in medians]
    rej  = [[ pretty(x) for x in l] for l in rej]
    aggr_stats  = [[ pretty(x) for x in l] for l in aggr_stats]
    time_steps = [pretty(x) for x in new_time_list[0]]
    
    # append AC names
    p_values = add_names(X=p_values, name_list=name_list)
    medians = add_names(X=medians, name_list=name_list)
    rej = add_names(X=rej, name_list=name_list)
    aggr_stats = add_names(X=aggr_stats, name_list=name_list)
    
    print(">>>p-values")
    print(tabulate.tabulate(tabular_data=p_values, headers=["AC"]+time_steps+["Mean"],tablefmt=args.table_style))
    print(">>>Reject (1)")
    print(tabulate.tabulate(tabular_data=rej, headers=["AC"]+time_steps+["Mean"],tablefmt=args.table_style))
    print(">>>medians")
    print(tabulate.tabulate(tabular_data=medians, headers=["AC"]+time_steps+["Mean"],tablefmt=args.table_style))
    print(">>>Aggregated Stats")
    print(tabulate.tabulate(tabular_data=aggr_stats, headers=["AC","Median","Sig. Test","AUC","Speedup"],tablefmt=args.table_style))
    

def cut_performances(performance, new_time_list, xmin):
    steps = new_time_list[0] # assume all time lists are identical
    rm_steps = 0
    for s in steps:
        if s < xmin:
            rm_steps += 1
        else:
            break
    new_time_list = [l[rm_steps:] for l in new_time_list]
    performance = [ac[:,rm_steps:] for ac in performance]
    return performance, new_time_list

def prettify(array, sig_one_zero=None, min=True, int_=False):
    '''
        first col str
        all others cols floats
        
        highlights best cols in bold 
        if 1 in sig_one_zero add *
    '''
    if min:
        opt_func = np.min
    else:
        opt_func = np.max 
    new_array = []
    for r_idx, d in enumerate(array):
        best = opt_func(d[1:])
        new_d = [d[0]]
        for c_idx, x in enumerate(d[1:]):
            if not int_:
                str = "%.2f" %(x)
            else:
                str = "%d" %(x)
            if x == best:
                str = "\mathbf{%s}" %(str)
            if sig_one_zero and sig_one_zero[r_idx][c_idx+1] == 1:
                str += "^*"
            new_d.append("$%s$" %(str))
        new_array.append(new_d)
    return new_array
    

if __name__ == "__main__":
    main()
