#!/bin/sh
tar xf ../archives/cadical*
mv cadical* cadical
cd cadical
./configure
make
install -s build/cadical ../../bin/
