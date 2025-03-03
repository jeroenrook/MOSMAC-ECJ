Author: Yasha Pushak
Created: 2017-12-20
Last updated: 2019-03-11

This is a readme file for the parameter configuration space of cadical.

Created from running ./cadical -h
Since bounds are unspecified for numeric parameters, we multiple the defaultvalues by 100 and 0.001 to get bounds for doubles. If the default is 0 or -1 for 'off', then we arbitrarily choose 1000 as the maximum.

I also modifed the configuration space a bit beyond the output of this command based on observations I made when running cadical. These modifications are as follows:

#Numbers greater than 1 seem to be mapped to 1 even though it is considered an integer
arenasort categorical {0, 1} [1]

This appears to control termination, so it has been removed.
#clim integer [-1, 1000] [-1]

This appears to control termination, so it has been removed.
#dlim integer [-1, 1000] [-1]

This one seems to be capped at 1
emagluefast real [0.0003, 1] [0.03]

This one is also capped at 1
probereleff real [0.0002, 1] [0.02]
