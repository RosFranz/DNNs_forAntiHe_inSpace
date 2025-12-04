import torch
import torch.nn as nn
from globals import g

class MaskedMSELoss(nn.Module):
    def __init__(self, mask_value=g.MYMASKVALUE):
        super().__init__()
        self.maskValue = mask_value
    
    def forward(self, y_pred, y_true):
        mask = (y_true != self.maskValue)  # True = valid, False = invalid (masked)
        squared_error = (y_pred - y_true) ** 2
        masked_loss = torch.sum(squared_error * mask,1) / torch.sum(mask,1)  # Normalize by features
        return masked_loss