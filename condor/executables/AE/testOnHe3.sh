#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV"
echo "Execute AE_testCustom.py --config yaml/He3.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python AE_testCustom.py --config yaml/He3.yml
