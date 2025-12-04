#!/bin/bash

echo "Clearing previous jobs:"
rm ../../condor_out/preprocessing/log/*.log
rm ../../condor_out/preprocessing/err/*.err
rm ../../condor_out/preprocessing/out/*.out


#ANTIHE
cat <<EOL > ../../executables/preprocessing/MakeAntiHe3_h5.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/preprocessing" 
echo "Execute TreeToH5.py"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/preprocessing
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python TreeToH5.py --config ../yaml/preprocessing/AntiHe.yml
EOL
cat <<EOL > ../../sub/preprocessing/AntiHe3.sub
universe = vanilla

executable = ../../executables/preprocessing/MakeAntiHe3_h5.sh
output     = ../../condor_out/preprocessing/out/AntiHe3.out
error      = ../../condor_out/preprocessing/err/AntiHe3.err
log        = ../../condor_out/preprocessing/log/AntiHe3.log

getenv     = True

+JobFlavour = "workday"
RequestCPUs = 5
queue
EOL

#ELECTRONS
cat <<EOL > ../../executables/preprocessing/MakeElectrons_h5.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/preprocessing" 
echo "Execute TreeToH5.py"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/preprocessing
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python TreeToH5.py --config ../yaml/preprocessing/Electrons.yml
EOL
cat <<EOL > ../../sub/preprocessing/Electrons.sub
universe = vanilla

executable = ../../executables/preprocessing/MakeElectrons_h5.sh
output     = ../../condor_out/preprocessing/out/Electrons.out
error      = ../../condor_out/preprocessing/err/Electrons.err
log        = ../../condor_out/preprocessing/log/Electrons.log

getenv     = True

+JobFlavour = "workday"
RequestCPUs = 5
queue
EOL



#He3
cat <<EOL > ../../executables/preprocessing/MakeHe321000h5.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/preprocessing" 
echo "Execute TreeToH5.py"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/preprocessing
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python TreeToH5.py --config ../yaml/preprocessing/He3.yml
EOL
cat <<EOL > ../../sub/preprocessing/He3.sub
universe = vanilla

executable = ../../executables/preprocessing/MakeHe321000h5.sh
output     = ../../condor_out/preprocessing/out/He3.out
error      = ../../condor_out/preprocessing/err/He3.err
log        = ../../condor_out/preprocessing/log/He3.log

getenv     = True

+JobFlavour = "workday"
RequestCPUs = 5
queue
EOL
    #CL specific
cat <<EOL > ../../executables/preprocessing/MakeHe3CL21000h5.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/preprocessing" 
echo "Execute TreeToH5.py"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/preprocessing
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python TreeToH5.py --config ../yaml/preprocessing/He3CL.yml
EOL
cat <<EOL > ../../sub/preprocessing/He3CL.sub
universe = vanilla

executable = ../../executables/preprocessing/MakeHe3CL21000h5.sh
output     = ../../condor_out/preprocessing/out/He3CL.out
error      = ../../condor_out/preprocessing/err/He3CL.err
log        = ../../condor_out/preprocessing/log/He3CL.log

getenv     = True

+JobFlavour = "workday"
RequestCPUs = 5
queue
EOL

#CC He3
cat <<EOL > ../../executables/preprocessing/MakeHe3CC21000h5.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/preprocessing" 
echo "Execute TreeToH5.py"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/preprocessing
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python TreeToH5.py --config ../yaml/preprocessing/He3cc.yml
EOL
cat <<EOL > ../../sub/preprocessing/He3cc.sub
universe = vanilla

executable = ../../executables/preprocessing/MakeHe3CC21000h5.sh
output     = ../../condor_out/preprocessing/out/He3cc.out
error      = ../../condor_out/preprocessing/err/He3cc.err
log        = ../../condor_out/preprocessing/log/He3cc.log

getenv     = True

+JobFlavour = "workday"
RequestCPUs = 5
queue
EOL



#CC He4
cat <<EOL > ../../executables/preprocessing/MakeHe4CCh5.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/preprocessing" 
echo "Execute TreeToH5.py"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/preprocessing
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python TreeToH5.py --config ../yaml/preprocessing/He4cc.yml
EOL
cat <<EOL > ../../sub/preprocessing/He4CC.sub
universe = vanilla

executable = ../../executables/preprocessing/MakeHe4CCh5.sh
output     = ../../condor_out/preprocessing/out/He4CC.out
error      = ../../condor_out/preprocessing/err/He4CC.err
log        = ../../condor_out/preprocessing/log/He4CC.log

getenv     = True

+JobFlavour = "workday"
RequestCPUs = 5
queue
EOL


#He4
cat <<EOL > ../../executables/preprocessing/MakeHe4h5.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/preprocessing" 
echo "Execute TreeToH5.py"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/preprocessing
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python TreeToH5.py --config ../yaml/preprocessing/He4.yml
EOL
cat <<EOL > ../../sub/preprocessing/He4.sub
universe = vanilla

executable = ../../executables/preprocessing/MakeHe4h5.sh
output     = ../../condor_out/preprocessing/out/He4.out
error      = ../../condor_out/preprocessing/err/He4.err
log        = ../../condor_out/preprocessing/log/He4.log

getenv     = True

+JobFlavour = "workday"
RequestCPUs = 5
queue
EOL
    #CL specific
cat <<EOL > ../../executables/preprocessing/MakeHe4CLh5.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/preprocessing" 
echo "Execute TreeToH5.py"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/preprocessing
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python TreeToH5.py --config ../yaml/preprocessing/He4CL.yml
EOL
cat <<EOL > ../../sub/preprocessing/He4CL.sub
universe = vanilla

executable = ../../executables/preprocessing/MakeHe4CLh5.sh
output     = ../../condor_out/preprocessing/out/He4CL.out
error      = ../../condor_out/preprocessing/err/He4CL.err
log        = ../../condor_out/preprocessing/log/He4CL.log

getenv     = True

+JobFlavour = "workday"
RequestCPUs = 5
queue
EOL

#ISS neg
cat <<EOL > ../../executables/preprocessing/MakeISSnegh5.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/preprocessing" 
echo "Execute TreeToH5.py"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/preprocessing
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python TreeToH5.py --config ../yaml/preprocessing/ISSneg.yml
EOL
cat <<EOL > ../../sub/preprocessing/MakeISSneg.sub
universe = vanilla

executable = ../../executables/preprocessing/MakeISSnegh5.sh
output     = ../../condor_out/preprocessing/out/ISSneg.out
error      = ../../condor_out/preprocessing/err/ISSneg.err
log        = ../../condor_out/preprocessing/log/ISSneg.log

getenv     = True

+JobFlavour = "workday"
RequestCPUs = 5
queue
EOL


#ISS pos
cat <<EOL > ../../executables/preprocessing/MakeISSposh5.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/preprocessing" 
echo "Execute TreeToH5.py"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/preprocessing
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python TreeToH5.py --config ../yaml/preprocessing/ISSpos.yml
EOL
cat <<EOL > ../../sub/preprocessing/MakeISSpos.sub
universe = vanilla

executable = ../../executables/preprocessing/MakeISSposh5.sh
output     = ../../condor_out/preprocessing/out/ISSpos.out
error      = ../../condor_out/preprocessing/err/ISSpos.err
log        = ../../condor_out/preprocessing/log/ISSpos.log

getenv     = True

+JobFlavour = "workday"
RequestCPUs = 5
queue
EOL
    #CL specific
cat <<EOL > ../../executables/preprocessing/MakeISSposCLh5.sh
#!/bin/bash
echo "PYTHON: /afs/cern.ch/user/f/frrossi/.conda/envs/AntiHe/bin/python"
echo "Moving to: /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/preprocessing" 
echo "Execute TreeToH5.py"
cd /afs/cern.ch/user/f/frrossi/work/ams_network_DEV/preprocessing
/afs/cern.ch/work/f/frrossi/miniconda3/envs/AntiHe/bin/python TreeToH5.py --config ../yaml/preprocessing/ISSposCL.yml
EOL
cat <<EOL > ../../sub/preprocessing/MakeISSposCL.sub
universe = vanilla

executable = ../../executables/preprocessing/MakeISSposCLh5.sh
output     = ../../condor_out/preprocessing/out/ISSposCL.out
error      = ../../condor_out/preprocessing/err/ISSposCL.err
log        = ../../condor_out/preprocessing/log/ISSposCL.log

getenv     = True

+JobFlavour = "workday"
RequestCPUs = 5
queue
EOL

#condor_submit ../../sub/preprocessing/AntiHe3.sub
#condor_submit ../../sub/preprocessing/Electrons.sub
condor_submit ../../sub/preprocessing/He3.sub
condor_submit ../../sub/preprocessing/He3CL.sub
condor_submit ../../sub/preprocessing/He3cc.sub
condor_submit ../../sub/preprocessing/He4.sub
condor_submit ../../sub/preprocessing/He4CL.sub
condor_submit ../../sub/preprocessing/He4CC.sub
condor_submit ../../sub/preprocessing/MakeISSpos.sub
condor_submit ../../sub/preprocessing/MakeISSposCL.sub
condor_submit ../../sub/preprocessing/MakeISSneg.sub
condor_q
