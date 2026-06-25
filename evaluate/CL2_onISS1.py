# ATTENTION
# ------------------------------------
#This script works
#if you trained your models on MC
# ------------------------------------
import numpy as np
print("NumPy version:", np.__version__)
import uproot as up
print("Uproot version:", up.__version__)
import pandas as pd
from sklearn.preprocessing import StandardScaler
from sklearn.model_selection import StratifiedShuffleSplit
import torch
import argparse
import os
import gc
import datetime

sys.path.append(str(Path(__file__).parent.parent))

from utils import override
from globals import g, g_CL2
from utils import h5Manager

device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")

#=====> Input arguments
parser = argparse.ArgumentParser(description="CL on ISS1 testing script")
parser.add_argument("--config", type=str, default=None, help="Path to config file (JSON/YAML/YML)")
args = parser.parse_args()
override(args.config)

#=====> setting seed for permutations
np.random.seed(g_CL2.CL2_SEED)

#=====> Initialize the H5 manager
#=====> Reading input h5 to pandas dataframe
now = datetime.datetime.now()
print ("Current date and time : ")
print (now.strftime("%Y-%m-%d %H:%M:%S"))
now = now.strftime("%Y_%m_%d") #save it in a string

g.NEW_FOLDER_PATH = g.NEW_FOLDER_PATH + now + '/'
os.makedirs(g.NEW_FOLDER_PATH, exist_ok=True)
print(f"Folder '{g.NEW_FOLDER_PATH}' created successfully.")


#=====> Loading Training and Validation h5
print('Loading positive MC dataset')
#INPUT
#TrainPath = g.DATASET_PATH+'MCSIG_dic.h5'
TrainPath = g.DATASET_PATH+'He4CL.h5'
#OUTPUT
if(g_CL2.CL2_LOSSWEIGHTS):
    rOUTpath = g.NEW_FOLDER_PATH + now+'_trainOnMC_withW_2CL_ISS_scores1.root'
else:
    rOUTpath = g.NEW_FOLDER_PATH + now+'_trainOnMC_2CL_ISS_scores1.root'
#DEBUG
if g.DEBUG:
    MyH5man = h5Manager(in_path=TrainPath, max_rows=100000)
else:
    MyH5man = h5Manager(in_path=TrainPath, useHe3=True, He3h5Name="He3CL_21000.h5")
UnNormData = MyH5man.ReadH5toDataFrame()

#=====> Restoring Validation dictionary
if g_CL2.CL2_LOSSWEIGHTS:
    if g.IS_OLD_RESULT:
        INpath = g.VALDIC_PATH + g.OLD_RES +"/val_dic_CL2_MC_withW.h5"
    else:
        INpath = g.VALDIC_PATH + "val_dic_CL2_MC_withW.h5"
else:
    if g.IS_OLD_RESULT:
        INpath = g.VALDIC_PATH + g.OLD_RES +"/val_dic_CL2_MC.h5"
    else:
        INpath = g.VALDIC_PATH + "val_dic_CL2_MC.h5"
MyH5man.SetInPath(INpath)
Restoredic = MyH5man.ReadH5toDic()

maskVal = np.zeros(len(UnNormData), dtype=bool)
maskVal[Restoredic['val_idx']] = True
UnNormData.drop(index=UnNormData.index[~maskVal], inplace=True)
gc.collect()
#=====> Appending network output
UnNormData.loc[Restoredic['val_idx'], 'scores'] = Restoredic['output']
UnNormData.loc[Restoredic['val_idx'], 'label']  = Restoredic['output_true']
UnNormData.loc[Restoredic['val_idx'], 'index']  = Restoredic['val_idx']


#=====>  Saving dictionary as root tree
print(f"Folder '{g.NEW_FOLDER_PATH}' created successfully.")
print(f"Full output path '{rOUTpath}")
print('Number of saved columns', len(UnNormData.columns))
with up.recreate(rOUTpath) as MyFile:
    MyFile["MC_pos"] = UnNormData
del UnNormData
gc.collect()

print(f"New tree with scores and discriminant at '{rOUTpath}")
print("End of the script!")
