import numpy as np
print("NumPy version:", np.__version__)
import uproot as up
print("Uproot version:", up.__version__)
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
import torch
import argparse
import os
import gc
import datetime
import sys

from utils import override
from globals import g, g_CL2, g_AE
from scaler import MaskedStandardScaler
from utils import h5Manager
import CL2

device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")

#=====> Input arguments
parser = argparse.ArgumentParser(description="CL2 training script")
parser.add_argument("--config", type=str, default=None, help="Path to config file (JSON/YAML/YML)")
args = parser.parse_args()
override(args.config)

if g.TRAIN_ON_ISS:
    print("TO BE USED ONLY WITH MC TRAINED MODELS")
    sys.exit(1)

#=====> setting the custom scaler
MyH5man = h5Manager(in_path=g.VALDIC_PATH+"trainOnMC_masked_scaler_fit_CL2.h5")
DicScaler = MyH5man.ReadH5toDic(group_name='scaler_fit_values')
if g.USEMASK:
    scaler = MaskedStandardScaler(mask_value=g.MYMASKVALUE)
else:
    scaler = StandardScaler()
scaler.set_MeanStd(DicScaler['mean'], DicScaler['std'])

#=====> Initialize the H5 manager
#=====> Reading input h5 to pandas dataframe
now = datetime.datetime.now()
print ("Current date and time : ")
print (now.strftime("%Y-%m-%d %H:%M:%S"))
now = now.strftime("%Y_%m_%d") #save it in a string

g.NEW_FOLDER_PATH = g.NEW_FOLDER_PATH + now + '/'
os.makedirs(g.NEW_FOLDER_PATH, exist_ok=True)
print(f"Folder '{g.NEW_FOLDER_PATH}' created successfully.")

#TRAINED on MC
#OUTPUT
if(g_CL2.CL2_LOSSWEIGHTS):
    rOUTpath = g.NEW_FOLDER_PATH + now+'_trainOnMC_withW_2CL_'+g.CUSTOM_ROOT_NAME+'_scores.root'
else:
    rOUTpath = g.NEW_FOLDER_PATH + now+'_trainOnMC_2CL_'+g.CUSTOM_ROOT_NAME+'_scores.root'
#=====> Load the same h5 as AE
print('Loading negative Custom MC dataset for testing as for AE', g.CUSTOM_H5_TEST_NAME)
#INPUT
if g.IS_OLD_RESULT:
    INpath = g.DATASET_PATH+ g.OLD_RES + g.CUSTOM_H5_TEST_NAME + '.h5'
else:
    INpath = g.DATASET_PATH+ g.CUSTOM_H5_TEST_NAME + '.h5'
#DEBUG
if g.DEBUG:
    MyH5man.SetInPath(in_path=INpath)
    MyH5man.SetMaxRows(max_rows=100000)
else:
    MyH5man.SetInPath(in_path=INpath)
    
print('I am cutting negatives REC RIG below MDR = 200 GV')
TestNeg_unNorm = MyH5man.ReadH5toDataFrame()
TestNeg_unNorm['orig_idx'] = np.arange(len(TestNeg_unNorm), dtype=np.int64)
label_neg = TestNeg_unNorm['RigLabel'].values
TestNeg_unNorm = TestNeg_unNorm[TestNeg_unNorm['Rinner']>-200]
g_CL2.CL2_COLUMNS.append('orig_idx') 
g_CL2.CL2_COLUMNS.remove('weight') 
#=====> Selecting only the input features (+weights)
TestNeg_unNorm.drop(columns=[col for col in TestNeg_unNorm.columns if col not in g_CL2.CL2_COLUMNS], inplace=True)
TrainValIndex = TestNeg_unNorm['orig_idx'].values
TestNeg_unNorm.drop(columns=['orig_idx'], inplace=True)
XnegUnNorm=TestNeg_unNorm.values
TestNegCol = TestNeg_unNorm.columns
del TestNeg_unNorm
gc.collect()
#-------------------------------------------

test_pos_sample  = scaler.transform(XnegUnNorm)
del XnegUnNorm
gc.collect()

test_dataset = CL2.myDataset(test_pos_sample, TrainValIndex, label_neg)
test_loader=torch.utils.data.DataLoader(dataset=test_dataset,
                                        batch_size=g_CL2.CL2_BATCH_SIZE,
                                        shuffle=False)

#=====> Load Model
if g.TRAIN_ON_ISS:
    g.MODEL_PATH = g.MODEL_PATH + 'trained_model_CL2_ISS.pth'
    if g.IS_OLD_RESULT:
        g.MODEL_PATH = g.MODEL_PATH + g.OLD_RES + 'trained_model_CL2_ISS.pth'
else:
    if g_CL2.CL2_LOSSWEIGHTS:
        g.MODEL_PATH = g.MODEL_PATH + 'trained_model_CL2_MC_withW.pth'
    else:
        g.MODEL_PATH = g.MODEL_PATH + 'trained_model_CL2_MC.pth'

if g.USEMASK:
    model = CL2.maskedNeuralNet(g_CL2.CL2_INPUT_SIZE, g_CL2.CL2_LAYERS, g_CL2.DROPOUT, mask_value=g.MYMASKVALUE).to(device)
else:
    model = CL2.NeuralNet(g_CL2.CL2_INPUT_SIZE, g_CL2.CL2_LAYERS, g_CL2.DROPOUT).to(device)
model.load_state_dict(torch.load(g.MODEL_PATH))

#=====> Evaluation 
dic={}
dic['scores']=[]
dic['test_idx']=[]
model.eval()
print('Processing Custom MC TEST sample, same as AE')
for i, (features,te_idx,MyLabels) in enumerate(test_loader):
    out = model(features.float().to(device))
    out = out.squeeze(-1)
    dic['scores'].append(out.detach().numpy())
    dic['test_idx'].append(te_idx.detach().numpy())
    if (i+1) % (len(test_loader.dataset) // 10) == 0:
        print(f"Processed {((i+1)/len(test_loader.dataset))*100:.1f}% of test_loader")

del test_dataset, test_loader
gc.collect()

#=====> Concatenate the dic elements in a single array
dic = MyH5man.CheckDicType(dic)

#Attaching scores to the negative dictionary
dfAE = MyH5man.ReadH5toDataFrame()
maskVal = np.zeros(len(dfAE), dtype=bool)
maskVal[dic['test_idx']] = True
dfAE.drop(index=dfAE.index[~maskVal], inplace=True)
gc.collect()
#=====> Appending scores and indexes
dfAE.loc[dic['test_idx'],'index'] = dic['test_idx']
dfAE.loc[dic['test_idx'],'scores'] = dic['scores']

#=====>  Saving dictionary as root tree
print(f"Folder '{g.NEW_FOLDER_PATH}' created successfully.")
print(f"Full output path '{rOUTpath}")
print('Number of saved columns', len(dfAE.columns))
with up.recreate(rOUTpath) as MyFile:
    MyFile[g.CUSTOM_ROOT_NAME] = dfAE
del dfAE
gc.collect()
print(f"New tree with scores and discriminant at '{rOUTpath}")
print("End of the script!")
