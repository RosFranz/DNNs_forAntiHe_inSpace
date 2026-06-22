import numpy as np
from sklearn.base import TransformerMixin
from utils import h5Manager
from globals import g


class MaskedStandardScaler(TransformerMixin):
    """Custom masked standard scaler"""
    def __init__(self, mask_value=g.MYMASKVALUE):
        self.mask_value = mask_value
        self.mean_ = None
        self.std_ = None

    def fit(self, X, y=None):
        maskedX = np.ma.masked_where(X == self.mask_value, X) #True = invalid (masked), False = valid, 
        self.mean_ = np.zeros(X.shape[1])
        self.std_  = np.zeros(X.shape[1])
        for i in range(X.shape[1]):
            self.mean_[i] = np.ma.mean(maskedX[:,i])
            self.std_[i]  = np.ma.std(maskedX[:,i], ddof=1)  # the denominator is N-ddof
        print("Mean avoiding masked values: ", self.mean_)
        print("Std avoiding masked values: ", self.std_)
        print("Current masked value: ", self.mask_value)
        return self
    
    def transform(self, X:np.ndarray): 
        X = X.astype(np.float64)
        print(f"X dtype: {X.dtype}, shape: {X.shape}")
        print(f"mean_ dtype: {self.mean_.dtype}, shape: {self.mean_.shape}")
        print(f"std_ dtype: {self.std_.dtype}, shape: {self.std_.shape}")
        if np.isnan(X).any():
            print(f"Input X dtype: {X.dtype}, shape: {X.shape}")
            print(f"NaN in X: {np.isnan(X).any()}")
            X[np.isnan(X)] = self.mask_value
        if np.isinf(X).any():
            print(f"Input X dtype: {X.dtype}, shape: {X.shape}")
            print(f"Inf in X: {np.isinf(X).any()}")
            X[np.isinf(X)] = self.mask_value
        if np.isnan(self.mean_).any() or np.isinf(self.mean_).any():
            print(f"NaN in mean_: {np.isnan(self.mean_).any()}")
            print(f"Inf in mean_: {np.isinf(self.mean_).any()}")
        if np.isnan(self.std_).any() or np.isinf(self.std_).any() or np.any(self.std_ == 0):
            print(f"NaN in std_: {np.isnan(self.std_).any()}")
            print(f"Inf in std_: {np.isinf(self.std_).any()}")
            print(f"Zero in std_: {np.any(self.std_ == 0)}")
        maskedX = np.ma.masked_where(X == self.mask_value, X) #True = invalid (masked), False = valid, 
        print(maskedX.shape)
        print(self.mean_.shape)
        for i in range(X.shape[1]):
            maskedX[:,i] = (maskedX[:,i] - self.mean_[i]) / self.std_[i]
        return maskedX.data
    
    def get_mean(self):
        return self.mean_
    def get_std(self):
        return self.std_
    
    def set_MeanStd(self, MyMean:np.ndarray, MyStd:np.ndarray):
        self.mean_ = MyMean
        self.std_ = MyStd
        if MyMean is None or MyStd is None:
            raise ValueError("Provide two np.ndarrays as mean and std")
        print("Mean set to: ", self.mean_)
        print("Std set to: ", self.std_)
        
    def save(self, path:str):
        h5_manager = h5Manager(in_path=None, out_path=path, max_rows=None)
        dic_scaler = {}
        if self.mean_ is None or self.std_ is None:
            raise ValueError("You must call fit() before saving the scaler info.")
        dic_scaler['mean'] = self.mean_
        dic_scaler['std']  = self.std_    
        h5_manager.SaveH5(dic_scaler, group_name='scaler_fit_values')
        del h5_manager