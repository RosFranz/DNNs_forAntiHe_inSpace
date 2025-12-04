# CL2/__init__.py
from .CL2_models import NeuralNet
from .CL2_models import maskedLinear
from .CL2_models import maskedNeuralNet

from .CL2_dataset import myDataset
from .CL2_dataset import myNoLabel_Dataset
from .CL2_dataset import CL2dic

__all__ = ['NeuralNet', 'maskedLinear',      'maskedNeuralNet',
           'myDataset', 'myNoLabel_Dataset', 'CL2dic']  