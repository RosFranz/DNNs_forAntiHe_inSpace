#!/bin/bash
echo "moving to /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/root_files"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/root_files
source /cvmfs/sft.cern.ch/lcg/views/LCG_104/x86_64-el9-gcc12-opt/setup.sh
echo "Executing the DataMC comparison macro ... "
root -l -b -q 'DataMC_comparison.cc("2025_10_27","2025_10_27","2025_10_27","2025_10_27")'

echo "Done!"
