import numpy as np
import pandas as pd
import h5py
import gc

from globals import g, PREg

def DFforCLtrain(df:pd.DataFrame) -> pd.DataFrame:
    df_bkg = df[df["RigLabel"] == 0]
    df_sig = df[df["RigLabel"] == 1]

    #--------
    #permutation of the signal events and keeping only lbkg events
    #--------
    permSig = np.random.permutation(len(df_sig))[:len(df_bkg)]
    df_sig = df_sig.iloc[permSig]
    
    #--------
    #putting all together
    #--------
    df_out = pd.concat([df_sig, df_bkg], ignore_index=True)
    df_out = df_out.sample(frac=1).reset_index(drop=True)

    #print("")
    #print("Original dimension: ", df.shape)
    #print("Updated dimension: ", df_out.shape)
    #print("")

    del df
    gc.collect()

    return df_out
