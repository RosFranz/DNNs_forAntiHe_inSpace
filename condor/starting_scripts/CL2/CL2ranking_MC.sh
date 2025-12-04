#!/bin/bash
echo "Clearing previous jobs"
rm ../../condor_out/CL2/log/rankingMC*.log
rm ../../condor_out/CL2/err/rankingMC*.err
rm ../../condor_out/CL2/out/rankingMC*.out

if [ "$1" = "useW" ]; then
    echo "Writing .sh rankingMC.sh"
    #####
    ## EXECUTABLE
    #####
    cat <<EOL > ../../executables/CL2/rankingMC.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/FCNNs/ranking"
echo "Execute ranking.py --config yaml/example.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/FCNNs/ranking
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python ranking.py --config ../../yaml/example.yml
EOL

    #####
    ## SUBMISSION
    #####
    echo "Writing .sub rankingMC.sub"
    cat <<EOL > ../../sub/CL2/rankingMC.sub
universe = vanilla

executable = ../../executables/CL2/rankingMC.sh
output     = ../../condor_out/CL2/out/rankingMC.out
error      = ../../condor_out/CL2/err/rankingMC.err
log        = ../../condor_out/CL2/log/rankingMC.log

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
    echo "Writing .sh rankingMC_noW.sh"
    cat <<EOL > ../../executables/CL2/rankingMC_noW.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/FCNNs/ranking"
echo "Execute ranking.py --config yaml/no_LossW.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/FCNNs/ranking
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python ranking.py --config ../../yaml/no_LossW.yml
EOL
    #####
    ## SUBMISSION
    #####
    echo "Writing .sub rankingMC_noW.sub"
    cat <<EOL > ../../sub/CL2/rankingMC_noW.sub
universe = vanilla

executable = ../../executables/CL2/rankingMC_noW.sh
output     = ../../condor_out/CL2/out/rankingMC_noW.out
error      = ../../condor_out/CL2/err/rankingMC_noW.err
log        = ../../condor_out/CL2/log/rankingMC_noW.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL

fi

#condor_submit ../../sub/CL2/ranking.sub
#condor_submit ../../sub/CL2/ranking_noW.sub
#condor_q

