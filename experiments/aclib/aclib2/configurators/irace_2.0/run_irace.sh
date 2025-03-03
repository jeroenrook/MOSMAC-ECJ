IRACE=./configurators/irace_2.0/installed/irace/bin/irace
if [ ! -x $IRACE ]; then
    echo "$0: error: $IRACE not found or not executable"
    exit 1
fi


BINDIR="$(dirname "$(readlink -f "$IRACE")")"
export R_LIBS=${BINDIR%irace/bin}:"$R_LIBS"

$IRACE -s $1 --debug-level 0 --runlog log_irace.txt --parallel $2

