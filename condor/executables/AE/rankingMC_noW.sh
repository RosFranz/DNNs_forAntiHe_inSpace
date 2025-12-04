#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/AE/ranking"
echo "Execute ranking.py --config yaml/no_LossW.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/AE/ranking
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python ranking.py --config ../../yaml/no_LossW.yml
