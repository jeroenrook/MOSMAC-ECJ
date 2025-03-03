#!/bin/bash
# This is a wrapper for translating irace from/to AClib wrapper format.

###############################################################################
# This script is the command that is executed every run.
# Check the examples in examples/
#
# This script is run in the execution directory (execDir, --exec-dir).
#
# PARAMETERS:
#--instance /home/lindauer/projects/SMAC_journal/cplex_regions100_short/IRACE2_3.3/run-1/instances/mip/data/Regions100/CATS-d_regions-goods100-bids500_0486.lp --seed 36595053 --config ---cutoff  5 -python3 target_algorithms/mip/cplex12.6/wrapper.py --mem-limit 1024 --obj-file ./instances/mip/sets/Regions100/solubility.txt --config
# $2 instance
# $4 seed
# $7 cutoff
# $9 wrapper
# $10: call
###############################################################################
INSTANCE=$2
SEED=$4
TIME=$7
WRAPPER=${9}
shift 10 

echo "python3 $WRAPPER --overwrite_cost_runtime --instance $INSTANCE --cutoff $TIME --seed $SEED $*"
python3 $WRAPPER --overwrite_cost_runtime --instance $INSTANCE --cutoff $TIME --seed $SEED $*
exit 0
