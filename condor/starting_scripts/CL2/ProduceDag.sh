#!/bin/bash

#####
## DAG file
#####
rm ../../dag/CL2/MC/*.out
rm ../../dag/CL2/MC/*.log
rm ../../dag/CL2/MC/*.err
rm ../../dag/CL2/MC/*.sub
rm ../../dag/CL2/MC/*.rescue*
rm ../../dag/CL2/MC/*.metrics

rm ../../dag/CL2/ISS/*.metrics
rm ../../dag/CL2/ISS/*.rescue*
rm ../../dag/CL2/ISS/*.sub
rm ../../dag/CL2/ISS/*.err
rm ../../dag/CL2/ISS/*.log
rm ../../dag/CL2/ISS/*.out


##TRAINING ON MC and TEST on ISS
cat <<EOL > ../../dag/CL2/MC/PipelineMCcl2.dag
JOB CL2_trainingMC    ../../sub/CL2/trainMC.sub
JOB CL2_trainingMCnoW ../../sub/CL2/trainMC_noW.sub

JOB CL2_rankingMC     ../../sub/CL2/rankingMC.sub
JOB CL2_rankingMCnoW  ../../sub/CL2/rankingMC_noW.sub

JOB CL2_Neg1          ../../sub/CL2/testOnNeg1.sub
JOB CL2_Neg2          ../../sub/CL2/testOnNeg2.sub
JOB CL2_Neg3          ../../sub/CL2/testOnNeg3.sub

JOB CL2_Neg1noW       ../../sub/CL2/testOnNeg1_noW.sub
JOB CL2_Neg2noW       ../../sub/CL2/testOnNeg2_noW.sub
JOB CL2_Neg3noW       ../../sub/CL2/testOnNeg3_noW.sub

JOB CL2_testOnISS1    ../../sub/CL2/testOnISS1.sub
JOB CL2_testOnISS2    ../../sub/CL2/testOnISS2.sub
JOB CL2_testOnISS3    ../../sub/CL2/testOnISS3.sub

JOB CL2_testOnISS1noW ../../sub/CL2/testOnISS1_noW.sub
JOB CL2_testOnISS2noW ../../sub/CL2/testOnISS2_noW.sub
JOB CL2_testOnISS3noW ../../sub/CL2/testOnISS3_noW.sub

JOB CL2_testOnAntiHe    ../../sub/CL2/testOnAntiHe.sub
JOB CL2_testOnAntiHenoW ../../sub/CL2/testOnAntiHe_noLossW.sub

JOB CL2_testOnEl    ../../sub/CL2/testOnElectrons.sub
JOB CL2_testOnElnoW ../../sub/CL2/testOnElectrons_noLossW.sub

JOB CL2_testOnHe3    ../../sub/CL2/testOnHe3.sub
JOB CL2_testOnHe3noW ../../sub/CL2/testOnHe3_noLossW.sub

 
SCRIPT PRE CL2_trainingMC     CL2train_MC.sh useW
SCRIPT PRE CL2_trainingMCnoW  CL2train_MC.sh noW
SCRIPT POST CL2_trainingMC    CL2testOnISS.sh useW
SCRIPT POST CL2_trainingMCnoW CL2testOnISS.sh noW

SCRIPT PRE CL2_rankingMC      CL2ranking_MC.sh useW
SCRIPT PRE CL2_rankingMCnoW   CL2ranking_MC.sh noW

PARENT CL2_trainingMC    CHILD CL2_rankingMC    CL2_testOnISS1    CL2_testOnISS2    CL2_testOnISS3
PARENT CL2_trainingMC    CHILD CL2_Neg1         CL2_Neg2          CL2_Neg3 
PARENT CL2_trainingMC    CHILD CL2_testOnAntiHe CL2_testOnEl      CL2_testOnHe3

PARENT CL2_trainingMCnoW CHILD CL2_rankingMCnoW    CL2_testOnISS1noW CL2_testOnISS2noW CL2_testOnISS3noW
PARENT CL2_trainingMCnoW CHILD CL2_Neg1noW         CL2_Neg2noW       CL2_Neg3noW
PARENT CL2_trainingMCnoW CHILD CL2_testOnAntiHenoW CL2_testOnElnoW   CL2_testOnHe3noW
EOL

##TRAINING ON ISS and TEST on MC
cat <<EOL > ../../dag/CL2/ISS/PipelineISScl2.dag
JOB CL2_trainingISS ../../sub/CL2/trainISS.sub

JOB CL2_rankingISS  ../../sub/CL2/ranking_ISS.sub

JOB CL2_Neg1fromISS ../../sub/CL2/testOnNeg_fromISS1.sub
JOB CL2_Neg2fromISS ../../sub/CL2/testOnNeg_fromISS2.sub
JOB CL2_Neg3fromISS ../../sub/CL2/testOnNeg_fromISS3.sub

JOB CL2_testOnMC1  ../../sub/CL2/testOnMC1.sub
JOB CL2_testOnMC2  ../../sub/CL2/testOnMC2.sub
JOB CL2_testOnMC3  ../../sub/CL2/testOnMC3.sub
 
SCRIPT PRE CL2_trainingISS  CL2train_ISS.sh 
SCRIPT POST CL2_trainingISS CL2testOnMC.sh

SCRIPT PRE CL2_rankingISS   CL2ranking_ISS.sh 

PARENT CL2_trainingISS    CHILD CL2_rankingISS    CL2_testOnMC1     CL2_testOnMC2    CL2_testOnMC3
PARENT CL2_trainingISS    CHILD CL2_Neg1fromISS   CL2_Neg2fromISS   CL2_Neg3fromISS
EOL


condor_submit_dag ../../dag/CL2/MC/PipelineMCcl2.dag
condor_submit_dag ../../dag/CL2/ISS/PipelineISScl2.dag
