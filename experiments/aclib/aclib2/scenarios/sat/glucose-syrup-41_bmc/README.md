Glucose crashes for very rare cases with a segmentation fault when run with a specific combination of SIM options (specificially `-cl-lim`, `-grow` and `sub-lim`)
The only instance for which we are aware that this happens is:

 > instances/sat/data/BMC08-CSSC14/instances/Sat_Data/bmc/bmc08-50/kenflashp07.k50.cnf
 
 A sample call would be:
 
 > target_algorithms/sat/glucose-syrup-41/glucose_static -model -rnd-seed=375162 -cl-lim=-1 -grow=0 instances/sat/data/BMC08-CSSC14/instances/Sat_Data/bmc/bmc08-50/kenflashp07.k50.cnf
 