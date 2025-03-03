import glob
import sys
import csv
import re
import logging

import numpy as np
import pandas as pd

from plottingscripts.utils import read_util

def read_traj_files(args):    
    
    file_list, name_list = get_files(args)
    
    # Get data from csv
    performance = list()
    time_ = list()
    show_from = -sys.maxsize

    for name in range(len(name_list)):
        # We have a new experiment
        performance.append(list())
        time_.append(list())
        for fl in file_list[name]:
            _none, csv_data = read_util.read_csv(fl, has_header=True)
            csv_data = np.array(csv_data)
            #Replace too high values with args.maxint
            if args.est_train: # estimated train perf
                data = [min([args.maxvalue, float(i.strip())])
                        for i in csv_data[:, 1]]
            else: # validated perf
                data = [min([args.maxvalue, float(i.strip())])
                        for i in csv_data[:, 2]]
            #===================================================================
            # else:
            #     print("This should not happen")
            #===================================================================
            # do we have only non maxint data?
            show_from = max(data.count(args.maxvalue), show_from)
            performance[-1].append(data)
            time_[-1].append([float(i.strip()) for i in csv_data[:, 0]])

    performance = [np.array(i) for i in performance]
    time_ = [np.array(i) for i in time_]

    if args.train:
        logging.info("Read TRAIN performance")
    elif args.test:
        logging.info("Read TEST performance")
    elif args.est_train:
        logging.info("Read estimated TRAIN performance")
    else:
        logging.info("Don't know what I'm printing")

    if args.xmin is None and show_from != 0:
        args.xmin = show_from

    #new_time_list = [time_ for i in range(len(performance))]
    
    if args.rm_tos_at:
        rm_tos(performance=performance, args=args)
    
    performance, time_ = unify_timestamps(performances=performance, times_=time_)
    
    return performance, time_, name_list

def unify_timestamps(performances, times_):
    '''
        fix time series such that all have the same time stamps
    '''
    
    new_perfs = []
    new_times = []
    for performance, time_  in zip(performances, times_):
        merged_pd = pd.DataFrame(np.array([time_[0],performance[0]]).T, columns=["t",0])
        for i, (t, p) in enumerate(zip(time_[1:],performance[1:])):
            pd_ = pd.DataFrame(np.array([t,p]).T, columns=["t",i+1])
            merged_pd = pd.ordered_merge(merged_pd, pd_, fill_method='ffill')
            
        merged_pd.fillna(method='bfill', inplace=True)
        merged_times = merged_pd["t"].values
        merged_perfs = merged_pd.values[:,1:].T
        new_perfs.append(merged_perfs)
        new_times.append(merged_times)
        
    return new_perfs, new_times


def rm_tos(performance, args):
    '''
        remove common timeouts (across runs and timesteps) from statistics
    '''
    file_list, name_list = get_files(args, reg="validationRunResultLineMatrix-traj*-*time*.csv")
    
    instances = set()
    solved_insts = set()
    for ac_files in file_list:
        for fn in ac_files:
            with open(fn) as fp:
                csv_reader = csv.reader(fp, delimiter=',', quotechar='"')
                next(csv_reader) # skip first entry
                for line in csv_reader:
                    inst = line[0]
                    instances.add(inst)
                    solved = False
                    for res in line[1:]:
                        if "SAT" in res or "SUCCESS" in res:
                            solved = True
                            break
                    if solved:
                        solved_insts.add(inst)
    unsolved_insts = instances.difference(solved_insts)
    n_insts = len(instances)
    n_unsol = len(unsolved_insts)
    logging.info("Unsolved instances: %d" %(n_unsol))
    
    for ac in performance:
        for idx, run in enumerate(ac):
            ac[idx] = [(p*n_insts -  args.rm_tos_at*10*n_unsol) / (n_insts-n_unsol) for p in run]
    
    return performance
    
def get_files(args, reg="validationResults*-traj*-*time*.csv"):
    '''
        get validation files for a given file name regex
    '''
    
        # Get files and names
    if args.test or args.est_train:
        all_file_list = glob.glob(
            "%s/*/run-*/validate-time-test/%s" % (args.scenario, reg))
    elif args.train:
        all_file_list = glob.glob(
            "%s/*/run-*/validate-time-train/%s" % (args.scenario, reg))
        
        
    file_dict = {}
    
    if args.subset_ac:
        ac_regex = "(" + ")|(".join(args.subset_ac) + ")"
        
    for fn in all_file_list:
        ac = fn.split("/")[1]
        if args.subset_ac and not re.match(ac_regex, ac):
            continue
        file_dict[ac] = file_dict.get(ac, [])
        file_dict[ac].append(fn)

    if len(file_dict) == 0:
        logging.warn("Could not find any file for %s" % args.scenario)
        return

    name_list = sorted(list(file_dict.keys()))
    file_list = [file_dict[ac] for ac in name_list]

    for idx in range(len(name_list)):
        print("%20s contains %d file(s)" %
              (name_list[idx], len(file_list[idx])))

    if args.verbose:
        name_list = [name_list[i] + " (" + str(len(file_list[i])) + ")"
                     for i in range(len(name_list))]
        
    return file_list, name_list
