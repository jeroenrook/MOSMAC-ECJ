Modified by Holger Hoos to accept target solution quality and cuttoff time (in CPU sec), 
as well as a random number seed as command line arguments, terminate once target quality 
or cutoff time are reached or exceeded. This version uses the original restart mechanism.

All changes marked 'hh' in source.

Example call (random number seed 123):

./jikken 10 DATA 100 30 rat575.tsp 6773 3600 123

Specify target quality 0 to specify no target solution quality:

./jikken 10 DATA 100 30 rat575.tsp 0 60 123

NOTE: EAX treats tour length as an integer; therefore, target solution quality also needs to be specified as an integer.

NOTE: cutoff time is specified in CPU seconds (i.e., first example above: 1h cutoff, 
second example: 1min).

NOTE: As done in the original code for trials, here, we read in the instance anew each time the search is restarted. This is probably not the most efficient thing to do, 
but neither is completely restarting the search.

Another example run (this one requiring restarts to solve reliably and using 'time' for time measurement):

time ./jikken 1 DATA 100 30 rbw2481.tsp 7724 3600 126