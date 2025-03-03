This software package contains the SAT solver "SparrowToRiss"
Adrian Balint, Norbert Manthey, 2014

The SAT solver "SparrowToRiss" combines the SLS solver Sparrow with 
the CDCL SAT solver Riss.

For both solvers, the CNF simplifier Coprocessor is used to simplify
the input formula best for the kind of solver that is attached. 
First, Sparrow is executed until 500000000 flips are made, or until
900 seconds cpu time have passed. If the formula is not solved,
Riss is started afterwards for the remaining run time of the solver.

To build the solve, run the script
	./build.sh
	
To run the solver, use the shell script
	./SparrowToRiss.sh <input.cnf> <seed> <tmpDir> <FLIPS> <ScpuSeconds> "<sparrowParams>" "<rissParams>"

