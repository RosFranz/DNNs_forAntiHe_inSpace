#!/bin/bash

##
#==== TRAINING ====
##
echo "Clearing previous training jobs"
rm ../../condor_out/AE/log/train_ISS*.log
rm ../../condor_out/AE/err/train_ISS*.err
rm ../../condor_out/AE/out/train_ISS*.out

## EXECUTABLES

cat <<EOL > ../../executables/AE/train_ISS.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV"
echo "Execute AE_train.py --config yaml/onISS.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python AE_train.py --config yaml/onISS.yml
EOL

## SUBMISSION

cat <<EOL > ../../sub/AE/train_ISS.sub
universe = vanilla

executable = ../../executables/AE/train_ISS.sh
output     = ../../condor_out/AE/out/train_ISS.out
error      = ../../condor_out/AE/err/train_ISS.err
log        = ../../condor_out/AE/log/train_ISS.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL

#condor_submit ../../sub/AE/train_ISS.sub
#condor_q

##
#==== VALIDATION AND TESTING ====
##
echo "Clearing previous validation jobs"
rm ../../condor_out/AE/log/ValTest_ISS*.log
rm ../../condor_out/AE/err/ValTest_ISS*.err
rm ../../condor_out/AE/out/ValTest_ISS*.out

## EXECUTABLES

cat <<EOL > ../../executables/AE/ValTest_ISS.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV"
echo "Execute AE_val.py --config yaml/onISS.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python AE_val.py --config yaml/onISS.yml
EOL


## SUBMISSION

cat <<EOL > ../../sub/AE/ValTest_ISS.sub
universe = vanilla

executable = ../../executables/AE/ValTest_ISS.sh
output     = ../../condor_out/AE/out/ValTest_ISS.out
error      = ../../condor_out/AE/err/ValTest_ISS.err
log        = ../../condor_out/AE/log/ValTest_ISS.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL

#condor_submit ../../sub/AE/ValTest_ISS.sub
#condor_q
