#!/bin/bash

if [ "$1" = "useW" ]; then
    echo "Writing .sh antiHe3-El-He3"
    #####
    ## EXECUTABLES - ANTIHE
    #####
    cat <<EOL > ../../executables/CL2/testOnAntiHe.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV"
echo "Execute CL2_testCustom.py --config yaml/antiHe.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python CL2_testCustom.py --config yaml/antiHe.yml
EOL
    #####
    ## EXECUTABLES - ELECTRONS
    #####
    cat <<EOL > ../../executables/CL2/testOnElectrons.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV"
echo "Execute CL2_testCustom.py --config yaml/electron.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python CL2_testCustom.py --config yaml/electron.yml
EOL
    #####
    ## EXECUTABLES - He3
    #####
    cat <<EOL > ../../executables/CL2/testOnHe3.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV"
echo "Execute CL2_testCustom.py --config yaml/He3.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python CL2_testCustom.py --config yaml/He3.yml
EOL


    echo "Writing .sub antiHe3-El-He3"
    #########################
    ## SUBMISSIONS - ANTIHe
    #########################
    cat <<EOL > ../../sub/CL2/testOnAntiHe.sub
universe = vanilla

executable = ../../executables/CL2/testOnAntiHe.sh
output     = ../../condor_out/CL2/out/testOnAntiHe.out
error      = ../../condor_out/CL2/err/testOnAntiHe.err
log        = ../../condor_out/CL2/log/testOnAntiHe.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL
    #########################
    ## SUBMISSIONS - Electrons
    #########################
    cat <<EOL > ../../sub/CL2/testOnElectrons.sub
universe = vanilla

executable = ../../executables/CL2/testOnElectrons.sh
output     = ../../condor_out/CL2/out/testOnElectrons.out
error      = ../../condor_out/CL2/err/testOnElectrons.err
log        = ../../condor_out/CL2/log/testOnElectrons.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL
    #########################
    ## SUBMISSIONS - He3
    #########################
    cat <<EOL > ../../sub/CL2/testOnHe3.sub
universe = vanilla

executable = ../../executables/CL2/testOnHe3.sh
output     = ../../condor_out/CL2/out/testOnHe3.out
error      = ../../condor_out/CL2/err/testOnHe3.err
log        = ../../condor_out/CL2/log/testOnHe3.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL




else
    echo "Writing .sh antiHe3_noLossW-El_noLossW-He3_noLossW"
    #####
    ## EXECUTABLES - ANTIHE
    #####
    cat <<EOL > ../../executables/CL2/testOnAntiHe_noLossW.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV"
echo "Execute CL2_testCustom.py --config yaml/antiHe_noLossW.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python CL2_testCustom.py --config yaml/antiHe_noLossW.yml
EOL
    #####
    ## EXECUTABLES - ELECTRONS
    #####
    cat <<EOL > ../../executables/CL2/testOnElectrons_noLossW.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV"
echo "Execute CL2_testCustom.py --config yaml/electron_noLossW.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python CL2_testCustom.py --config yaml/electron_noLossW.yml
EOL
    #####
    ## EXECUTABLES - He3
    #####
    cat <<EOL > ../../executables/CL2/testOnHe3_noLossW.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV"
echo "Execute CL2_testCustom.py --config yaml/He3_noLossW.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python CL2_testCustom.py --config yaml/He3_noLossW.yml
EOL

    echo "Writing .sub antiHe3_noLossW-El_noLossW-He3_noLossW"
    #########################
    ## SUBMISSIONS - ANTIHe
    #########################
cat <<EOL > ../../sub/CL2/testOnAntiHe_noLossW.sub
universe = vanilla

executable = ../../executables/CL2/testOnAntiHe_noLossW.sh
output     = ../../condor_out/CL2/out/testOnAntiHe_noLossW.out
error      = ../../condor_out/CL2/err/testOnAntiHe_noLossW.err
log        = ../../condor_out/CL2/log/testOnAntiHe_noLossW.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL
    #########################
    ## SUBMISSIONS - Electrons
    #########################
    cat <<EOL > ../../sub/CL2/testOnElectrons_noLossW.sub
universe = vanilla

executable = ../../executables/CL2/testOnElectrons_noLossW.sh
output     = ../../condor_out/CL2/out/testOnElectrons_noLossW.out
error      = ../../condor_out/CL2/err/testOnElectrons_noLossW.err
log        = ../../condor_out/CL2/log/testOnElectrons_noLossW.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL
    #########################
    ## SUBMISSIONS - He3
    #########################
    cat <<EOL > ../../sub/CL2/testOnHe3_noLossW.sub
universe = vanilla

executable = ../../executables/CL2/testOnHe3_noLossW.sh
output     = ../../condor_out/CL2/out/testOnHe3_noLossW.out
error      = ../../condor_out/CL2/err/testOnHe3_noLossW.err
log        = ../../condor_out/CL2/log/testOnHe3_noLossW.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL

fi