#!/bin/bash
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/root_files"
echo "Execute SigEfficiencyContour.cc"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/root_files

echo "Executing the SigEfficiencyContour macro ... "
#false for ISS training
root -l -q 'SigEfficiencyContour.cc(true, "2025_06_30","2025_06_30","2025_06_30","2025_06_30")'
root -l -q 'SigEfficiencyContour.cc(false,"2025_06_30","2025_06_30","2025_06_30","2025_06_30")'

echo "Done!"
