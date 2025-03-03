#Algorithm Configuration Library 2.0

## Remark

This repository is cloned from https://bitbucket.org/mlindauer/aclib2/src/master/ but scenarios are altered to work with multiple objectives

## Install

AClib requires Python 3.5 (we implemented AClib under Anaconda 3.4) and Java >8

`pip install -r requirements.txt`

Some target algorithms may have further dependencies.


### Installation of Instances

Since the instance sets are by far too large to upload on bitbucket (compressed >20GB), 
please download the instance sets manually.

Please extract the instances in the root directory of AClib:

`tar xvfz XXX.tar.gz`

* [mip_Regions200](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/mip_Regions200.tar.gz)
* [mip_BCOL-CLS](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/mip_BCOL-CLS.tar.gz)
* [mip_RCW2](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/mip_RCW2.tar.gz)
* [mip_COR-LAT](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/mip_COR-LAT.tar.gz)
* [planning_depots](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/planning_depots.tar.gz)
* [planning_satellite](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/planning_satellite.tar.gz)
* [planning_zenotravel](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/planning_zenotravel.tar.gz)
* [ml_secom](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/ml_secom.tar.gz)
* [ml_covertype](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/ml_covertype.tar.gz)
* [ml_shuttle](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/ml_shuttle.tar.gz)
* [ml_krvskp](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/ml_krvskp.tar.gz)
* [ml_winequalitywhite](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/ml_winequalitywhite.tar.gz)
* [ml_abalone](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/ml_abalone.tar.gz)
* [ml_mnist](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/ml_mnist.tar.gz)
* [ml_semeion](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/ml_semeion.tar.gz)
* [ml_waveform](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/ml_waveform.tar.gz)
* [ml_iris](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/ml_iris.tar.gz)
* [ml_car](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/ml_car.tar.gz)
* [ml_germancredit](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/ml_germancredit.tar.gz)
* [ml_madelon](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/ml_madelon.tar.gz)
* [ml_yeast](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/ml_yeast.tar.gz)
* [asp_weighted-sequence](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/asp_weighted-sequence.tar.gz)
* [sat_QCP](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/sat_QCP.tar.gz)
* [sat_GI-CSSC14](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/sat_GI-CSSC14.tar.gz)
* [sat_7sat90](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/sat_7sat90.tar.gz)
* [sat_FACTORING](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/sat_FACTORING.tar.gz)
* [sat_7SAT90-SAT-CSSC14](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/sat_7SAT90-SAT-CSSC14.tar.gz)
* [sat_3CNF-V350-CSSC14](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/sat_3CNF-V350-CSSC14.tar.gz)
* [sat_5SAT500-SAT-CSSC14](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/sat_5SAT500-SAT-CSSC14.tar.gz)
* [sat_K3-CSSC14](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/sat_K3-CSSC14.tar.gz)
* [sat_hgen2-small](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/sat_hgen2-small.tar.gz)
* [sat_circuit_fuzz](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/sat_circuit_fuzz.tar.gz)
* [sat_SWGCP](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/sat_SWGCP.tar.gz)
* [sat_CBMC](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/sat_CBMC.tar.gz)
* [sat_instances_satenstein.tar.gz](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/sat_instances_satenstein.tar.gz.tar.gz)
* [sat_CIRCUITFUZZ-CSSC14](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/sat_CIRCUITFUZZ-CSSC14.tar.gz)
* [sat_QUEENS-CSSC14](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/sat_QUEENS-CSSC14.tar.gz)
* [sat_LABS-CSSC14](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/sat_LABS-CSSC14.tar.gz)
* [sat_SWGCP_SAT](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/sat_SWGCP_SAT.tar.gz)
* [sat_UNSAT-UNIF-K5-CSSC14](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/sat_UNSAT-UNIF-K5-CSSC14.tar.gz)
* [sat_k3-r4_26-v600](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/sat_k3-r4_26-v600.tar.gz)
* [sat_3SAT1K-SAT-CSSC14](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/sat_3SAT1K-SAT-CSSC14.tar.gz)
* [sat_SWV-Calysto](http://aad.informatik.uni-freiburg.de/~lindauer/aclib/sat_SWV-Calysto.tar.gz)

For some instance sets, we don't know the actual license they were published under --- not listed above.
Please write us an e-mail to get details how to access these instance sets.

## Requirements for AC procedures

### SMAC

* Java 8

### ParamILS

* ruby 1.9.3 (https://www.ruby-lang.org/de/news/2014/05/16/ruby-1-9-3-p547-released/)

### GGA

* precompiled on Ubuntu 14.04 LTS with gcc -- probably needs to be recompiled

### IRACE

* R (see irace_2.0 README)
* jsonlite package in R (`install.packages('jsonlite')`)

Call the following command to install irace:

`cd configurators/ && make install` 

## Example

### Configuration

To run a scenario call:

`python aclib2/aclib/run.py -s cryptominisat_circuit_fuzz -c SMAC2 -n 2 --env local`

It runs the scenario __circuit_fuzz_cryptominisat__ with 2 independent SMAC (v2) runs. 

To use the same resources for GGA, call:

`python aclib2/aclib/run.py -s cryptominisat_circuit_fuzz -c GGA -n 1 --ac_cores 2 --cores_per_job 2 --env local`

### Validation without Workers

To validate the runs (here training and test performance of final incumbent):

`python aclib2/aclib/validate.py -s cryptominisat_circuit_fuzz -c SMAC2 -n 2 --set TRAIN+TEST --mode INC --env local`


### Validation with Workers

To validate the runs (here performance over time with using workers on a MYSQL data base -- this will work only on our meta cluster in Freiburg. Send us an e-mail if you also want to use it on your system.):

`python aclib2/aclib/validate.py -s cryptominisat_circuit_fuzz -c SMAC2 -n 2 --set TEST --mode TIME --pool test_aclib2 --env local`

and to submit workers:

`python aclib2/aclib/submit_workers.py --pool test_aclib2 -n 10000 --time_limit 910`

To get read results from the database, run again:

`python aclib2/aclib/validate.py -s cryptominisat_circuit_fuzz -c SMAC2 -n 8 --set TEST --mode TIME --pool test_aclib2 --env local`

### Statistics and Plots

If you have validated your runs, you can run the following command to get some basic statistics and scatter plots:

`python aclib2/aclib/get_evaluation_stats.py` 

This script will look into "." for runs generated with the previous scripts.

If you have validated your runs over time (`--mode TIME`), you can plot the performance of the configurators over time:

`python aclib2/aclib/plot_perf_over_time.py -s cryptominisat_circuit_fuzz`

## Issue Tracker

https://bitbucket.org/mlindauer/aclib2/issues

## Scenarios

Here is a small selection of well-studied AC scenarios to show the diversity of AClib:

| Scenario 							| Domain	| Configurators 			| #Params 	| #Instances 	| Budget 	|
| --------------------------------- | --------- | ------------------------- |:---------:|:-------------:|:---------:|
| clasp-weighted-sequence		 	| ASP		| SMAC, ROAR, PILS			| 90		| 240/240	 	| 4d	 	|
| cplex_regions200	 				| MIP 		| SMAC, ROAR, PILS, GGA		| 73		| 1000/1000		| 2d	 	|
| lpg-zenotravel					| Planning 	| SMAC, ROAR, PILS	 		| 67		| 2000/2000		| 2d	 	|
| cryptominisat_circuit_fuzz		| SAT		| SMAC, ROAR, PILS, GGA		| 36		| 299/302		| 2d     	|
| probsat_7sat90					| SAT		| SMAC, ROAR, PILS, GGA?	| 9			| 250/250		| 3h     	|
| spear_swgcp						| SAT		| SMAC, ROAR, PILS			| 26		| 1000/2000		| 5h     	|
| branin (multi-instance)			| BBOB		| SMAC, ROAR, PILS			| 1			| 76/75			| 1000 runs |
| xgboost_covertype					| ML		| SMAC, ROAR, PILS, IRACE	| 11		| 10/1			| 500 runs |
| svm_mnist							| ML		| SMAC, ROAR, PILS, IRACE	| 7			| 10/1			| 500 runs |


## Adding new scenarios

Please see ADD_SCENARIOS.md for all format requirements.


## Contact

Marius Lindauer lindauer@cs.uni-freiburg.de
