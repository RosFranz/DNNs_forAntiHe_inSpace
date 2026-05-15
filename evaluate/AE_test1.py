import numpy as np
print("NumPy version:", np.__version__)
import uproot as up
print("Uproot version:", up.__version__)
import pandas as pd
from sklearn.preprocessing import StandardScaler
import torch
import torch.nn as nn
import argparse
import os
import gc
import datetime

from utils import override
from globals import g, g_AE
from scaler import MaskedStandardScaler
from utils import h5Manager
import MyAE

device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")

#=====> Input arguments
parser = argparse.ArgumentParser(description="AE testing script 1")
parser.add_argument("--config", type=str, default=None, help="Path to config file (JSON/YAML/YML)")
args = parser.parse_args()
override(args.config)


#=====> setting the custom scaler
if g.TRAIN_ON_ISS:
    MyH5man = h5Manager(in_path=g.VALDIC_PATH+"trainOnISS_masked_scaler_fit_AE.h5")
else:
    MyH5man = h5Manager(in_path=g.VALDIC_PATH+"trainOnMC_masked_scaler_fit_AE.h5")
DicScaler = MyH5man.ReadH5toDic(group_name='scaler_fit_values')
if g.USEMASK:
    scaler = MaskedStandardScaler(mask_value=g.MYMASKVALUE)
else:
    scaler = StandardScaler()
scaler.set_MeanStd(DicScaler['mean'], DicScaler['std'])
del DicScaler

#=====> Reading input h5 to pandas dataframe
now = datetime.datetime.now()
print ("Current date and time : ")
print (now.strftime("%Y-%m-%d %H:%M:%S"))
now = now.strftime("%Y_%m_%d") #save it in a string

g.NEW_FOLDER_PATH = g.NEW_FOLDER_PATH + now + '/'
os.makedirs(g.NEW_FOLDER_PATH, exist_ok=True)
print(f"Folder '{g.NEW_FOLDER_PATH}' created successfully.")

np.random.seed(g_AE.AE_SEED)
g_AE.AE_COLUMNS.append('orig_idx') 

if not g.TRAIN_ON_ISS: #TRAIN ON MC
    #OUTPUT root
    if(g_AE.AE_LOSSWEIGHTS):
        if g_AE.SWITCH:
            rOUTpath = g.NEW_FOLDER_PATH + now+ '_trainOnMC_withW_AE_ISS_scores_switch1.root'
        else:
            rOUTpath = g.NEW_FOLDER_PATH + now+'_trainOnMC_withW_AE_ISS_scores1.root'
    else:
        if g_AE.SWITCH:
            rOUTpath = g.NEW_FOLDER_PATH + now+'_trainOnMC_AE_ISS_scores_switch1.root'
        else:
            rOUTpath = g.NEW_FOLDER_PATH + now+'_trainOnMC_AE_ISS_scores1.root'
        
    print('Loading positive ISS dataset')    
    #INPUT
    if g.IS_OLD_RESULT:
        INpath = g.DATASET_PATH+ g.OLD_RES + 'ISSpos_dic.h5'
    else:
        INpath = g.DATASET_PATH+'ISSpos_dic.h5'
    #DEBUG
    if g.DEBUG:
        MyH5man.SetInPath(in_path=INpath)
        MyH5man.SetMaxRows(max_rows=100000)
    else:
        MyH5man.SetInPath(in_path=INpath)
        MyH5man.SetMaxRows(max_rows=10000000)
    
else: #TRAIN ON ISS-DATA
    #OUTPUT root
    if g_AE.SWITCH:
        rOUTpath = g.NEW_FOLDER_PATH + now+ '_trainOnISS_AE_MC_scores_switch1.root'
    else:
        rOUTpath = g.NEW_FOLDER_PATH + now+'_trainOnISS_AE_MC_scores1.root'
        
    print('Loading positive MC dataset')    
    #INPUT
    if g.IS_OLD_RESULT:
        INpath = g.DATASET_PATH+ g.OLD_RES + 'MCSIG_dic.h5'
    else:
        #INpath = g.DATASET_PATH+'MCSIG_dic.h5'
        INpath = g.DATASET_PATH+'He4.h5'
    #DEBUG
    if g.DEBUG:
        MyH5man.SetInPath(in_path=INpath)
        MyH5man.SetMaxRows(max_rows=100000)
    else:
        MyH5man.SetInPath(in_path=INpath)
        MyH5man.SetMaxRows(max_rows=10000000)
        MyH5man.SetHe3h5Name("He3_21000.h5")
        MyH5man.SetUseHe3(True)
    

TestPos_unNorm = MyH5man.ReadH5toDataFrame()
TestPos_unNorm['orig_idx'] = np.arange(len(TestPos_unNorm), dtype=np.int64)
TestPos_unNorm = TestPos_unNorm.iloc[np.random.permutation(len(TestPos_unNorm))]
#=====> Selecting only the input features
TestPos_unNorm.drop(columns=[col for col in TestPos_unNorm.columns if col not in g_AE.AE_COLUMNS], inplace=True)
TestPosIndex = TestPos_unNorm['orig_idx'].values
TestPos_unNorm.drop(columns=['orig_idx'], inplace=True)
XposUnNorm=TestPos_unNorm.values
TestPosCol=TestPos_unNorm.columns
del TestPos_unNorm
gc.collect()
#-------------------------------------------

#=====> transform   
Xpos = scaler.transform(XposUnNorm)
del XposUnNorm
gc.collect()

#=====> Test dataset and loader
testdataset    = MyAE.MyDatasetWithIdx(Xpos,TestPosIndex)
testPos_loader = torch.utils.data.DataLoader(dataset=testdataset,
                                batch_size=g_AE.AE_BATCH_SIZE, 
                                shuffle=False,
                                pin_memory=False)

#=====> Load Model
if not g.TRAIN_ON_ISS:
    if g_AE.AE_LOSSWEIGHTS:
        if g.IS_OLD_RESULT:
            g.MODEL_PATH = g.MODEL_PATH + g.OLD_RES + 'trained_model_AE_MC_withW.pth'
        else:
            g.MODEL_PATH = g.MODEL_PATH + 'trained_model_AE_MC_withW.pth'
    else:
        if g.IS_OLD_RESULT:
            g.MODEL_PATH = g.MODEL_PATH + g.OLD_RES + 'trained_model_AE_MC.pth'
        else:
            g.MODEL_PATH = g.MODEL_PATH + 'trained_model_AE_MC.pth'
else:
    if g_AE.SWITCH:
        if g.IS_OLD_RESULT:
            g.MODEL_PATH = g.MODEL_PATH + g.OLD_RES + 'trained_model_AE_ISS_switch.pth'
        else:
            g.MODEL_PATH = g.MODEL_PATH + 'trained_model_AE_ISS_switch.pth'
    else:
        if g.IS_OLD_RESULT:
            g.MODEL_PATH = g.MODEL_PATH + g.OLD_RES + 'trained_model_AE_ISS.pth'
        else:
            g.MODEL_PATH = g.MODEL_PATH + 'trained_model_AE_ISS.pth'

if g.USEMASK:
    model = MyAE.maskedAE(g_AE.AE_INPUT_SIZE, g_AE.AE_LAYERS, mask_value=g.MYMASKVALUE).to(device)
else:
    model = MyAE.AE(g_AE.AE_INPUT_SIZE, g_AE.AE_LAYERS).to(device)
print("Model path: ", g.MODEL_PATH)
model.load_state_dict(torch.load(g.MODEL_PATH))


#=====> Evaluation 
model.eval()
dic={}
dic['output_error']=[]
dic['anomaly_score']=[]
dic['test_idx']=[]
sigmoid = nn.Sigmoid()

print('Processing positive TEST sample')
for i, (features,te_idx) in enumerate(testPos_loader):
    test_outputs = model(features.float())  
    errors = (test_outputs - features.float())**2
    mask = (features.float() != g.MYMASKVALUE) # True = valid, False = invalid (masked)
    errors = torch.sum(errors * mask, 1) / torch.sum(mask, 1)  # Normalize by features
    dic['output_error'].append(errors.detach().numpy())
    dic['anomaly_score'].append(sigmoid(errors).detach().numpy())
    dic['test_idx'].append(te_idx.detach().numpy())
    if (i+1) % (len(testPos_loader.dataset) // 10) == 0:
        print(f"Processed {((i+1)/len(testPos_loader.dataset))*100:.1f}% of testPos_loader")

del testPos_loader
gc.collect()
print('Output Errors: ', dic['output_error'][0])
print('Anomaly score: ', dic['anomaly_score'][0])
print('Len output_error: ', len(dic['output_error']))
print('Len anomaly_score: ', len(dic['anomaly_score']))
print('MyTestPosErrors:',dic['output_error'][0])
if(not g.TRAIN_ON_ISS):
    print('AS ISS positive test[0]:',dic['anomaly_score'][0])
else:
    print('AS MC positive test[0]:',dic['anomaly_score'][0])

#=====> Concatenate the dic elements in a single array
dic = MyH5man.CheckDicType(dic)

#=====> Appending the anomalys scores
dfTestPos = MyH5man.ReadH5toDataFrame()
maskVal = np.zeros(len(dfTestPos), dtype=bool)
maskVal[dic['test_idx']] = True
dfTestPos.drop(index=dfTestPos.index[~maskVal], inplace=True)
gc.collect()
#=====> Appending indexes, MSE and AS
dfTestPos.loc[dic['test_idx'], 'anomaly_score'] = dic['anomaly_score']
dfTestPos.loc[dic['test_idx'], 'MSE']           = dic['output_error']
dfTestPos.loc[dic['test_idx'], 'index']         = dic['test_idx']

#=====>  Saving dictionary as root tree
print(f"Folder '{g.NEW_FOLDER_PATH}' created successfully.")
print(f"Full output path '{rOUTpath}")
with up.recreate(rOUTpath) as MyFile:
    if not g.TRAIN_ON_ISS:
        MyFile["ISS"] = dfTestPos
    else:
        MyFile["MC"] = dfTestPos 
del dfTestPos
gc.collect()
print(f"New tree with scores and discriminant at '{rOUTpath}")
print("End of the script!")
