#!/usr/bin/env python3

import json
import argparse
import csv


def extract_irace_traj(fn_in:str, fn_out:str="traj_irace"):
    '''
        reads irace stdout log file and extracts the trajectory info
        
        Arguments
        ---------
            fn_in: str
                filename of irace log
            fn_out: str:
                filename of trajectory file to be written
    '''
    
    header = ["CPU Time Used","Estimated Training Performance","Wallclock Time","Incumbent ID","Automatic Configurator (CPU) Time","Configuration..."]
    
    trajectory_wc = []
    trajectory_n_eva = []
    id = 0
    with open(fn_in) as fp:
        while True:
            try:
                line = next(fp)
            except StopIteration:
                break
            
            if line.startswith("Description of the best configuration:"):
                id += 1
                params = list(filter(lambda x: x != "", next(fp).replace("\n","").split(" ")))[3:-1] # ignore id, cutoff, real_target_runner, .PARENT
                config = list(filter(lambda x: x != "", next(fp).replace("\n","").split(" ")))[3:-1]
                config = ["%s='%s'" %(k,v.strip()) for k,v in zip(params, config) if v != "<NA>" and v != "NA"]
               
            try:
                traj_entry = json.loads(line)
                add_to_traj = [traj_entry["wallclock_time"], traj_entry["cost"], traj_entry["wallclock_time"], id, "0"]
                add_to_traj.extend(config)
                trajectory_wc.append(add_to_traj)
                add_to_traj = [traj_entry["evaluations"], traj_entry["cost"], traj_entry["evaluations"], id, "0"]
                add_to_traj.extend(config)
                trajectory_n_eva.append(add_to_traj)
            except json.decoder.JSONDecodeError:
                pass

    with open(fn_out+"_wc.csv", "w") as fp:
        cw = csv.writer(fp)
        cw.writerow(header)
        for entry in trajectory_wc:
            cw.writerow(entry)
    with open(fn_out+"_neva.csv", "w") as fp:
        cw = csv.writer(fp)
        cw.writerow(header)
        for entry in trajectory_n_eva:
            cw.writerow(entry)            

if __name__ == '__main__':
    
    argparse = argparse.ArgumentParser()
    argparse.add_argument("-f","--file", required=True, help="output log file of irace (stdout)")
    
    args = argparse.parse_args()
    
    extract_irace_traj(fn_in=args.file)
    
    

