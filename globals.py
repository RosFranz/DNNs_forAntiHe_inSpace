#Global info used in prepocessing, training, validation and testing 
class Config_g:
    #Debug flag
    DEBUG = False
    #Some default path
    if DEBUG:
        DATASET_PATH  = '/home/franz/AMS/ams_network/dataset/'
        MODEL_PATH    = '/home/franz/AMS/ams_network/models/'
        VALDIC_PATH   = '/home/franz/AMS/ams_network/val_dic/'
        NEW_FOLDER_PATH = '/home/franz/AMS/output/organized_output/scores/'
    else:
        DATASET_PATH  = '/eos/home-f/frrossi/AMS/ams_network/dataset/'
        MODEL_PATH    = '/eos/home-f/frrossi/AMS/ams_network/models/dev/'
        VALDIC_PATH   = '/eos/home-f/frrossi/AMS/ams_network/val_dic/dev/'
        NEW_FOLDER_PATH = '/eos/home-f/frrossi/AMS/output/organized_output/scores/dev/'

    #MC R<0 dataset
    MC_NEG_DATASET = -1 # -1 for no MC, 0->label5, 1->label10, 2->label15
    
    TRAIN_ON_ISS = False #Train on ISS data
    IS_SIGNAL = True     #True to train on R>0 samples

    #Mask missing values in training
    USEMASK = True
    MYMASKVALUE = -9999

    MIN_LOSS = 1.e+8 #Starting value to save on the best epoch  

    #Reproduce old results
    IS_OLD_RESULT = False
    OLD_RES = '20_02_2025' #CL2 and AE"

    if not TRAIN_ON_ISS:
        CUSTOM_H5_TEST_NAME = ''
        CUSTOM_ROOT_NAME = ''

g = Config_g()

#CL2 info used in training, validation and testing 
class Config_CL2:
    CL2_COLUMNS = ['TRK_TrackResidualL2_Y', 'TRK_TrackResidualL3_Y', 'TRK_TrackResidualL4_Y',
                    'TRK_TrackResidualL5_Y', 'TRK_TrackResidualL6_Y', 'TRK_TrackResidualL7_Y', 
                    'TRK_TrackResidualL3_X', 'TRK_TrackResidualL5_X', 'TRK_TrackResidualL6_X',
                    'InnerHit',              'SigmaInnerL1',          
                    'totPE_Uncorr_rich',     'charge2_rich',          'ringPMTs2_rich',
                    'TRK_MAXchXYonTrackPlane1', 'TRK_MAXchXYonTrackPlane2', 'TRK_MAXchXYonTrackPlane3', 'TRK_MAXchXYonTrackPlane4',
                    'TRK_MINchXYonTrackPlane1', 'TRK_MINchXYonTrackPlane2', 'TRK_MINchXYonTrackPlane3', 'TRK_MINchXYonTrackPlane4']

    #CL2 model parameters
    # # if USEMASK is enabled there is a "hidden" (hard-coded) first layer 
    # that has the same dimension of the starting value this allow to neglect
    # the propagation of biases for masked values. 
    CL2_INPUT_SIZE = len(CL2_COLUMNS)
    CL2_LAYERS  = [30,20]

    CL2_SEED = 30       #Seed for reproducibility
    TEST_FRACTION = 0.3
    CL2_LR   = 1.e-2
    DROPOUT  = 0.
    if g.TRAIN_ON_ISS:
        CL2_LOSSWEIGHTS = False #Use MC weights on the training loss 
        CL2_EPOCHS     = 500
        CL2_BATCH_SIZE = int(1.e+3)
    else:
        CL2_LOSSWEIGHTS = True #Use MC weights on the training loss
        CL2_COLUMNS.append("weight")
        if g.DEBUG:
            CL2_EPOCHS = 2
            CL2_BATCH_SIZE = int(5.e+3)
        else:
            CL2_EPOCHS = 70
            CL2_BATCH_SIZE = int(2.e+5)

g_CL2 = Config_CL2()

#AE info used in training, validation and testing 
class Config_AE:
    AE_COLUMNS = ['SigmaInnerL1',          'SigmaUpLow',
                   'TRK_TrackResidualL2_Y', 'TRK_TrackResidualL3_Y', 'TRK_TrackResidualL4_Y',
                   'TRK_TrackResidualL5_Y', 'TRK_TrackResidualL6_Y', 'TRK_TrackResidualL7_Y', 
                   'TRK_TrackResidualL8_Y',
                   'TRK_TrackResidualL3_X', 'TRK_TrackResidualL5_X', 'TRK_TrackResidualL7_X',
                   'TRK_TrackResidualL8_X',
                   'TRK_NormEdep2L5_XY',    'TRK_NormEdep2L6_XY',    'TRK_NormEdep2L7_XY',
                   'TRK_NormEdep2L8_XY',
                   'InnerHit',      
                   'ACC_AntiCounter', 
                   'TOF_OnTimeClusterL3',   'TOF_OnTimeClusterL4',
                   'TRK_MAXchXYonTrackPlane1', 'TRK_MAXchXYonTrackPlane2', 'TRK_MAXchXYonTrackPlane3', 'TRK_MAXchXYonTrackPlane4',
                   'TRK_MINchXYonTrackPlane1', 'TRK_MINchXYonTrackPlane2', 'TRK_MINchXYonTrackPlane3', 'TRK_MINchXYonTrackPlane4']
                   
    # if USEMASK is enabled there is a "hidden" (hard-coded) first layer that has the same 
    # dimension of the starting value this allow to neglect the propagation of biases
    # for masked values. 
    AE_INPUT_SIZE = len(AE_COLUMNS)
    AE_LAYERS = [10, 5, 10, AE_INPUT_SIZE]
    #AE model parameters
    AE_SEED = 10       #Seed for reproducibility
    TEST_FRACTION = 0.3
    AE_LR  = 5.e-4
    SWITCH = False #Invert training and validation set
    if g.TRAIN_ON_ISS:
        AE_LOSSWEIGHTS = False #Use MC weights on the training loss
        AE_BATCH_SIZE = int(3.e+3)
        AE_EPOCHS = 200
    else:
        AE_LOSSWEIGHTS = True #Use MC weights on the training loss
        AE_COLUMNS.append('weight')
        if g.DEBUG:
            AE_BATCH_SIZE = int(5.e+3)
            AE_EPOCHS = 2
        else:
            AE_BATCH_SIZE = int(5.e+5)
            AE_EPOCHS = 70
    
g_AE = Config_AE()

#Prepocessing info used to build .h5 file from root tress
class Config_Pre_g:
    if g.DEBUG:
        FILE_PATH       = '/home/franz/AMS/output/organized_output/MLDs/'
        FILE_DATE       = '2025_02_26'
        FILE_NAME       = 'Positive'
    else:
        FILE_PATH       = '/eos/home-f/frrossi/AMS/output/organized_output/MLDs/'
        FILE_DATE       = '2025_04_22'
        FILE_NAME       = 'Positive'
        
    #Other possible dates:
    # MC pos -> 2025_02_26  / 2025_03_22 / 2025_04_25
    # MC neg -> 2025_02_25  / 2025_03_22 / 2025_04_22
    # ISS pos -> 2025_01_27 / 2025_04_25
    # ISS neg -> 2025_01_27 / 2025_04_17
    
    COLUMNS_DEF = ['ACC_AntiCounter',
                #TOF
                'TOF_Z',                 'TOF_Zup',               'TOF_Zlow',
                'TOF_Z_noPL',            'TOF_Zup_noPL',          'TOF_Zlow_noPL',
                'TOF_OnTimeClusterL1',   'TOF_OnTimeClusterL2',   'TOF_OnTimeClusterL3',   'TOF_OnTimeClusterL4',
                'TOF_OffTimeClusterL1',  'TOF_OffTimeClusterL2',  'TOF_OffTimeClusterL3',  'TOF_OffTimeClusterL4',
                'TOF_TrkCluster',        'TOF_BetaCluster',
                'beta_tof',              'gamma_tof',             'chi2Coo_tof',           'chi2Time_tof',
                'mass_tof',              'massInv_tof',           'mass2_tof',
                #TRK
                'TRK_InnerZ',               'TRK_InnerZrms',
                'TRK_FeetMinDistPlane1',    'TRK_FeetMinDistPlane2',    'TRK_FeetMinDistPlane3',    'TRK_FeetMinDistPlane4',
                'TRK_FitL7X',               'TRK_FitL7Y',               'TRK_HitL7X',               'TRK_HitL7Y',
                'TRK_FitL8X',               'TRK_FitL8Y',               'TRK_HitL8X',               'TRK_HitL8Y',
                'TRK_MINchXonTrackPlane1',  'TRK_MINchXonTrackPlane2',  'TRK_MINchXonTrackPlane3',  'TRK_MINchXonTrackPlane4',
                'TRK_MINchYonTrackPlane1',  'TRK_MINchYonTrackPlane2',  'TRK_MINchYonTrackPlane3',  'TRK_MINchYonTrackPlane4',
                'TRK_MINchXYonTrackPlane1', 'TRK_MINchXYonTrackPlane2', 'TRK_MINchXYonTrackPlane3', 'TRK_MINchXYonTrackPlane4',
                'TRK_MAXchXonTrackPlane1',  'TRK_MAXchXonTrackPlane2',  'TRK_MAXchXonTrackPlane3',  'TRK_MAXchXonTrackPlane4',
                'TRK_MAXchYonTrackPlane1',  'TRK_MAXchYonTrackPlane2',  'TRK_MAXchYonTrackPlane3',  'TRK_MAXchYonTrackPlane4',
                'TRK_MAXchXYonTrackPlane1', 'TRK_MAXchXYonTrackPlane2', 'TRK_MAXchXYonTrackPlane3', 'TRK_MAXchXYonTrackPlane4',
                'TRK_ClusterOnL2_Y',        'TRK_ClusterOnL3_Y',        'TRK_ClusterOnL4_Y',        'TRK_ClusterOnL5_Y',
                'TRK_ClusterOnL6_Y',        'TRK_ClusterOnL7_Y',        'TRK_ClusterOnL8_Y',
                'TRK_NormEdep2L2_XY',       'TRK_NormEdep2L3_XY',       'TRK_NormEdep2L4_XY',       'TRK_NormEdep2L5_XY',
                'TRK_NormEdep2L6_XY',       'TRK_NormEdep2L7_XY',       'TRK_NormEdep2L8_XY',
                'TRK_TrackResidualL2_Y',    'TRK_TrackResidualL3_Y',    'TRK_TrackResidualL4_Y',    'TRK_TrackResidualL5_Y',
                'TRK_TrackResidualL6_Y',    'TRK_TrackResidualL7_Y',    'TRK_TrackResidualL8_Y',
                'TRK_TrackResidualL2_X',    'TRK_TrackResidualL3_X',    'TRK_TrackResidualL4_X',    'TRK_TrackResidualL5_X',
                'TRK_TrackResidualL6_X',    'TRK_TrackResidualL7_X',    'TRK_TrackResidualL8_X',
                'InnerHitX',                'InnerHitY',                
                'SigmaUpLow',               'SigmaInnerL1',
                'RinnerUH',     'RinnerLH',
                'invRerr_INUH', 'invRerr_INLH',
                'TrChiSqXUH',   'TrChiSqXLH',   
                'Rinner',            'RinnerL1',     'RinnerL9',    'RfullSpan',             
                'invRerr_IN',        'invRerr_INL1', 'invRerr_INL9',
                'TrChiSqXInnerOnly', 'TrChiSqXInL1', 'TrChiSqXInL9',
                'TrChiSqYInnerOnly', 'TrChiSqYInL1', 'TrChiSqYInL9',
                #RICH
                'hasRich',               'isNaF',                 
                'hasGoodImpact',         'isBorder_rich',         'isBadAglTiles',         'isAglTilesEdge',        'isBadAglRegions',        
                'beta_rich',             'beta_err_rich',         'gamma_rich',            'beta_consistency_rich', 'beta_consistencyTOF',
                'kprob_rich',            'charge_rich',           'charge2_rich',          'totPE_Uncorr_rich',     'charge_consistency_rich',
                'measPE_Uncorr_rich',    'measPE_Corr_rich',      'expPE_Uncorr_rich',     'expPE_Corr_rich', 
                'chargedPMTs_rich',      'ringPMTs_rich',         'ringPMTs2_rich',        'totHits_rich',          'ringHits_dir_rich',
                'ringHits_ref_rich',     'badClusters_rich',      'secHits_rich',
                'mass_rich',             'massInv_rich',          'massInv_err_rich',      'mass2_rich',
                #EVENT SUMMARY
                'NParticle',             'NTrTrack',               'NTofCluster',          'NRichRing',
                #EVENT
                'EvRun',                 'EvNum']
                #Currently excluded columns
                #    'TRK_MaxClusterDistanceL2_Y', 'TRK_MaxClusterDistanceL3_Y', 'TRK_MaxClusterDistanceL4_Y', 'TRK_MaxClusterDistanceL5_Y',
                #    'TRK_MaxClusterDistanceL6_Y', 'TRK_MaxClusterDistanceL7_Y', 'TRK_MaxClusterDistanceL8_Y',
                #    'TRK_TrackHitL2_Y',      'TRK_TrackHitL3_Y',      'TRK_TrackHitL4_Y',      'TRK_TrackHitL5_Y',
                #    'TRK_TrackHitL6_Y',      'TRK_TrackHitL7_Y',      'TRK_TrackHitL8_Y',
                #    'TRK_ClusterOnL2_X',     'TRK_ClusterOnL3_X',     'TRK_ClusterOnL4_X',     'TRK_ClusterOnL5_X',
                #    'TRK_ClusterOnL6_X',     'TRK_ClusterOnL7_X',     'TRK_ClusterOnL8_X',
                #    'TRK_MaxClusterDistanceL2_X', 'TRK_MaxClusterDistanceL3_X', 'TRK_MaxClusterDistanceL4_X', 'TRK_MaxClusterDistanceL5_X',
                #    'TRK_MaxClusterDistanceL6_X', 'TRK_MaxClusterDistanceL7_X', 'TRK_MaxClusterDistanceL8_X',
                #    'TRK_TrackHitL2_X',      'TRK_TrackHitL3_X',      'TRK_TrackHitL4_X',      'TRK_TrackHitL5_X',
                #    'TRK_TrackHitL6_X',      'TRK_TrackHitL7_X',      'TRK_TrackHitL8_X',

    COLUMNS_MC = ['TRK_TrueHitResidualL1_X', 'TRK_TrueHitResidualL2_X', 'TRK_TrueHitResidualL3_X', 'TRK_TrueHitResidualL4_X',
                  'TRK_TrueHitResidualL5_X', 'TRK_TrueHitResidualL6_X', 'TRK_TrueHitResidualL7_X', 'TRK_TrueHitResidualL8_X',
                  'TRK_TrueHitResidualL1_Y', 'TRK_TrueHitResidualL2_Y', 'TRK_TrueHitResidualL3_Y', 'TRK_TrueHitResidualL4_Y',
                  'TRK_TrueHitResidualL5_Y', 'TRK_TrueHitResidualL6_Y', 'TRK_TrueHitResidualL7_Y', 'TRK_TrueHitResidualL8_Y',
                  'Rtrue',                   'label5',                  'label10',                  'label15',
                  'weight',                  'weightISO',               'weightCO']
    COLUMNS_ISS = ['IGRFpos', 'IGRFneg','isAbIGRFpos','isAbIGRFneg','BartelNum', 'PhRun']
    #True to build h5 for MC
    BUILD_MC = False
    #True for Charge Confused samples
    IS_CC    = False
    CL_TRAINorVAL = False
    
    #Force the name of the H5 file (with extension)
    H5_NAME  = ''
    #Current default names:
    #MCHe4posdeb_dic.h5 -> He4 R>0
    #MCHe4negdeb_dic.h5 -> He4 R<0
    #ISSposdeb_dic.h5   -> DATA R>0
    #ISSnegdeb_dic.h5   -> DATA R<0
    
    #OLD (before 10.06.2025) default names depending from BUILD_MC and IS_CC
    #MCSIG_dic.h5     -> He4 R>0
    #MC15_dic.h5      -> He4 R<0 (label15)
    #MC10deb_dic.h5   -> He4 R<0 (label10)
    #MC5_dic.h5       -> He4 R<0 (label5)
    #ISSpos_dic.h5    -> DATA R>0 
    #ISSnegdeb_dic.h5 -> DATA R<0
    
    #################################
    # Values used to cover missing information 
    MISSING_RICH   = 0
    MISSING_TRKres = -2
    MISSING_TRKmin = 100
    MISSING_TRKmax = 0
    # Define columns to mask
    MASK_RICH_COLS = ['kprob_rich',          'charge_rich',             'charge2_rich',      'totPE_Uncorr_rich',
                      'measPE_Uncorr_rich',  'measPE_Corr_rich',        'chargedPMTs_rich',  'ringPMTs_rich',
                      'ringPMTs2_rich',      'totHits_rich',            'ringHits_rich',     'beta_consistency_rich',
                      'beta_consistencyTOF', 'charge_consistency_rich', 'expPE_Uncorr_rich', 'expPE_Corr_rich',
                      'ringHits_dir_rich',   'ringHits_ref_rich',       'badClusters_rich',  'secHits_rich']
              
    MASK_TRK_RES = ['TRK_TrackResidualL2_Y', 'TRK_TrackResidualL3_Y', 'TRK_TrackResidualL4_Y', 'TRK_TrackResidualL5_Y',
                    'TRK_TrackResidualL6_Y', 'TRK_TrackResidualL7_Y', 'TRK_TrackResidualL8_Y',
                    'TRK_TrackResidualL2_X', 'TRK_TrackResidualL3_X', 'TRK_TrackResidualL4_X', 'TRK_TrackResidualL5_X',
                    'TRK_TrackResidualL6_X', 'TRK_TrackResidualL7_X', 'TRK_TrackResidualL8_X',
                    'TRK_NormEdep2L2_XY',    'TRK_NormEdep2L3_XY',    'TRK_NormEdep2L4_XY',    'TRK_NormEdep2L5_XY',
                    'TRK_NormEdep2L6_XY',    'TRK_NormEdep2L7_XY',    'TRK_NormEdep2L8_XY']
    MASK_TRK_RES_MC = ['TRK_TrueHitResidualL1_X', 'TRK_TrueHitResidualL2_X', 'TRK_TrueHitResidualL3_X', 'TRK_TrueHitResidualL4_X',
                       'TRK_TrueHitResidualL5_X', 'TRK_TrueHitResidualL6_X', 'TRK_TrueHitResidualL7_X', 'TRK_TrueHitResidualL8_X',
                       'TRK_TrueHitResidualL1_Y', 'TRK_TrueHitResidualL2_Y', 'TRK_TrueHitResidualL3_Y', 'TRK_TrueHitResidualL4_Y',
                       'TRK_TrueHitResidualL5_Y', 'TRK_TrueHitResidualL6_Y', 'TRK_TrueHitResidualL7_Y', 'TRK_TrueHitResidualL8_Y']
    
    MASK_TRK_MIN = ['TRK_MINchXonTrackPlane1',  'TRK_MINchXonTrackPlane2',  'TRK_MINchXonTrackPlane3',  'TRK_MINchXonTrackPlane4',
                    'TRK_MINchYonTrackPlane1',  'TRK_MINchYonTrackPlane2',  'TRK_MINchYonTrackPlane3',  'TRK_MINchYonTrackPlane4',
                    'TRK_MINchXYonTrackPlane1', 'TRK_MINchXYonTrackPlane2', 'TRK_MINchXYonTrackPlane3', 'TRK_MINchXYonTrackPlane4']
    MASK_TRK_MAX = ['TRK_MAXchXonTrackPlane1',  'TRK_MAXchXonTrackPlane2',  'TRK_MAXchXonTrackPlane3',  'TRK_MAXchXonTrackPlane4',
                    'TRK_MAXchYonTrackPlane1',  'TRK_MAXchYonTrackPlane2',  'TRK_MAXchYonTrackPlane3',  'TRK_MAXchYonTrackPlane4',
                    'TRK_MAXchXYonTrackPlane1', 'TRK_MAXchXYonTrackPlane2', 'TRK_MAXchXYonTrackPlane3', 'TRK_MAXchXYonTrackPlane4']
    #################################
    

PREg = Config_Pre_g()
