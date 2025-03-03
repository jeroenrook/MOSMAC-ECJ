#!/usr/bin/python

# Wrapper for clasp subset example. Due to the many different types of
# parameter formats read by various solvers, GGA only natively supports very
# basic command line formats. For solvers such as clasp, a wrapper is needed to
# put the parameters into the correct format.

import sys, os

args = sys.argv

eargs = ["clasp"]

if len(sys.argv) <= 2:
    sys.stderr.write("Clasp wrapper command line error.\n")
    sys.exit(1)

eargs.append("--seed={0}".format(args[2]))
eargs.append("--time-limit=2")

ii = 3
while ii < len(args):
    pre,post = args[ii].split('=')
    if pre == "init-moms":
        if int(post) == 0:
            eargs.append("--no-init-moms")
        else:
            eargs.append("--init-moms")
    elif pre == "restarts":
        restarts = ''
        if post != 'no':
            nn,nnval = args[ii + 1].split('=')
            ii += 1
            if post in ['x','+','D']:
                ee,eeval = args[ii + 1].split('=')
                ii += 1
                restarts = "--restarts={0},{1},{2}".format(post,nnval,eeval)
            else:
                restarts = "--restarts={0},{1}".format(post,nnval)
        else:
            restarts = "--restarts=no"
        eargs.append(restarts)
    else:
        eargs.append("--{0}={1}".format(pre,post))
    ii += 1

eargs.append(args[1])

# Note: I recommend using one of the exec* family sys calls to execute a child
# process. The reason is that this fully replaces this wrapper in memory,
# rather than simply running a child process as in the case of system(). This
# means that messages from GGA (such as SIGUSR1 and SIGTERM) are passed to the
# program, wrather than to the wrapper.
os.execvp("./clasp", eargs)

# print eargs



