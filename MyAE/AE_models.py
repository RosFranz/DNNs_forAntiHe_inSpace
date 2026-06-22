import torch.nn as nn
from globals import g

# Symmetric Auto encoder model definition
class AEsym(nn.Module):
    '''
    Autoencoder symmetric model with customizable layers and nodes
    '''
    def __init__(self, input_size, layers):
        super(AE, self).__init__()
        self.layers = nn.ModuleList()
        last_size = input_size
        self.relu = nn.ReLU()
        self.softmax = nn.Softmax(dim=1)
        self.sigmoid = nn.Sigmoid()
        
        for layer_size in layers:
            self.layers.append(nn.Linear(last_size, layer_size))
            last_size = layer_size
        
        self.encoder = nn.Sequential(
            *[nn.Sequential(layer, self.relu) for layer in self.layers]
        )
        
        # Create the decoder by reversing the layers
        self.decoder_layers = nn.ModuleList()
        reversed_layers = list(reversed(self.layers))
        for i in range(len(reversed_layers)):
            self.decoder_layers.append(nn.Linear(reversed_layers[i].out_features, reversed_layers[i].in_features))

        # self.decoder = nn.Sequential(
        #     *[nn.Sequential(layer, self.relu) for layer in self.decoder_layers[:-1]],
        #     self.decoder_layers[-1],  # Last layer without ReLU
        #     self.softmax
        # )
        
        self.decoder = nn.Sequential(
            *[nn.Sequential(layer, self.relu) for layer in self.decoder_layers[:-1]],
            self.decoder_layers[-1], self.sigmoid
        )

    def forward(self, x):
        encoded = self.encoder(x)
        decoded = self.decoder(encoded)
        return decoded

# Arbitrary AE
class AE(nn.Module):
    '''
    Autoencoder (even asymettric) model with customizable layers and nodes
    '''
    def __init__(self, input_size, layers):
        super(AE, self).__init__()
        self.layers = nn.ModuleList()
        last_size = input_size
        self.relu = nn.ReLU()
        self.softmax = nn.Softmax(dim=1)
        self.sigmoid = nn.Sigmoid()
        
        for layer_size in layers:
            self.layers.append(nn.Linear(last_size, layer_size))
            last_size = layer_size
        
        self.EncDec = nn.Sequential(
            *[nn.Sequential(layer, self.relu) for layer in self.layers[:-1]],
            self.layers[-1])
        # , self.sigmoid
        # )

    def forward(self, x):
        return self.EncDec(x)

class maskedLinear(nn.Module):
    """Masked linear layer."""
    def __init__(self, in_features, out_features, mask_value=g.MYMASKVALUE):
        super().__init__()
        self.linear = nn.Linear(in_features, out_features)
        self.maskValue = mask_value
    
    def forward(self, x):
        mask = (x!= self.maskValue)     # True = valid, False = invalid (masked)
        masked_x = x * mask            # masking the input
        masked_x.requires_grad_(True)  # Explicitly enable gradients
        return self.linear(masked_x)*mask #avoid to propagate the biases also for masked values

# Arbitrary masked AE
class maskedAE(nn.Module):
    '''
    Masked autoencoder model with customizable layers and nodes
    '''
    def __init__(self, input_size:int, layers:list, mask_value:int=g.MYMASKVALUE):
        super(maskedAE, self).__init__()
        self.layers = nn.ModuleList()
        self.start_size = input_size
        self.maskValue = mask_value
        last_size = input_size
        self.FirstLayer = maskedLinear(self.start_size, self.start_size, mask_value=self.maskValue)  
        self.relu = nn.ReLU()
        self.softmax = nn.Softmax(dim=1)
        self.sigmoid = nn.Sigmoid()
        for layer_size in layers:
            self.layers.append(nn.Linear(last_size, layer_size))
            last_size = layer_size
        
        self.EncDec = nn.Sequential(
            *[nn.Sequential(layer, self.relu) for layer in self.layers[:-1]],
            self.layers[-1])

    def forward(self, x):
        #x is the unmasked input      
        reconstructed = self.EncDec(self.relu(self.FirstLayer(x)))  #missing values are set to 0
        mask = (x!= self.maskValue) # True = valid, False = invalid (masked)
        return reconstructed * mask # the reconstructed masked values is always 0
