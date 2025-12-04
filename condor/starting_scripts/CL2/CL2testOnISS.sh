#!/bin/bash
if [ "$1" = "useW" ]; then
    echo "Writing .sh testOnISS1 - ISS2 - ISS3"
    #####
    ## EXECUTABLE
    #####
    cat <<EOL > ../../executables/CL2/testOnISS1.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/"
echo "Execute CL2_onISS1.py --config yaml/example.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python CL2_onISS1.py --config yaml/example.yml
EOL
    cat <<EOL > ../../executables/CL2/testOnISS2.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/"
echo "Execute CL2_onISS2.py --config yaml/example.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python CL2_onISS2.py --config yaml/example.yml
EOL
    cat <<EOL > ../../executables/CL2/testOnISS3.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/"
echo "Execute CL2_onISS3.py --config yaml/example.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python CL2_onISS3.py --config yaml/example.yml
EOL
    #####
    ## SUBMISSION
    #####
    echo "Writing .sub testOnISS1 - ISS2 - ISS3"
    cat <<EOL > ../../sub/CL2/testOnISS1.sub
universe = vanilla

executable = ../../executables/CL2/testOnISS1.sh
output     = ../../condor_out/CL2/out/testOnISS1.out
error      = ../../condor_out/CL2/err/testOnISS1.err
log        = ../../condor_out/CL2/log/testOnISS1.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL
    cat <<EOL > ../../sub/CL2/testOnISS2.sub
universe = vanilla

executable = ../../executables/CL2/testOnISS2.sh
output     = ../../condor_out/CL2/out/testOnISS2.out
error      = ../../condor_out/CL2/err/testOnISS2.err
log        = ../../condor_out/CL2/log/testOnISS2.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL
    cat <<EOL > ../../sub/CL2/testOnISS3.sub
universe = vanilla

executable = ../../executables/CL2/testOnISS3.sh
output     = ../../condor_out/CL2/out/testOnISS3.out
error      = ../../condor_out/CL2/err/testOnISS3.err
log        = ../../condor_out/CL2/log/testOnISS3.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL

else
    echo "Writing .sh testOnISS1_noW - ISS2_noW - ISS3_noW"
    #####
    ## EXECUTABLE
    #####
cat <<EOL > ../../executables/CL2/testOnISS1_noW.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/"
echo "Execute CL2_onISS1.py --config yaml/no_LossW.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python CL2_onISS1.py --config yaml/no_LossW.yml
EOL
cat <<EOL > ../../executables/CL2/testOnISS2_noW.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/"
echo "Execute CL2_onISS2.py --config yaml/no_LossW.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python CL2_onISS2.py --config yaml/no_LossW.yml
EOL
cat <<EOL > ../../executables/CL2/testOnISS3_noW.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/"
echo "Execute CL2_onISS3.py --config yaml/no_LossW.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python CL2_onISS3.py --config yaml/no_LossW.yml
EOL
    #####
    ## SUBMISSION
    #####
    echo "Writing .sub testOnISS1_noW - ISS2_noW - ISS3_noW"
    cat <<EOL > ../../sub/CL2/testOnISS1_noW.sub
universe = vanilla

executable = ../../executables/CL2/testOnISS1_noW.sh
output     = ../../condor_out/CL2/out/testOnISS1_noW.out
error      = ../../condor_out/CL2/err/testOnISS1_noW.err
log        = ../../condor_out/CL2/log/testOnISS1_noW.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL
    cat <<EOL > ../../sub/CL2/testOnISS2_noW.sub
universe = vanilla

executable = ../../executables/CL2/testOnISS2_noW.sh
output     = ../../condor_out/CL2/out/testOnISS2_noW.out
error      = ../../condor_out/CL2/err/testOnISS2_noW.err
log        = ../../condor_out/CL2/log/testOnISS2_noW.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL
    cat <<EOL > ../../sub/CL2/testOnISS3_noW.sub
universe = vanilla

executable = ../../executables/CL2/testOnISS3_noW.sh
output     = ../../condor_out/CL2/out/testOnISS3_noW.out
error      = ../../condor_out/CL2/err/testOnISS3_noW.err
log        = ../../condor_out/CL2/log/testOnISS3_noW.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL

fi

./testOnNeg_fromMCtraining.sh "$@"
./testOnMC_custom.sh "$@"
#condor_submit ../../sub/CL2/testOnISS1.sub
#condor_submit ../../sub/CL2/testOnISS1_noW.sub
#condor_submit ../../sub/CL2/testOnISS2.sub
#condor_submit ../../sub/CL2/testOnISS2_noW.sub
#condor_submit ../../sub/CL2/testOnISS3.sub
#condor_submit ../../sub/CL2/testOnISS3_noW.sub
#condor_q
