# Requirements

* A configurator needs to be able to read all AClib files (see ADD_SCENARIOS.md)
* All files are written in the old SMAC'syntax. See `http://www.cs.ubc.ca/labs/beta/Projects/SMAC/v2.08.00/manual.pdf`
* The configurator has to output a trajectory file (i.e., a csv file with each line corresponding to a new incumbent configuration and some meta-information)
* (We have a branch called `aclib2`. Here, we use a more recent version of the file syntax (first of all the pcs format). See `AClib_Format.md` in the `aclib2` branches for more information. Please note that the `aclib2` branch is currently not well maintained, but it could be the future.)

# Adding a configurator

* all files of the configurator should live in `./configurators/<configurator name>`
* the configurator has to be registered in `./aclib/run.py` and `./aclib/validate.py`
    * add the configurator in the `ArgumentParser`
    * add the name translation in `aclib/configurators/ac_interface.py`
    * add a class for the configurator in `aclib/configurators/` inheriting `aclib.configurators.base_configurator.BaseConfigurator`
      (see `aclib/configurators/smac2.py` for an example)