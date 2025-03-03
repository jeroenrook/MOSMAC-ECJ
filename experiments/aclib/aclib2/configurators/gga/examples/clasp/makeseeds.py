#!/usr/bin/python

# Generates a seed-instance list for a list of files on standard out

import sys, random

usage = "Usage: ./makeseeds.py <# of seeds per instance> <path to file 1 ... n>\n"
if len(sys.argv) < 3 or sys.argv[1] in ["-h", "-help", "--help"]:
    sys.stderr.write(usage)
    sys.exit(1)

numSeeds = 1
try:
    numSeeds = int(sys.argv[1])
except:
    sys.stderr.write("Number of seeds must be an integer!\n")
    sys.stderr.write(usage)
    sys.exit(1)

for ff in sys.argv[2:]:
    for ii in xrange(numSeeds):
        print "{0} {1}".format(random.randint(1,99999), ff)


