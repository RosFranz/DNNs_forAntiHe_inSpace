import numpy as np
print("NumPy version:", np.__version__)
import uproot as up
print("Uproot version:", up.__version__)
import pandas as pd
from sklearn.preprocessing import label_binarize
import torch
import argparse
import sys
import os
import gc
import h5py
from sklearn.utils import resample

from pathlib import Path
# Add the parent directory to Python's module search path
sys.path.append(str(Path(__file__).parent.parent))  # Goes up to AMS/ams_network_DEV/

from utils import override
from globals import g, PREg
from utils import h5Manager
from utils import DFforCLtrain

device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")

#=====> Input arguments
parser = argparse.ArgumentParser(description="ROOT Tree to h5 file prepocessing script")
parser.add_argument("--config", type=str, default=None, help="Path to config file (JSON/YAML/YML)")
args = parser.parse_args()
override(args.config)

###############################################
if g.DEBUG:
    names = ['MCHe4deb_dic.h5', 'MCHe4CCdeb_dic.h5', 'ISSposdeb_dic.h5', 'ISSnegdeb_dic.h5']
else:
    names = ['MCHe4deb_dic.h5', 'MCHe4CCdeb_dic.h5', 'ISSposdeb_dic.h5', 'ISSnegdeb_dic.h5']

###############################################

columns_to_keep = PREg.COLUMNS_DEF
##manager for h5 files just to use its function
# the h5 saving is hard coded to use append mode
MyH5man = h5Manager(in_path=None, out_path=None)
#Name of the h5 file
h5Name = PREg.H5_NAME

#MonteCarlo
if PREg.BUILD_MC:
    columns_to_keep.extend(PREg.COLUMNS_MC)
    if len(PREg.FILE_NAME) == 0:
        input_path = PREg.FILE_PATH+"MC/"+PREg.FILE_DATE+".root"
    else:
        input_path = PREg.FILE_PATH+"MC/"+PREg.FILE_NAME+"_"+PREg.FILE_DATE+".root"
    
    print("Input ROOT file path: ", input_path)

    #MAnipulate only 400 MB of the signal tree
    if(not PREg.IS_CC):
        if len(h5Name) == 0:
            h5Name = names[0]
        print('Working on MC signal file')
        if os.path.exists(g.DATASET_PATH+h5Name):
            print('Removing old dictionary')
            os.remove(g.DATASET_PATH+h5Name)
        tree = up.open(input_path)["MC"]
        list_branch=tree.keys()
        for UnNormData0 in tree.iterate(step_size="400 MB", library="pd"):
            UnNormData0.fillna(g.MYMASKVALUE)
            UnNormData0 = UnNormData0[abs(UnNormData0['Rtrue']) >= 1.92]
            UnNormData0 = UnNormData0[abs(UnNormData0['Rtrue']) <= 3000]
            UnNormData0.replace([np.inf, -np.inf], g.MYMASKVALUE, inplace=True)
            
            #LABELS
            #transforming label into vectors of 4 elements, with 1 in the position of the label
            label15 = UnNormData0['label15'].values
            label15 = label_binarize(label15, classes=[0, 1, 2, 3])
            label10 = UnNormData0['label10'].values
            label10 = label_binarize(label10, classes=[0, 1, 2, 3])
            label5 = UnNormData0['label5'].values
            label5 = label_binarize(label5, classes=[0, 1, 2, 3])
                
            UnNormData0['SigmaUpLow']   = (abs(UnNormData0['RinnerUH']) - abs(UnNormData0['RinnerLH']))/(abs(UnNormData0['RinnerUH']) + abs(UnNormData0['RinnerLH']))
            UnNormData0['charge2_rich'] = np.sqrt(UnNormData0['charge2_rich'])
            
            UnNormData0.drop(columns=[col for col in UnNormData0.columns if col not in columns_to_keep], inplace=True)
            
            #Defining the label for rigidities
            UnNormData0['RigLabel']     = np.zeros(UnNormData0.shape[0])
            UnNormData0.loc[abs(UnNormData0['SigmaUpLow'])<=0.2, 'RigLabel'] = 1 
            
            UnNormData0 = MyH5man.MaskDataFrame(UnNormData0, g.MYMASKVALUE)            
            
            if(PREg.CL_TRAINorVAL): #CLASSIFIER train-validation sample
                UnNormData0 = DFforCLtrain(UnNormData0)
            else: #AUTOENCODER train-validation sampel
                UnNormData0 = UnNormData0[abs(UnNormData0['Rinner'])<200]

            #Create dictionary        
            MC_infoSig = {col: UnNormData0[col].values for col in UnNormData0.columns}
            del UnNormData0
            gc.collect()
            
            MC_infoSig = MyH5man.CheckDicType(MC_infoSig)
                
            # Open h5 file in append mode
            with h5py.File(g.DATASET_PATH+h5Name, "a") as myFile:
                if "dic" in myFile:
                    existing_group = myFile["dic"]
                else:
                    existing_group = myFile.create_group("dic")
                # Append or create new dataset 
                for key, value in MC_infoSig.items():
                    try:
                        if key in existing_group.keys():
                            existing_dataset = existing_group[key]
                            new_shape = (existing_dataset.shape[0] + value.shape[0],) + value.shape[1:]
                            existing_dataset.resize(new_shape)
                            existing_dataset[-value.shape[0]:] = value
                        else:
                            maxshape = (None,) + value.shape[1:]  # Allow resizing along the first dimension
                            existing_group.create_dataset(
                                key,
                                data=value,
                                maxshape=maxshape,  # Enable resizing
                                chunks=True,        # Enable chunking
                                compression="gzip",  # Enable compression
                            )
                    except Exception as e:
                        print(f"Error checking key {key}: {e}")
                        raise
                del MC_infoSig
                gc.collect()
            if g.DEBUG:
                sys.exit(0)
        print("done!")    
         
    # CHARGE CONFUSED samples
    else:
        if len(h5Name) == 0:
            h5Name = names[1]
        print('Working on MC bkg file')
        if os.path.exists(g.DATASET_PATH+h5Name):
            print('Removing old dictionary')
            os.remove(g.DATASET_PATH+h5Name)
        tree = up.open(input_path)["MC"]
        list_branch=tree.keys()
        print('Number of nans in the tree: ', tree.num_entries - tree.arrays(list_branch, library='pd').dropna().shape[0])
        UnNormData0 = tree.arrays(list_branch, library='pd').fillna(g.MYMASKVALUE)
        UnNormData0.replace([np.inf, -np.inf], g.MYMASKVALUE, inplace=True)
        UnNormData0 = UnNormData0[UnNormData0['Rtrue'] >= 1.92]
        UnNormData0 = UnNormData0[UnNormData0['Rtrue'] <= 3000]
        print('Number of entries: ', tree.num_entries, ', Not NAN: ',UnNormData0.shape)
        
        #LABELS
        #transforming label into vectors of 4 elements, with 1 in the position of the label
        label15 = UnNormData0['label15'].values
        label15 = label_binarize(label15, classes=[0, 1, 2, 3])
        label10 = UnNormData0['label10'].values
        label10 = label_binarize(label10, classes=[0, 1, 2, 3])
        label5 = UnNormData0['label5'].values
        label5 = label_binarize(label5, classes=[0, 1, 2, 3])
            
        UnNormData0['SigmaUpLow']   = (abs(UnNormData0['RinnerUH']) - abs(UnNormData0['RinnerLH']))/(abs(UnNormData0['RinnerUH']) + abs(UnNormData0['RinnerLH']))
        UnNormData0['charge2_rich'] = np.sqrt(UnNormData0['charge2_rich'])
        
        print("Dropping columns ... Zero means fine :)")
        print('Total number of columns:', len(UnNormData0.columns))
        print(UnNormData0.columns)
        UnNormData0.drop(columns=[col for col in UnNormData0.columns if col not in columns_to_keep], inplace=True)
        print('Number of columns (not labels)',len(UnNormData0.columns))
        
        #Defining the label for rigidities
        UnNormData0['RigLabel']     = np.zeros(UnNormData0.shape[0])
        UnNormData0.loc[abs(UnNormData0['SigmaUpLow'])<=0.2, 'RigLabel'] = 1

        UnNormData0['label15'] = np.argmax(label15, axis=1)
        UnNormData0['label10'] = np.argmax(label10, axis=1)
        UnNormData0['label5'] = np.argmax(label5, axis=1)
        UnNormData0 = MyH5man.MaskDataFrame(UnNormData0, g.MYMASKVALUE)        

        print("Saving the dictionary ...")
        
        for column_name in columns_to_keep:
            if column_name not in UnNormData0.columns:
                print(f"Column '{column_name}' is not in the df but it should !")
        print('Total entries in each dictionary')
        print(UnNormData0.shape)
        MC_info = {col: UnNormData0[col].values for col in UnNormData0.columns}
        nameIDX = 1
        nameIDX = 2
        nameIDX = 3
            
        MC_info = MyH5man.CheckDicType(MC_info)
        
        
        # Open h5 file in write mode
        with h5py.File(g.DATASET_PATH+h5Name, "w") as myFile:
            existing_group = myFile.create_group("dic")
            # Append or create new dataset 
            for key, value in MC_info.items():
                maxshape = (None,) + value.shape[1:]  # Allow resizing along the first dimension
                existing_group.create_dataset(
                    key,
                    data=value,
                    maxshape=maxshape,  # Enable resizing
                    chunks=True,        # Enable chunking
                    compression="gzip",  # Enable compression
                )
            del MC_info
    
    

#ISS data
else:
    columns_to_keep.extend(PREg.COLUMNS_ISS)
    if len(PREg.FILE_NAME) == 0:
        input_path = PREg.FILE_PATH+"ISS/"+PREg.FILE_DATE+".root"
    else:
        input_path = PREg.FILE_PATH+"ISS/"+PREg.FILE_NAME+"_"+PREg.FILE_DATE+".root"
    print("Input ROOT file path: ", input_path)
    
                    
    #MAnipulate only 400 MB of the signal tree
    if(not PREg.IS_CC):
        if len(h5Name) == 0:
            h5Name = names[2]
        print('Working on ISS positive file')
        if os.path.exists(g.DATASET_PATH+h5Name):
            print('Removing old dictionary')
            os.remove(g.DATASET_PATH+h5Name)
        tree = up.open(input_path)["ISS"]
        list_branch=tree.keys()
        for UnNormData0 in tree.iterate(step_size="400 MB", library="pd"):
            UnNormData0.fillna(g.MYMASKVALUE)
            UnNormData0.replace([np.inf, -np.inf], g.MYMASKVALUE, inplace=True)
            print('Number of entries: ', tree.num_entries, ', Not NAN: ',UnNormData0.shape)

            UnNormData0['SigmaUpLow']   = (UnNormData0['RinnerUH'] - UnNormData0['RinnerLH'])/(UnNormData0['RinnerUH'] + UnNormData0['RinnerLH'])
            UnNormData0['charge2_rich'] = np.sqrt(UnNormData0['charge2_rich'])
            
            UnNormData0.drop(columns=[col for col in UnNormData0.columns if col not in columns_to_keep], inplace=True)

            #Defining the label for rigidities
            UnNormData0['RigLabel']     = np.zeros(UnNormData0.shape[0])
            UnNormData0.loc[abs(UnNormData0['SigmaUpLow'])<=0.2, 'RigLabel'] = 1
            
            print('Masking the dataframe ...')
            UnNormData0 = MyH5man.MaskDataFrame(UnNormData0, g.MYMASKVALUE)
           
            #Check if is for CL training
            if(PREg.CL_TRAINorVAL): #CLASSIFIER training-validation sample
                UnNormData0 = DFforCLtrain(UnNormData0)
            else: #AUTOENCODER training-validation sample
                UnNormData0 = UnNormData0[abs(UnNormData0['Rinner'])<200]
            
            #Create dictionary
            ISS_info = {col: UnNormData0[col].values for col in UnNormData0.columns}
            del UnNormData0
            gc.collect()
            
            ISS_info = MyH5man.CheckDicType(ISS_info)
            
            # Open h5 file in append mode
            with h5py.File(g.DATASET_PATH+h5Name, "a") as myFile:
                if "dic" in myFile:
                    existing_group = myFile["dic"]
                else:
                    existing_group = myFile.create_group("dic")
                # Append or create new dataset 
                for key, value in ISS_info.items():
                    if key in existing_group:
                        existing_dataset = existing_group[key]
                        new_shape = (existing_dataset.shape[0] + value.shape[0],) + value.shape[1:]
                        existing_dataset.resize(new_shape)
                        existing_dataset[-value.shape[0]:] = value
                    else:
                        maxshape = (None,) + value.shape[1:]  # Allow resizing along the first dimension
                        existing_group.create_dataset(
                            key,
                            data=value,
                            maxshape=maxshape,  # Enable resizing
                            chunks=True,        # Enable chunking
                            compression="gzip",  # Enable compression
                        )
                del ISS_info
            gc.collect()
            
    else:
        if len(h5Name) == 0:
            h5Name = names[3]
        print('Working on ISS negative file')
        if os.path.exists(g.DATASET_PATH+h5Name):
            print('Removing old dictionary')
            os.remove(g.DATASET_PATH+h5Name)
        tree = up.open(input_path)["ISS"]
        list_branch=tree.keys()
        UnNormData0 = tree.arrays(list_branch, library='pd').fillna(g.MYMASKVALUE)
        UnNormData0.replace([np.inf, -np.inf], g.MYMASKVALUE, inplace=True)

        UnNormData0['SigmaUpLow']   = (UnNormData0['RinnerUH'] - UnNormData0['RinnerLH'])/(UnNormData0['RinnerUH'] + UnNormData0['RinnerLH'])
        UnNormData0['charge2_rich'] = np.sqrt(UnNormData0['charge2_rich'])
        
        UnNormData0.drop(columns=[col for col in UnNormData0.columns if col not in columns_to_keep], inplace=True)

        #Defining the label for rigidities
        UnNormData0['RigLabel']     = np.zeros(UnNormData0.shape[0])
        UnNormData0.loc[abs(UnNormData0['SigmaUpLow'])<=0.2, 'RigLabel'] = 1
        
        UnNormData0 = MyH5man.MaskDataFrame(UnNormData0, g.MYMASKVALUE)
            
        print("Saving the dictionary ...")
        
        ISS_info = {col: UnNormData0[col].values for col in UnNormData0.columns}
        print(len(UnNormData0.columns) - len(columns_to_keep)-1)
        print('Total entries in each dictionary')
        print(UnNormData0.shape)
        print('Number of nans', UnNormData0.isnull().sum())
        del UnNormData0
        
        ISS_info = MyH5man.CheckDicType(ISS_info)
        
        # Open h5 file in write mode
        with h5py.File(g.DATASET_PATH+h5Name, "w") as myFile:
            existing_group = myFile.create_group("dic")
            # Append or create new dataset 
            for key, value in ISS_info.items():
                maxshape = (None,) + value.shape[1:]  # Allow resizing along the first dimension
                existing_group.create_dataset(
                    key,
                    data=value,
                    maxshape=maxshape,  # Enable resizing
                    chunks=True,        # Enable chunking
                    compression="gzip",  # Enable compression
                )
            del ISS_info
    
    
    
