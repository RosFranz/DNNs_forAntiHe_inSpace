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
plot_path   = '/eos/home-f/frrossi/AMS/ams_network/plot/CL2/'
isISS = True
UseWeights = False
UseLossWeights = False
name = '_2025_10_29_'
###############################################

def ReadH5toDic(file_path):
    group_name = 'dic'
    myDic = {}
    with h5py.File(file_path, 'r') as myF:
        group = myF[group_name]
        for dataset_name in group.keys():
            myDic[dataset_name] = group[dataset_name][:]
    return myDic

#get current date
import datetime
now = datetime.datetime.now()
print ("Current date and time : ")
print (now.strftime("%Y-%m-%d %H:%M:%S"))
now = now.strftime("%Y_%m_%d")
name = name+now

# ### Load saved dictonary
if(isISS):
     in_path = valDic_path+"val_dic_CL2_ISS.h5"
else:
    if(UseLossWeights):
        in_path = valDic_path+"val_dic_CL2_MC_withW.h5"
    else:
        in_path = valDic_path+"val_dic_CL2_MC.h5"
  
Restoredic = ReadH5toDic(in_path)
print(Restoredic.keys())

# ### Loss functions
plt.figure(figsize=(10, 8))
y_min = min(min(Restoredic['train_loss']), min(Restoredic['val_loss']))*0.95
y_max = max(max(Restoredic['train_loss']), max(Restoredic['val_loss']))*1.05
plt.ylim(y_min, y_max)

plt.tick_params(axis='both', which='major', labelsize=18)
plt.tick_params(axis='both', which='minor', labelsize=18)
plt.xlabel('Epochs', fontsize=30)
plt.ylabel('Loss value', fontsize=30)
plt.yscale('log')

epochs=len(Restoredic['train_loss'])
x_ticks = np.arange(0, epochs+10, 50.) 

min_val = min(min(Restoredic['train_loss']), min(Restoredic['val_loss']))
max_val = max(max(Restoredic['train_loss']), max(Restoredic['val_loss']))

plt.plot(range(epochs),Restoredic['train_loss'],label='trainining loss function', linewidth=3)
plt.plot(range(epochs),Restoredic['val_loss'],label='validation loss function', linewidth=3)
plt.legend(prop={'size': 30}, framealpha=0., loc='upper right')
plt.grid(True, which='both', color='gray', linestyle='--', linewidth=0.5)
plt.xticks(x_ticks)

if(isISS):
    savepath = plot_path+'ISS/Loss_'+name+'.png'
else:
    if UseLossWeights:
        savepath = plot_path+'MC/Loss_'+name+'_withW.png'
    else:
        savepath = plot_path+'MC/Loss_'+name+'.png'
plt.savefig(savepath, dpi=300)

# ### Accuracy
plt.figure(figsize=(10, 8))
accuracy = Restoredic['accuracy']
y_min = min(accuracy)*0.95
y_max = max(accuracy)*1.05
plt.ylim(y_min, y_max)

plt.tick_params(axis='both', which='major', labelsize=18)
plt.tick_params(axis='both', which='minor', labelsize=18)
plt.xlabel('Epochs', fontsize=30)
plt.ylabel('Accuracy', fontsize=30)

epochs=len(Restoredic['train_loss'])
x_ticks = np.arange(0, epochs+10, 50.) 

plt.plot(range(epochs),accuracy,label='Validation accuracy', linewidth=3)
plt.legend(prop={'size': 30}, framealpha=0., loc='lower right')
plt.grid(True, which='both', color='gray', linestyle='--', linewidth=0.5)
plt.xticks(x_ticks)
if(isISS):
    savepath = plot_path+'ISS/Accuracy_'+name+'.png'
else:
    savepath = plot_path+'MC/Accuracy_'+name+'.png'
plt.savefig(savepath, dpi=300)  
print(Restoredic.keys())

#Plotting scores
out = Restoredic['output']
true_out = Restoredic['output_true']
if(not isISS and UseWeights):
    pos_weights = Restoredic['weights']
    print(len(out))
    print(out)
    print(len(pos_weights))
    print(pos_weights)
plt.figure(figsize=(10, 8)) 

data_min = -0.05
data_max = 1.05
print('Data min', data_min, 'Data max', data_max)
bin_width = (data_max-data_min)/51.
bins = np.arange(data_min, data_max, bin_width)

plt.ylim(5.e-5, 5e+1)
if(not isISS and UseWeights):
    plt.hist(out[true_out==1], bins=bins, label='Good reco.', histtype='stepfilled',\
        weights=pos_weights[true_out==1], color='g', alpha=0.5, density=True)
    plt.hist(out[true_out==0], bins=bins, label='Bad reco.', histtype='stepfilled',\
        weights=pos_weights[true_out==0], color='r', alpha=0.5, density=True)
    pos_weights
else:
    plt.hist(out[true_out==1], bins=bins, label='Good reco.', histtype='stepfilled',\
        color='g', alpha=0.5, density=True)
    plt.hist(out[true_out==0], bins=bins, label='Bad reco.', histtype='stepfilled',\
        color='r', alpha=0.5, density=True)
    
plt.legend(prop={'size': 20}, framealpha=0., loc='upper left')

x_ticks = np.arange(0., 1.1, 0.1) 
plt.xticks(x_ticks)

plt.tick_params(axis='both', which='major', labelsize=18)
plt.xlabel('FCNN score', fontsize=30)
plt.ylabel('Arbitrary units', fontsize=30)
plt.yscale('log')

plt.grid(True, which='both', color='gray', linestyle='--', linewidth=0.5)
plt.gca().yaxis.set_major_locator(ticker.LogLocator(base=10))  # Set major locator
plt.gca().yaxis.set_minor_locator(ticker.LogLocator(base=10, subs='all'))  # Set minor locator with all subs

if(isISS):
    savepath = plot_path+'ISS/score_'+name+'.png'
else:
    if(UseLossWeights):
        savepath = plot_path+'MC/score_'+name+'_withW.png'
    else:
        savepath = plot_path+'MC/score_'+name+'.png'
plt.savefig(savepath, dpi=300)

 
f0=len(out[true_out==0])/len(true_out)
f1=len(out[true_out==1])/len(true_out)
print("Bad reco: ",f0,"Good reco: ",f1)
print("Bad > 0.70", len(out[(true_out==0)&(out>=0.70)])/len(out[true_out==0]))
print("Good > 0.70", len(out[(true_out==1)&(out>=0.70)])/len(out[true_out==1]))


# ###  ROC curves
GoodEff  = []
BadEff   = []
Tot_Good = len(true_out[true_out==1])
Tot_Bad  = len(true_out[true_out==0])
thresholds=np.arange(data_min, data_max, (data_max-data_min)/2.e+2)
for th in thresholds:
    good = len(out[(out>th)&(true_out==1)])
    bad  = len(out[(out>th)&(true_out==0)])
    GoodEff.append(good/Tot_Good)
    BadEff.append(bad/Tot_Bad)

plt.figure(figsize=(10, 8))
plt.xlabel('Good-reco efficiency', fontsize=20)
x_ticks = np.arange(0, 1.05, 0.1) 
plt.xticks(x_ticks)
y_ticks = np.arange(0., 1.1, 0.1) 
plt.yticks(y_ticks)
plt.tick_params(axis='both', which='major', labelsize=15)

plt.ylim(0, 1.05)
plt.ylabel('1- (bad-reco efficiency)', fontsize=20)

plt.plot(np.array(GoodEff), 1-np.array(BadEff), label='ROC curve', color='b', lw=3)
plt.legend(prop={'size': 30}, framealpha=0., loc='lower left')
plt.grid(True, which='both', color='gray', linestyle='--', linewidth=0.5)

if(isISS):
    savepath = plot_path+'ISS/ROC_'+name+'.png'
else:
    savepath = plot_path+'MC/ROC_'+name+'.png'
plt.savefig(savepath, dpi=300)

aucSo = np.trapz(np.array(GoodEff), 1-np.array(BadEff))
print('AUC good Reco', aucSo)


plt.close()
