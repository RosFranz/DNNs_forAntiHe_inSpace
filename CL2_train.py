import numpy as np
print("NumPy version:", np.__version__)
import uproot as up
print("Uproot version:", up.__version__)
import pandas as pd
from sklearn.preprocessing import StandardScaler
import torch
import torch.nn as nn
from statistics import mean
from sklearn.model_selection import StratifiedShuffleSplit
import argparse
import sys
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
parser = argparse.ArgumentParser(description="CL2 training script")
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

#TRAINING and VALIDATION
if(g.TRAIN_ON_ISS):
    print('Working on ISS positives')
    #INPUT
    if(g.IS_OLD_RESULT):
        TRAINpath = g.DATASET_PATH+g.OLD_RES+"/ISSpos_dic.h5"
    else:
        TRAINpath = g.DATASET_PATH+"ISSposCL_dic.h5"
    #OUTPUT
    OUTpath = g.VALDIC_PATH+'val_dic_CL2_ISS.h5'
    #DEBUG
    if g.DEBUG:
        MyH5man = h5Manager(in_path=TRAINpath, out_path=OUTpath, max_rows=100000) ##manager for h5 files
    else:
        MyH5man = h5Manager(in_path=TRAINpath, out_path=OUTpath) ##manager for h5 files
else:
    print('Working on MC positives')
    #INPUT
    INpath = g.DATASET_PATH+'He4CL.h5'
    #OUTPUT
    if g_CL2.CL2_LOSSWEIGHTS:
        OUTpath = g.VALDIC_PATH +'val_dic_CL2_MC_withW.h5'
    else:
        OUTpath = g.VALDIC_PATH +'val_dic_CL2_MC.h5'
    #DEBUG
    if g.DEBUG:
        MyH5man = h5Manager(in_path=INpath, out_path=OUTpath, max_rows=100000)
    else:
        MyH5man = h5Manager(in_path=INpath, out_path=OUTpath, useHe3=True, He3h5Name="He3CL_21000.h5")
    
#Training + validation
UnNormData = MyH5man.ReadH5toDataFrame()
UnNormData['orig_idx'] = np.arange(len(UnNormData), dtype=np.int64)
UnNormData = UnNormData.iloc[np.random.permutation(len(UnNormData))]
label = UnNormData['RigLabel'].values
UnNormData.drop(columns=['RigLabel'], inplace=True)
UnNormData.drop(columns=['SigmaUpLow'], inplace=True)
g_CL2.CL2_COLUMNS.append('orig_idx')
g_CL2.CL2_COLUMNS.append('isHe3')
#=====> Selecting only the input features (+weights)
UnNormData.drop(columns=[col for col in UnNormData.columns if col not in g_CL2.CL2_COLUMNS], inplace=True)
print('Input features: ', UnNormData.columns)  
gc.collect()
#-------------------------------------------

#=====> Selecting balanced dataset
print("Dropping signal events to match background events number")
print('Signal events', len(label[label==1]))
print('Bkg events', len(label[label==0]))

 #Indexes
TrainValIndex=UnNormData['orig_idx'].values
UnNormData.drop(columns=['orig_idx'], inplace=True)
 #Artificial action to have He3 weights with the same ofm of He4 weights
if not g.TRAIN_ON_ISS:
    UnNormData.loc[UnNormData['isHe3'] == 1, 'weight'] = UnNormData['weight'] / 70.
    UnNormData.drop(columns=['isHe3'], inplace=True)
 #Features
X=UnNormData.values
 #Columns
TrainValCol = UnNormData.columns
del UnNormData
gc.collect()
print('X shape', X.shape[0])

#OLD WAY now in the preprocessing
 #--------
#lbkg = label[label==0]
#lsig = label[label==1][:len(lbkg)]
 #--------
#TrainValIndex_bkg=UnNormData[label==0]['orig_idx'].values
#TrainValIndex_sig=UnNormData[label==1]['orig_idx'].values
#UnNormData.drop(columns=['orig_idx'], inplace=True)
 #--------
#Xbkg=UnNormData[label==0].values
#Xsig=UnNormData[label==1].values
 #--------
#TrainValCol = UnNormData.columns
#del UnNormData
#gc.collect()

 #permutation of the signal events and keeping only lbkg events
#permSig = np.random.permutation(len(lsig))[:len(lbkg)]
#lsig = lsig[permSig][:len(lbkg)]
#Xsig = Xsig[permSig][:len(lbkg)]
#TrainValIndex_sig = TrainValIndex_sig[permSig][:len(lbkg)]
#del permSig

 #putting all together
#X=np.vstack((Xsig,Xbkg))
#label=np.hstack((lsig, lbkg))
#TrainValIndex=np.hstack((TrainValIndex_sig,TrainValIndex_bkg))
#print('Signal events', len(label[label==1]))
#print('Bkg events', len(label[label==0]))
#print('Label shape', label.shape[0])
#print('X shape', X.shape[0])
#permutation = np.random.permutation(X.shape[0])
#label = label[permutation]
#X = X[permutation]
#TrainValIndex = TrainValIndex[permutation]
#del Xsig, Xbkg, lsig, lbkg, TrainValIndex_sig, TrainValIndex_bkg, permutation
#gc.collect()

#=====> Splitting in training and validation
sss = StratifiedShuffleSplit(n_splits=1, test_size=g_CL2.TEST_FRACTION, random_state=0)
for train_index, test_index in sss.split(X, label):
    x_trainUnNorm, x_valUnNorm = X[train_index], X[test_index]
    y_train, y_val = label[train_index], label[test_index]
    train_idx, val_idx = TrainValIndex[train_index], TrainValIndex[test_index]
del X, label, TrainValIndex
gc.collect()
    
if(not g.TRAIN_ON_ISS):
    print('Removing weights from the datasets')
    #Training
    w_train = x_trainUnNorm[:,TrainValCol.get_loc('weight')]
    x_trainUnNorm = np.delete(x_trainUnNorm, TrainValCol.get_loc('weight'), axis=1)
    #Validation
    w_val = x_valUnNorm[:,TrainValCol.get_loc('weight')]
    x_valUnNorm = np.delete(x_valUnNorm, TrainValCol.get_loc('weight'), axis=1)
    

#=====> fit-transform and transform on training set
print()
print("Debuggin the masked scaler:")
print(x_trainUnNorm[0])
if g.USEMASK:
    scaler = MaskedStandardScaler(mask_value=g.MYMASKVALUE)
else:
    scaler = StandardScaler() #Feature scaling
scaler.fit(x_trainUnNorm)
if g.TRAIN_ON_ISS:
    scaler.save(g.VALDIC_PATH+"trainOnISS_masked_scaler_fit_CL2.h5")
else:
    scaler.save(g.VALDIC_PATH+"trainOnMC_masked_scaler_fit_CL2.h5")
x_train = scaler.transform(x_trainUnNorm)
x_val  = scaler.transform(x_valUnNorm)
del x_trainUnNorm, x_valUnNorm
gc.collect()


#=====> Training and (ISS-MC) test loaders
if g.TRAIN_ON_ISS:
    traindataset = CL2.myDataset(x_train,y_train,train_idx)
    valdataset  = CL2.myDataset(x_val,y_val,val_idx)
else:
    traindataset = CL2.myDataset(x_train,y_train,train_idx,w_train)
    valdataset  = CL2.myDataset(x_val,y_val,val_idx,w_val)

train_loader = torch.utils.data.DataLoader(dataset=traindataset,
                                           batch_size=g_CL2.CL2_BATCH_SIZE, 
                                           shuffle=True)
val_loader = torch.utils.data.DataLoader(dataset=valdataset,
                                          batch_size=g_CL2.CL2_BATCH_SIZE,  
                                          shuffle=False)
print("x_train shape[0]: ",x_train.shape[0], "x_train shape[1]:", x_train.shape[1])
print("x_val shape[0]:", x_val.shape[0])

#=====> Model and Loss definition
dic = CL2.CL2dic.Initialize()
print()    
print("Model structure: ")
if g.USEMASK:
    # there is a "hidden" (hard-coded) first layer that has the same dimension of the starting value 
    # this allow to neglect the propagation of biases for masked values. 
    model = CL2.maskedNeuralNet(g_CL2.CL2_INPUT_SIZE, g_CL2.CL2_LAYERS, g_CL2.DROPOUT, mask_value=g.MYMASKVALUE).to(device)
    print("(-2): CustomLinear(in_features=", x_train.shape[1], ", out_features=", x_train.shape[1], ", bias=True)")
    print("(-1): ReLU()")
else:
    model = CL2.NeuralNet(g_CL2.CL2_INPUT_SIZE, g_CL2.CL2_LAYERS, g_CL2.DROPOUT).to(device)
print(model)
optimizer = torch.optim.Adam(model.parameters(), lr=g_CL2.CL2_LR)
if(g_CL2.CL2_LOSSWEIGHTS):
    # if g.TRAIN_ON_ISS:
    #     class_counts = np.array([len(y_train[y_train==0]), len(y_train[y_train==1])])
    #     total_samples = class_counts.sum()
    #     loss_weights = total_samples / (len(class_counts) * class_counts)
    #     w = torch.ones(g_CL2.CL2_BATCH_SIZE, dtype=torch.float32)
    #     criterion = nn.BCELoss(weight=w, reduction='mean') # see to fix: https://pytorch.org/docs/stable/generated/torch.nn.BCELoss.html
    # else:
    if not g.TRAIN_ON_ISS:
        loss_weights = w_train
        #w = torch.ones(g_CL2.CL2_BATCH_SIZE, dtype=torch.float32)
        w = torch.tensor(g_CL2.CL2_BATCH_SIZE, dtype=torch.float32)
        criterion = nn.BCELoss(weight=w, reduction='mean')
else:
    criterion = nn.BCELoss(reduction='mean')
if g.TRAIN_ON_ISS:
    class_counts = np.array([len(y_train[y_train==0]), len(y_train[y_train==1])])
    total_samples = class_counts.sum()
    loss_weights = np.ones(len(class_counts))
    criterion = nn.BCELoss() 
    print('Class weight for LOSS function: ', loss_weights)


#=====> TRAINING ON ISS DATA
if(g.TRAIN_ON_ISS):
    #=====> LOOP ON THE EPOCHS
    print("Starting training for epochs: ",g_CL2.CL2_EPOCHS)
    for epoch in range(g_CL2.CL2_EPOCHS):
        loss = 0.
        test_loss = 0.
        
        accuracy_list = []
        loss_per_batch = []
        temp_out=[]
        temp_true_out=[]
        temp_idx=[]
        #=====> TRAINING
        model.train()
        for i, (features,labels,t_idx) in enumerate(train_loader):
            outputs = model(features.float())
            outputs = outputs.squeeze(-1)
            loss = criterion(outputs, labels.float())
            
            if (np.isnan(loss.item())):
                print("Batch num: ",i)
                print("features: ",features)
                print("labels: ",labels)
                print("outputs: ",outputs)
                print("loss: ",loss.item())
                sys.exit(1)
            optimizer.zero_grad()
            loss.backward()
            optimizer.step()
            loss_per_batch.append(loss.item())
        dic['train_loss'].append(mean(loss_per_batch))
        
        #=====> Validation
        test_loss_per_batch = []   
        model.eval()
        for i, (features,labels,v_idx) in enumerate(val_loader):
            test_outputs = model(features.float())  
            test_outputs = test_outputs.squeeze(-1)
            test_loss = criterion(test_outputs, labels.float())
            test_loss_per_batch.append(test_loss.item()) 
            #Accuracy
            temp = test_outputs.cpu().detach().numpy()
            predicted_labels = np.zeros(len(temp))
            predicted_labels[temp>0.5] = 1
            true_labels = labels.cpu().detach().numpy()
            accuracy = (predicted_labels == true_labels).sum()/len(true_labels)
            accuracy_list.append(accuracy)
            
            temp_out.append(test_outputs.detach().numpy())
            temp_true_out.append(labels.detach().numpy())
            temp_idx.append(v_idx.detach().numpy())
        dic['val_loss'].append(mean(test_loss_per_batch))
        dic['accuracy'].append(mean(accuracy_list))
        
        #Best epoch check
        if(g.MIN_LOSS>mean(test_loss_per_batch)):
            g.MIN_LOSS = mean(test_loss_per_batch)
            dic['output']=[]
            dic['output_true']=[]
            dic['val_idx']=[]
            dic['output']=temp_out
            dic['output_true']=temp_true_out
            dic['val_idx']=temp_idx

        #Printing the status
        if epoch%5==0:
            print(f"Train_loss: {mean(loss_per_batch):.4f}, "
                f"Val_loss: {mean(test_loss_per_batch):.4f}, "
                f"Accuracy: {mean(accuracy_list):.4f}, "
                f"Epoch : {epoch}")
        if epoch==g_CL2.CL2_EPOCHS-1:
            print(f"Train_loss: {mean(loss_per_batch):.4f}, "
                f"Val_loss: {mean(test_loss_per_batch):.4f}, "
                f"Epoch : {epoch}")
            del temp_out, temp_true_out
            gc.collect()

#=====> TRAINING ON MC
else:
    #=====> LOOP ON THE EPOCHS
    print("Starting training for epochs: ",g_CL2.CL2_EPOCHS)
    for epoch in range(g_CL2.CL2_EPOCHS):
        loss = 0.
        test_loss = 0.
        
        accuracy_list = []
        loss_per_batch = []
        temp_out=[]
        temp_true_out=[]
        temp_idx=[]
        #=====> TRAINING
        model.train()
        for i, (features,labels,t_idx,wTrain) in enumerate(train_loader):
            outputs = model(features.float())
            outputs = outputs.squeeze(-1)
            if(g_CL2.CL2_LOSSWEIGHTS):
                w= wTrain
            loss = criterion(outputs, labels.float())
            
            if (np.isnan(loss.item())):
                print("Batch num: ",i)
                print("features: ",features)
                print("labels: ",labels)
                print("outputs: ",outputs)
                print("loss: ",loss.item())
                sys.exit(1)
            optimizer.zero_grad()
            loss.backward()
            optimizer.step()
            loss_per_batch.append(loss.item()) 
        dic['train_loss'].append(mean(loss_per_batch))
    
        #=====> Validation
        test_loss_per_batch = []   
        model.eval()
        for i, (features,labels,v_idx,wVal) in enumerate(val_loader):
            test_outputs = model(features.float())  
            test_outputs = test_outputs.squeeze(-1)
            if(g_CL2.CL2_LOSSWEIGHTS):
                w=wVal
            test_loss = criterion(test_outputs, labels.float())
            test_loss_per_batch.append(test_loss.item()) 
            #Accuracy
            temp = test_outputs.cpu().detach().numpy()
            predicted_labels = np.zeros(len(temp))
            predicted_labels[temp>0.5] = 1
            true_labels = labels.cpu().detach().numpy()
            accuracy = (predicted_labels == true_labels).sum()/len(true_labels)
            accuracy_list.append(accuracy)
            
            temp_out.append(test_outputs.detach().numpy())
            temp_true_out.append(labels.detach().numpy())
            temp_idx.append(v_idx.detach().numpy())
            if (epoch==g_CL2.CL2_EPOCHS-1):
                dic['weights'].append(wVal.detach().numpy())
        dic['val_loss'].append(mean(test_loss_per_batch))
        dic['accuracy'].append(mean(accuracy_list))
    
        #Best epoch check
        if(g.MIN_LOSS>mean(test_loss_per_batch)):
            g.MIN_LOSS = mean(test_loss_per_batch)
            dic['output']=[]
            dic['output_true']=[]
            dic['val_idx']=[]
            dic['output']=temp_out
            dic['output_true']=temp_true_out
            dic['val_idx']=temp_idx

        #Printing the status
        if epoch%5==0:
            print(f"Train_loss: {mean(loss_per_batch):.4f}, "
                f"Val_loss: {mean(test_loss_per_batch):.4f}, "
                f"Accuracy: {mean(accuracy_list):.4f}, "
                f"Epoch : {epoch}")
        if epoch==g_CL2.CL2_EPOCHS-1:
            print(f"Train_loss: {mean(loss_per_batch):.4f}, "
                f"Val_loss: {mean(test_loss_per_batch):.4f}, "
                f"Epoch : {epoch}")
            del temp_out, temp_true_out
            gc.collect()


#=====> Saving the trained model, the validation and test scores
print("Saving the model and the dictionary ...")
if(g.TRAIN_ON_ISS):
    torch.save(model.state_dict(), g.MODEL_PATH+"trained_model_CL2_ISS.pth")
    MyH5man.SaveH5(MyH5man.CheckDicType(dic), group_name="dic")
else:
    if(g_CL2.CL2_LOSSWEIGHTS):
        torch.save(model.state_dict(), g.MODEL_PATH+"trained_model_CL2_MC_withW.pth")
        MyH5man.SaveH5(MyH5man.CheckDicType(dic), group_name="dic")
    else:
        torch.save(model.state_dict(), g.MODEL_PATH+"trained_model_CL2_MC.pth")
        MyH5man.SaveH5(MyH5man.CheckDicType(dic), group_name="dic")
print("Done!")
