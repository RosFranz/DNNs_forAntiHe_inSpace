#!/bin/bash

#####
## DAG file
#####

rm ../../dag/AE/ISS/*.metrics
rm ../../dag/AE/ISS/*.rescue*
rm ../../dag/AE/ISS/*.sub
rm ../../dag/AE/ISS/*.err
rm ../../dag/AE/ISS/*.log
rm ../../dag/AE/ISS/*.out


##TRAINING ON ISS and TEST on MC
cat <<EOL > ../../dag/AE/ISS/PipelineISS.dag
JOB AE_trainingISS ../../sub/AE/train_ISS.sub

JOB AE_rankingISS  ../../sub/AE/ranking_ISS.sub

JOB AE_testOnMC1  ../../sub/AE/testOnMC1.sub
JOB AE_testOnMC2  ../../sub/AE/testOnMC2.sub

 
SCRIPT PRE  AE_trainingISS AEtrain_ISS.sh
SCRIPT POST AE_trainingISS AEtestOnMC.sh

SCRIPT PRE AE_rankingISS   AEranking_ISS.sh

PARENT AE_trainingISS CHILD AE_rankingISS AE_testOnMC1 AE_testOnMC2
EOL

condor_submit_dag ../../dag/AE/ISS/PipelineISS.dag
