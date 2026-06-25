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
import argparse
import sys
import os
import gc
import datetime

sys.path.append(str(Path(__file__).parent.parent))


from utils import override
from globals import g, g_AE
from scaler import MaskedStandardScaler
from utils import h5Manager
import MyAE

if (not g.TRAIN_ON_ISS and not g.IS_SIGNAL):
    from sklearn.preprocessing import label_binarize

device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")

#=====> Input arguments
parser = argparse.ArgumentParser(description="AE validation script")
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


#VALIDATION
if(g.TRAIN_ON_ISS):
    print('Working on ISS positives')
    #INPUT
    if(g.IS_OLD_RESULT):
        TRAINpath = g.DATASET_PATH+g.OLD_RES+"/ISSpos_dic.h5"
    else:
        TRAINpath = g.DATASET_PATH+"ISSpos_dic.h5"
    #OUTPUT
    if(g_AE.SWITCH):
        OUTpath  = g.VALDIC_PATH+"val_dic_AE_ISS_switch.h5"
        rOUTpath = g.NEW_FOLDER_PATH + now+'_trainOnISS_AE_ISS_scores_switch.root'
    else:
        OUTpath  = g.VALDIC_PATH+"val_dic_AE_ISS.h5"
        rOUTpath = g.NEW_FOLDER_PATH + now+'_trainOnISS_AE_ISS_scores1.root'
        rOUTpathTest = g.NEW_FOLDER_PATH + now+'_trainOnISS_AE_ISS_scores2.root'
    #DEBUG
    if g.DEBUG:
        MyH5man.SetInPath(in_path=TRAINpath)
        MyH5man.SetOutPath(out_path=OUTpath)
        MyH5man.SetMaxRows(max_rows=100000)
    else:
        MyH5man.SetInPath(in_path=TRAINpath)
        MyH5man.SetOutPath(out_path=OUTpath)
        MyH5man.SetStartRow(start_row=12000000)
    
else:
    print('Working on MC positives')
    #INPUT
    TRAINpath = g.DATASET_PATH+'He4.h5'
    #OUTPUT
    if(g_AE.AE_LOSSWEIGHTS):
        OUTpath  = g.VALDIC_PATH+"val_dic_AE_MC_withW.h5"
        rOUTpath = g.NEW_FOLDER_PATH + now+'_trainOnMC_withW_AE_MC_scores1.root'
        rOUTpathTest = g.NEW_FOLDER_PATH + now+'_trainOnMC_withW_AE_MC_scores2.root'
    else:
        OUTpath  = g.VALDIC_PATH+"val_dic_AE_MC.h5"
        rOUTpath = g.NEW_FOLDER_PATH + now+'_trainOnMC_AE_MC_scores1.root'
        rOUTpathTest = g.NEW_FOLDER_PATH + now+'_trainOnMC_AE_MC_scores2.root'
    #DEBUG
    if g.DEBUG:
        MyH5man.SetInPath(in_path=TRAINpath)
        MyH5man.SetOutPath(out_path=OUTpath)
        MyH5man.SetMaxRows(max_rows=100000)
    else:
        MyH5man.SetInPath(in_path=TRAINpath)
        MyH5man.SetOutPath(out_path=OUTpath)
        MyH5man.SetStartRow(start_row=12000000)
        MyH5man.SetMaxRows(max_rows=27000000)
        MyH5man.SetUseHe3(useHe3=True)
        MyH5man.SetHe3h5Name(He3h5Name="He3_21000.h5")
    
#Only validation
UnNormData = MyH5man.ReadH5toDataFrame()
UnNormData['orig_idx'] = np.arange(len(UnNormData), dtype=np.int64)
UnNormData = UnNormData.iloc[np.random.permutation(len(UnNormData))]
#=====> Selecting only the input features (+weights)
UnNormData.drop(columns=[col for col in UnNormData.columns if col not in g_AE.AE_COLUMNS], inplace=True)
if not g.TRAIN_ON_ISS:
    UnNormData.drop(columns=['weight'], inplace=True)
ValIndex = UnNormData['orig_idx'].values
UnNormData.drop(columns=['orig_idx'], inplace=True)
x_valUnNorm = UnNormData.values
ValCol = UnNormData.columns
del UnNormData
gc.collect()
#-------------------------------------------

#=====> transform validation  
x_val = scaler.transform(x_valUnNorm)
del x_valUnNorm
gc.collect()

#=====> Training and (ISS-MC) test loaders
valdataset   = MyAE.MyDatasetWithIdx(x_val,ValIndex)
val_loader = torch.utils.data.DataLoader(dataset=valdataset,
                                        batch_size=g_AE.AE_BATCH_SIZE,  
                                        shuffle=False,
                                        pin_memory=False)
print("x_val shape[0]:", x_val.shape[0], "x_val shape[1]:", x_val.shape[1])
del x_val
gc.collect()

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
dic['val_idx']=[]
sigmoid = nn.Sigmoid()

print('Processing positive TEST sample')
for i, (features,te_idx) in enumerate(val_loader):
    test_outputs = model(features.float())  
    errors = (test_outputs - features.float())**2
    mask = (features.float() != g.MYMASKVALUE) # True = valid, False = invalid (masked)
    errors = torch.sum(errors * mask, 1) / torch.sum(mask, 1)  # Normalize by features
    dic['output_error'].append(errors.detach().numpy())
    dic['anomaly_score'].append(sigmoid(errors).detach().numpy())
    dic['val_idx'].append(te_idx.detach().numpy())
    if (i+1) % (len(val_loader.dataset) // 10) == 0:
        print(f"Processed {((i+1)/len(val_loader.dataset))*100:.1f}% of val_loader")

del val_loader
gc.collect()
print('Output Errors: ', dic['output_error'][0])
print('Anomaly score: ', dic['anomaly_score'][0])
print('Len output_error: ', len(dic['output_error']))
print('Len anomaly_score: ', len(dic['anomaly_score']))
print('MyTestPosErrors:',dic['output_error'][0])
if(not g.TRAIN_ON_ISS):
    print('AS ISS positive validation[0]:',dic['anomaly_score'][0])
else:
    print('AS MC positive validation[0]:',dic['anomaly_score'][0])

#=====> Concatenate the dic elements in a single array
dic = MyH5man.CheckDicType(dic)

#=====> Appending the anomalys scores
dfValPos = MyH5man.ReadH5toDataFrame()
maskVal = np.zeros(len(dfValPos), dtype=bool)
maskVal[dic['val_idx']] = True
dfValPos.drop(index=dfValPos.index[~maskVal], inplace=True)
gc.collect()
#=====> Appending indexes, MSE and AS
dfValPos.loc[dic['val_idx'], 'anomaly_score'] = dic['anomaly_score']
dfValPos.loc[dic['val_idx'], 'MSE']           = dic['output_error']
dfValPos.loc[dic['val_idx'], 'index']         = dic['val_idx']

#=====>  Saving dictionary as root tree
print(f"Folder '{g.NEW_FOLDER_PATH}' created successfully.")
print(f"Full output path '{rOUTpath}")
with up.recreate(rOUTpath) as MyFile:
    if g.TRAIN_ON_ISS:
        MyFile["ISS"] = dfValPos
    else:
        MyFile["MC"] = dfValPos 
del dfValPos
gc.collect()
print(f"New tree with scores and discriminant at '{rOUTpath}")
print("End of validation moving to negative rigidities... ")






#TEST
if g.TRAIN_ON_ISS:
    if(g.IS_OLD_RESULT):
        TESTpath = g.DATASET_PATH+g.OLD_RES+"/ISSneg_dic.h5"
    else:
        TESTpath = g.DATASET_PATH+"ISSneg_dic.h5"
    MyH5man.SetInPath(TESTpath)
    MyH5man.SetStartRow(start_row=0)
else:
    TESTpath = g.DATASET_PATH+"He4cc.h5"
    MyH5man.SetInPath(in_path=TESTpath)
    MyH5man.SetHe3h5Name(He3h5Name="He3cc_21000.h5")
    MyH5man.SetIsCC(True)
    MyH5man.SetStartRow(start_row=0)
    MyH5man.SetMaxRows(max_rows=None)
    
    
TestDataset_UnNorm = MyH5man.ReadH5toDataFrame()
TestDataset_UnNorm['orig_idx'] = np.arange(len(TestDataset_UnNorm), dtype=np.int64)
TestDataset_UnNorm = TestDataset_UnNorm.iloc[np.random.permutation(len(TestDataset_UnNorm))]
#=====> Selecting only the input features (+weights)
TestDataset_UnNorm.drop(columns=[col for col in TestDataset_UnNorm.columns if col not in g_AE.AE_COLUMNS], inplace=True)
if not g.TRAIN_ON_ISS:
    TestDataset_UnNorm.drop(columns=['weight'], inplace=True)
val_idx = TestDataset_UnNorm['orig_idx'].values
TestDataset_UnNorm.drop(columns=['orig_idx'], inplace=True)
unnorm_test=TestDataset_UnNorm.values
TestCol = TestDataset_UnNorm.columns
del TestDataset_UnNorm
gc.collect()
#-------------------------------------------

#=====> transform test 
x_test = scaler.transform(unnorm_test)
del unnorm_test
gc.collect()

#=====> (ISS-MC) test loaders
testdataset  = MyAE.MyDatasetWithIdx(x_test,val_idx)
test_loader = torch.utils.data.DataLoader(dataset=testdataset,
                                            batch_size=g_AE.AE_BATCH_SIZE,  
                                            shuffle=False,
                                            pin_memory=False)
print("x_test shape[0]:", x_test.shape[0], "x_test shape[1]:", x_test.shape[1])
del x_test
gc.collect()

#=====> Evaluation 
model.eval()
dic={}
dic['NegOut_error']=[]
dic['NEGanomaly_score']=[]
dic['test_idx']=[]
sigmoid = nn.Sigmoid()

print('Processing negative TEST sample')
for i, (features,te_idx) in enumerate(test_loader):
    test_outputs = model(features.float())  
    errors = (test_outputs - features.float())**2
    mask = (features.float() != g.MYMASKVALUE) # True = valid, False = invalid (masked)
    errors = torch.sum(errors * mask, 1) / torch.sum(mask, 1)  # Normalize by features
    dic['NegOut_error'].append(errors.detach().numpy())
    dic['NEGanomaly_score'].append(sigmoid(errors).detach().numpy())
    dic['test_idx'].append(te_idx.detach().numpy())
    if (i+1) % (len(test_loader.dataset) // 10) == 0:
        print(f"Processed {((i+1)/len(test_loader.dataset))*100:.1f}% of test_loader")

del test_loader
gc.collect()
print('Output Errors: ', dic['NegOut_error'][0])
print('Anomaly score: ', dic['NEGanomaly_score'][0])
print('Len output_error: ', len(dic['NegOut_error']))
print('Len anomaly_score: ', len(dic['NEGanomaly_score']))
print('MyTestPosErrors:',dic['NegOut_error'][0])
if(not g.TRAIN_ON_ISS):
    print('AS ISS negatives test[0]:',dic['NEGanomaly_score'][0])
else:
    print('AS MC negatives test[0]:',dic['NEGanomaly_score'][0])

#=====> Concatenate the dic elements in a single array
dic = MyH5man.CheckDicType(dic)

#=====> Appending the anomalys scores
dfTestNeg = MyH5man.ReadH5toDataFrame()
maskVal = np.zeros(len(dfTestNeg), dtype=bool)
maskVal[dic['test_idx']] = True
dfTestNeg.drop(index=dfTestNeg.index[~maskVal], inplace=True)
gc.collect()
#=====> Appending indexes, MSE and AS
dfTestNeg.loc[dic['test_idx'], 'anomaly_score'] = dic['NEGanomaly_score']
dfTestNeg.loc[dic['test_idx'], 'MSE']           = dic['NegOut_error']
dfTestNeg.loc[dic['test_idx'], 'index']         = dic['test_idx']

#=====>  Saving dictionary as root tree
print(f"Folder '{g.NEW_FOLDER_PATH}' created successfully.")
print(f"Full output path '{rOUTpathTest}")
with up.recreate(rOUTpathTest) as MyFile:
    if g.TRAIN_ON_ISS:
        MyFile["ISSneg"] = dfTestNeg
    else:
        MyFile["MCneg"] = dfTestNeg 
del dfTestNeg
gc.collect()
print(f"Full output path '{rOUTpathTest}")

gc.collect()
print("Done!")
