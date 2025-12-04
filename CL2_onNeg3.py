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

#=====> setting seed for permutations
np.random.seed(g_AE.AE_SEED)

#=====> setting the custom scaler
if g.TRAIN_ON_ISS:
    MyH5man = h5Manager(in_path=g.VALDIC_PATH+"trainOnISS_masked_scaler_fit_CL2.h5")
else:
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

#=====> Load the same h5 as AE
if(g.TRAIN_ON_ISS):
    print('Loading positive ISS-data for testing as for AE')
    #INPUT
    if(g.IS_OLD_RESULT):
        INpath = g.DATASET_PATH+g.OLD_RES+"/ISSpos_dic.h5"
    else:
        INpath = g.DATASET_PATH+"ISSpos_dic.h5"
    #OUTPUT
    rOUTpath = g.NEW_FOLDER_PATH + now+'_trainOnISS_2CL_ISS_scores3.root'
    #DEBUG
    if g.DEBUG:
        MyH5man.SetInPath(in_path=INpath)
        MyH5man.SetMaxRows(max_rows=100000)
    else:
        MyH5man.SetInPath(in_path=INpath) ##manager for h5 files
        MyH5man.SetStartRow(start_row=12000000)
else:
    print('Loading positive MC for testing as for AE')
    #INPUT
    #INpath = g.DATASET_PATH+'MCSIG_dic.h5'
    INpath = g.DATASET_PATH+'He4.h5'
    #OUTPUT
    if(g_CL2.CL2_LOSSWEIGHTS):
        rOUTpath = g.NEW_FOLDER_PATH + now+'_trainOnMC_withW_2CL_MC_scores3.root'
    else:
        rOUTpath = g.NEW_FOLDER_PATH + now+'_trainOnMC_2CL_MC_scores3.root'
    #DEBUG
    if g.DEBUG:
        MyH5man.SetInPath(in_path=INpath)
        MyH5man.SetMaxRows(max_rows=100000)
    else:
        MyH5man.SetInPath(in_path=INpath)
        MyH5man.SetHe3h5Name("He3_21000.h5")
        MyH5man.SetUseHe3(True)
        MyH5man.SetStartRow(start_row=12000000)
        MyH5man.SetMaxRows(max_rows=27000000)
print('I am cutting positives MC in REC RIG below MDR = 200 GV')
PosFrame = MyH5man.ReadH5toDataFrame()
PosFrame['orig_idx'] = np.arange(len(PosFrame), dtype=np.int64)
label_pos = PosFrame['RigLabel'].values
PosFrame = PosFrame[PosFrame['Rinner']<200]
PosFrame = PosFrame.iloc[np.random.permutation(len(PosFrame))]
g_CL2.CL2_COLUMNS.append('orig_idx') 
#=====> Selecting only the input features (+weights)
PosFrame.drop(columns=[col for col in PosFrame.columns if col not in g_CL2.CL2_COLUMNS], inplace=True)
TrainValIndex = PosFrame['orig_idx'].values
PosFrame.drop(columns=['orig_idx'], inplace=True)
Xpos=PosFrame.values
TrainValCol = PosFrame.columns
print(f"Input Xpos type: {type(Xpos)} dtype: {Xpos.dtype}, shape: {Xpos.shape}, shape label: {label_pos.shape}")
del PosFrame
gc.collect()
#-------------------------------------------

if not g.TRAIN_ON_ISS:
    print('Removing weights from the datasets')
    Xpos = np.delete(Xpos, TrainValCol.get_loc('weight'), axis=1)

test_pos_sample  = scaler.transform(Xpos)
del Xpos
gc.collect()

test_dataset = CL2.myDataset(test_pos_sample, TrainValIndex, label_pos)
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
print('Processing the same positive features as AE')
for i, (features,te_idx,MyLabels) in enumerate(test_loader):
    out = model(features.float().to(device))
    out = out.squeeze(-1)
    dic['scores'].append(out.detach().numpy())
    dic['test_idx'].append(te_idx.detach().numpy())
    if (i+1) % (len(test_loader.dataset) // 10) == 0:
        print(f"Processed {((i+1)/len(test_loader.dataset))*100:.1f}% of test_loader")

del test_dataset, test_loader
gc.collect()

dic = MyH5man.CheckDicType(dic)
# output=torch.cat([torch.tensor(arr) for arr in dic['scores']]).cpu().detach().numpy()

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
    if(g.TRAIN_ON_ISS):
        MyFile["ISS_pos"] = dfAE
    else:
        MyFile["MC_pos"] = dfAE
del dfAE
gc.collect()

print(f"New tree with scores and discriminant at '{rOUTpath}")
print("End of the script!")
