# Requirements for a New Scenario

__Goal:__ only a small hand-selected set of scenarios in the core; further available scenarios in branches

* Using the below listed specification will help use to easily convert all scenarios later in the upcoming unified AClib format (which is not yet supported by the all configurators)

    * furthermore, by using the specification, we will not need a json file any more to store all meta information
	* see `aclib/converters/` for scripts to convert into the new format
 
* scenario name has to be unique (also across different domains); name scheme: <algorithm>_<instance set>
* all scenarios, target_algorithms and instances are associated with a domain (ASP, ML, SAT, MIP ...)
 
    * there will not be a "multi problem" domain (previously used for clasp)
   
* all target algorithms are wrapped by the [generic wrapper](https://github.com/mlindauer/GenericWrapper4AC)

    * the generic wrapper already includes the installation of the [runsolver](http://www.cril.univ-artois.fr/~roussel/runsolver/)
    
* instance set should be sufficient large and provides instances features (trivial features are often sufficient) for training and test instances
* no support of "instance specifics" in instance files

    * "instance specifics" are not supported in the upcoming unified AClib format 
    * if necessary, please provide a file with instance specific information directly for the wrapper (see ml scenarios for examples); this file should live in `instances/<type>/sets/<set key>/`

* as long as we do not use the new PCS format (as specified in the upcoming unified AClib format), all pcs files are in the SMAC 2.08 format
* All instance related files live in their own folder (`<type>/sets/<set key>/`)

    * this includes instance specific files and files for solution checking (see mip/sets/Regions200)
    * no instance-related file should live on the <type>/set/ level 
    
* fixed file name scheme:

    * scenario.txt
    * params.pcs
    * wrapper.py
    * instance files: training.txt, test.txt, features.txt
    
* fixed scenario keywords:
  
     * algo (str)
     * execdir (has to be ".")
     * deterministic ("0" or "1")
     * run_obj ("runtime" or "quality)
     * cutoff_time (int)
     * wallclock_limit (int)
     * runcount_limit (int)
     * paramfile (str)
     * instance_file (str)
     * test_instance_file (str)
     * feature_file (str) 

* if possible, returned solution should be verified by wrapper
* if possible, the scenario should have a bibtex reference

## Folder structure of AClib 

AClib provides an archive with the following structure and components:

```
|- AClib/
	|- README.md
	|- aclib/							contains all scripts to run aclib
	|- configurators/					contains algorithm configurators
		|- paramils/
		|- smac/
		|- irace/
	|- instances/
		|- <problem type>/ 				e.g., SAT, TSP, ASP, Planning
			|- sets/
				|- <set key>/ 			e.g., CSSC2013_SWV
					|- README			description, origin, details
					|- features.txt		(optional) contains instance features
					|- training.txt		all training instances (refers to AClib/instances/.../data/...)
					|- test.txt			all test instances (refers to AClib/instances/.../data/...)
			|- data/					uncompressed(!) instance files (can be compressed with a script later)
				|- <set key>/ 
					|- <subsets>/ 		possibly other sub-directories, each set with a README for description, origin, license
	|- scenarios/
		|- <problem type>/ 				e.g., SAT, TSP, ASP, Planning
			|- <scenario key>/
				|- scenario.txt			scenario file
				|- README				description, origin, license
	|- target_algorithms/
		|- <problem type>/
			|- <algorithm key>/			e.g., "spear"
				|- README				description, origin, license
				|- params.pcs			configuration space file
				|- wrapper.py			wrapper of target algorithm (inherits from GenericWrapper)
				|- ...					binary and/or source files to execute algorithm 
			|- tools/					problem specific tools (e.g., SAT solution checking)
```		