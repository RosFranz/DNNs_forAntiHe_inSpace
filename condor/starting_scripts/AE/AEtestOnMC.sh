#!/bin/bash

echo "Clearing previous jobs"
rm ../../condor_out/AE/log/testOnMC*.log
rm ../../condor_out/AE/err/testOnMC*.err
rm ../../condor_out/AE/out/testOnMC*.out

#####
## EXECUTABLES 
#####
cat <<EOL > ../../executables/AE/testOnMC1.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV"
echo "Execute AE_test1.py --config yaml/onISS.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python AE_test1.py --config yaml/onISS.yml
EOL

cat <<EOL > ../../executables/AE/testOnMC2.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV"
echo "Execute AE_test2.py --config yaml/onISS.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python AE_test2.py --config yaml/onISS.yml
EOL


#########################
## SUBMISSIONS
#########################
cat <<EOL > ../../sub/AE/testOnMC1.sub
universe = vanilla

executable = ../../executables/AE/testOnMC1.sh
output     = ../../condor_out/AE/out/testOnMC1.out
error      = ../../condor_out/AE/err/testOnMC1.err
log        = ../../condor_out/AE/log/testOnMC1.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL



cat <<EOL > ../../sub/AE/testOnMC2.sub
universe = vanilla

executable = ../../executables/AE/testOnMC2.sh
output     = ../../condor_out/AE/out/testOnMC2.out
error      = ../../condor_out/AE/err/testOnMC2.err
log        = ../../condor_out/AE/log/testOnMC2.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL


#condor_submit ../../sub/AE/testOnMC1.sub
#condor_submit ../../sub/AE/testOnMC2.sub
#condor_q
