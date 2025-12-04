#!/bin/bash
echo "Clearing previous jobs"
rm ../../condor_out/CL2/log/ranking_ISS*.log
rm ../../condor_out/CL2/err/ranking_ISS*.err
rm ../../condor_out/CL2/out/ranking_ISS*.out

echo "Writing .sh ranking_ISS.sh"
#####
## EXECUTABLE
#####
cat <<EOL > ../../executables/CL2/ranking_ISS.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/FCNNs/ranking"
echo "Execute ranking.py --config yaml/example.yml"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/FCNNs/ranking
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python ranking.py --config ../../yaml/onISS.yml
EOL

#####
## SUBMISSION
#####
echo "Writing .sub ranking_ISS.sub"
cat <<EOL > ../../sub/CL2/ranking_ISS.sub
universe = vanilla

executable = ../../executables/CL2/ranking_ISS.sh
output     = ../../condor_out/CL2/out/ranking_ISS.out
error      = ../../condor_out/CL2/err/ranking_ISS.err
log        = ../../condor_out/CL2/log/ranking_ISS.log

stream_output = true
stream_error = true

getenv     = True
+JobFlavour = "tomorrow"
RequestCPUs = 5
queue
EOL
