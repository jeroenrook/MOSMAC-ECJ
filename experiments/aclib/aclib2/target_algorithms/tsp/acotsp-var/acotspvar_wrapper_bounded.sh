#!/bin/bash
success() {
    cost=$1
    time=$2
    echo 'Result of this algorithm run:  {"status": "SUCCESS", "cost": ' $cost ', "runtime": ' $time '}'
}

abort() {
    echo "$0: $@"
    echo 'Result of this algorithm run:  {"status": "ABORT", "cost": 0, "runtime": 0}'
    exit 1
}
crashed() {
    echo "$0: $@"
    echo 'Result of this algorithm run:  {"status": "CRASHED", "cost": 0, "runtime": 0}'
    exit 1
}

verbose(){
    ## print the command to the logfile
    echo "$@"
    ## run the command and redirect it's error output
    ## to the logfile
    eval $@
}

ARGS=
while [[ $# > 1 ]]; do
param="$1"
case $param in
    --cutoff)
        TIME="$2"
        shift
        ;;
    --instance)
        INSTANCE="$2"
        shift
        ;;
    --seed)
        SEED="$2"
        shift
        ;;
    --config)
        shift
        break
        ;;
    *)
        # unknown option
        ARGS="$ARGS $1"
        ;;
esac
shift
done
CAND_PARAMS=" $ARGS $*"

# Path to the ACOTSP software:
BINDIR=$(dirname "$(readlink -f "$(type -P $0 || echo $0)")")
EXE=${BINDIR}/src/acotsp
test -x ${EXE} || abort "$0: ${EXE}: not executable"
source $BINDIR/utils.sh

# What "fixed" parameters should be always passed to ACOTSP?
# The time to be used is always 5 seconds, and we want only one run:
FIXED_PARAMS=" --tries 1 --quiet --mmas --localsearch 1 --alpha 1 "

# Add the second - to the param line
CAND_PARAMS=${CAND_PARAMS//' -'/' --'}
CAND_PARAMS=${CAND_PARAMS//"'"/''}
CAND_PARAMS=${CAND_PARAMS//"--algorithm "/'--'}
# Delete -start from the end.
CAND_PARAMS=${CAND_PARAMS//"-start-"/'-'}


STDOUT=$(mktemp --suffix -${SEED}.stdout)
STDERR=$(mktemp --suffix -${SEED}.stderr)

# Now we can call ACOTSP by building a command line with all parameters for it
verbose $EXE ${FIXED_PARAMS} -i $INSTANCE ${CAND_PARAMS} --time $TIME --seed ${SEED} "1> $STDOUT 2> $STDERR"

# In case of error...
if [ ! -s "${STDOUT}" ]; then
    crashed "${STDOUT} not found or empty, see also $STDERR"
fi

NORMALIZE="$BINDIR/mo-tools/nondominated"
HV="$BINDIR/hv/hv"

cat $STDOUT | grep '^best [0-9]' | sed 's/best \([0-9]\+\).*time -\?\([0-9.]\+\)/\2 \1/' > ${STDOUT}_dat
INSTANCEDIR=$(dirname $INSTANCE)
LBOUNDFILE="${INSTANCEDIR}/RUE.Instances.3000.optimal.txt"

#echo "instance: $(basename $INSTANCE)"
LBOUND=$(grep $(basename $INSTANCE) $LBOUNDFILE | cut -f2 -d ' ')
HBOUND=$(float_eval "$LBOUND + ($LBOUND * 0.15)")
echo "lbound: $LBOUND  hbound: $HBOUND" >> $STDERR
$NORMALIZE --force-bound --filter --upper-bound "$TIME $HBOUND" --lower-bound "0 $LBOUND" --quiet -n "0.0 0.9" ${STDOUT}_dat -s "_nor" #|| crashed "running nondominated"
$HV --quiet -r "1.0 1.0" ${STDOUT}_dat_nor -s "_hv" || crashed "calculating hypervolume failed"
if [ ! -s "${STDOUT}_dat_nor_hv" ]; then
    crashed "${STDOUT}_dat_nor_hv: No such file or directory"
fi
COST="-$(cat "${STDOUT}_dat_nor_hv" | grep -e '^[0-9]' | cut -f1)"

# Print it!
success $COST $TIME

# We are done with our duty. Clean files and exit with 0 (no error).
rm -f "${STDOUT}*" "${STDERR}"
exit 0

