#!/bin/bash
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/root_files"
echo "Execute SigEfficiencyContour.cc"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/root_files

echo "Executing the SigEfficiencyContour macro ... "
#false for ISS training
root -l -b -q Significance3Det.cc

echo "Done!"
