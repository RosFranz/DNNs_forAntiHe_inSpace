import numpy as np
import uproot as up
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import MinMaxScaler
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
import h5py

###############################################
# CHECK CAREFULLY THE FOLLOWING PARAMETERS:
valDic_path = '/eos/home-f/frrossi/AMS/ams_network/val_dic/dev/'
plot_path   = '/eos/home-f/frrossi/AMS/ams_network/plot/AE/'
isISS = False #False is MC
UseLossWeights = False
UseWeights = False
Switch = False
Nbins = 52
name = '_2025_08_17'
# name = '_FRank_302010210_TEST'
###############################################

def ReadH5toDic(file_path):
    group_name = 'dic'
    myDic = {}
    with h5py.File(file_path, 'r') as myF:
        group = myF[group_name]
        for dataset_name in group.keys():
            myDic[dataset_name] = group[dataset_name][:]
    return myDic

# ### Load saved dictonary
if(isISS):
    if(Switch):
        in_path = valDic_path+"val_dic_AE_ISS_switch.h5"
    else:
        in_path = valDic_path+"val_dic_AE_ISS.h5"
else:
    if(UseLossWeights):
        in_path = valDic_path+"val_dic_AE_MC_withW.h5"
    else:
        in_path = valDic_path+"val_dic_AE_MC.h5"
                                 
Restoredic = ReadH5toDic(in_path)
print(Restoredic.keys())

### Loss functions
plt.figure(figsize=(10, 8))
y_min = min(min(Restoredic['train_loss']), min(Restoredic['val_loss']))*0.95
y_max = max(max(Restoredic['train_loss']), max(Restoredic['val_loss']))*1.05
plt.ylim(y_min, y_max)
plt.yscale('log')

plt.tick_params(axis='both', which='major', labelsize=15)
plt.tick_params(axis='both', which='minor', labelsize=15)
plt.xlabel('Epochs', fontsize=30)
plt.ylabel('Loss value', fontsize=30)

epochs=len(Restoredic['train_loss'])
x_ticks = np.arange(0, epochs+10, 20.) 

min_val = min(min(Restoredic['train_loss']), min(Restoredic['val_loss']))
max_val = max(max(Restoredic['train_loss']), max(Restoredic['val_loss']))

plt.plot(range(epochs),Restoredic['train_loss'],label='trainining loss function', linewidth=3)
plt.plot(range(epochs),Restoredic['val_loss'],label='validation loss function', linewidth=3)
plt.legend(prop={'size': 20}, framealpha=0., loc='upper right')
plt.grid(True, which='both', color='gray', linestyle='--', linewidth=0.5)
plt.xticks(x_ticks)
plt.xticks(fontsize=20)
plt.yticks(fontsize=20)

#get current date
import datetime
now = datetime.datetime.now()
print ("Current date and time : ")
print (now.strftime("%Y-%m-%d %H:%M:%S"))
#save it in a string
now = now.strftime("%Y_%m_%d")
if(Switch):
    now = now + '_switch'
if(isISS):
    path = plot_path+'ISS/Loss_'+now+name+'.png'
else:
    if(UseLossWeights):
        path = plot_path+'MC/Loss_'+now+name+'_withW.png'
    else:
        path = plot_path+'MC/Loss_'+now+name+'.png'
plt.savefig(path, bbox_inches='tight', dpi=300)  # Adjust dpi as needed


#Anomaly score plot
ValErrors=Restoredic['anomaly_score']
TestErrors =Restoredic['NEGanomaly_score']
if(not isISS and UseWeights):
    pos_weights=Restoredic['weights']
    neg_weights=Restoredic['weights_neg']
    print('Test: ', len(TestErrors), len(neg_weights), len(ValErrors), len(pos_weights) )

minX = 0.49
maxX = 1.01
print('Min/Max anomaly score on validation dataset', min(ValErrors), '/', max(ValErrors))
print(len(ValErrors))
print('Frac. anomaly score < 0.8: ',len(ValErrors[(ValErrors<0.8)])/len(ValErrors))
print('Min/Max anomaly score on test dataset', min(TestErrors), '/', max(TestErrors))
print(len(TestErrors))
print('Frac anomaly score > 0.8: ', len(TestErrors[TestErrors>0.8])/len(TestErrors))


plt.figure(figsize=(10, 8))
bin_width = (maxX-minX)/Nbins
bins = np.arange(minX, maxX, bin_width)
print('Number of edges', len(bins), 'Number of bins', len(bins)-1)
print(bins)
plt.ylim(1.e-4, 1.5e+0)
plt.xlim(minX, maxX)

#Making normalized histos
if(not isISS and UseWeights):
    h1_Test, bins_ISS_test = np.histogram(TestErrors, bins=Nbins, range=(minX, maxX), weights=neg_weights)
    h1_Val, bins_ISS       = np.histogram(ValErrors, bins=Nbins, range=(minX, maxX), weights=pos_weights)
else:
    h1_Test, bins_ISS_test = np.histogram(TestErrors, bins=Nbins, range=(minX, maxX))
    h1_Val, bins_ISS       = np.histogram(ValErrors, bins=Nbins, range=(minX, maxX))
    
h1_Test_er = np.sqrt(h1_Test)
h1_Val_er     = np.sqrt(h1_Val)

BinNrm   = bin_width*bins.size

h1_Test_er = h1_Test_er/(sum(h1_Test)*BinNrm)
h1_Test    = h1_Test/(sum(h1_Test)*BinNrm)
h1_Val_er     = h1_Val_er/(sum(h1_Val)*BinNrm)
h1_Val        = h1_Val/(sum(h1_Val)*BinNrm)

print("ISS_area validation: ",(sum(h1_Val)*BinNrm))
print("ISS_area test: ",(sum(h1_Test)*BinNrm))

if(isISS):
    plt.scatter(bins_ISS[:-1], h1_Val, label='ISS positive', color='black', marker='o', s=30)
    #plt.errorbar(bins_ISS[:-1], h1_Val, yerr=h1_Val_er, fmt='o', color='black')
    plt.scatter(bins_ISS_test[:-1], h1_Test, label='ISS negative', color='tab:blue', marker='o', s=30)
    # plt.errorbar(bins_ISS_test[:-1], h1_Test, yerr=h1_Test_er, fmt='o', color='tab:blue')
else:
    plt.scatter(bins_ISS[:-1], h1_Val, label='MC R>0', color='black', marker='o', s=30)
    #plt.errorbar(bins_ISS[:-1], h1_Val, yerr=h1_Val_er, fmt='o', color='black')
    plt.scatter(bins_ISS_test[:-1], h1_Test, label='MC R<0', color='tab:blue', marker='o', s=30)
    # plt.errorbar(bins_ISS_test[:-1], h1_Test, yerr=h1_Test_er, fmt='o', color='tab:blue')


plt.legend(prop={'size': 20}, framealpha=0., loc='upper center')
plt.xlabel('Anomaly score', fontsize=30)
plt.ylabel('Arbitray units', fontsize=30)
plt.yscale('log')

plt.gca().xaxis.set_major_locator(ticker.MultipleLocator(0.1))  # Set major locator
plt.gca().xaxis.set_minor_locator(ticker.MultipleLocator(bin_width))  # Set major locator
plt.gca().yaxis.set_major_locator(ticker.LogLocator(base=10))  # Set major locator
plt.gca().yaxis.set_minor_locator(ticker.LogLocator(base=10, subs='all'))  # Set minor locator with all subs
plt.grid(True, which='both', color='gray', linestyle='--', linewidth=0.5)
#increase size of X axis labels
plt.xticks(fontsize=20)
plt.yticks(fontsize=20)

if(isISS):
    path = plot_path+'ISS/AS_'+now+name+'.png'
else:
    if(UseLossWeights):
        path = plot_path+'MC/AS_'+now+name+'_withW.png'
    else:
        path = plot_path+'MC/AS_'+now+name+'.png'
plt.savefig(path, bbox_inches='tight', dpi=300)  # Adjust dpi as needed

# ###  ROC curves
