#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/preprocessing" 
echo "Execute TreeToH5.py"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/preprocessing
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python TreeToH5.py --config ../yaml/preprocessing/He4cc.yml
