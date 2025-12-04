#!/bin/bash

#echo "Clearing previous jobs"
rm ../../condor_out/AE/log/testOnISS*.log
rm ../../condor_out/AE/err/testOnISS*.err
rm ../../condor_out/AE/out/testOnISS*.out

if [ "$1" = "useW" ]; then
    #####
    ## EXECUTABLE
    #####
    cat <<EOL > ../../executables/AE/testOnISS1.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV"
echo "Execute AE_test1.py --config yaml/example.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python AE_test1.py --config yaml/example.yml
EOL

    cat <<EOL > ../../executables/AE/testOnISS2.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV"
echo "Execute AE_test2.py --config yaml/example.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python AE_test2.py --config yaml/example.yml
EOL

    #####
    ## SUBMISSION
    #####
    cat <<EOL > ../../sub/AE/testOnISS1.sub
universe = vanilla

executable = ../../executables/AE/testOnISS1.sh
output     = ../../condor_out/AE/out/testOnISS1.out
error      = ../../condor_out/AE/err/testOnISS1.err
log        = ../../condor_out/AE/log/testOnISS1.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL

    cat <<EOL > ../../sub/AE/testOnISS2.sub
universe = vanilla

executable = ../../executables/AE/testOnISS2.sh
output     = ../../condor_out/AE/out/testOnISS2.out
error      = ../../condor_out/AE/err/testOnISS2.err
log        = ../../condor_out/AE/log/testOnISS2.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL


else
    #####
    ## EXECUTABLE
    #####
    cat <<EOL > ../../executables/AE/testOnISS1_noW.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV"
echo "Execute AE_test1.py --config yaml/no_LossW.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python AE_test1.py --config yaml/no_LossW.yml
EOL
    cat <<EOL > ../../executables/AE/testOnISS2_noW.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV"
echo "Execute AE_test2.py --config yaml/no_LossW.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python AE_test2.py --config yaml/no_LossW.yml
EOL
    #####
    ## SUBMISSION
    #####
    cat <<EOL > ../../sub/AE/testOnISS1_noW.sub
universe = vanilla

executable = ../../executables/AE/testOnISS1_noW.sh
output     = ../../condor_out/AE/out/testOnISS1_noW.out
error      = ../../condor_out/AE/err/testOnISS1_noW.err
log        = ../../condor_out/AE/log/testOnISS1_noW.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL

    cat <<EOL > ../../sub/AE/testOnISS2_noW.sub
universe = vanilla

executable = ../../executables/AE/testOnISS2_noW.sh
output     = ../../condor_out/AE/out/testOnISS2_noW.out
error      = ../../condor_out/AE/err/testOnISS2_noW.err
log        = ../../condor_out/AE/log/testOnISS2_noW.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL

fi

./AEtestOnMC_custom.sh "$@"

#condor_submit ../../sub/AE/testOnISS1.sub
#condor_submit ../../sub/AE/testOnISS1_noW.sub
#condor_submit ../../sub/AE/testOnISS2.sub
#condor_submit ../../sub/AE/testOnISS2_noW.sub
#condor_q
