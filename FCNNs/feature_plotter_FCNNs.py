# ## FEATURE PLOTTER


import numpy as np
import uproot as up
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import MinMaxScaler
from sklearn.preprocessing import StandardScaler
from sklearn.preprocessing import label_binarize
import torch
import torch.nn as nn
from torch.utils.data import Dataset, DataLoader
import torch.nn.functional as F
from statistics import mean
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import sklearn.metrics as sk
import seaborn as sns
from sklearn.model_selection import StratifiedShuffleSplit


device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
###############################################
# CHECK CAREFULLY THE FOLLOWING PARAMETERS:
    #This number selects the right dictionary and 
    #the corresponding label:
    #-1->ISS data, 0->label15, 1->label10, 2->label5
DatasetNumber = -1
###############################################


# ### Loading dataset and normalization
if(DatasetNumber==-1):
    dataset    = np.load("/home/franz/AMS/ams_network/dataset/ISSpos_dic.npy", allow_pickle=True).item()
    datasetNeg = np.load("/home/franz/AMS/ams_network/dataset/ISSneg_dic.npy", allow_pickle=True).item()
elif(DatasetNumber==0):
    dataset = np.load("/home/franz/AMS/ams_network/dataset/MC15_dic.npy", allow_pickle=True).item()
elif(DatasetNumber==1):
    dataset = np.load("/home/franz/AMS/ams_network/dataset/MC10_dic.npy", allow_pickle=True).item()
elif(DatasetNumber==2):
    dataset = np.load("/home/franz/AMS/ams_network/dataset/MC5_dic.npy", allow_pickle=True).item()
print("Dataset loaded")
print(dataset.keys())

UnNormData = pd.DataFrame(dataset)
if(DatasetNumber==-1):
    UnNormDataNeg = pd.DataFrame(datasetNeg)
    del dataset, datasetNeg
else:
    del dataset

print('Number of entries: ', UnNormData.shape)
UnNormData.fillna(-2, inplace=True)

#three choices for labels: label15, label10, label5
# label 15 -> 15% difference in SO/EL scattering ChiSq ratio
# label 10 -> 10% difference in SO/EL scattering ChiSq ratio
# label 5 -> 5% difference in SO/EL scattering ChiSq ratio
if(DatasetNumber==-1):
    label = UnNormData['RigLabel'].values
    UnNormData.drop(columns=['RigLabel'], inplace=True)
    labelNeg = UnNormDataNeg['RigLabel'].values
    UnNormDataNeg.drop(columns=['RigLabel'], inplace=True)
elif(DatasetNumber==0):
    label = UnNormData['label15'].values
    UnNormData.drop(columns=['label15'], inplace=True)
    label = label_binarize(label, classes=[0, 1, 2, 3])
elif(DatasetNumber==1):
    label = UnNormData['label10'].values
    UnNormData.drop(columns=['label10'], inplace=True)
    label = label_binarize(label, classes=[0, 1, 2, 3])
elif(DatasetNumber==2):
    label = UnNormData['label5'].values
    UnNormData.drop(columns=['label5'], inplace=True)
    label = label_binarize(label, classes=[0, 1, 2, 3])

#TOF
UnNormData.drop(columns=['beta_tof'], inplace=True)
UnNormData.drop(columns=['gamma_tof'], inplace=True)
# UnNormData.drop(columns=['chi2Coo_tof'], inplace=True)
# UnNormData.drop(columns=['chi2Time_tof'], inplace=True)
#RICH
UnNormData.drop(columns=['hasRich'], inplace=True)
UnNormData.drop(columns=['isNaF'], inplace=True)
UnNormData.drop(columns=['isBorder_rich'], inplace=True)
UnNormData.drop(columns=['beta_rich'], inplace=True)
UnNormData.drop(columns=['beta_err_rich'], inplace=True)
UnNormData.drop(columns=['gamma_rich'], inplace=True)
UnNormData.drop(columns=['beta_consistency_rich'], inplace=True)
UnNormData.drop(columns=['beta_consistencyTOF'], inplace=True)
# UnNormData.drop(columns=['kprob_rich'], inplace=True)
# UnNormData.drop(columns=['charge_rich'], inplace=True)
# UnNormData.drop(columns=['charge2_rich'], inplace=True)
# UnNormData.drop(columns=['totPE_Uncorr_rich'], inplace=True)
# UnNormData.drop(columns=['measPE_Uncorr_rich'], inplace=True)
# UnNormData.drop(columns=['measPE_Corr_rich'], inplace=True)
# UnNormData.drop(columns=['chargedPMTs_rich'], inplace=True)
# UnNormData.drop(columns=['ringPMTs_rich'], inplace=True)
# UnNormData.drop(columns=['ringPMTs2_rich'], inplace=True)
# UnNormData.drop(columns=['totHits_rich'], inplace=True)
# UnNormData.drop(columns=['ringHits_rich'], inplace=True)
#TRK
UnNormData.drop(columns=['Rinner'], inplace=True)
UnNormData.drop(columns=['RinnerL1'], inplace=True)
UnNormData.drop(columns=['RfullSpan'], inplace=True)
UnNormData.drop(columns=['RinnerUH'], inplace=True)
UnNormData.drop(columns=['RinnerLH'], inplace=True)
#GENERAL INFO
UnNormData.drop(columns=['EvRun'], inplace=True)
UnNormData.drop(columns=['EvNum'], inplace=True)
if(DatasetNumber!=-1):
    UnNormData.drop(columns=['Rtrue'], inplace=True)
else:
    UnNormData.drop(columns=['BartelNum'], inplace=True)
        #TOF
    UnNormDataNeg.drop(columns=['beta_tof'], inplace=True)
    UnNormDataNeg.drop(columns=['gamma_tof'], inplace=True)
    # UnNormDataNeg.drop(columns=['chi2Coo_tof'], inplace=True)
    # UnNormDataNeg.drop(columns=['chi2Time_tof'], inplace=True)
    #RICH
    UnNormDataNeg.drop(columns=['hasRich'], inplace=True)
    UnNormDataNeg.drop(columns=['isNaF'], inplace=True)
    UnNormDataNeg.drop(columns=['isBorder_rich'], inplace=True)
    UnNormDataNeg.drop(columns=['beta_rich'], inplace=True)
    UnNormDataNeg.drop(columns=['beta_err_rich'], inplace=True)
    UnNormDataNeg.drop(columns=['gamma_rich'], inplace=True)
    UnNormDataNeg.drop(columns=['beta_consistency_rich'], inplace=True)
    UnNormDataNeg.drop(columns=['beta_consistencyTOF'], inplace=True)
    # UnNormDataNeg.drop(columns=['kprob_rich'], inplace=True)
    # UnNormDataNeg.drop(columns=['charge_rich'], inplace=True)
    # UnNormDataNeg.drop(columns=['charge2_rich'], inplace=True)
    # UnNormDataNeg.drop(columns=['totPE_Uncorr_rich'], inplace=True)
    # UnNormDataNeg.drop(columns=['measPE_Uncorr_rich'], inplace=True)
    # UnNormDataNeg.drop(columns=['measPE_Corr_rich'], inplace=True)
    # UnNormDataNeg.drop(columns=['chargedPMTs_rich'], inplace=True)
    # UnNormDataNeg.drop(columns=['ringPMTs_rich'], inplace=True)
    # UnNormDataNeg.drop(columns=['ringPMTs2_rich'], inplace=True)
    # UnNormDataNeg.drop(columns=['totHits_rich'], inplace=True)
    # UnNormDataNeg.drop(columns=['ringHits_rich'], inplace=True)
    #TRK
    UnNormDataNeg.drop(columns=['Rinner'], inplace=True)
    UnNormDataNeg.drop(columns=['RinnerL1'], inplace=True)
    UnNormDataNeg.drop(columns=['RfullSpan'], inplace=True)
    UnNormDataNeg.drop(columns=['RinnerUH'], inplace=True)
    UnNormDataNeg.drop(columns=['RinnerLH'], inplace=True)
    #GENERAL INFO
    UnNormDataNeg.drop(columns=['EvRun'], inplace=True)
    UnNormDataNeg.drop(columns=['EvNum'], inplace=True)
    UnNormDataNeg.drop(columns=['BartelNum'], inplace=True)
    
    XNeg = UnNormDataNeg.values    

X=UnNormData.values

if(DatasetNumber!=-1):
    x_trainUnNorm, x_testUnNorm, \
    y_train, y_test = train_test_split(X, label, \
                                    test_size=0.3, \
                                    random_state=0, shuffle=True)
    del x_testUnNorm
else:
    sss = StratifiedShuffleSplit(n_splits=1, test_size=0.3, random_state=0)
    # Split the data
    for train_index, test_index in sss.split(X, label):
        x_trainUnNorm, x_testUnNorm = X[train_index], X[test_index]
        y_train, y_test = label[train_index], label[test_index]
    for train_index, test_index in sss.split(XNeg, labelNeg):
        x_trainUnNormNeg, x_testUnNormNeg = XNeg[train_index], XNeg[test_index]
        y_trainNeg, y_testNeg = label[train_index], label[test_index]
    del x_testUnNorm, x_testUnNormNeg    

#Feature scaling
stdScaler = StandardScaler()
# minmaxScaler = MinMaxScaler()
x_train = stdScaler.fit_transform(x_trainUnNorm)
# x_train = minmaxScaler.fit_transform(x_train)
NormalizedData = pd.DataFrame(x_train, columns=UnNormData.columns)
del x_train
if(DatasetNumber==-1):
    x_trainNeg = stdScaler.transform(x_trainUnNormNeg)
    # x_trainNeg = minmaxScaler.transform(x_trainNeg)
    NormalizedDataNeg = pd.DataFrame(x_trainNeg, columns=UnNormDataNeg.columns)
    del x_trainNeg

### Plotting the features
FeatureName = [i for i in NormalizedData.columns]
print('Number of features: ', FeatureName.__len__())

for FeautureNumber in range(0, FeatureName.__len__()):
    plt.figure(figsize=(10, 8))
    if(DatasetNumber!=-1):
        featureSO=NormalizedData[(y_train[:,0] == 1)].iloc[:, FeautureNumber].tolist()
        featureHI=NormalizedData[(y_train[:,1] == 1)].iloc[:, FeautureNumber].tolist()
        featureEL=NormalizedData[(y_train[:,2] == 1)].iloc[:, FeautureNumber].tolist()
        featureOt=NormalizedData[(y_train[:,3] == 1)].iloc[:, FeautureNumber].tolist()

        data_min = min(min(featureSO), min(featureHI), min(featureEL), min(featureOt))
        data_max = max(max(featureSO), max(featureHI), max(featureEL), max(featureOt))
        bin_width = 0.04
        bins = np.arange(data_min, data_max + bin_width, bin_width)

        plt.hist(featureSO, bins=bins, label='Spillover', histtype='stepfilled', color='tab:blue', alpha=0.5)
        plt.hist(featureEL, bins=bins, label='El. Scat.', histtype='stepfilled', color='tab:green', alpha=0.5)
        plt.hist(featureHI, bins=bins, label='Had. Inel.', histtype='stepfilled', color='tab:orange', alpha=0.5)
        plt.hist(featureOt, bins=bins, label='Other', histtype='stepfilled', color='tab:red', alpha=0.5)
        plt.legend(prop={'size': 20}, framealpha=0., loc='upper left')

        plt.tick_params(axis='both', which='major', labelsize=18)
        plt.xlabel(FeatureName[FeautureNumber], fontsize=20)
        plt.ylabel('Events', fontsize=20)
        plt.yscale('log')

        plt.xticks(np.arange(0, 1.1, 0.1))

        plt.grid(True, which='both', color='gray', linestyle='--', linewidth=0.5)
        plt.gca().yaxis.set_major_locator(ticker.LogLocator(base=10))  # Set major locator
        plt.gca().yaxis.set_minor_locator(ticker.LogLocator(base=10, subs='all'))  # Set minor locator with all subs
        plt.savefig('plot_features/4CL/'+FeatureName[FeautureNumber]+'.png')
        plt.close()
    else:
        featurePosSig=NormalizedData[(y_train == 1)].iloc[:, FeautureNumber].tolist()
        featurePosBkg=NormalizedData[(y_train == 0)].iloc[:, FeautureNumber].tolist()
        featureNeg=NormalizedDataNeg.iloc[:, FeautureNumber].tolist()
        max_data = max(max(featurePosSig), max(featurePosBkg), max(featureNeg))
        min_data = min(min(featurePosSig), min(featurePosBkg), min(featureNeg))
        bin_width = (max_data-min_data)/25.
        if(bin_width==0):
            continue
        bins = np.arange(min_data-bin_width, max_data+bin_width, bin_width)

        plt.hist(featurePosSig, bins=bins, label='ISS data positives SIG', histtype='stepfilled',\
            color='tab:blue', alpha=0.5, density=True)
        plt.hist(featurePosBkg, bins=bins, label='ISS data positives BKG', histtype='stepfilled',\
            color='tab:red', alpha=0.5, density=True)
        plt.hist(featureNeg, bins=bins, label='ISS data negatives', histtype='step', \
            color='tab:green', alpha=0.5, density=True)
        plt.legend(prop={'size': 20}, framealpha=0., loc='upper left')

        plt.tick_params(axis='both', which='major', labelsize=18)
        plt.xlabel(FeatureName[FeautureNumber], fontsize=20)
        # plt.xticks(np.arange(0, 1.1, 0.1))
        
        plt.ylabel('Events', fontsize=20)
        plt.yscale('log')

        plt.grid(True, which='both', color='gray', linestyle='--', linewidth=0.5)
        plt.gca().yaxis.set_major_locator(ticker.LogLocator(base=10))  # Set major locator
        plt.gca().yaxis.set_minor_locator(ticker.LogLocator(base=10, subs='all'))  # Set minor locator with all subs
        plt.savefig('plot_features/2CL/'+FeatureName[FeautureNumber]+'.png')
        plt.close()

if(DatasetNumber!=-1):
    print('Saved figures at plot_features/4CL/FeautureNumber*.png')
    print('Done!')
else:
    print('Saved figures at plot_features/2CL/FeautureNumber*.png')
    print('Done!')


