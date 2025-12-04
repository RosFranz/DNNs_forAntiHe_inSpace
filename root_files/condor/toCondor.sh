#!/bin/bash

echo "Clearing previous jobs"
rm condor_out/*.out
rm condor_out/*.err
rm condor_out/*.log

echo "Writing .sh files ..."

#root -l -b 'DataMC_comparison.cc("2025_03_22","2025_03_22","2025_01_27","2025_01_27")'
cat <<EOL > DataMC.sh
#!/bin/bash
echo "moving to /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/root_files"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/root_files
source /cvmfs/sft.cern.ch/lcg/views/LCG_104/x86_64-el9-gcc12-opt/setup.sh
echo "Executing the DataMC comparison macro ... "
root -l -b -q 'DataMC_comparison.cc("2025_04_25","2025_04_22","2025_04_25","2025_04_17")'

echo "Done!"
EOL

#Training without loss weights
#root -l -b 'trainOnMC_mass.cc("2025_03_17", "2025_03_17", "2025_03_23", "2025_03_23")'
#Training with loss weights
#root -l -b 'trainOnMC_mass.cc("2025_03_23", "2025_03_23", "2025_03_23", "2025_03_23")'
#Masked networks
#root -l -b 'trainOnMC_mass.cc("2025_05_08", "2025_05_08", "2025_05_08", "2025_05_08")'
#MaxMinCH (same network structure as the previous masked NNs)
#root -l -b 'trainOnMC_mass.cc("2025_05_11", "2025_05_11", "2025_05_11", "2025_05_11")'
#Masked networks with charges WITH/OUT WEIGHT
#root -l -b 'trainOnMC_mass.cc("2025_05_15", "2025_05_15", "2025_05_15", "2025_05_15")'
cat <<EOL > TrainOnMC.sh
#!/bin/bash
echo "moving to /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/root_files"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/root_files
source /cvmfs/sft.cern.ch/lcg/views/LCG_104/x86_64-el9-gcc12-opt/setup.sh

echo "Executing the trainOnMC macro ... "
root -l -b -q 'trainOnMC_mass.cc("2025_06_30", "2025_06_30", "2025_06_30", "2025_06_30")'

echo "Done!"
EOL


#Masked networks trained on ISS-data
# ISS samples trained on 3 RigLabels bands with same number of events (integrated in rigidity)
#root -l -b 'trainOnMC_mass.cc("2025_05_25", "2025_05_25", "2025_05_25", "2025_05_25")'
cat <<EOL > TrainOnISS.sh
#!/bin/bash
echo "moving to /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/root_files"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/root_files
source /cvmfs/sft.cern.ch/lcg/views/LCG_104/x86_64-el9-gcc12-opt/setup.sh

echo "Executing the trainOnISS macro ... "
root -l -b -q 'trainOnISS_mass.cc("2025_06_30", "2025_06_30", "2025_06_30", "2025_06_30")'
echo "Done!"
EOL


#Masked networks with charges WITH/OUT WEIGHT
#Masked networks trained on ISS-data
# ISS samples trained on 3 RigLabels bands with same number of events (integrated in rigidity)
#root -l -b -q 'mass_inRbins.cc(false,"2025_05_15","2025_05_15","2025_05_15","2025_05_15")'
#Masked networks trained on MC
#root -l -b -q 'mass_inRbins.cc(true,"2025_05_15","2025_05_15","2025_05_15","2025_05_15")'
cat <<EOL > mass_inRbins.sh
#!/bin/bash
echo "moving to /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/root_files"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/root_files
source /cvmfs/sft.cern.ch/lcg/views/LCG_104/x86_64-el9-gcc12-opt/setup.sh

echo "Executing the mass_inRbins macro ... "
#false for ISS training
root -l -b -q 'mass_inRbins.cc(false,"2025_06_30","2025_06_30","2025_06_30","2025_06_30")'
root -l -b -q 'mass_inRbins.cc(true, "2025_06_30","2025_06_30","2025_06_30","2025_06_30")'

echo "Done!"
EOL

#Masked networks
#root -l -q 'SigEfficiencyContour.cc("2025_05_08","2025_05_08","2025_05_08","2025_05_08")'
#MAX MIN ch input features
#root -l -q 'SigEfficiencyContour.cc("2025_05_11","2025_05_11","2025_05_11","2025_05_11")'
#Max Min CH no Loss weights
# root -l -q 'SigEfficiencyContour.cc(true, "2025_05_15","2025_05_15","2025_05_15","2025_05_15")'
#Masked networks trained on ISS-data
# ISS samples trained on 3 RigLabels bands with same number of events (integrated in rigidity)
#root -l -b 'SigEfficiencyContour.cc(false,"2025_05_25","2025_05_25","2025_05_25","2025_05_25")'
cat <<EOL > SigEfficiencyContour.sh
#!/bin/bash
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/root_files"
echo "Execute SigEfficiencyContour.cc"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/root_files

echo "Executing the SigEfficiencyContour macro ... "
#false for ISS training
root -l -q 'SigEfficiencyContour.cc(true, "2025_06_30","2025_06_30","2025_06_30","2025_06_30")'
root -l -q 'SigEfficiencyContour.cc(false,"2025_06_30","2025_06_30","2025_06_30","2025_06_30")'

echo "Done!"
EOL

##################################################
echo "Writing .sub files ..."
cat <<EOL > DataMC.sub
universe = vanilla

executable = DataMC.sh
output     = condor_out/DataMC.out
error      = condor_out/DataMC.err
log        = condor_out/DataMC.log

getenv     = True
+JobFlavour = "longlunch"
RequestCPUs = 5
queue
EOL

cat <<EOL > TrainOnMC.sub
universe = vanilla

executable = TrainOnMC.sh
output     = condor_out/TrainOnMC.out
error      = condor_out/TrainOnMC.err
log        = condor_out/TrainOnMC.log

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL

cat <<EOL > TrainOnISS.sub
universe = vanilla

executable = TrainOnISS.sh
output     = condor_out/TrainOnISS.out
error      = condor_out/TrainOnISS.err
log        = condor_out/TrainOnISS.log

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL

cat <<EOL > mass_inRbins.sub
universe = vanilla

executable = mass_inRbins.sh
output     = condor_out/mass_inRbins.out
error      = condor_out/mass_inRbins.err
log        = condor_out/mass_inRbins.log

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL

cat <<EOL > SigEfficiencyContour.sub
universe = vanilla

executable = SigEfficiencyContour.sh
output     = condor_out/SigEfficiencyContour.out
error      = condor_out/SigEfficiencyContour.err
log        = condor_out/SigEfficiencyContour.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 1
queue
EOL

###############################################
# condor_submit DataMC.sub
condor_submit TrainOnMC.sub
condor_submit TrainOnISS.sub
condor_submit mass_inRbins.sub
condor_submit SigEfficiencyContour.sub
condor_q
