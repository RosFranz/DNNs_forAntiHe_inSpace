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

device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
###############################################
# CHECK CAREFULLY THE FOLLOWING PARAMETERS:
    #Working on ISS positive and negative data
###############################################


# ### Loading dataset and normalization

dataset    = np.load("/home/franz/AMS/ams_network/dataset/ISSpos_dic.npy", allow_pickle=True).item()
datasetNeg = np.load("/home/franz/AMS/ams_network/dataset/ISSneg_dic.npy", allow_pickle=True).item()
print("Dataset loaded")
print(dataset.keys())

UnNormData = pd.DataFrame(dataset)
UnNormDataNeg = pd.DataFrame(datasetNeg)
del dataset, datasetNeg

print('Number of entries: ', UnNormData.shape)
UnNormData.fillna(0, inplace=True)

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
UnNormData.drop(columns=['BartelNum'], inplace=True)
# UnNormData.drop(columns=['RigLabel'], inplace=True)


#Dropping negatives columns
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
# UnNormDataNeg.drop(columns=['RigLabel'], inplace=True)

XNeg = UnNormDataNeg.values    

X=UnNormData.values

print(UnNormData.columns)
print(UnNormDataNeg.columns)
x_trainUnNorm, x_testUnNorm = train_test_split(X, test_size=0.5, random_state=0, shuffle=True)
x_trainUnNormNeg, x_testUnNormNeg = train_test_split(XNeg, test_size=0.5, random_state=0, shuffle=True)
del x_testUnNorm, x_testUnNormNeg    

#Feature scaling
stdScaler = StandardScaler()
# minmaxScaler = MinMaxScaler()
x_train = stdScaler.fit_transform(x_trainUnNorm)
# x_train = minmaxScaler.fit_transform(x_train)
NormalizedData = pd.DataFrame(x_train, columns=UnNormData.columns)
del x_train

x_trainNeg = stdScaler.transform(x_trainUnNormNeg)
# x_trainNeg = minmaxScaler.transform(x_trainNeg)
NormalizedDataNeg = pd.DataFrame(x_trainNeg, columns=UnNormDataNeg.columns)
del x_trainNeg

### Plotting the features
FeatureName = [i for i in NormalizedData.columns]
print('Number of features: ', FeatureName.__len__())

for FeautureNumber in range(0, FeatureName.__len__()):
    plt.figure(figsize=(10, 8))
        
    featurePos=NormalizedData.iloc[:, FeautureNumber].tolist()
    featureNeg=NormalizedDataNeg.iloc[:, FeautureNumber].tolist()
    max_data = max(max(featurePos), max(featureNeg))
    min_data = min(min(featurePos), min(featureNeg))
    bin_width = (max_data-min_data)/25.
    if(bin_width==0):
        continue
    bins = np.arange(min_data-bin_width, max_data+bin_width, bin_width)

    plt.hist(featurePos, bins=bins, label='ISS data positives', histtype='stepfilled', \
        color='tab:blue', alpha=0.5, density=True)
    plt.hist(featureNeg, bins=bins, label='ISS data negatives', histtype='stepfilled', \
        color='tab:red', alpha=0.5, density=True)
    plt.legend(prop={'size': 20}, framealpha=0., loc='upper left')

    plt.tick_params(axis='both', which='major', labelsize=18)
    plt.xlabel(FeatureName[FeautureNumber], fontsize=20)
    plt.ylabel('Events', fontsize=20)
    plt.yscale('log')

    # plt.xticks(np.arange(0, 1.1, 0.1))

    plt.grid(True, which='both', color='gray', linestyle='--', linewidth=0.5)
    plt.gca().yaxis.set_major_locator(ticker.LogLocator(base=10))  # Set major locator
    plt.gca().yaxis.set_minor_locator(ticker.LogLocator(base=10, subs='all'))  # Set minor locator with all subs
    plt.savefig('plot_features/AE/'+FeatureName[FeautureNumber]+'.png')
    plt.close()

print('Saved figures at plot_features/AE/FeautureNumber*.png')
print('Done!')


