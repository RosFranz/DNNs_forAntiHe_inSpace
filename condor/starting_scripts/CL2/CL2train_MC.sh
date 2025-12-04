#!/bin/bash
echo "Clearing previous jobs"
rm ../../condor_out/CL2/log/train*.log
rm ../../condor_out/CL2/err/train*.err
rm ../../condor_out/CL2/out/train*.out

if [ "$1" = "useW" ]; then
    echo "Writing .sh trainMC.sh"
    #####
    ## EXECUTABLE
    #####
    cat <<EOL > ../../executables/CL2/trainMC.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV"
echo "Execute CL2_train.py --config yaml/example.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python CL2_train.py --config yaml/example.yml
EOL
    #####
    ## SUBMISSION
    #####
    echo "Writing .sub trainMC.sub"
    cat <<EOL > ../../sub/CL2/trainMC.sub
universe = vanilla

executable = ../../executables/CL2/trainMC.sh
output     = ../../condor_out/CL2/out/trainMC.out
error      = ../../condor_out/CL2/err/trainMC.err
log        = ../../condor_out/CL2/log/trainMC.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL

else
    echo "Writing .sh trainMC_noW.sh"
    #####
    ## EXECUTABLE
    #####
    cat <<EOL > ../../executables/CL2/trainMC_noW.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV"
echo "Execute CL2_train.py --config yaml/no_LossW.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python CL2_train.py --config yaml/no_LossW.yml
EOL

    #####
    ## SUBMISSION
    #####
    echo "Writing .sub trainMC_noW.sub"
    cat <<EOL > ../../sub/CL2/trainMC_noW.sub
universe = vanilla

executable = ../../executables/CL2/trainMC_noW.sh
output     = ../../condor_out/CL2/out/trainMC_noW.out
error      = ../../condor_out/CL2/err/trainMC_noW.err
log        = ../../condor_out/CL2/log/trainMC_noW.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL
fi
#condor_submit ../../sub/CL2/train.sub
#condor_submit ../../sub/CL2/train_noW.sub
#condor_q
