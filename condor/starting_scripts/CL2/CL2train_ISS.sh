#!/bin/bash
echo "Clearing previous jobs"
rm ../../condor_out/CL2/log/train_ISS*.log
rm ../../condor_out/CL2/err/train_ISS*.err
rm ../../condor_out/CL2/out/train_ISS*.out

#####
## EXECUTABLE
#####
cat <<EOL > ../../executables/CL2/train_ISS.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV"
echo "Execute CL2_train.py --config yaml/onISS.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python CL2_train.py --config yaml/onISS.yml
EOL

###################################
## SUBMISSION
###################################
cat <<EOL > ../../sub/CL2/trainISS.sub
universe = vanilla

executable = ../../executables/CL2/train_ISS.sh
output     = ../../condor_out/CL2/out/train_ISS.out
error      = ../../condor_out/CL2/err/train_ISS.err
log        = ../../condor_out/CL2/log/train_ISS.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL

#condor_submit ../../sub/CL2/train_ISS.sub
#condor_q

