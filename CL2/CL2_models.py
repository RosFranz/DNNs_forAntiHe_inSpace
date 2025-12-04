import torch.nn as nn
from globals import g

## Neural network model definition
class NeuralNet(nn.Module):
    '''
    A DL model with customizable layers and nodes for multi-class (4) classification.
    '''
    def __init__(self, input_size, layers, drop_prob=0.1):
        super(NeuralNet, self).__init__()
        self.layers = nn.ModuleList()
        last_size = input_size
        for layer_size in layers:
            self.layers.append(nn.Linear(last_size, layer_size))
            last_size = layer_size
        self.layers.append(nn.Linear(last_size, 1))
        self.relu = nn.ReLU()
        self.dp   = nn.Dropout(p=drop_prob)
        self.sigmoid = nn.Sigmoid()

    def forward(self, x):
        for layer in self.layers[:-1]:
            x = layer(x)
            x = self.relu(x)
            x = self.dp(x)
        x = self.layers[-1](x)
        return self.sigmoid(x)
    
##Custom masked layers
class maskedLinear(nn.Module):
    def __init__(self, in_features, out_features, mask_value=g.MYMASKVALUE):
        super().__init__()
        self.linear = nn.Linear(in_features, out_features)
        self.maskValue = mask_value
    
    def forward(self, x):
        mask = (x!= self.maskValue)    # True = valid, False = invalid (masked)
        masked_x = x * mask            # masking the input
        masked_x.requires_grad_(True)  # Explicitly enable gradients
        return self.linear(masked_x)*mask #avoid to propagate the biases also for masked values

## Masked Neural network model definition
class maskedNeuralNet(nn.Module):
    '''
    A DL model with masked layers for binary classification.
    '''
    def __init__(self, input_size, layers, drop_prob=0.1, mask_value=g.MYMASKVALUE):
        super(maskedNeuralNet, self).__init__()
        self.layers = nn.ModuleList()
        self.start_size = input_size
        self.maskValue = mask_value
        self.FirstLayer = maskedLinear(self.start_size, self.start_size, mask_value=self.maskValue)  
        last_size = input_size
        for layer_size in layers:
            self.layers.append(nn.Linear(last_size, layer_size))
            last_size = layer_size
        self.layers.append(nn.Linear(last_size, 1))
        self.relu = nn.ReLU()
        self.dp   = nn.Dropout(p=drop_prob)
        self.sigmoid = nn.Sigmoid()

    def forward(self, x):
        #x is the unmasked input      
        masked_x = self.relu(self.FirstLayer(x))
        for layer in self.layers[:-1]:
            masked_x = layer(masked_x)
            masked_x = self.relu(masked_x)
            masked_x = self.dp(masked_x)
        masked_x = self.layers[-1](masked_x)
        return self.sigmoid(masked_x)   