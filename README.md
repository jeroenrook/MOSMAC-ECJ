## Installation

```bash
conda create -n mosmac python=3.10
conda activate mosmac

git submodule add https://github.com/jeroenrook/SMAC3.git
git submodule update --init --recursive
cd SMAC3
git checkout mosmac
pip install -e .

git submodule add git@github.com:jeroenrook/random_forest_run.git
git submodule update --init --recursive
cd random_forest_run
git checkout leaf_values_over_instances
mkdir build
cd build
cmake ..
make -j4
cd python_package
pip install -e .

cd ..
pip install -r requirements.txt
 conda install -c conda-forge pygmo
```

## Understanding SMAC
Create class diagram 
```bash
pip install pylint
cd SMAC3
pyreverse -o pdf --colorized --filter-mode SPECIAL --max-color-depth 3 -p smac smac
```

## Running the experiments
The experiments rely on a SLURM scheduler for a HPC cluster to run on. 
Make sure the configuration file in `experiments/ac/config.yaml` contains the proper SLURM commands for your cluster and point to the right directories for running/logging and results. 
Also adjust the `base_dir` in the `run_experiments.py` with the absolute path to the `ac_lib` directory

### Experimental procedure
First perform all the configuration runs and the validation and test phase after each run. 
```
cd experiments/ac
./run_experiments.py launch
```
Once all jobs completed the results for each scenario can be collected using
```
./run_experiments.py --action collect launch
```

The Jupyter notebook `experiments/ac/parse_results.ipynb` uses the obtained results and generates the result representation as used in the paper.