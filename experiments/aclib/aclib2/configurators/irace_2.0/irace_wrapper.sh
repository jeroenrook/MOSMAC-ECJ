#!/bin/bash
# This is a wrapper for translating irace from/to AClib wrapper format.

###############################################################################
# This script is the command that is executed every run.
# Check the examples in examples/
#
# This script is run in the execution directory (execDir, --exec-dir).
#
# PARAMETERS:
# $1 is the candidate configuration number
# $2 is the instance ID
# $3 is the seed
# $4 is the instance name
# The rest ($* after `shift 4') are parameters to the run
#
# RETURN VALUE:
# This script should print one numerical value: the cost that must be minimized.
# Exit with 0 if no error, with 1 in case of error
###############################################################################
CANDIDATE=$1
INSTANCEID=$2
SEED=$3
INSTANCE=$4
shift 4 || exit 1

ACLIB_WRAPPER=
while [[ $# > 1 ]]; do
param="$1"
case $param in
    --cutoff)
        TIME="$2"
        shift
        ;;
    --config)
        break
        ;;
    *)
        # unknown option
        ACLIB_WRAPPER="$ACLIB_WRAPPER $1"
        ;;
esac
shift
done
ARGS="$*"

BINDIR="$(dirname "$(readlink -f "$0")")"
#cd "${BINDIR}/../"

# if [ ! -x $ACLIB_WRAPPER ]; then
#     BINDIR="$(dirname "$(readlink -f "$0")")"
#     ACLIB_WRAPPER="${BINDIR}/../${ACLIB_WRAPPER}"
#     if [ ! -x $ACLIB_WRAPPER ]; then
#         echo "$ACLIB_WRAPPER not found!"
#         exit 1
#     fi
# fi
echo "$ACLIB_WRAPPER --overwrite_cost_runtime --instance $INSTANCE --cutoff $TIME --seed $SEED $ARGS"
$ACLIB_WRAPPER --overwrite_cost_runtime --instance $INSTANCE --cutoff $TIME --seed $SEED $ARGS
exit 0
