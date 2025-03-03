import sys
import os
import glob
import json
from argparse import ArgumentParser, ArgumentDefaultsHelpFormatter

import pandas as pd

parser = ArgumentParser(formatter_class=ArgumentDefaultsHelpFormatter)

parser.add_argument("--scenario", required=True, help="scenario name")
parser.add_argument("--configurator", default=None, help="configurator name -- if not given, looking in all available configurator files")

args_ = parser.parse_args()

if args_.configurator is None:
    ac = "*"
else:
    ac = args_.configurator
    
run_dirs = glob.glob("{0}/{1}/run-*/".format(args_.scenario, ac))

results = []

for dn in run_dirs:
    try:
        val_res_fn = glob.glob(os.path.join(dn, "validate-inc-train", "validationResults*"))[0]
    except IndexError:
        continue
    
    try:
        call_fn = glob.glob(os.path.join(dn, "validate-inc-train", "validationCallStrings*"))[0]
    except IndexError:
        continue
    
    val_pd = pd.read_csv(val_res_fn,index_col=0)
    call_pd = pd.read_csv(call_fn, index_col=0)
    
    perf = float(val_pd["Test Set Performance"].values[-1])
    conf = call_pd["Configuration "].values[-1]
    
    results.append((perf,conf))
    
results = sorted(results, key=lambda x: x[0])

#print(json.dumps(results, indent=2))

print(args_.scenario)
print(results[0][1])
