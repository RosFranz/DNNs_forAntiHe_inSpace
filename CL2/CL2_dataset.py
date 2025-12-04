from torch.utils.data import Dataset
from globals import g

class CL2dic:
    def Initialize(inDIC:dict=None):
        if g.TRAIN_ON_ISS:
            inDIC = {
                'train_loss': [],
                'val_loss': [],
                'val_idx': [],
                'output': [],
                'output_true': [],
                'accuracy': []}
        else:
            inDIC = {
                'train_loss': [],
                'val_loss': [],
                'val_idx': [],
                'output': [],
                'output_true': [],
                'accuracy': [],
                'weights': []}
        return inDIC

class myDataset(Dataset):
    '''
    a dataset loader 
    '''
    def __init__(self, X, y, orig_idx, weights=None):
        self.X = X
        self.y = y
        self.orig_idx = orig_idx
        self.weights = weights
    def __getitem__(self, index):
        if self.weights is not None:
            return self.X[index], self.y[index], self.orig_idx[index], self.weights[index]
        else:
            return self.X[index], self.y[index], self.orig_idx[index]
    def __len__(self):
        return len(self.X)

#For ISS-data
class myNoLabel_Dataset(Dataset):
    '''
    a dataset loader
    '''
    def __init__(self, X):
        self.X = X
    def __getitem__(self, index):
        return self.X[index]
    def __len__(self):
        return len(self.X)