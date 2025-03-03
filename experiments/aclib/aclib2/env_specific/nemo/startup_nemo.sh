#!/bin/bash

echo $TMPDIR
conda create -q -y -p $TMPDIR --clone /home/fr/fr_fr/fr_tl1023/aclib/aclib2-master/env_specific/nemo/env_aclib/ --copy
source activate $TMPDIR
pip freeze

#pip install -r /work/ws/nemo/fr_tl1023-aclib-0/aclib2-master/requirements.txt/



