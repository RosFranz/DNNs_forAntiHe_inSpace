#!/bin/bash

if [ "$1" = "useW" ]; then
    echo "Writing .sh Neg1-Neg2-Neg3"
    #####
    ## EXECUTABLE
    #####
    cat <<EOL > ../../executables/CL2/testOnNeg1.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/FCNNs"
echo "Execute CL2_onNeg1.py --config yaml/example.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python CL2_onNeg1.py --config yaml/example.yml
EOL
    cat <<EOL > ../../executables/CL2/testOnNeg2.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/FCNNs"
echo "Execute CL2_onNeg2.py --config yaml/example.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python CL2_onNeg2.py --config yaml/example.yml
EOL
    cat <<EOL > ../../executables/CL2/testOnNeg3.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/FCNNs"
echo "Execute CL2_onNeg3.py --config yaml/example.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python CL2_onNeg3.py --config yaml/example.yml
EOL
    echo "Writing .sub Neg1-Neg2-Neg3"
    #####
    ## SUBMISSION
    #####
    cat <<EOL > ../../sub/CL2/testOnNeg1.sub
universe = vanilla

executable = ../../executables/CL2/testOnNeg1.sh
output     = ../../condor_out/CL2/out/testOnNeg1.out
error      = ../../condor_out/CL2/err/testOnNeg1.err
log        = ../../condor_out/CL2/log/testOnNeg1.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL
    cat <<EOL > ../../sub/CL2/testOnNeg2.sub
universe = vanilla

executable = ../../executables/CL2/testOnNeg2.sh
output     = ../../condor_out/CL2/out/testOnNeg2.out
error      = ../../condor_out/CL2/err/testOnNeg2.err
log        = ../../condor_out/CL2/log/testOnNeg2.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL
    cat <<EOL > ../../sub/CL2/testOnNeg3.sub
universe = vanilla

executable = ../../executables/CL2/testOnNeg3.sh
output     = ../../condor_out/CL2/out/testOnNeg3.out
error      = ../../condor_out/CL2/err/testOnNeg3.err
log        = ../../condor_out/CL2/log/testOnNeg3.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL


else
    echo "Writing .sh Neg1_noW-Neg2_noW-Neg3_noW"
    #####
    ## EXECUTABLE
    #####
    cat <<EOL > ../../executables/CL2/testOnNeg1_noW.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/FCNNs"
echo "Execute CL2_onNeg1.py --config yaml/no_LossW.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python CL2_onNeg1.py --config yaml/no_LossW.yml
EOL
    cat <<EOL > ../../executables/CL2/testOnNeg2_noW.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/FCNNs"
echo "Execute CL2_onNeg2.py --config yaml/no_LossW.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python CL2_onNeg2.py --config yaml/no_LossW.yml
EOL
    cat <<EOL > ../../executables/CL2/testOnNeg3_noW.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/FCNNs"
echo "Execute CL2_onNeg3.py --config yaml/no_LossW.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python CL2_onNeg3.py --config yaml/no_LossW.yml
EOL

    echo "Writing .sub Neg1_noW-Neg2_noW-Neg3_noW"
    #####
    ## SUBMISSION
    #####
    cat <<EOL > ../../sub/CL2/testOnNeg1_noW.sub
universe = vanilla

executable = ../../executables/CL2/testOnNeg1_noW.sh
output     = ../../condor_out/CL2/out/testOnNeg1_noW.out
error      = ../../condor_out/CL2/err/testOnNeg1_noW.err
log        = ../../condor_out/CL2/log/testOnNeg1_noW.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL
    cat <<EOL > ../../sub/CL2/testOnNeg2_noW.sub
universe = vanilla

executable = ../../executables/CL2/testOnNeg2_noW.sh
output     = ../../condor_out/CL2/out/testOnNeg2_noW.out
error      = ../../condor_out/CL2/err/testOnNeg2_noW.err
log        = ../../condor_out/CL2/log/testOnNeg2_noW.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL
    cat <<EOL > ../../sub/CL2/testOnNeg3_noW.sub
universe = vanilla

executable = ../../executables/CL2/testOnNeg3_noW.sh
output     = ../../condor_out/CL2/out/testOnNeg3_noW.out
error      = ../../condor_out/CL2/err/testOnNeg3_noW.err
log        = ../../condor_out/CL2/log/testOnNeg3_noW.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL
fi


#condor_submit ../../sub/CL2/testOnNeg1.sub
#condor_submit ../../sub/CL2/testOnNeg1_noW.sub
#condor_submit ../../sub/CL2/testOnNeg2.sub
#condor_submit ../../sub/CL2/testOnNeg2_noW.sub
#condor_submit ../../sub/CL2/testOnNeg3.sub
#condor_submit ../../sub/CL2/testOnNeg3_noW.sub
#condor_q
