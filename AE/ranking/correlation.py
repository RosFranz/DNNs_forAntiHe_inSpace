
import numpy as np
import uproot as up
import pandas as pd
from sklearn.model_selection import train_test_split, ParameterGrid
from sklearn.preprocessing import StandardScaler
from torch.utils.data import Dataset, DataLoader
import torch.nn.functional as F
from statistics import mean
import argparse
import pandas as pd
import sklearn.metrics as sk
import pickle
import matplotlib.pyplot as plt
import seaborn as sns
from sklearn.model_selection import StratifiedShuffleSplit


###############################################
# CHECK CAREFULLY THE FOLLOWING PARAMETERS:
test_fraction = 0.3
seed = 30
SigEvent = 100000
###############################################

#setting seed for permutations
np.random.seed(seed)

columns_to_keep = ['ACC_AntiCounter',
                   'TOF_OnTimeClusterL1',   'TOF_OnTimeClusterL2',   'TOF_OnTimeClusterL3',   'TOF_OnTimeClusterL4',
                   'TOF_OffTimeClusterL1',  'TOF_OffTimeClusterL2',  'TOF_OffTimeClusterL3',  'TOF_OffTimeClusterL4',
                   'TRK_ClusterOnL7_Y',     'TRK_ClusterOnL8_Y',
                   'TRK_NormEdep2L2_XY',    'TRK_NormEdep2L3_XY',    'TRK_NormEdep2L4_XY',    'TRK_NormEdep2L5_XY',
                   'TRK_NormEdep2L6_XY',    'TRK_NormEdep2L7_XY',    'TRK_NormEdep2L8_XY',
                   'TRK_TrackResidualL2_Y', 'TRK_TrackResidualL3_Y', 'TRK_TrackResidualL4_Y', 'TRK_TrackResidualL5_Y',
                   'TRK_TrackResidualL6_Y', 'TRK_TrackResidualL7_Y', 'TRK_TrackResidualL8_Y',
                   'TRK_TrackResidualL2_X', 'TRK_TrackResidualL3_X', 'TRK_TrackResidualL4_X', 'TRK_TrackResidualL5_X',
                   'TRK_TrackResidualL6_X', 'TRK_TrackResidualL7_X', 'TRK_TrackResidualL8_X',
                   'InnerHit',
                   'chi2Coo_tof',           'chi2Time_tof',
                   'beta_consistency_rich', 'beta_consistencyTOF',
                   'kprob_rich',            'charge_rich',           'charge2_rich',         'totPE_Uncorr_rich',
                   'measPE_Uncorr_rich',    'measPE_Corr_rich',      'chargedPMTs_rich',     'ringPMTs_rich',
                   'ringPMTs2_rich',        'totHits_rich',          'ringHits_rich',
                   'SigmaInnerL1',          'SigmaUpLow']
      
print('Columns to keep:')          
print(columns_to_keep)
dataset    = np.load("/home/franz/AMS/ams_network/dataset/ISSpos_dic.npy", allow_pickle=True).item()
UnNormData = pd.DataFrame(dataset)
UnNormData.replace([np.inf, -np.inf], -2, inplace=True)
UnNormData.fillna(-2, inplace=True)
#Shuffle
UnNormData=UnNormData.sample(frac=1, random_state=seed)
#Selecting only some columns
UnNormData.drop(columns=[col for col in UnNormData.columns if col not in columns_to_keep], inplace=True)
print('Current columns:', UnNormData.columns)

#Values
X = UnNormData.values

#Permutation
permutation = np.random.permutation(X.shape[1])
X = X[permutation]

# Split the data
x_trainUnNorm, x_testUnNorm = train_test_split(X, test_size=test_fraction, random_state=seed)

#Standard scaler
stdScaler = StandardScaler()
x_train = stdScaler.fit_transform(x_trainUnNorm)
x_test = stdScaler.transform(x_testUnNorm)

train_data=pd.DataFrame(x_train, columns=UnNormData.columns)

plt.rcParams.update({'font.size': 10})   
fig, ax = plt.subplots(figsize=(20,20))

sns.heatmap(train_data.corr(method='pearson'), linecolor='w', linewidths=2, annot=True, fmt='.2f', 
            cmap=plt.get_cmap('coolwarm'), cbar=False, ax=ax)
plt.rcParams.update({'font.size': 2}) #change the font size for the number inside the heatmap

plt.ylim(len(train_data.columns),-0.5)
plt.title('Correlation matrix', fontsize=30)
plt.tight_layout()
plt.savefig('corrMatrix.png', dpi=300)  # Adjust dpi as needed


