IRACE=./configurators/irace_3.3/installed/irace/bin/irace
if [ ! -x $IRACE ]; then
    echo "$0: error: $IRACE not found or not executable"
    exit 1
fi


BINDIR="$(dirname "$(readlink -f "$IRACE")")"
export R_LIBS=${BINDIR%irace/bin}:"$R_LIBS"

$IRACE -s $1 --debug-level 0 --parallel $2 --capping 1 --bound-par 10

