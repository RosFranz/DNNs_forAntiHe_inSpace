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
parser = argparse.ArgumentParser(description="AE training script")
parser.add_argument("--config", type=str, default=None, help="Path to config file (JSON/YAML/YML)")
args = parser.parse_args()
override(args.config)


#=====> Initialize the H5 manager
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
g_AE.AE_COLUMNS.append('isHe3') #Distinguish between He4 and He3 for weight norm


#TRAINING and VALIDATION
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
        rOUTpath = g.NEW_FOLDER_PATH + now+'_trainOnISS_AE_ISS_scores.root'
    #DEBUG
    if g.DEBUG:
        MyH5man = h5Manager(in_path=TRAINpath, out_path=OUTpath, max_rows=100000) ##manager for h5 files
    else:
        MyH5man = h5Manager(in_path=TRAINpath, out_path=OUTpath, max_rows=12000000) ##manager for h5 files
    
else:
    print('Working on MC positives')
    #INPUT
    TRAINpath = g.DATASET_PATH+'He4.h5'
    #OUTPUT
    if(g_AE.AE_LOSSWEIGHTS):
        OUTpath  = g.VALDIC_PATH+"val_dic_AE_MC_withW.h5"
        rOUTpath = g.NEW_FOLDER_PATH + now+'_trainOnMC_withW_AE_MC_scores.root'
    else:
        OUTpath  = g.VALDIC_PATH+"val_dic_AE_MC.h5"
        rOUTpath = g.NEW_FOLDER_PATH + now+'_trainOnMC_AE_MC_scores.root'
    #DEBUG
    if g.DEBUG:
        MyH5man = h5Manager(in_path=TRAINpath, out_path=OUTpath, max_rows=100000)
    else:
        MyH5man = h5Manager(in_path=TRAINpath, out_path=OUTpath, max_rows=12000000, useHe3=True, He3h5Name="He3_21000.h5")
    
#Training + validation
UnNormData = MyH5man.ReadH5toDataFrame()
UnNormData['orig_idx'] = np.arange(len(UnNormData), dtype=np.int64)
UnNormData = UnNormData.iloc[np.random.permutation(len(UnNormData))]
#=====> Selecting only the input features (+weights)
UnNormData.drop(columns=[col for col in UnNormData.columns if col not in g_AE.AE_COLUMNS], inplace=True)
TrainValIndex = UnNormData['orig_idx'].values
UnNormData.drop(columns=['orig_idx'], inplace=True)
if(not g.TRAIN_ON_ISS):
    #Artificial action to have He3 weights with the same ofm of He4 weights
    UnNormData.loc[UnNormData['isHe3'] == 1, 'weight'] = UnNormData['weight'] / 70.
    UnNormData.drop(columns=['isHe3'], inplace=True)
gc.collect()
#-------------------------------------------


#=====> Splitting training and validation R>0
x_trainUnNorm, x_valUnNorm, train_idx, val_idx = train_test_split(UnNormData.values,
                                                                  TrainValIndex,
                                                                  test_size=g_AE.TEST_FRACTION,
                                                                  random_state=g_AE.AE_SEED)
TrainValCol = UnNormData.columns
del UnNormData
gc.collect()
    
#=====> Gettin Test and remove weights Train-Val-Test MC samples
if(not g.TRAIN_ON_ISS):
    print('Removing weights from the datasets')
    #Training
    w_train = x_trainUnNorm[:,TrainValCol.get_loc('weight')]
    x_trainUnNorm = np.delete(x_trainUnNorm, TrainValCol.get_loc('weight'), axis=1)
    #Validation
    w_val = x_valUnNorm[:,TrainValCol.get_loc('weight')]
    x_valUnNorm = np.delete(x_valUnNorm, TrainValCol.get_loc('weight'), axis=1)
gc.collect()

#=====> fit-transform and transform on training set 
print()
print("Debuggin the masked scaler:")
print(x_trainUnNorm[0])
if(g.USEMASK):
    scaler = MaskedStandardScaler(mask_value=g.MYMASKVALUE)
else:
    scaler = StandardScaler()
if g_AE.SWITCH:
    scaler.fit(x_valUnNorm)
    x_train = scaler.transform(x_valUnNorm)
    x_val = scaler.transform(x_trainUnNorm)
else:
    scaler.fit(x_trainUnNorm)
    x_train = scaler.transform(x_trainUnNorm)
    x_val = scaler.transform(x_valUnNorm)
if g.TRAIN_ON_ISS:
    scaler.save(g.VALDIC_PATH+"trainOnISS_masked_scaler_fit_AE.h5")
else:
    scaler.save(g.VALDIC_PATH+"trainOnMC_masked_scaler_fit_AE.h5")
print(x_train[0])
print()
del x_trainUnNorm, x_valUnNorm
gc.collect()


#=====> Training and (ISS-MC) validation loaders
if(g.TRAIN_ON_ISS):
    traindataset = MyAE.MyDatasetWithIdx(x_train,train_idx)
    valdataset   = MyAE.MyDatasetWithIdx(x_val,val_idx)
else:
    traindataset = MyAE.MyDatasetWithIdxW(x_train,train_idx,w_train)
    valdataset   = MyAE.MyDatasetWithIdxW(x_val,val_idx,w_val)

train_loader = torch.utils.data.DataLoader(dataset=traindataset,
                                        batch_size=g_AE.AE_BATCH_SIZE, 
                                        shuffle=True,
                                        pin_memory=False)
val_loader = torch.utils.data.DataLoader(dataset=valdataset,
                                        batch_size=g_AE.AE_BATCH_SIZE,  
                                        shuffle=False,
                                        pin_memory=False)

print("x_train shape[0]: ",x_train.shape[0])
print("x_val shape[0]:", x_val.shape[0], "x_val shape[1]:", x_train.shape[1])
del x_train, x_val
gc.collect()


#=====> Model and Loss definition
dic = MyAE.AEdic.Initialize()
sigmoid = nn.Sigmoid()

print()    
print("Model structure: ")
if g.USEMASK: 
    print("Types: ", type(g_AE.AE_INPUT_SIZE), type(g_AE.AE_LAYERS))
    model = MyAE.maskedAE(g_AE.AE_INPUT_SIZE, g_AE.AE_LAYERS, mask_value=g.MYMASKVALUE).to(device)
    print("(-2): CustomLinear(in_features=", g_AE.AE_INPUT_SIZE, ", out_features=", g_AE.AE_INPUT_SIZE, ", bias=True)")
    print("(-1): ReLU()")
    criterion = MyAE.MaskedMSELoss(mask_value=g.MYMASKVALUE)
else:
    model = MyAE.AE(g_AE.AE_INPUT_SIZE, g_AE.AE_LAYERS).to(device)
    if g.TRAIN_ON_ISS:
        ##########################################
        #Not sure if this reduction is correct or sufficient
        #Possible bug
        criterion = nn.MSELoss(reduction='mean') #
        ##########################################
    else:
        criterion = nn.MSELoss(reduction='none')
print(model)
# Adam optimizer and learning rate
optimizer = torch.optim.Adam(model.parameters(), lr=g_AE.AE_LR)
        
#=====> TRAINING ON ISS DATA
if(g.TRAIN_ON_ISS):
    #=====> LOOP ON THE EPOCHS
    print("Starting training for epochs: ",g_AE.AE_EPOCHS)
    for epoch in range(g_AE.AE_EPOCHS):
        loss = 0.
        val_loss = 0.
        
        model.train()
        loss_per_batch = []
        temp_out=[]
        temp_err=[]
        temp_as=[]
        temp_idx=[]
    
        #=====> TRAINING
        for i, (features,t_idx) in enumerate(train_loader):
            outputs = model(features.float())
            loss = criterion(outputs, features.float())
            loss = torch.sum(loss)/features.shape[0] # mean over events -> 1 value
            
            if (np.isnan(loss.item())):
                print("Batch num: ",i)
                print("features: ",features)
                print("outputs: ",outputs)
                print("loss: ",loss.item())
                sys.exit(1)
            optimizer.zero_grad()
            loss.backward()
            optimizer.step()
            loss_per_batch.append(loss.item()) 
        dic['train_loss'].append(mean(loss_per_batch))
        
        #=====> Validation
        val_loss_per_batch = []   
        model.eval() 
        for i, (features,v_idx) in enumerate(val_loader):
            test_outputs = model(features.float())  
            val_loss = criterion(test_outputs, features.float())
            val_loss = torch.sum(val_loss)/features.shape[0] # mean over events -> 1 value
            val_loss_per_batch.append(val_loss.item())
            
            temp_out.append(test_outputs.detach().numpy())
            errors = (test_outputs - features.float())**2
            mask = (features.float() != g.MYMASKVALUE) # True = valid, False = invalid (masked)
            errors = torch.sum(errors * mask, 1) / torch.sum(mask, 1)  # Normalize by features
            # for e in errors:
            #     temp_err.append(e.detach().numpy())
            # print('CHECK')
            # print(errors.detach().numpy())
            # print(temp_err)
            # for ascore in sigmoid(errors):
            #     temp_as.append(ascore.detach().numpy())
            temp_err.append(errors.detach().numpy())
            temp_as.append(sigmoid(errors).detach().numpy())
            temp_idx.append(v_idx.detach().numpy())
        dic['val_loss'].append(mean(val_loss_per_batch))

        #Printing the status
        if epoch%5==0:
            print(f"Train_loss: {mean(loss_per_batch):.5f}, "
                f"Val_loss: {mean(val_loss_per_batch):.5f}, "
                f"Epoch : {epoch}")
            
        #Best epoch check
        if(g.MIN_LOSS>mean(val_loss_per_batch)):
            g.MIN_LOSS = mean(val_loss_per_batch)
            dic['output']=[]
            dic['output_error']=[]
            dic['anomaly_score']=[]
            dic['output']=temp_out
            dic['output_error']=temp_err
            dic['anomaly_score']=temp_as
            dic['val_idx']=temp_idx


#=====> TRAINING ON MC  
else:
    #=====> LOOP ON THE EPOCHS
    print("Starting training for epochs: ",g_AE.AE_EPOCHS)
    for epoch in range(g_AE.AE_EPOCHS):
        loss = 0.
        val_loss = 0.
        
        model.train()
        loss_per_batch = []
        temp_out=[]
        temp_err=[]
        temp_as=[]
        temp_idx=[]
        
        #=====> TRAINING
        for i, (features, t_idx, wTrain) in enumerate(train_loader):
            outputs = model(features.float())
            loss = criterion(outputs, features.float())
            if not g_AE.AE_LOSSWEIGHTS:
                loss = torch.sum(loss)/features.shape[0]     # mean over events         -> 1 value
            if(g_AE.AE_LOSSWEIGHTS and g.USEMASK):
                loss = torch.sum(loss*wTrain)/wTrain.size(0) # mean over events       -> 1 value
            if(g_AE.AE_LOSSWEIGHTS and not g.USEMASK):
                loss = torch.sum(loss,1)/g_AE.AE_INPUT_SIZE  # mean over the features -> 1 value per event
                loss = torch.sum(loss*wTrain)/wTrain.size(0) # mean over events       -> 1 value
            
            if (np.isnan(loss.item())):
                print("Batch num: ",i)
                print("features: ",features)
                print("outputs: ",outputs)
                print("loss: ",loss.item())
                sys.exit(1)
            optimizer.zero_grad()
            loss.backward()
            optimizer.step()
            loss_per_batch.append(loss.item())
        dic['train_loss'].append(mean(loss_per_batch))
        
        #=====> Validation
        val_loss_per_batch = []   
        model.eval() 
        for i, (features,v_idx,wVal) in enumerate(val_loader):
            test_outputs = model(features.float())
            val_loss = criterion(test_outputs, features.float())
            if (epoch==g_AE.AE_EPOCHS-1):
                dic['weights'].append(wVal.detach().numpy())
            if not g_AE.AE_LOSSWEIGHTS:
                val_loss = torch.sum(val_loss)/features.shape[0]      # mean over events       -> 1 value
            if(g_AE.AE_LOSSWEIGHTS and g.USEMASK):
                val_loss = torch.sum(val_loss*wVal)/wVal.size(0)    # mean over events       -> 1 value
            if(g_AE.AE_LOSSWEIGHTS and not g.USEMASK):
                val_loss = torch.sum(val_loss,1)/g_AE.AE_INPUT_SIZE # mean over the features -> 1 value per event
                val_loss = torch.sum(val_loss*wVal)/wVal.size(0)    # mean over events       -> 1 value
            val_loss_per_batch.append(val_loss.item())
            temp_out.append(test_outputs.detach().numpy())
            errors = (test_outputs - features.float())**2
            mask = (features.float() != g.MYMASKVALUE) # True = valid, False = invalid (masked)
            errors = torch.sum(errors * mask, 1) / torch.sum(mask, 1)  # Normalize by features
            temp_err.append(errors.detach().numpy())
            temp_as.append(sigmoid(errors).detach().numpy())
            temp_idx.append(v_idx.detach().numpy())
        dic['val_loss'].append(mean(val_loss_per_batch))
        
        if epoch==g_AE.AE_EPOCHS-1:
            print(f"Train_loss: {mean(loss_per_batch):.4f}, "
                f"Val_loss: {mean(val_loss_per_batch):.4f}, "
                f"Epoch : {epoch}")
    
        #Printing the status
        if epoch%5==0:
            print(f"Train_loss: {mean(loss_per_batch):.5f}, "
                f"Val_loss: {mean(val_loss_per_batch):.5f}, "
                f"Epoch : {epoch}")
            
        #Best epoch check
        if(g.MIN_LOSS>mean(val_loss_per_batch)):
            g.MIN_LOSS = mean(val_loss_per_batch)
            dic['output']=[]
            dic['output_error']=[]
            dic['anomaly_score']=[]
            dic['output']=temp_out
            dic['output_error']=temp_err
            dic['anomaly_score']=temp_as
            dic['val_idx']=temp_idx

del traindataset, valdataset
del train_loader, val_loader
gc.collect()
 
print('MyValErrors:',dic['output_error'][0])
print('AS validation:',dic['anomaly_score'][0])

#=====> Saving the trained model, the validation and test scores
print("Saving the model and the dictionary...")
if(g.TRAIN_ON_ISS):
    if(g_AE.SWITCH):
        torch.save(model.state_dict(), g.MODEL_PATH+"trained_model_AE_ISS_switch.pth")
        MyH5man.SaveH5(MyH5man.CheckDicType(dic), group_name="dic")
    else:
        torch.save(model.state_dict(), g.MODEL_PATH+"trained_model_AE_ISS.pth")
        MyH5man.SaveH5(MyH5man.CheckDicType(dic), group_name="dic")
else:
    if(g_AE.AE_LOSSWEIGHTS):
        torch.save(model.state_dict(), g.MODEL_PATH+"trained_model_AE_MC_withW.pth")
        MyH5man.SaveH5(MyH5man.CheckDicType(dic), group_name="dic")
    else:
        torch.save(model.state_dict(), g.MODEL_PATH+"trained_model_AE_MC.pth")
        MyH5man.SaveH5(MyH5man.CheckDicType(dic), group_name="dic")
print("Done!")
