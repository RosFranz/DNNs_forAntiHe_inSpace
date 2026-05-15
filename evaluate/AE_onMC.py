import numpy as np
print("NumPy version:", np.__version__)
import uproot as up
print("Uproot version:", up.__version__)
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
import torch
import torch.nn as nn
from torch.utils.data import Dataset
from statistics import mean
import sys
import os
import gc
import datetime

from globals import g, g_AE
from scaler import MaskedStandardScaler
from utils import h5Manager
import MyAE

if (not g.TRAIN_ON_ISS and not g.IS_SIGNAL):
    from sklearn.preprocessing import label_binarize

device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")

#=====> setting the custom scaler
# MyH5man = h5Manager(in_path=g.VALDIC_PATH+"masked_scaler_fit_AE.h5")
# DicScaler = MyH5man.ReadH5toDic(group_name='scaler_fit_values')
if g.USEMASK:
    scaler = MaskedStandardScaler(mask_value=g.MYMASKVALUE)
else:
    scaler = StandardScaler()
# scaler.set_MeanStd(DicScaler['mean'], DicScaler['std'])
trainMean = np.array([2.04118861e-01,  7.25989957e+00, -1.20194000e-02, -1.18331573e-04,
                      1.16795393e-01,  1.15435091e-01,  2.98802121e-02,  4.75306401e-02,
                      7.46441187e-02,  7.63874801e-02,  9.58940696e-03, -1.16180319e-04,
                      5.19927168e-03,  4.54801334e-03,  6.68919878e-05,  1.19675546e-04,
                      -4.61010536e-04, -6.77187868e-04, -4.79893209e-03, -2.43292836e-04,
                      -5.47938719e-03], dtype=np.float64)
trainStd = np.array([0.61755042, 0.73946145, 0.08987303, 0.23174316, 0.37221897, 0.37682078,
                     0.50681291, 0.51873472, 0.52664006, 0.52781614, 0.01628694, 0.00594292,
                     0.00872647, 0.00887765, 0.00217011, 0.00148331, 0.00449538, 0.01043804,
                     0.01138303, 0.00749351, 0.00966885], dtype=np.float64)
scaler.set_MeanStd(trainMean, trainStd)

#=====> Initialize root output folder
now = datetime.datetime.now()
print ("Current date and time : ")
print (now.strftime("%Y-%m-%d %H:%M:%S"))
now = now.strftime("%Y_%m_%d") #save it in a string

g.NEW_FOLDER_PATH = g.NEW_FOLDER_PATH + now + '/'
os.makedirs(g.NEW_FOLDER_PATH, exist_ok=True)
print(f"Folder '{g.NEW_FOLDER_PATH}' created successfully.")

#=====> Reading Training+Validation (R>0) h5 to pandas dataframe
print('Working on MC positives') 
#INPUT
#INpath = g.DATASET_PATH+'MCSIG_dic.h5'
INpath = g.DATASET_PATH+'He4.h5'
#OUTPUT
if(g_AE.AE_LOSSWEIGHTS):
    rOUTpath = g.NEW_FOLDER_PATH + now+'_trainOnMC_withW_AE_MC_scores.root'
else:
    rOUTpath = g.NEW_FOLDER_PATH + now+'_trainOnMC_AE_MCscores.root'
#DEBUG
if g.DEBUG:
    MyH5man = h5Manager(in_path=INpath, max_rows=100000)
else:
    MyH5man = h5Manager(in_path=INpath)

#Training + validation
UnNormData = MyH5man.ReadH5toDataFrame()
print('I am cutting positives MC in REC RIG below MDR = 200 GV')
UnNormData = UnNormData[UnNormData['Rinner']<200]
gc.collect()
UnNormData=UnNormData.sample(frac=1, random_state=g_AE.AE_SEED)

#=====> Create the R>0 (validation) dic for future tree
X=UnNormData.values
x_trainUnNorm, x_valUnNorm = train_test_split(X, test_size=g_AE.TEST_FRACTION, random_state=g_AE.AE_SEED)
if g_AE.SWITCH:
    UnNormVal = pd.DataFrame(x_trainUnNorm, columns=UnNormData.columns)
else:
    UnNormVal = pd.DataFrame(x_valUnNorm, columns=UnNormData.columns)
Pos_info = {col: UnNormVal[col].values for col in UnNormVal.columns}
del UnNormData, UnNormVal, X, x_trainUnNorm, x_valUnNorm
gc.collect()

#=====> Restoring Validation dictionary
print('Restoring validation dictionary')
if g_AE.AE_LOSSWEIGHTS:
    if g.IS_OLD_RESULT:
        INpath = g.VALDIC_PATH + g.OLD_RES +"/val_dic_AE_MC_withW.h5"
    else:
        INpath = g.VALDIC_PATH + "val_dic_AE_MC_withW.h5"
else:
    if g.IS_OLD_RESULT:
        INpath = g.VALDIC_PATH + g.OLD_RES +"/val_dic_AE_MC.h5"
    else:
        INpath = g.VALDIC_PATH + "val_dic_AE_MC.h5"
MyH5man.SetInPath(INpath)
Restoredic = MyH5man.ReadH5toDic()

#=====> Appending network output
Pos_info['anomaly_score'] = Restoredic['anomaly_score']
Pos_info['MSE'] = Restoredic['output_error']



#=====> Reading test (R<0) h5 to pandas dataframe
INpath = g.DATASET_PATH+"He4cc.h5"
MyH5man.SetInPath(in_path=INpath)
TestDataset_UnNorm = MyH5man.ReadH5toDataFrame()
print('I am cutting negatives MC in REC RIG below MDR = -200 GV')
TestDataset_UnNorm = TestDataset_UnNorm[TestDataset_UnNorm['Rinner']>-200]
gc.collect()
TestDataset_UnNorm=TestDataset_UnNorm.sample(frac=1, random_state=g_AE.AE_SEED)
gc.collect()

#=====> Create the R<0 (test) dic for future tree
#=====> Appending network output
Test_info = {col: TestDataset_UnNorm[col].values for col in TestDataset_UnNorm.columns}
Test_info['anomaly_score'] = Restoredic['anomaly_score']
Test_info['MSE'] = Restoredic['NegOut_error']



#=====> Saving root trees
print(f"Full output path '{rOUTpath}")
with up.recreate(rOUTpath) as MyFile:
    if(g.TRAIN_ON_ISS):
        MyFile["ISS"] = Pos_info
        del Pos_info
        MyFile["ISSneg"] = Test_info
        del Test_info
    else:
        MyFile["MC"] = Pos_info
        del Pos_info
        MyFile["MCneg"] = Test_info
        del Test_info
gc.collect()
print("Done!")
