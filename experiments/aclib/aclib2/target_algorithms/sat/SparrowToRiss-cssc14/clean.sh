#!/bin/sh
# Norbert Manthey, 2014
#
# script to clean the SAT solver SparrowToRiss
#

# clean Riss
cd code/Riss427;
make clean

# clean Sparrow
cd ../Sparrow
make clean

# return to calling directory
cd ../..

rm -f riss sparrow coprocessor
