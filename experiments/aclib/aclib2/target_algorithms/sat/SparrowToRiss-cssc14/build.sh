#!/bin/sh
# Norbert Manthey, 2014
#
# script to build the SAT solver SparrowToRiss
#

cd code/Riss427;
# build Pcasso
make rissRS ARGS="-DTOOLVERSION=427"
cp riss ../../riss

# build Coprocessor
make clean
make coprocessorRS ARGS="-DTOOLVERSION=427"
cp coprocessor ../../

# build sparrow
cd ../Sparrow
make sparrow
cp sparrow ../../

# copy the call script
cd ..
cp scripts/SparrowToRiss.sh ../

# return to calling directory
cd ..
