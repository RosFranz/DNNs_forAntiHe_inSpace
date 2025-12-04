#!/bin/bash
#####
## EXECUTABLE 
#####
cat <<EOL > ../../executables/CL2/testOnNeg_fromISS1.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/FCNNs"
echo "Execute CL2_onNeg1.py --config yaml/onISS.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python CL2_onNeg1.py --config yaml/onISS.yml
EOL

cat <<EOL > ../../executables/CL2/testOnNeg_fromISS2.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/FCNNs"
echo "Execute CL2_onNeg2.py --config yaml/onISS.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python CL2_onNeg2.py --config yaml/onISS.yml
EOL

cat <<EOL > ../../executables/CL2/testOnNeg_fromISS3.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/FCNNs"
echo "Execute CL2_onNeg3.py --config yaml/onISS.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python CL2_onNeg3.py --config yaml/onISS.yml
EOL

###################################
## SUBMISSION
###################################
cat <<EOL > ../../sub/CL2/testOnNeg_fromISS1.sub
universe = vanilla

executable = ../../executables/CL2/testOnNeg_fromISS1.sh
output     = ../../condor_out/CL2/out/testOnNeg_fromISS1.out
error      = ../../condor_out/CL2/err/testOnNeg_fromISS1.err
log        = ../../condor_out/CL2/log/testOnNeg_fromISS1.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL

cat <<EOL > ../../sub/CL2/testOnNeg_fromISS2.sub
universe = vanilla

executable = ../../executables/CL2/testOnNeg_fromISS2.sh
output     = ../../condor_out/CL2/out/testOnNeg_fromISS2.out
error      = ../../condor_out/CL2/err/testOnNeg_fromISS2.err
log        = ../../condor_out/CL2/log/testOnNeg_fromISS2.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL

cat <<EOL > ../../sub/CL2/testOnNeg_fromISS3.sub
universe = vanilla

executable = ../../executables/CL2/testOnNeg_fromISS3.sh
output     = ../../condor_out/CL2/out/testOnNeg_fromISS3.out
error      = ../../condor_out/CL2/err/testOnNeg_fromISS3.err
log        = ../../condor_out/CL2/log/testOnNeg_fromISS3.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL

#condor_submit ../../sub/CL2/testOnNeg_fromISS1.sub
#condor_submit ../../sub/CL2/testOnNeg_fromISS2.sub
#condor_submit ../../sub/CL2/testOnNeg_fromISS3.sub
#condor_q

