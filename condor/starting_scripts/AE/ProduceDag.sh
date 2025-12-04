#!/bin/bash

#####
## DAG file
#####
rm ../../dag/AE/MC/*.out
rm ../../dag/AE/MC/*.log
rm ../../dag/AE/MC/*.err
rm ../../dag/AE/MC/*.sub
rm ../../dag/AE/MC/*.rescue*
rm ../../dag/AE/MC/*.metrics

rm ../../dag/AE/ISS/*.metrics
rm ../../dag/AE/ISS/*.rescue*
rm ../../dag/AE/ISS/*.sub
rm ../../dag/AE/ISS/*.err
rm ../../dag/AE/ISS/*.log
rm ../../dag/AE/ISS/*.out


##TRAINING ON MC and TEST on ISS
cat <<EOL > ../../dag/AE/MC/PipelineMC.dag

JOB AE_trainingMC    ../../sub/AE/trainMC.sub
JOB AE_ValTestMC     ../../sub/AE/ValTestMC.sub

JOB AE_trainingMCnoW ../../sub/AE/trainMC_noW.sub
JOB AE_ValTestMCnoW  ../../sub/AE/ValTestMC_noW.sub

JOB AE_rankingMC     ../../sub/AE/rankingMC.sub
JOB AE_rankingMCnoW  ../../sub/AE/rankingMC_noW.sub

JOB AE_testOnISS1    ../../sub/AE/testOnISS1.sub
JOB AE_testOnISS2    ../../sub/AE/testOnISS2.sub

JOB AE_testOnISS1noW ../../sub/AE/testOnISS1_noW.sub
JOB AE_testOnISS2noW ../../sub/AE/testOnISS2_noW.sub

JOB AE_testOnAntiHe    ../../sub/AE/testOnAntiHe.sub
JOB AE_testOnAntiHenoW ../../sub/AE/testOnAntiHe_noLossW.sub

JOB AE_testOnEl    ../../sub/AE/testOnElectrons.sub
JOB AE_testOnElnoW ../../sub/AE/testOnElectrons_noLossW.sub

JOB AE_testOnHe3    ../../sub/AE/testOnHe3.sub
JOB AE_testOnHe3noW ../../sub/AE/testOnHe3_noLossW.sub


 
SCRIPT PRE AE_trainingMC      AEtrain_MC.sh useW
SCRIPT PRE AE_trainingMCnoW   AEtrain_MC.sh noW

SCRIPT POST AE_trainingMC     AEtestOnISS.sh useW
SCRIPT POST AE_trainingMCnoW  AEtestOnISS.sh noW

SCRIPT PRE AE_rankingMC       AEranking_MC.sh useW
SCRIPT PRE AE_rankingMCnoW    AEranking_MC.sh noW

PARENT AE_trainingMC    CHILD AE_rankingMC    AE_testOnISS1    AE_testOnISS2
PARENT AE_trainingMC    CHILD AE_testOnAntiHe AE_testOnEl      AE_testOnHe3

PARENT AE_trainingMCnoW CHILD AE_ValTestMC    AE_rankingMCnoW    AE_testOnISS1noW AE_testOnISS2noW
PARENT AE_trainingMCnoW CHILD AE_ValTestMCnoW AE_testOnAntiHenoW AE_testOnElnoW   AE_testOnHe3noW
EOL

##TRAINING ON ISS and TEST on MC
cat <<EOL > ../../dag/AE/ISS/PipelineISS.dag

JOB AE_trainingISS ../../sub/AE/train_ISS.sub
JOB AE_ValTestISS  ../../sub/AE/ValTest_ISS.sub

JOB AE_rankingISS  ../../sub/AE/ranking_ISS.sub

JOB AE_testOnMC1  ../../sub/AE/testOnMC1.sub
JOB AE_testOnMC2  ../../sub/AE/testOnMC2.sub

 
SCRIPT PRE  AE_trainingISS AEtrain_ISS.sh
SCRIPT POST AE_trainingISS AEtestOnMC.sh

SCRIPT PRE AE_rankingISS AEranking_ISS.sh

PARENT AE_trainingISS CHILD AE_ValTestISS AE_rankingISS AE_testOnMC1 AE_testOnMC2
EOL


condor_submit_dag ../../dag/AE/MC/PipelineMC.dag
condor_submit_dag ../../dag/AE/ISS/PipelineISS.dag
