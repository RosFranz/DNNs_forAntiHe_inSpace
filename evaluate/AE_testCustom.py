import numpy as np
print("NumPy version:", np.__version__)
import uproot as up
print("Uproot version:", up.__version__)
from sklearn.preprocessing import StandardScaler
import torch
import torch.nn as nn
import argparse
import os
import gc
import datetime
import sys

sys.path.append(str(Path(__file__).parent.parent))

from utils import override
from globals import g, g_AE
from scaler import MaskedStandardScaler
from utils import h5Manager
import MyAE

device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")

#=====> Input arguments
parser = argparse.ArgumentParser(description="AE testing on custom MC sample")
parser.add_argument("--config", type=str, default=None, help="Path to config file (JSON/YAML/YML)")
args = parser.parse_args()
override(args.config)

if g.TRAIN_ON_ISS:
    print("TO BE USED ONLY WITH MC TRAINED MODELS")
    sys.exit(1)

#=====> setting the custom scaler
MyH5man = h5Manager(in_path=g.VALDIC_PATH+"trainOnMC_masked_scaler_fit_AE.h5")
DicScaler = MyH5man.ReadH5toDic(group_name='scaler_fit_values')
if g.USEMASK:
    scaler = MaskedStandardScaler(mask_value=g.MYMASKVALUE)
else:
    scaler = StandardScaler()
scaler.set_MeanStd(DicScaler['mean'], DicScaler['std'])

#=====> Reading input h5 to pandas dataframe
now = datetime.datetime.now()
print ("Current date and time : ")
print (now.strftime("%Y-%m-%d %H:%M:%S"))
now = now.strftime("%Y_%m_%d") #save it in a string

g.NEW_FOLDER_PATH = g.NEW_FOLDER_PATH + now + '/'
os.makedirs(g.NEW_FOLDER_PATH, exist_ok=True)
print(f"Folder '{g.NEW_FOLDER_PATH}' created successfully.")


#TRAIN ON MC
#OUTPUT root
if(g_AE.AE_LOSSWEIGHTS):
    if g_AE.SWITCH:
        rOUTpath = g.NEW_FOLDER_PATH + now+ '_trainOnMC_withW_AE_'+g.CUSTOM_ROOT_NAME+'_scores_switch.root'
    else:
         rOUTpath = g.NEW_FOLDER_PATH + now+'_trainOnMC_withW_AE_'+g.CUSTOM_ROOT_NAME+'_scores.root'
else:
    if g_AE.SWITCH:
        rOUTpath = g.NEW_FOLDER_PATH + now+'_trainOnMC_AE_'+g.CUSTOM_ROOT_NAME+'_scores_switch2.root'
    else:
        rOUTpath = g.NEW_FOLDER_PATH + now+'_trainOnMC_AE_'+g.CUSTOM_ROOT_NAME+'_scores.root'
#=====> Load custom h5 MC sample
print('Loading negative Custom MC dataset ', g.CUSTOM_H5_TEST_NAME)    
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
    
TestNeg_unNorm = MyH5man.ReadH5toDataFrame()
TestNeg_unNorm['orig_idx'] = np.arange(len(TestNeg_unNorm), dtype=np.int64)
g_AE.AE_COLUMNS.append('orig_idx') 
g_AE.AE_COLUMNS.remove('weight') 
#=====> Selecting only the input features
TestNeg_unNorm.drop(columns=[col for col in TestNeg_unNorm.columns if col not in g_AE.AE_COLUMNS], inplace=True)
TestNegIndex = TestNeg_unNorm['orig_idx'].values
TestNeg_unNorm.drop(columns=['orig_idx'], inplace=True)
XnegUnNorm=TestNeg_unNorm.values
TestNegCol=TestNeg_unNorm.columns
del TestNeg_unNorm
gc.collect()
#-------------------------------------------

#=====> transform   
Xneg = scaler.transform(XnegUnNorm)
del XnegUnNorm
gc.collect()

#=====> Test dataset and loader
testdataset    = MyAE.MyDatasetWithIdx(Xneg,TestNegIndex)
testNeg_loader = torch.utils.data.DataLoader(dataset=testdataset,
                                        batch_size=g_AE.AE_BATCH_SIZE,  
                                        shuffle=False,
                                        pin_memory=False)

#=====> Load Model
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
      
print('Processing Custom MC TEST sample')
for i, (sample,te_idx) in enumerate(testNeg_loader):
    test_outputs = model(sample.float())  
    errors = (test_outputs - sample.float())**2
    mask = (sample.float() != g.MYMASKVALUE) # True = valid, False = invalid (masked)
    errors = torch.sum(errors * mask, 1) / torch.sum(mask, 1)  # Normalize by features
    dic['output_error'].append(errors.detach().numpy())
    dic['anomaly_score'].append(sigmoid(errors).detach().numpy())
    dic['test_idx'].append(te_idx.detach().numpy())
    if (i+1) % (len(testNeg_loader.dataset) // 10) == 0:
        print(f"Processed {((i+1)/len(testNeg_loader.dataset))*100:.1f}% of testNeg_loader")
        
del testdataset, testNeg_loader
gc.collect()

print('Output Errors: ', dic['output_error'][0])
print('Anomaly score: ', dic['anomaly_score'][0])
print('Len output_error: ', len(dic['output_error']))
print('Len anomaly_score: ', len(dic['anomaly_score']))
print('MyTestPosErrors:',dic['output_error'][0])
print('AS MC negative ', g.CUSTOM_H5_TEST_NAME, ' test[0]:',dic['anomaly_score'][0])

#=====> Concatenate the dic elements in a single array
dic = MyH5man.CheckDicType(dic)

#=====> Appending the anomalys scores
dfTestNeg = MyH5man.ReadH5toDataFrame()
maskVal = np.zeros(len(dfTestNeg), dtype=bool)
maskVal[dic['test_idx']] = True
dfTestNeg.drop(index=dfTestNeg.index[~maskVal], inplace=True)
gc.collect()
#=====> Appending indexes, MSE and AS
dfTestNeg.loc[dic['test_idx'], 'anomaly_score'] = dic['anomaly_score']
dfTestNeg.loc[dic['test_idx'], 'MSE']           = dic['output_error']
dfTestNeg.loc[dic['test_idx'], 'index']         = dic['test_idx']

#=====>  Saving dictionary as root tree
print(f"Folder '{g.NEW_FOLDER_PATH}' created successfully.")
print(f"Full output path '{rOUTpath}")
with up.recreate(rOUTpath) as MyFile:
    MyFile[g.CUSTOM_ROOT_NAME] = dfTestNeg
del dfTestNeg
gc.collect()
print(f"New tree with scores and discriminant at '{rOUTpath}")
print("End of the script!")
