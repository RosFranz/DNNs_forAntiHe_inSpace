# DNNs for Anti-Helium Searches in Space

Deep neural networks for anti-helium nucleus searches using AMS-02 data from
the International Space Station. Presented at ICHEP 2024 — proceedings:
https://pos.sissa.it/476/747

See also the PhD thesis available here: 
https://iris.unitn.it/handle/11572/490770 

## What this does

AMS-02 measures charged cosmic rays in space. To search for anti-helium, two
types of networks are trained separately on Monte Carlo and real ISS data:

- **Binary classifiers (CL2)**: separate well-reconstructed rigidity measurements
  from poorly reconstructed ones
- **Autoencoders (AE)**: anomaly detection for signal/background separation

Training separately on MC and ISS data reduces systematic MC/data discrepancies.
The combined outputs feed into a ROOT-based analysis pipeline that computes
exclusion limits on the anti-He/He flux ratio as a function of kinetic energy
per nucleon.

## Repository layout

globals.py central configuration (features, architecture, paths)
globals.py
├── MyAE/ autoencoder models, datasets, masked loss
├── CL2/ binary classifier models and datasets
├── train/ training scripts (AE_train.py, CL2_train.py)
├── evaluate/ inference scripts for MC and ISS data
├── preprocessing/ ROOT TTree → HDF5 conversion
├── utils/ config loading, HDF5 I/O, dataset balancing
├── scaler/ MaskedStandardScaler for missing detector data
├── AE/ post-training analysis and feature plots (AE)
├── FCNNs/ post-training analysis and feature plots (CL2)
├── root_files/ ROOT C++ macros and independence checks
└── condor/ HTCondor/DAGman job submission for CERN clusters


## Data flow

ROOT files (AMS-02)
→ preprocessing/TreeToH5.py extract 184 features, write HDF5
  The initial datasets are ROOT TTrees created using the AMS-02 collaboration code. 
  Most of the customisations in the repos are handled through yaml files in the yaml folder. 
  For example, the yaml files in /yaml/preprocessing/ handle the creation of h5 files starting from the root TTrees. The executable responsible for this preprocessing stage is TreeToH5.py in /preprocessing/. 
  Each dataset can be created using the .sh script in condor/starting_scripts/preprocessing/ path. This script launches jobs on HTCondor (on LXPLUS at CERN) and creates the h5 files. 
→ train/CL2_train.py train binary classifier
→ train/AE_train.py train autoencoder
→ evaluate/CL2_onMC.py etc. score MC and ISS data separately
→ root_files/EfficiencyCheck.cc verify CL2/AE output independence
→ ROOT analysis pipeline compute exclusion limits



The .py files contained in train/ and evaluate/ receive as input a .yaml file, contained in yaml/. 
All the pipelines are handled through the HTCondor dagman. Each network (classifier or autoencoder) has a single bash script that takes care of everything. For the autoencoders, the script is condor/starting_scripts/AE/ProduceDag.sh; whilst for the classifiers, it is condor/starting_scripts/CL2/ProduceDag.sh. 
The outputs are stored as ROOT TTrees. 


The analysis and the plotting are carried out in the /root_files/ folder.
For example, the independence hypothesis is confirmed through the root_files/EfficiencyCheck.cc.
The final results are an exclusion limit on the ratio of anti-He/He nuclei as a function of the kinetic energy per nucleon. 
Some studies on the confidence interval of the true anti-He rate are performed, although not available here. 


## Missing data handling

Detector gaps are encoded as -9999. Three components handle this consistently:

- `scaler/custom_scaler.py` — `MaskedStandardScaler` normalises only valid entries
- `MyAE/AE_MaskedLoss.py` — `MaskedMSELoss` ignores masked features in the loss
- `MyAE/AE_models.py`, `CL2/CL2_models.py` — `maskedLinear` zeroes masked inputs
  before the linear operation and masks again on the output

Set `USEMASK = True` and `MYMASKVALUE = -9999` in `globals.py` to enable.

## Setup

```bash
conda env create -f environment_full.yml
conda activate <env-name>

Requires access to AMS-02 ROOT files (not distributed here) and, for cluster
jobs, a working HTCondor installation with EOS storage.

Running locally (small scale)

# 1. Convert ROOT to HDF5
python preprocessing/TreeToH5.py --config configs/preprocess.json

# 2. Train
python train/CL2_train.py --config configs/cl2.json
python train/AE_train.py  --config configs/ae.json

# 3. Evaluate
python evaluate/CL2_onMC.py --config configs/cl2.json
python evaluate/AE_onMC.py  --config configs/ae.json

Running on CERN cluster (full scale)

cd condor/starting_scripts
bash ProduceDag.sh
condor_submit_dag ../dag/<workflow>.dag

Configuration
All hyperparameters and paths live in globals.py. For experiment variants,
pass a JSON or YAML override with --config:

{
  "learning_rate": 5e-4,
  "epochs": 100,
  "layers": [10, 5, 10, 28]
}
Values in the config file override the corresponding globals.py defaults.

Feature ranking
CatBoost-based importance ranking and correlation analysis are in AE/ranking/
and FCNNs/ranking/. Run after evaluation to identify which detector variables
drive each network's decisions.


## Support
francesco.rossi@cern.ch

## Authors and acknowledgement
Francesco Rossi