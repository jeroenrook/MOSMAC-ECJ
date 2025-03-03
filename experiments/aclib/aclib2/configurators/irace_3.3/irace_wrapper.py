#!/usr/bin/env python

#/home/lindauer/projects/SMAC_journal/aclib2/configurators/irace_2.0_3.3/irace_wrapper.py --cutoff 5 --instance /home/lindauer/projects/SMAC_journal/cplex_regions100_short/IRACE2_3.3/run-1/instances/mip/data/Regions100/CATS-d_regions-goods100-bids500_1614.lp --seed 1090200180 --config -python3 target_algorithms/mip/cplex12.6/wrapper.py --mem-limit 1024 --obj-file ./instances/mip/sets/Regions100/solubility.txt --config   --barrier_limits_growth  1e+12 --barrier_algorithm  0

import sys
import subprocess

cutoff = sys.argv[2]
instance=sys.argv[4]
seed=sys.argv[6]
wrapper=sys.argv[9]
config=sys.argv[10:]

for i,c in enumerate(config):
    if c.startswith("--") and c not in ["--config", "--mem-limit", "--obj-file", "--sol-file"]:
        config[i] = c[1:] # remove leading -
config = " ".join(config)

cmd = "python3 {wrapper} --overwrite_cost_runtime --instance {instance} --cutoff {cutoff} --seed {seed} {config}".format(**{"wrapper": wrapper, "config": config, "instance": instance, "seed": seed, "cutoff":cutoff})

print(cmd)

p = subprocess.Popen(cmd, shell=True)
p.communicate()
