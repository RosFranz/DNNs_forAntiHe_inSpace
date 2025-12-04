#!/bin/bash
echo "Clearing previous jobs"
rm ../../condor_out/AE/log/rankingISS*.log
rm ../../condor_out/AE/err/rankingISS*.err
rm ../../condor_out/AE/out/rankingISS*.out

echo "Writing .sh rankingISS.sh"
#####
## EXECUTABLE
#####
cat <<EOL > ../../executables/AE/rankingISS.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/AE/ranking"
echo "Execute ranking.py --config yaml/example.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/AE/ranking
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python ranking.py --config ../../yaml/onISS.yml
EOL
#####
## SUBMISSION
#####
    echo "Writing .sub ranking_ISS.sub"
    cat <<EOL > ../../sub/AE/ranking_ISS.sub
universe = vanilla

executable = ../../executables/AE/rankingISS.sh
output     = ../../condor_out/AE/out/rankingISS.out
error      = ../../condor_out/AE/err/rankingISS.err
log        = ../../condor_out/AE/log/rankingISS.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL


#condor_submit ../../sub/AE/rankingISS.sub
# condor_submit ../../sub/AE/rankingISS_noW.sub
#condor_q
