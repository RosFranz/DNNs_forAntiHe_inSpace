#!/bin/bash
#####
## EXECUTABLES
#####
cat <<EOL > ../../executables/CL2/testOnMC1.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/"
echo "Execute CL2_onMC1.py --config yaml/onISS.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python CL2_onMC1.py --config yaml/onISS.yml
EOL

cat <<EOL > ../../executables/CL2/testOnMC2.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/"
echo "Execute CL2_onMC2.py --config yaml/onISS.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python CL2_onMC2.py --config yaml/onISS.yml
EOL

cat <<EOL > ../../executables/CL2/testOnMC3.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/"
echo "Execute CL2_onMC3.py --config yaml/onISS.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python CL2_onMC3.py --config yaml/onISS.yml
EOL


###################################
## SUBMISSION
###################################
cat <<EOL > ../../sub/CL2/testOnMC1.sub
universe = vanilla

executable = ../../executables/CL2/testOnMC1.sh
output     = ../../condor_out/CL2/out/testOnMC1.out
error      = ../../condor_out/CL2/err/testOnMC1.err
log        = ../../condor_out/CL2/log/testOnMC1.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL

cat <<EOL > ../../sub/CL2/testOnMC2.sub
universe = vanilla

executable = ../../executables/CL2/testOnMC2.sh
output     = ../../condor_out/CL2/out/testOnMC2.out
error      = ../../condor_out/CL2/err/testOnMC2.err
log        = ../../condor_out/CL2/log/testOnMC2.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL

cat <<EOL > ../../sub/CL2/testOnMC3.sub
universe = vanilla

executable = ../../executables/CL2/testOnMC3.sh
output     = ../../condor_out/CL2/out/testOnMC3.out
error      = ../../condor_out/CL2/err/testOnMC3.err
log        = ../../condor_out/CL2/log/testOnMC3.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL

./testOnNeg_fromISStraining.sh "$@"
#condor_submit ../../sub/CL2/testOnMC1.sub
#condor_submit ../../sub/CL2/testOnMC2.sub
#condor_submit ../../sub/CL2/testOnMC3.sub
#condor_q

