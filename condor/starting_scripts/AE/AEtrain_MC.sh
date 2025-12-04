#!/bin/bash
echo "Clearing previous training jobs"
rm ../../condor_out/AE/log/trainMC*.log
rm ../../condor_out/AE/err/trainMC*.err
rm ../../condor_out/AE/out/trainMC*.out


if [ "$1" = "useW" ]; then
    echo "Using MC weights"
    #####
    #==== TRAINING ====
    #####

    ## EXECUTABLE
    echo "Writing .sh executables"
    cat <<EOL > ../../executables/AE/trainMC.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV"
echo "Execute AE_train.py --config yaml/example.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python AE_train.py --config yaml/example.yml
EOL
    
    ## SUBMISSION
    echo "Writing .sub files"
    cat <<EOL > ../../sub/AE/trainMC.sub
universe = vanilla

executable = ../../executables/AE/trainMC.sh
output     = ../../condor_out/AE/out/trainMC.out
error      = ../../condor_out/AE/err/trainMC.err
log        = ../../condor_out/AE/log/trainMC.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL
    
    #####
    #==== VALIDATION - TEST ====
    #####
    echo "Clearing previous training jobs"
    rm ../../condor_out/AE/log/ValTestMC*.log
    rm ../../condor_out/AE/err/ValTestMC*.err
    rm ../../condor_out/AE/out/ValTestMC*.out
    
    ## EXECUTABLE
    echo "Writing .sh executables"
    cat <<EOL > ../../executables/AE/ValTestMC.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV"
echo "Execute AE_val.py --config yaml/example.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python AE_val.py --config yaml/example.yml
EOL
    
    ## SUBMISSION
    echo "Writing .sub files"
    cat <<EOL > ../../sub/AE/ValTestMC.sub
universe = vanilla

executable = ../../executables/AE/ValTestMC.sh
output     = ../../condor_out/AE/out/ValTestMC.out
error      = ../../condor_out/AE/err/ValTestMC.err
log        = ../../condor_out/AE/log/ValTestMC.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL
    
    #condor_submit ../../sub/AE/trainMC.sub
    #condor_submit ../../sub/AE/ValTestMC.sub

else
    echo "Not using MC weights"
    #####
    #==== TRAINING ====
    #####

    ## EXECUTABLE
    echo "Writing .sh executables"
    cat <<EOL > ../../executables/AE/trainMC_noW.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV"
echo "Execute AE_train.py --config yaml/no_LossW.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python AE_train.py --config yaml/no_LossW.yml
EOL

    ## SUBMISSION
    echo "Writing .sub files"
    cat <<EOL > ../../sub/AE/trainMC_noW.sub
universe = vanilla

executable = ../../executables/AE/trainMC_noW.sh
output     = ../../condor_out/AE/out/trainMC_noW.out
error      = ../../condor_out/AE/err/trainMC_noW.err
log        = ../../condor_out/AE/log/trainMC_noW.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL
    
    #####
    #==== VALIDATION - TEST ====
    
    echo "Writing .sh executables"
    cat <<EOL > ../../executables/AE/ValTestMC_noW.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV"
echo "Execute AE_val.py --config yaml/no_LossW.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python AE_val.py --config yaml/no_LossW.yml
EOL

    ## SUBMISSION
    echo "Writing .sub files"
    cat <<EOL > ../../sub/AE/ValTestMC_noW.sub
universe = vanilla

executable = ../../executables/AE/ValTestMC_noW.sh
output     = ../../condor_out/AE/out/ValTestMC_noW.out
error      = ../../condor_out/AE/err/ValTestMC_noW.err
log        = ../../condor_out/AE/log/ValTestMC_noW.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL
    #condor_submit ../../sub/AE/trainMC_noW.sub
    #condor_submit ../../sub/AE/ValTestMC_noW.sub
fi



