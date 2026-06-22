from torch.utils.data import Dataset
from globals import g

class AEdic:
    """Autoencoder dictionary"""
    def Initialize(inDIC=None):
        if g.TRAIN_ON_ISS:
            inDIC = {
                'train_loss': [],
                'val_loss': [],
                'val_idx': [],
                'output': [],
                'output_error': [],
                'anomaly_score': []}
                # 'test_idx': [],
                # 'NegOut': [],
                # 'NegOut_error': [],
                # 'NEGanomaly_score': []}
        else:
            inDIC = {
                'train_loss': [],
                'val_loss': [],
                'val_idx': [],
                'output': [],
                'output_error': [],
                'anomaly_score': [],
                'weights': []}
                # 'test_idx': [],
                # 'NegOut': [],
                # 'NegOut_error': [],
                # 'NEGanomaly_score': [],
                # 'weights_neg': []}
        return inDIC

# Load the MC dataset for validation
#To have the labels in the validation set
class MyDatasetWithIdx(Dataset):
    """
    A dataset loader that returns:
      - features X
      - original indices (orig_idx) for bookkeeping
    """
    def __init__(self, X, orig_idx):
        self.X = X
        self.orig_idx = orig_idx
    def __getitem__(self, index):
            return self.X[index], self.orig_idx[index]
    def __len__(self):
        return len(self.X)

# Load the MC dataset with weights
class MyDatasetWithIdxW(Dataset):
    """
    A dataset loader that returns:
      - features X
      - optional weights
      - original indices (orig_idx) for bookkeeping
    """
    def __init__(self, X, orig_idx, weights=None):
        self.X = X
        self.orig_idx = orig_idx
        self.weights = weights
    def __getitem__(self, index):
        if self.weights is not None:
            return self.X[index], self.orig_idx[index], self.weights[index]
        else:
            return self.X[index], self.orig_idx[index]
    def __len__(self):
        return len(self.X)