#!/bin/bash

conda create -q -y -p $PWD/env_aclib pip 
source activate $PWD/env_aclib
git clone https://github.com/mlindauer/GenericWrapper4AC.git
cd GenericWrapper4AC 
python setup.py install
cd ..
pip install -r ../../requirements.txt

#echo $TMPDIR
#conda create -q -y -p $TMPDIR pip
#source activate 
#pip install -r /work/ws/nemo/fr_tl1023-aclib-0/aclib2-master/requirements.txt/



