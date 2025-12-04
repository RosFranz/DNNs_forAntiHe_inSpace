import numpy as np
import pandas as pd
import h5py
import gc

from globals import g, PREg

class h5Manager:
    def __init__(
            self,
            in_path:str=None,
            out_path:str=None,
            start_row:int=0,
            max_rows:int=None,
            useHe3:bool=False,
            He3h5Name:str="",
            isCC:bool=False
            ):
        self.in_path_ = in_path
        self.out_path_ = out_path
        self.start_row_ = start_row
        self.max_rows_ = max_rows
        self.useHe3_ = useHe3
        self.He3Path_ = "/eos/home-f/frrossi/AMS/ams_network/dataset/"+He3h5Name 
        self.isCC_ = isCC
    
    def SetStartRow(
            self,
            start_row:int=0
            ):
            self.start_row_ = start_row
            print("Starting raw number to be read from h5 file set to: ", self.start_row_)
    
    def SetMaxRows(
            self,
            max_rows:int=None
            ):
        if max_rows is not None:
            self.max_rows_ = max_rows
            print("Max number of rows to be read from h5 file set to: ", self.max_rows_)
    
    def SetUseHe3(
            self,
            useHe3:bool=False
            ):
            self.useHe3_ = useHe3
            print("He3 flag set to: ", self.useHe3_)
            
    def SetIsCC(
            self,
            isCC:bool=False
            ):
            self.isCC_ = isCC
            print("CC flag set to: ", self.isCC_)
    
    def SetHe3h5Name(
            self,
            He3h5Name:str=None
            ):
        if He3h5Name is not None:
            self.He3Path_ = "/eos/home-f/frrossi/AMS/ams_network/dataset/"+He3h5Name
            print("Input path to iHe3 h5 file set to: ", self.He3Path_)
    
    def SetInPath(
            self,
            in_path:str=None
            ):
        if in_path is not None:
            self.in_path_ = in_path
            print("Input path to h5 file set to: ", self.in_path_)
    
    def SetOutPath(
            self,
            out_path:str=None
            ):
        if out_path is not None:
            self.out_path_ = out_path
            print("Output path to h5 file set to: ", self.out_path_)
        
    def ReadH5toDataFrame(
            self,
            group_name:str='dic'
            ):
        myDic = {}
        with h5py.File(self.in_path_, 'r') as myF:
            group = myF[group_name]
            for dataset_name in group.keys():
                total_rows = len(group[dataset_name])
                end_row = total_rows if self.max_rows_ is None else min(self.max_rows_, total_rows)
                myDic[dataset_name] = group[dataset_name][self.start_row_:end_row]
        if(self.useHe3_):
            print("Here I am adding He3 from ", self.He3Path_)
            n_rows = len(next(iter(myDic.values())))
            myDic["isHe3"] = np.zeros(n_rows,dtype=np.int32)
            with h5py.File(self.He3Path_, 'r') as myF:
                group = myF[group_name]
                for dataset_name in group.keys():
                    #max rows is None for testing, otherwise is training (or debugging)
                    total_rows = len(group[dataset_name])
                    if self.max_rows_ is None or self.start_row_ != 0:
                        end_row = total_rows
                        He3_start_row = int(0.7*total_rows)
                        if self.isCC_:
                            He3_start_row = 0
                    else:
                        end_row = int(min(self.start_row_ + self.max_rows_, int(total_rows*0.7)))
                        He3_start_row = 0
                        if self.isCC_:
                            end_row = total_rows
                        
                    data = group[dataset_name][He3_start_row:end_row]
                    if dataset_name in myDic:
                        myDic[dataset_name] = np.concatenate([myDic[dataset_name], data])
                    else:
                        myDic[dataset_name] = data
            myDic["isHe3"] = np.concatenate([myDic["isHe3"],np.ones(len(next(iter(myDic.values())))-n_rows)])        
        for k, v in myDic.items():
            print(k, len(v))
        df = pd.DataFrame(myDic)
        del myDic
        gc.collect()
        df.replace([np.inf, -np.inf], g.MYMASKVALUE, inplace=True)
        df.fillna(g.MYMASKVALUE, inplace=True)
        print("Length of the dic: ", len(df))
        return df
    
    def ReadH5toDic(
            self,
            group_name:str='dic'
            ):
        myDic = {}
        with h5py.File(self.in_path_, 'r') as myF:
            group = myF[group_name]
            for dataset_name in group.keys():
                myDic[dataset_name] = group[dataset_name][:]
        return myDic

    # Function to check if a dictionary contains non-numeric data and convert it to float64
    def CheckDicType(
            self,
            dic:dict
            ):
        print('My dictionary: ', dic.keys())
        for key, value in dic.items():
            if not isinstance(value, list):
                continue
            if all(isinstance(item, (list, np.ndarray)) for item in value):
                if value[0].ndim == 0:
                    print(f"Key: {key}, Value0 type: {type(value[0])}, Value0 dim: {value[0].ndim}")
                    flattened = np.array(value)
                else:
                    flattened = np.concatenate(value) # Flatten the sublists into a single array
                dic[key] = flattened
            else:  # If the value is a list of scalars, convert directly to a NumPy array
                dic[key] = np.array(value, dtype=np.float64)
            
            if isinstance(value, np.ndarray) and value.dtype == object:
                try:
                    dic[key] = value.astype(np.float64)  # Convert to float64
                except ValueError as e:
                    print(f"Error converting {key} to float64: {e}")
                    dic[key] = np.nan * np.ones_like(value, dtype=np.float64)
        return dic
    
    def SaveH5(
            self,
            dict:dict,
            group_name:str
            ):
        with h5py.File(self.out_path_, "w") as myFile:
            existing_group = myFile.create_group(group_name)
            for key, value in dict.items():
                maxshape = (None,) + value.shape[1:]
                existing_group.create_dataset(
                    key,
                    data=value,
                    maxshape=maxshape,  
                    chunks=True,        
                    compression="gzip",
                )
        del dict
        gc.collect()
    
    def MaskDataFrame(
            self,
            df:pd.DataFrame,
            mask_value=g.MYMASKVALUE
            ):
        df['SigmaInnerL1'] = (abs(df['Rinner']) - abs(df['RinnerL1']))/(abs(df['Rinner']) + abs(df['RinnerL1']))
        df.loc[df['RinnerL1'] == 0, 'SigmaInnerL1'] = mask_value
        df.loc[df['Rinner'] * df['RinnerL1'] < 0, 'SigmaInnerL1'] = mask_value
        assert (df['SigmaInnerL1'] == mask_value).sum() > 0, f"No replacements found in column: SigmaInnerL1!"
        
        # Replace values in selected columns where 'Column_A' is 0
        df.loc[df['hasRich'] == PREg.MISSING_RICH, PREg.MASK_RICH_COLS] = mask_value
        df.loc[(df[PREg.MASK_RICH_COLS] == PREg.MISSING_RICH).all(axis=1), PREg.MASK_RICH_COLS] = mask_value
        for colR in PREg.MASK_RICH_COLS:
            assert (df[colR] == mask_value).sum() > 0, f"No replacements found in column: {colR}!"
        
        if PREg.BUILD_MC:
            PREg.MASK_TRK_RES.extend(PREg.MASK_TRK_RES_MC)
        print('Checking MAS_TRK_RES: ', PREg.MASK_TRK_RES)
        print('Checking df columns: ', df.columns)
        for res in PREg.MASK_TRK_RES:
            df.loc[df[res] == PREg.MISSING_TRKres, res] = mask_value
            assert (df[res] == PREg.MISSING_TRKres).sum() == 0, f"Substitution failed in column: {res}"
        
        if (not g.DEBUG):
            for minQ in PREg.MASK_TRK_MIN:
                df.loc[df[minQ] == PREg.MISSING_TRKmin, minQ] = mask_value
                assert (df[minQ] == PREg.MISSING_TRKmin).sum() == 0, f"Substitution failed in column: {minQ}"
            for maxQ in PREg.MASK_TRK_MAX:
                df.loc[df[maxQ] == PREg.MISSING_TRKmax, maxQ] = mask_value
                assert (df[maxQ] == PREg.MISSING_TRKmax).sum() == 0, f"Substitution failed in column: {maxQ}"
        return df
