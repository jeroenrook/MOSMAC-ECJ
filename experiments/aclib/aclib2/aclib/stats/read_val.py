import os
import glob
import logging

import pandas as pd
import numpy as np

test_inc_exp_dir = 'validate-inc-test'
test_def_exp_dir = 'validate-def-test'
train_inc_exp_dir = "validate-inc-train"
train_def_exp_dir = "validate-def-train"
obj_matrix_exp = 'validationObjectiveMatrix*-*-walltime*.csv'
result_traj_exp = 'validationResults-*-walltime*.csv'
overtime_test_exp_dir = "validate-time-test"
overtime_train_exp_dir = "validate-time-train"


def read_val_files(scen_name: str, config: dict):

    ac_dict = {}
    for ac in config["configurators"]:
        dir_exp = os.path.join(scen_name, ac, "run-*")
        all_runs = glob.glob(dir_exp)
        failed_to_read = 0

        if not all_runs:
            logging.warn("No runs found for %s with %s" % (scen_name, ac))

        ac_dict[ac] = {}
        for run in all_runs:
            id_ = int(run.strip("/").split("-")[-1])
            if id_ > config["configurators"][ac].get("max_run_id", np.inf):
                continue
            if id_ < config["configurators"][ac].get("min_run_id", 0):
                continue

            logging.debug("Try first to read validate over time")
            logging.debug("Test performance over time")
            test_traj_data, test_obj_matrix = None, None
            if os.path.isdir(os.path.join(run, overtime_test_exp_dir)):
                test_traj_data, test_obj_matrix = read_over_time(
                    dn=os.path.join(run, overtime_test_exp_dir))
                logging.debug("Success")

            else:
                logging.debug(
                    "Have not found test performance over time (%s on %s-%d)" % (ac, scen_name, id_))
                logging.debug(os.path.join(run, overtime_test_exp_dir))

            logging.debug("Training performance over time")
            train_traj_data, train_obj_matrix = None, None
            if os.path.isdir(os.path.join(run, overtime_train_exp_dir)):
                train_traj_data, train_obj_matrix = read_over_time(
                    dn=os.path.join(run, overtime_train_exp_dir))
                logging.debug("Success")
            else:
                logging.debug(
                    "Have not found train performance over time (%s on %s-%d)" % (ac, scen_name, id_))
                logging.debug(os.path.join(run, overtime_train_exp_dir))

            if test_traj_data is not None\
               and test_obj_matrix is not None:
                ac_dict[ac][id_] = {}
                ac_dict[ac][id_]["test"] = {
                    "traj": test_traj_data, "obj_matrix": test_obj_matrix}
            else:
                failed_to_read += 1
                continue # don't store train data if test was not available
                
            if train_traj_data is not None\
               and train_obj_matrix is not None:
                ac_dict[ac][id_]["train"] = {
                    "traj": train_traj_data, "obj_matrix": train_obj_matrix}

            # TODO: read only def + inc

        if failed_to_read > 0:
            logging.warn("For %s on %s, the validation of %d runs could not be read." % (
                ac, scen_name, failed_to_read))

    return ac_dict


def read_over_time(dn: str):
    result_traj_fn = glob.glob(os.path.join(dn, result_traj_exp))
    if not result_traj_fn:
        logging.warn("Not found: %s" % (os.path.join(dn, result_traj_exp)))
        return None, None
    traj_data = read_trajectory_file(fn=result_traj_fn[0])

    obj_matrix_fn = glob.glob(os.path.join(dn, obj_matrix_exp))
    if not result_traj_fn:
        logging.warn("Not found: %s" % (os.path.join(dn, result_traj_exp)))
        return None, None
    obj_matrix = read_validationObjectiveMatrix_file(fn=obj_matrix_fn[0])

    return traj_data, obj_matrix


def read_trajectory_file(fn):
    return pd.read_csv(fn)


def read_validationObjectiveMatrix_file(fn):

    df = pd.read_csv(fn, index_col=0)
    del df["Seed"]
    return df


def rm_common_tos_test(ac_dict: dict, cutoff: float):

    acs = list(ac_dict.keys())
    common_timeouts = None
    for run_id in ac_dict[acs[0]].keys():
        try:
            # initialize with all instances
            common_timeouts = set(ac_dict[acs[0]][run_id]["test"]["obj_matrix"].index)
        except KeyError:
            continue
        break
    
    if not common_timeouts:
        logging.warn("No valid runs found to rm common TOs")
        return
    
    n_insts = len(common_timeouts)
    for ac_data in ac_dict.values():
        for run_data in ac_data.values():
            obj_matrix = run_data["test"]["obj_matrix"]
            solved_insts = ((obj_matrix < cutoff).sum(axis=1)) >= 1
            solved_insts = list(solved_insts[solved_insts].index)
            common_timeouts = common_timeouts.difference(solved_insts)

    n_common_timeouts = len(common_timeouts)
    logging.debug("Removed %d timeouts" %(n_common_timeouts))
    for ac_data in ac_dict.values():
        for run_data in ac_data.values():
            traj_pd = run_data["test"]["traj"]
            perf = traj_pd["Test Set Performance"]
            perf_corrected = (perf * n_insts - 10 * cutoff *
                              n_common_timeouts) / (n_insts - n_common_timeouts)
            traj_pd["Test Set Performance"] = perf_corrected
