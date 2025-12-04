# ATTENTION
# ------------------------------------
#This script works
#if you trained your models on MC
# ------------------------------------
import numpy as np
print("NumPy version:", np.__version__)
import uproot as up
print("Uproot version:", up.__version__)
from sklearn.preprocessing import StandardScaler
import torch
import argparse
import os
import gc
import datetime

from utils import override
from globals import g, g_CL2
from scaler import MaskedStandardScaler
from utils import h5Manager
import CL2

device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")

#=====> Input arguments
parser = argparse.ArgumentParser(description="CL on ISS2 testing script")
parser.add_argument("--config", type=str, default=None, help="Path to config file (JSON/YAML/YML)")
args = parser.parse_args()
override(args.config)

#=====> setting seed for permutations
np.random.seed(g_CL2.CL2_SEED)

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

print('Loading negative ISS dataset')
#INPUT
if g.IS_OLD_RESULT:
    INpath = g.DATASET_PATH+ g.OLD_RES + 'ISSneg_dic.h5'
else:
    INpath = g.DATASET_PATH+'ISSneg_dic.h5'
#OUTPUT
if(g_CL2.CL2_LOSSWEIGHTS):
    rOUTpath = g.NEW_FOLDER_PATH + now+'_trainOnMC_withW_2CL_ISS_scores2.root'
else:
    rOUTpath = g.NEW_FOLDER_PATH + now+'_trainOnMC_2CL_ISS_scores2.root'
#DEBUG
if g.DEBUG:
    MyH5man.SetInPath(in_path=INpath)
    MyH5man.SetMaxRows(max_rows=100000)
else:
    MyH5man.SetInPath(in_path=INpath)
ISSneg = MyH5man.ReadH5toDataFrame()
ISSneg['orig_idx'] = np.arange(len(ISSneg), dtype=np.int64)
g_CL2.CL2_COLUMNS.append('orig_idx')
label_neg = ISSneg['RigLabel'].values
ISSneg.drop(columns=[col for col in ISSneg.columns if col not in g_CL2.CL2_COLUMNS], inplace=True)
print('Input features: ', ISSneg.columns)  
test_idx = ISSneg['orig_idx'].values
ISSneg.drop(columns=['orig_idx'], inplace=True)
X=ISSneg.values
del ISSneg
gc.collect()

test_sample  = scaler.transform(X)
test_dataset = CL2.myDataset(test_sample,test_idx,label_neg)
test_loader=torch.utils.data.DataLoader(dataset=test_dataset,
                                        batch_size=g_CL2.CL2_BATCH_SIZE,
                                        shuffle=False)

#=====> Load Model
if g_CL2.CL2_LOSSWEIGHTS:
    if g.IS_OLD_RESULT:
        g.MODEL_PATH = g.MODEL_PATH + g.OLD_RES + 'trained_model_CL2_MC_withW.pth'
    else:
        g.MODEL_PATH = g.MODEL_PATH + 'trained_model_CL2_MC_withW.pth'
else:
    if g.IS_OLD_RESULT:
        g.MODEL_PATH = g.MODEL_PATH + g.OLD_RES + 'trained_model_CL2_MC.pth'
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
print('Processing negative sample')
for i, (features, te_idx, MyLabels) in enumerate(test_loader):
    out = model(features.float().to(device))
    out = out.squeeze(-1)
    dic['scores'].append(out.detach().numpy())
    dic['test_idx'].append(te_idx.detach().numpy())
    if (i+1) % (len(test_loader.dataset) // 10) == 0:
        print(f"Processed {((i+1)/len(test_loader.dataset))*100:.1f}% of test_loader")

dic = MyH5man.CheckDicType(dic)
# output=torch.cat([torch.tensor(arr) for arr in dic['scores']]).cpu().detach().numpy()

dfTest = MyH5man.ReadH5toDataFrame()
maskVal = np.zeros(len(dfTest), dtype=bool)
maskVal[dic['test_idx']] = True
dfTest.drop(index=dfTest.index[~maskVal], inplace=True)
gc.collect()
#=====> Appending network output
dfTest.loc[dic['test_idx'], 'scores']   = dic['scores']
dfTest.loc[dic['test_idx'], 'test_idx'] = dic['test_idx']


#=====>  Saving dictionary as root tree
print(f"Folder '{g.NEW_FOLDER_PATH}' created successfully.")
print(f"Full output path '{rOUTpath}")
print('Number of saved columns', len(dfTest.columns))
with up.recreate(rOUTpath) as MyFile:
    MyFile["ISS_neg"] = dfTest
    del dfTest
    gc.collect()

print(f"New tree with scores and discriminant at '{rOUTpath}")
print("End of the script!")