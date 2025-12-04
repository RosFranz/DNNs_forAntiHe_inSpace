#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/FCNNs"
echo "Execute CL2_onNeg1.py --config yaml/onISS.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python CL2_onNeg1.py --config yaml/onISS.yml
