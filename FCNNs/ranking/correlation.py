
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
                   'SigmaInnerL1',          'RigLabel']
      
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

#Gettin labels but not dropping it 
label = UnNormData['RigLabel'].values

#Dropping signal events
print('---->Dropping signal events:', SigEvent)
print('Signal events', len(label[label==1]))
print('Bkg events', len(label[label==0]))
lsig = label[label==1][:SigEvent]
lbkg = label[label==0]
Xsig=UnNormData[label==1][:SigEvent].values
Xbkg=UnNormData[label==0].values
X=np.vstack((Xsig,Xbkg))
label=np.hstack((lsig, lbkg))
print('-----> CHECK <-----')
print('Signal events', len(label[label==1]))
print('Bkg events', len(label[label==0]))
print('Label shape', label.shape[0])
print('X shape', X.shape[0])
permutation = np.random.permutation(X.shape[0])
del label
X = X[permutation]
UnNormData=pd.DataFrame(X, columns=UnNormData.columns)

#Labels
label_train =UnNormData['RigLabel'].values
UnNormData.drop(columns=['RigLabel'], inplace=True)
#Values
X = UnNormData.values

#Permutation
permutation = np.random.permutation(label_train.shape[0])
X = X[permutation]
label_train = label_train[permutation]

# Create the StratifiedShuffleSplit object
sss = StratifiedShuffleSplit(n_splits=1, test_size=0.3, random_state=0)
# Split the data
for train_index, test_index in sss.split(X, label_train):
    x_trainUnNorm, x_testUnNorm = X[train_index], X[test_index]
    y_train, y_test = label_train[train_index], label_train[test_index]

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


