from catboost import CatBoostClassifier, Pool
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from sklearn.model_selection import StratifiedKFold
from collections import Counter
import matplotlib.ticker as ticker
from colour import Color
import sys
import argparse
import gc
from pathlib import Path
plt.rc('font', size=20)

sys.path.append(str(Path(__file__).resolve().parent.parent.parent))
sys.path.append(str(Path(__file__).resolve().parent.parent.parent / "utils"))
sys.path.append(str(Path(__file__).resolve().parent.parent.parent / "scaler"))
from utils import override
from globals import g, g_CL2
from utils import h5Manager
from scaler import MaskedStandardScaler

###############################################
# CHECK CAREFULLY THE FOLLOWING PARAMETERS:
random = True
target_like = False 
catboost_seed = 10
n_fold = 5
###############################################

def ConvertMaskedToNan(Xarr:np.ndarray, ColumnsName):
    tempDf = pd.DataFrame(Xarr, columns=ColumnsName)
    tempDf = tempDf.replace(g.MYMASKVALUE, np.nan)
    Xarr = tempDf.values
    del tempDf
    gc.collect()
    return Xarr
def GetAndRemoveWeights(Xarr:np.ndarray, ColumnsName, getColNames=False):
    print("X shape: ", X.shape)
    print("len(ColumnNmes): ", len(ColumnsName))
    tempDf = pd.DataFrame(Xarr, columns=ColumnsName)
    MyWeights = tempDf['weight']
    MyWeights = np.abs(MyWeights)
    tempDf.drop(columns=['weight'], inplace=True)
    Xarr = tempDf.values
    if getColNames:
        ColumnsName = tempDf.columns
        return Xarr, MyWeights, ColumnsName
    else:
        return Xarr, MyWeights
    

#=====> Input arguments
parser = argparse.ArgumentParser(description="CL2 ranking script")
parser.add_argument("--config", type=str, default=None, help="Path to config file (JSON/YAML/YML)")
args = parser.parse_args()
override(args.config)

#=====> Setting seed for permutations
np.random.seed(g_CL2.CL2_SEED)

#=====> Columns to use
columns_to_keep = [#'ACC_AntiCounter',
                   #'TOF_OnTimeClusterL1',   'TOF_OnTimeClusterL2',   'TOF_OnTimeClusterL3',   'TOF_OnTimeClusterL4',
                   #'TOF_OffTimeClusterL1',  'TOF_OffTimeClusterL2',  'TOF_OffTimeClusterL3',  'TOF_OffTimeClusterL4',
                   #'TRK_ClusterOnL7_Y',     'TRK_ClusterOnL8_Y',
                   #'TRK_NormEdep2L2_XY',    'TRK_NormEdep2L3_XY',    'TRK_NormEdep2L4_XY',    'TRK_NormEdep2L5_XY',
                   #'TRK_NormEdep2L6_XY',    'TRK_NormEdep2L7_XY',    'TRK_NormEdep2L8_XY',
                   'TRK_TrackResidualL2_Y', 'TRK_TrackResidualL3_Y', 'TRK_TrackResidualL4_Y', 'TRK_TrackResidualL5_Y',
                   'TRK_TrackResidualL6_Y', 'TRK_TrackResidualL7_Y', 'TRK_TrackResidualL8_Y',
                   'TRK_TrackResidualL2_X', 'TRK_TrackResidualL3_X', 'TRK_TrackResidualL4_X', 'TRK_TrackResidualL5_X',
                   'TRK_TrackResidualL6_X', 'TRK_TrackResidualL7_X', 'TRK_TrackResidualL8_X',
                   'InnerHitX',             'InnerHitY',
                   'TRK_MAXchXYonTrackPlane1', 'TRK_MAXchXYonTrackPlane2', 'TRK_MAXchXYonTrackPlane3', 'TRK_MAXchXYonTrackPlane4',
                   'TRK_MINchXYonTrackPlane1', 'TRK_MINchXYonTrackPlane2', 'TRK_MINchXYonTrackPlane3', 'TRK_MINchXYonTrackPlane4',
                   #'chi2Coo_tof',           'chi2Time_tof',
                   #'beta_consistency_rich', 'beta_consistencyTOF',
                   #'kprob_rich',            'charge_rich',           'ringPMTs_rich',
                   'charge2_rich',          'totPE_Uncorr_rich',     'measPE_Uncorr_rich',
                   'measPE_Corr_rich',      'chargedPMTs_rich',      'ringPMTs2_rich',
                   'totHits_rich',          'ringHits_rich',         'SigmaInnerL1',          'weight']
      
print('Columns to keep:')          
print(columns_to_keep)

#=====> h5 manager
print('Initializing the h5 manager')
if g.TRAIN_ON_ISS:
    INpath = g.DATASET_PATH+'ISSpos_dic.h5'
else:
    INpath = g.DATASET_PATH+'He4.h5'
if g.DEBUG:
    MyH5man = h5Manager(in_path=INpath, max_rows=2000000)
else: 
    MyH5man = h5Manager(in_path=INpath, max_rows=10000000)
    
UnNormData = MyH5man.ReadH5toDataFrame()
#Selecting only some columns
label = UnNormData['RigLabel'].values
UnNormData.drop(columns=[col for col in UnNormData.columns if col not in columns_to_keep], inplace=True)
gc.collect()


print('Current columns POSITIVES:', UnNormData.columns, 'Total columns:', len(UnNormData.columns))

#=====> Selecting balanced dataset
print("Dropping signal events to match background events number")
print('Signal events', len(label[label==1]))
print('Bkg events', len(label[label==0]))
 #--------
lbkg = label[label==0]
lsig = label[label==1][:len(lbkg)]
 #--------
Xbkg=UnNormData[label==0].values
Xsig=UnNormData[label==1].values
 #--------
PosColumns = UnNormData.columns.tolist()
del UnNormData
gc.collect()

 #permutation of the signal events and keeping only lbkg events
permSig = np.random.permutation(len(lsig))[:len(lbkg)]
lsig = lsig[permSig][:len(lbkg)]
Xsig = Xsig[permSig][:len(lbkg)]
del permSig

 #putting all together
X=np.vstack((Xsig,Xbkg))
label=np.hstack((lsig, lbkg))
print('Signal events', len(label[label==1]))
print('Bkg events', len(label[label==0]))
print('Label shape', label.shape[0])
print('X shape', X.shape[0])
permutation = np.random.permutation(X.shape[0])
label = label[permutation]
X = X[permutation]
del Xsig, Xbkg, lsig, lbkg, permutation
gc.collect()

#=====> Random or signal-like var
if random:
    columns_to_keep= columns_to_keep+['random_var']
    PosColumns.append('random_var')
    X = np.column_stack((X, np.random.rand(X.shape[0], 1)))
if target_like: 
    columns_to_keep= columns_to_keep+['target_like']
    PosColumns.append('target_like')
    X = np.column_stack((X, np.random.rand(len(X.shape[0]))))
    print('Correlation coeff. of target like var:', np.corrcoef(label,X[:, -1]))
print('Current length of columns Pos', len(PosColumns))
print("Positive columns ", PosColumns)

#=====> Labels
# print("Type of label:", type(label))
# print("Shape of label:", label.shape)
# print("Unique values in label:", np.unique(label))
# print('Label_train', label)
# print("Number of NaN values in label:", np.isnan(label).sum())
# print("Number of inf values in label:", np.isinf(label).sum())
# print('Check on deleting correctly')
# print(columns_to_keep)
# if(random and target_like):
#     del columns_to_keep[-3]
# if(random and (not target_like)):
#     del columns_to_keep[-2]
# if(target_like and (not random)):
#     del columns_to_keep[-2]
# if((not random) and (not target_like)):
#     del columns_to_keep[-1]
# print(columns_to_keep)


#=====> Standard scaler
scaler = MaskedStandardScaler(mask_value=g.MYMASKVALUE)

kf = StratifiedKFold(n_splits = n_fold)
feat_all = []
score_all=[]

i=0
loss_cv = []
loss_cv_val = []
for train_index, test_index in kf.split(X, label):
    X_cv, x_test_cv = np.array(X)[train_index], np.array(X)[test_index]
    label_cv, y_test_cv = np.array(label)[train_index], np.array(label)[test_index]
    if not g.TRAIN_ON_ISS:
        X_cv, X_cv_W = GetAndRemoveWeights(Xarr=X_cv,ColumnsName=PosColumns, getColNames=False)
        x_test_cv, x_test_cv_W, NoWColumns = GetAndRemoveWeights(Xarr=x_test_cv,ColumnsName=PosColumns, getColNames=True)
    scaler.fit(X_cv)
    X_cv = scaler.transform(X_cv)
    x_test_cv = scaler.transform(x_test_cv)
    
    
    if g.TRAIN_ON_ISS:
        #=====> Set Masked Values to nan: CatBoost automatically handles missing values (nan_mode)
        X_cv = ConvertMaskedToNan(Xarr=X_cv,ColumnsName=PosColumns)
        x_test_cv = ConvertMaskedToNan(Xarr=x_test_cv,ColumnsName=PosColumns)
        train_pool = Pool(data=X_cv, label=label_cv)
        eval_pool  = Pool(data=x_test_cv, label=y_test_cv)
    else:
        #=====> Set Masked Values to nan: CatBoost automatically handles missing values (nan_mode)
        X_cv = ConvertMaskedToNan(Xarr=X_cv,ColumnsName=NoWColumns)
        x_test_cv = ConvertMaskedToNan(Xarr=x_test_cv,ColumnsName=NoWColumns)
        train_pool = Pool(data=X_cv, label=label_cv, weight=X_cv_W)
        eval_pool  = Pool(data=x_test_cv, label=y_test_cv, weight=x_test_cv_W)

    model = CatBoostClassifier(iterations = 5000,
                            depth= 8, 
                            l2_leaf_reg = 3,
                            learning_rate =0.01, 
                            nan_mode='Min',  # "Min": When missing values might indicate "absence"
                            loss_function='Logloss', 
                            task_type="CPU",
                            devices='0:1',
                            verbose=False,
                            random_seed=catboost_seed)
    model.fit(train_pool, eval_set=eval_pool,  early_stopping_rounds = 20)
    loss_cv.append(model.get_evals_result()['learn']['Logloss'])
    loss_cv_val.append(model.get_evals_result()['validation']['Logloss'])
    importances = model.get_feature_importance()
    print('---> Fold N.:',i, ' Importances:')
    print(importances)
    a=pd.DataFrame()

    a['Importance']= importances
    if g.TRAIN_ON_ISS:
        a['Feature'] = PosColumns
    else:
        a['Feature'] = NoWColumns


    a = a.sort_values('Importance', ascending=False)
    a.insert(0,'Ranking', np.arange(len(a))+1)

    feat_all.append(a['Feature'].values)
    score_all.append(a['Importance'].values)
    i += 1
    
print('Dictionary A:')
print(a)

for i in range(0,len(score_all)):
    score_all[i] = score_all[i]/sum(score_all[i])

len_feat = []
for it in feat_all:
    len_feat.append(len(it))
num_feats = min(len_feat)

print('Num feats:', num_feats)

feat_numfeats = []
score_numfeats = []
for feats in feat_all:
    feat_numfeats.append(feats[:num_feats])

cc = Counter(np.vstack(feat_numfeats).flatten())
most_comm_feats = cc.most_common(num_feats)
label_plot = [el[0] for el in most_comm_feats]

scores_best = np.zeros((num_feats, n_fold))
for i,el in enumerate(feat_all):
    for j,feat in enumerate(most_comm_feats):
        try:
            posf = np.where(el[:num_feats] == most_comm_feats[j][0])[0][0]
            scores_best[j, i] = score_all[i][posf]
        except:
            continue
        
sorted_labels_plot=np.array(label_plot)[np.argsort(np.median(scores_best, axis=1))[::-1]]

plt.figure(figsize=(16,12))
plt.yticks(rotation=0, size=20)  # Changed from plt.xticks to plt.yticks for label orientation
plt.tick_params(axis='x', which='major', labelsize=20)
plt.xlabel('Feature importance', fontsize=25)

plot = []
for i in np.argsort(np.median(scores_best, axis=1)):
    plot.append(scores_best[i][scores_best[i]>=0])
plt.boxplot(plot, tick_labels=sorted_labels_plot[::-1], whiskerprops=dict(linewidth=4), medianprops=dict(linewidth=5), vert=False)
plt.xscale('log') 
plt.rc('axes', linewidth=2)
plt.grid(which='both')
plt.gca().xaxis.set_minor_locator(ticker.LogLocator(base=10, subs='all'))
plt.title('Feature ranking', fontsize=35)
plt.tight_layout()

if(random):
    if g.TRAIN_ON_ISS:
        plt.savefig('ISS_BW_feature_ranking_rndm.png', dpi=300)
    else:
        plt.savefig('MC_BW_feature_ranking_rndm.png', dpi=300)
if(target_like):
    if g.TRAIN_ON_ISS:
        plt.savefig('ISS_Color_feature_ranking_trgtLike.png', dpi=300)  
    else:
        plt.savefig('MC_Color_feature_ranking_trgtLike.png', dpi=300)  

if((not random) and (not target_like)):
    if g.TRAIN_ON_ISS:
        plt.savefig('ISS_feature_ranking.png', dpi=300)  
    else:
        plt.savefig('MC_feature_ranking.png', dpi=300)  

red = Color("red")
colors = list(red.range_to(Color("green"),50))
plt.figure(figsize=(20,15))
plt.yticks(rotation=0, size=20) 
plt.tick_params(axis='x', which='major', labelsize=20)
plt.xlabel('Feature importance', fontsize=25)
print('Loop on enumerate plot')
for i,j in enumerate(plot):
    print(np.median(plot[i]), sorted_labels_plot[::-1][i])
    if np.median(plot[i]) > 0.02 :
        color= 'limegreen'
    elif np.median(plot[i]) > 0.003 :
        color = 'gold'
    elif  np.median(plot[i]) > 0.00005 :
        color = 'orange'
    else:
        color = colors[0].rgb
    plt.boxplot(np.array(plot[i]), tick_labels=[sorted_labels_plot[::-1][i]],positions=[i], widths=.5,patch_artist=True, 
                boxprops=dict(facecolor=color, color=color, linewidth=4),
            capprops=dict(color=color, linewidth=4),
            whiskerprops=dict(color=color, linewidth=3),
            flierprops=dict(color=color, markerfacecolor=color,markeredgecolor=color, markersize=9),
            medianprops=dict(color='black',linewidth=4),
            vert=False)
plt.xscale('log')
plt.rc('axes', linewidth=2)
plt.grid(which='both')
plt.gca().xaxis.set_minor_locator(ticker.LogLocator(base=10, subs='all'))
plt.title('Feature ranking', fontsize=35)
plt.tight_layout()

if(random):
    if g.TRAIN_ON_ISS:
        plt.savefig('ISS_Color_feature_ranking_rndm.png', dpi=300)
    else:
        plt.savefig('MC_Color_feature_ranking_rndm.png', dpi=300)

if(target_like):
    if g.TRAIN_ON_ISS:
        plt.savefig('ISS_Color_feature_ranking_trgtLike.png', dpi=300)  
    else:
        plt.savefig('MC_Color_feature_ranking_trgtLike.png', dpi=300)  

if((not random) and (not target_like)):
    if g.TRAIN_ON_ISS:
        plt.savefig('ISS_feature_ranking.png', dpi=300)
    else:
        plt.savefig('MC_feature_ranking.png', dpi=300)


b=pd.DataFrame()
b['Feature'] = sorted_labels_plot[::-1]
b['Importance']= np.mean(plot, axis = 1)
b['Sigma']= np.std(plot, axis = 1)
b['Relative']= np.std(plot, axis = 1)/np.mean(plot, axis = 1)
if g.TRAIN_ON_ISS:
    b.to_csv('ISS_ranking_catboost.csv')
else:
    b.to_csv('MC_ranking_catboost.csv')
print("Done!")

