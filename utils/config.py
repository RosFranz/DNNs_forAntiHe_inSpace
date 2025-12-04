import json
import yaml
from pathlib import Path
from typing import Dict, Any
from globals import g, g_CL2, g_AE, PREg

def update_config_from_file(config_instance, config_dict: Dict[str, Any]):
    """Update a config class instance with values from a dictionary"""
    for key, value in config_dict.items():
        if hasattr(config_instance, key):
            setattr(config_instance, key, value)
            
            
def load_config(config_path: str):
    """Load JSON or YAML config file"""
    path = Path(config_path)
    if path.suffix == '.json':
        with open(path) as f:
            return json.load(f)
    elif path.suffix in ('.yaml', '.yml'):
        with open(path) as f:
            return yaml.safe_load(f)
    raise ValueError(f"Unsupported config format: {path.suffix}")


def override(config_path: str = None):
    """
    Apply config overrides from file to base configs
    Args:
        base_configs: Your existing g or g_CL2 or g_AE
        config_path: Path to JSON/YAML config file
        config_dict: Direct config dictionary
    Returns:
        Updated config namespace
    """
    if config_path is None:
        print("No config (.json/.yml/.yaml) file path provided, using default values contained in globals.py")
    else:
        # Load from file if provided
        override_config = {}
        if config_path:
            override_config = load_config(config_path)
        # Apply overrides
        if 'g' in override_config:
            update_config_from_file(g, override_config['g'])
        if 'PREg' in override_config:
            update_config_from_file(PREg, override_config['PREg'])
            print()
            print('This config file has a prepocessing global info:')
            print('Current FILE date: ', PREg.FILE_DATE)
            print('Current FILE name: ', PREg.FILE_NAME)
            print('Current h5 file name: ', PREg.H5_NAME)
            print('Current value for missing RICH info: ', PREg.MISSING_RICH)
            print('Current value for missing TRK res info: ', PREg.MISSING_TRKres)
            print('Current value for missing TRK min Z info: ', PREg.MISSING_TRKmin)
            print('Current value for missing TRK max Z info: ', PREg.MISSING_TRKmax)  
            print()
            
        if 'g_CL2' in override_config:
            update_config_from_file(g_CL2, override_config['g_CL2'])
            g_CL2.CL2_INPUT_SIZE=len(g_CL2.CL2_COLUMNS)
            g_CL2.CL2_BATCH_SIZE=int(g_CL2.CL2_BATCH_SIZE)
        if 'g_AE' in override_config:
            update_config_from_file(g_AE, override_config['g_AE'])
            g_AE.AE_INPUT_SIZE=len(g_AE.AE_COLUMNS)
            g_AE.AE_LAYERS.append(g_AE.AE_INPUT_SIZE)
            g_AE.AE_BATCH_SIZE=int(g_AE.AE_BATCH_SIZE)
        if not g.TRAIN_ON_ISS:
            g_CL2.CL2_COLUMNS.append('weight')
            g_AE.AE_COLUMNS.append('weight')
    print()
    print('Current debug-flag:', g.DEBUG)
    print('Dataset path: ', g.DATASET_PATH)
    print('Model path  : ', g.MODEL_PATH)
    print('Dataset path: ', g.DATASET_PATH)
    print('New folder  : ', g.NEW_FOLDER_PATH)
    print()
    