
## DNNs for anti-helium searches in CRs
A DNN for a classification task and an autoencoder for anomaly detection

## Description
Some information on the scientific basis of this project can be found in the following proceeding [PoS-ICHEP 2024](https://pos.sissa.it/476/747).
This project contains four DNNs: two binary classifiers and two autoncoders. 
The classifiers are used to quantify the reconstruction quality of a measured Rigidity (p/Z). One of them is trained, validated and tested only on Monte Carlo simulations. The second classifier is trained, validated and tested only on real data. 
Similarly, the autoencoders are applied only to MC or data. 
This data-driven approach reduces any possible discrepancy between data and MC. 
Moreover, the classifier and the autoencoder used on the data (or MC) are almost independent. For this reason, the network's outputs can be combined, resulting in an increment of the background rejection, without decreasing the signal efficiency for possible antimatter candidates. 


## Usage
### Preprocessing
The initial datasets are ROOT TTrees created using the AMS-02 collaboration code. 
Most of the customisations in the repos are handled through yaml files in the yaml folder. 
For example, the yaml files in /yaml/preprocessing/ handle the creation of h5 files starting from the root TTrees. The executable responsible for this preprocessing stage is TreeToH5.py in /preprocessing/. 
Each dataset can be created using the .sh script in condor/starting_scripts/preprocessing/ path. This script launches jobs on HTCondor and creates the h5 files. 

### Training, validation and test
The .py files contained in the repo are used to train, validate and test the four networks. As for the preprocessing, each .py executable receives as input a .yaml file. These configuration files are in yaml/. 
All the pipelines are handled through the HTCondor dagman. Each network (classifier or autoencoder) has a single bash script that takes care of everything. For the autoencoders, the script is condor/starting_scripts/AE/ProduceDag.sh; whilst for the classifiers, it is condor/starting_scripts/CL2/ProduceDag.sh. 
The outputs are stored as ROOT TTrees. 

### Analysis
The analysis and the plotting are carried out in the /root_files/ folder. Here, many root macros are used for different purposes. For example, the independence hypothesis is confirmed through the root_files/EfficiencyCheck.cc.
The final results are an exclusion limit on the ratio of anti-He/He nuclei as a function of the kinetic energy per nucleon. 
Some studies on the confidence interval of the true anti-He rate are performed, although not available here. 


## Support
francesco.rossi@cern.ch

## Authors and acknowledgement
Francesco Rossi