// Author: F.Rossi
// Created on: 24.02.2025
// Last modified on: 25.05.2025
// Revision 1.0 -> checking compatibility
// Revision 1.1 -> dividing in rigidity bins
/*
 To be used on models trained on MC
 Plot inverse mass vs scores
 -------------------------------------------------------
    How to use it:

    give the folders date as input parameters and
    just run this command:
        root -l 'trainOnMC_mass.cc("CLtrainOnMCFile", "CLvalOnISSFile", "AEtrainOnMCFile", "AEvalOnISSFile")'
*/

//Works with NAIA v.1.2.
#include <iostream>
#include <fstream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <array>
#include <vector>

//ROOT
#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>
#include <TH1.h>
#include <TCanvas.h>
#include <TMarker.h>
#include <TSystem.h>
#include <TH2.h>
#include <TH3F.h>
#include <TStyle.h>
#include <TSpline.h>
#include <TVector3.h>
#include <TString.h>

using namespace std;

TH1D *LogToLin1D(TH1D *h1);
TH2D *LogToLin2D(TH2D *h2);

void trainOnMC_mass(TString CLtrainOnMCFile="000", TString CLvalOnISSFile="000", TString AEtrainOnMCFile="000", TString AEvalOnISSFile="000"){
    gStyle->SetPalette(55);//kRainBow palette

    const bool useSafetyFactor = false; //Safety factor = 1.2
    const bool useCutOff       = false; // Value of the cutoff (no safety factor)
    const bool UseLossWeights  = true ; // Loss weights have been used in the training
    if(useSafetyFactor) printf("---> Using GM-cutoff with a safety fafctor of 1.2\n");
    if(useCutOff) printf("---> Using Cutoff with a safety fafctor of 1.\n");
    const float uma    = 0.9315; //GeV/c^2
    const float LabelSize = 0.045, TitleSize = 0.05;

    const int TotFiles = 4; //number of files
    const int TotTree  = 4; //n. of CLtrees
    const int TotRig   = 2; //n. of rigidities
    const int TotDet   = 2; //n. of detectors

    const int Ncut     = 10; // n. of interval on scores

    TString InPath("/eos/home-f/frrossi/AMS/output/organized_output/scores/");
    array<TString, TotFiles> WithWeightsName{"_trainOnMC_withW_2CL_MC_scores.root",     "_trainOnMC_withW_2CL_ISS_scores.root",
                                             "_trainOnMC_withW_AE_MC_scores.root",      "_trainOnMC_withW_AE_ISS_scores.root"};
    array<TString, TotFiles> NoWeightsName{"_trainOnMC_2CL_MC_scores.root",             "_trainOnMC_2CL_ISS_scores.root",
                                           "_trainOnMC_AE_MC_scores.root",               "_trainOnMC_AE_ISS_scores.root"};

    array<TString, TotFiles> PathToFile{InPath+CLtrainOnMCFile+"/"+CLtrainOnMCFile,     InPath+CLvalOnISSFile+"/"+CLvalOnISSFile,
                                        InPath+AEtrainOnMCFile+"/"+AEtrainOnMCFile,     InPath+AEvalOnISSFile+"/"+AEvalOnISSFile};
    for(int i=0; i<TotFiles; ++i) (UseLossWeights) ? PathToFile.at(i) += WithWeightsName.at(i) : PathToFile.at(i) += NoWeightsName.at(i);

    TString outName("/eos/home-f/frrossi/AMS/ams_network/root_files/TrainOnMC/");
    (UseLossWeights) ? outName +=  "ISS_MC_mass_withW.root" : outName += "ISS_MC_mass.root";
    array<TFile*, TotFiles> files;
    TFile *f_out = new TFile(outName.Data(), "RECREATE");
    array<TTree*, TotTree> CLtrees;
    array<TTree*, TotTree> AEtrees;
    //TH1 vector
    vector<TH1*> H1cl_R, H1cl_TEST_R, H1ae_R, H1ae_TEST_R;
    vector<TH1*> H1_CSinvMassWAgl, H1_TEST_CSinvMassWAgl, H1_ASinvMassWAgl, H1_TEST_ASinvMassWAgl;
    vector<TH1*> H1_CSinvMassWNaF, H1_TEST_CSinvMassWNaF, H1_ASinvMassWNaF, H1_TEST_ASinvMassWNaF;
    //TH2 vectors
    vector<TH2D*> H2_combinvMassWAgl, H2_combinvMassWNaF;
    vector<TH2D*> H2_combinvMassWAglN, H2_combinvMassWNaFN;
    vector<TH2D*> H2_CSinvMassNaF,  H2_TEST_CSinvMassNaF,  H2_ASinvMassNaF,  H2_TEST_ASinvMassNaF; 
    vector<TH2D*> H2cl_RinvMassNaF, H2cl_TEST_RinvMassNaF, H2ae_RinvMassNaF, H2ae_TEST_RinvMassNaF;
    vector<TH2D*> H2_CSinvMassAgl,  H2_TEST_CSinvMassAgl,  H2_ASinvMassAgl,  H2_TEST_ASinvMassAgl;
    vector<TH2D*> H2cl_RinvMassAgl, H2cl_TEST_RinvMassAgl, H2ae_RinvMassAgl, H2ae_TEST_RinvMassAgl;
    vector<TH2D*> H2_RCS,           H2_TEST_RCS,           H2_RAS,           H2_TEST_RAS;
    array<double, Ncut> clCutValues; // from 0 to 1
    for(int i=0; i<clCutValues.size(); ++i) clCutValues.at(i) = 1./clCutValues.size()-0.005+(i*1./clCutValues.size());
    array<double, Ncut> aeCutValues; // from 1 to 0.5
    for(int i=0; i<aeCutValues.size(); ++i) aeCutValues.at(i) = 1. - (i*0.5/aeCutValues.size());
    printf("Check on last CS cut: %f\n", clCutValues.at(clCutValues.size()-1));
    printf("Check on last AS cut: %f\n", aeCutValues.at(aeCutValues.size()-1));


    array<TString, TotRig> NameSub{"CL","AE"};
    array<TString, TotRig> NameR{"Rinner", "RinnerL1"};
    array<TString, TotTree> DirName{"MC_pos", "MC_neg", "ISS_pos", "ISS_neg"};
    //CLassifier score
    array<TString, TotTree/2> nCSinvMassWAglISS{"CSinvMassWPosAgl",  "CSinvMassWNegAgl"};
    array<TString, TotTree/2> nCSinvMassWNaFISS{"CSinvMassWPosNaF",  "CSinvMassWNegNaF"};
    array<TString, TotTree/2> nCSinvMassWAglTEST{"CSinvMassWISSPosAgl", "CSinvMassWISSNegAgl"};
    array<TString, TotTree/2> nCSinvMassWNaFTEST{"CSinvMassWISSPosNaF", "CSinvMassWISSNegNaF"};
    //Anomaly score
    array<TString, TotTree/2> nASinvMassWAglISS{"ASinvMassWPosAgl",  "ASinvMassWNegAgl"};
    array<TString, TotTree/2> nASinvMassWNaFISS{"ASinvMassWPosNaF",  "ASinvMassWNegNaF"};
    array<TString, TotTree/2> nASinvMassWAglTEST{"ASinvMassWISSPosAgl", "ASinvMassWISSNegAgl"};
    array<TString, TotTree/2> nASinvMassWNaFTEST{"ASinvMassWISSPosNaF", "ASinvMassWISSNegNaF"};
    //Combination
    array<TString, TotTree/2> ncombinvMassWAglISS{"combinvMassWPosAgl",  "combinvMassWNegAgl"};
    array<TString, TotTree/2> ncombinvMassWNaFISS{"combinvMassWPosNaF",  "combinvMassWNegNaF"};
    array<TString, TotTree/2> ncombinvMassWAglISSN{"combinvMassWPosAglN",  "combinvMassWNegAglN"};
    array<TString, TotTree/2> ncombinvMassWNaFISSN{"combinvMassWPosNaFN",  "combinvMassWNegNaFN"};
    array<TString, TotTree/2> ncombinvMassWAglTEST{"combinvMassWISSPosAgl", "combinvMassWISSNegAgl"};
    array<TString, TotTree/2> ncombinvMassWNaFTEST{"combinvMassWISSPosNaF", "combinvMassWISSNegNaF"};
    TString tCSinvMassW    = ";Classifier score cut value; Event fraction m^{Agl}#in[2, 5] [a.m.u.]";
    TString tASinvMassW    = ";Anomaly score cut value; Event fraction m^{Agl}#in[2, 5] [a.m.u.]";
    TString tcombinvMassW  = ";Anomaly score cut value; Classifier score cut value; Event fraction m^{Agl}#in[2, 5] [a.m.u.]";
    TString tcombinvMassWN = ";Anomaly score cut value; Classifier score cut value; Event number m^{Agl}#in[2, 5] [a.m.u.]";

    //Selections
    TString NaFSel = "hasRich==1 && beta_rich>0.75 && isNaF==1 && "
                     "isBorder_rich==0 && kprob_rich>0.01 && 1<charge2_rich && "
                     "charge2_rich<4 && ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4";
    TString AglSel = "hasRich==1 && beta_rich>0.953 && isNaF==0 && "
                     "hasGoodImpact==1 && kprob_rich>0.01 && 1<charge2_rich && "
                     "charge2_rich<4 && ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4";
    TString RichSel = "hasRich==1 && beta_rich>0.75 && hasGoodImpact==1 && "
                      "kprob_rich>0.01 && 1<charge2_rich && charge2_rich<4 && "
                      "ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4";
    TString AglRinnerInvMass   = " && 0.2 <= (1./((2.*abs(Rinner)*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))) && "
                                       "(1./((2.*abs(Rinner)*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))) <= 0.5";
    TString NaFRinnerInvMass   = " && 0.2 <= (1./((2.*abs(Rinner)*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))) && "
                            "(1./((2.*abs(Rinner)*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))) <= 0.5";
    TString isAbIGRFpos  = "&& isAbIGRFpos==1";
    TString isAbIGRFneg  = "&& isAbIGRFneg==1";
    TString IGRFpos = "&& IGRFpos<Rinner";
    TString IGRFneg = "&& abs(IGRFneg)<abs(Rinner)";
    // TString AglRinnerL1InvMass = AglSel+"0.2 <= (1./((2.*abs(RinnerL1)*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))) && 
    //                                    (1./((2.*abs(RinnerL1)*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))) <= 0.4";
    array<TString, TotTree> InL1Sel{" && RinnerL1>0", " && RinnerL1<0", " && RinnerL1>0", " && RinnerL1<0",};
    vector<TString> RSel, RCSSel, invMassNaFSel, invMassAglSel;
    printf("NaF selection: %s\n", NaFSel.Data());
    printf("Agl selection: %s\n", AglSel.Data());
    printf("Rich selection: %s\n\n\n", RichSel.Data());
    TString TotSelection, TemName;
    //classifier
    vector<TString> clAglVALposNumSel, clAglVALposDenomSel, clAglVALnegNumSel, clAglVALnegDenomSel, clAglTESTposNumSel, clAglTESTposDenomSel, clAglTESTnegNumSel, clAglTESTnegDenomSel;
    vector<TString> clNaFVALposNumSel, clNaFVALposDenomSel, clNaFVALnegNumSel, clNaFVALnegDenomSel, clNaFTESTposNumSel, clNaFTESTposDenomSel, clNaFTESTnegNumSel, clNaFTESTnegDenomSel;
    //ae
    vector<TString> aeAglVALposNumSel, aeAglVALposDenomSel, aeAglVALnegNumSel, aeAglVALnegDenomSel, aeAglTESTposNumSel, aeAglTESTposDenomSel, aeAglTESTnegNumSel, aeAglTESTnegDenomSel;
    vector<TString> aeNaFVALposNumSel, aeNaFVALposDenomSel, aeNaFVALnegNumSel, aeNaFVALnegDenomSel, aeNaFTESTposNumSel, aeNaFTESTposDenomSel, aeNaFTESTnegNumSel, aeNaFTESTnegDenomSel;
    //comb
    vector<TString> combAglVALposNumSel, combAglVALposDenomSel, combAglVALnegNumSel, combAglVALnegDenomSel, combAglTESTposNumSel, combAglTESTposDenomSel, combAglTESTnegNumSel, combAglTESTnegDenomSel;
    vector<TString> combNaFVALposNumSel, combNaFVALposDenomSel, combNaFVALnegNumSel, combNaFVALnegDenomSel, combNaFTESTposNumSel, combNaFTESTposDenomSel, combNaFTESTnegNumSel, combNaFTESTnegDenomSel;
    //initialize the event count selection
    for(int i=0; i<aeCutValues.size(); ++i){
        //ae
        aeAglVALposNumSel.push_back("&& MC.anomaly_score<"+to_string(aeCutValues.at(i)));
        aeAglVALposDenomSel.push_back("&& MC.anomaly_score<"+to_string(aeCutValues.at(i)));
        aeAglVALnegNumSel.push_back("&& MCneg.anomaly_score<"+to_string(aeCutValues.at(i)));
        aeAglVALnegDenomSel.push_back("&& MCneg.anomaly_score<"+to_string(aeCutValues.at(i)));
        aeAglTESTposNumSel.push_back("&& ISS.anomaly_score<"+to_string(aeCutValues.at(i)));
        aeAglTESTposDenomSel.push_back("&& ISS.anomaly_score<"+to_string(aeCutValues.at(i)));
        aeAglTESTnegNumSel.push_back("&& ISSneg.anomaly_score<"+to_string(aeCutValues.at(i)));
        aeAglTESTnegDenomSel.push_back("&& ISSneg.anomaly_score<"+to_string(aeCutValues.at(i)));

        aeNaFVALposNumSel.push_back("&& MC.anomaly_score<"+to_string(aeCutValues.at(i)));
        aeNaFVALposDenomSel.push_back("&& MC.anomaly_score<"+to_string(aeCutValues.at(i)));
        aeNaFVALnegNumSel.push_back("&& MCneg.anomaly_score<"+to_string(aeCutValues.at(i)));
        aeNaFVALnegDenomSel.push_back("&& MCneg.anomaly_score<"+to_string(aeCutValues.at(i)));
        aeNaFTESTposNumSel.push_back("&& ISS.anomaly_score<"+to_string(aeCutValues.at(i)));
        aeNaFTESTposDenomSel.push_back("&& ISS.anomaly_score<"+to_string(aeCutValues.at(i)));
        aeNaFTESTnegNumSel.push_back("&& ISSneg.anomaly_score<"+to_string(aeCutValues.at(i)));
        aeNaFTESTnegDenomSel.push_back("&& ISSneg.anomaly_score<"+to_string(aeCutValues.at(i)));
        if(useSafetyFactor) {
            aeAglTESTposNumSel.back()   = AglSel+aeAglTESTposNumSel.back()+isAbIGRFpos+AglRinnerInvMass;
            aeAglTESTposDenomSel.back() = AglSel+aeAglTESTposDenomSel.back()+isAbIGRFpos;
            aeAglTESTnegNumSel.back()   = AglSel+aeAglTESTnegNumSel.back()+isAbIGRFneg+AglRinnerInvMass;
            aeAglTESTnegDenomSel.back() = AglSel+aeAglTESTnegDenomSel.back()+isAbIGRFneg;

            aeNaFTESTposNumSel.back()   = NaFSel+aeNaFTESTposNumSel.back()+isAbIGRFpos+NaFRinnerInvMass;
            aeNaFTESTposDenomSel.back() = NaFSel+aeNaFTESTposDenomSel.back()+isAbIGRFpos;
            aeNaFTESTnegNumSel.back()   = NaFSel+aeNaFTESTnegNumSel.back()+isAbIGRFneg+NaFRinnerInvMass;
            aeNaFTESTnegDenomSel.back() = NaFSel+aeNaFTESTnegDenomSel.back()+isAbIGRFneg;
        }
        if(useCutOff){
            aeAglTESTposNumSel.back()   = AglSel+aeAglTESTposNumSel.back()+IGRFpos+AglRinnerInvMass;
            aeAglTESTposDenomSel.back() = AglSel+aeAglTESTposDenomSel.back()+IGRFpos;
            aeAglTESTnegNumSel.back()   = AglSel+aeAglTESTnegNumSel.back()+IGRFneg+AglRinnerInvMass;
            aeAglTESTnegDenomSel.back() = AglSel+aeAglTESTnegDenomSel.back()+IGRFneg;

            aeNaFTESTposNumSel.back()   = NaFSel+aeNaFTESTposNumSel.back()+IGRFpos+NaFRinnerInvMass;
            aeNaFTESTposDenomSel.back() = NaFSel+aeNaFTESTposDenomSel.back()+IGRFpos;
            aeNaFTESTnegNumSel.back()   = NaFSel+aeNaFTESTnegNumSel.back()+IGRFneg+NaFRinnerInvMass;
            aeNaFTESTnegDenomSel.back() = NaFSel+aeNaFTESTnegDenomSel.back()+IGRFneg;
        }
        if(!useCutOff && !useSafetyFactor) {
            aeAglTESTposNumSel.back()   = AglSel+aeAglTESTposNumSel.back()+AglRinnerInvMass;
            aeAglTESTposDenomSel.back() = AglSel+aeAglTESTposDenomSel.back();
            aeAglTESTnegNumSel.back()   = AglSel+aeAglTESTnegNumSel.back()+AglRinnerInvMass;
            aeAglTESTnegDenomSel.back() = AglSel+aeAglTESTnegDenomSel.back();

            aeNaFTESTposNumSel.back()   = NaFSel+aeNaFTESTposNumSel.back()+NaFRinnerInvMass;
            aeNaFTESTposDenomSel.back() = NaFSel+aeNaFTESTposDenomSel.back();
            aeNaFTESTnegNumSel.back()   = NaFSel+aeNaFTESTnegNumSel.back()+NaFRinnerInvMass;
            aeNaFTESTnegDenomSel.back() = NaFSel+aeNaFTESTnegDenomSel.back();
        }
        aeAglVALposNumSel.back()   = AglSel+aeAglVALposNumSel.back()+AglRinnerInvMass;
        aeAglVALposDenomSel.back() = AglSel+aeAglVALposDenomSel.back();
        aeAglVALnegNumSel.back()   = AglSel+aeAglVALnegNumSel.back()+AglRinnerInvMass;
        aeAglVALnegDenomSel.back() = AglSel+aeAglVALnegDenomSel.back();

        aeNaFVALposNumSel.back()   = NaFSel+aeNaFVALposNumSel.back()+NaFRinnerInvMass;
        aeNaFVALposDenomSel.back() = NaFSel+aeNaFVALposDenomSel.back();
        aeNaFVALnegNumSel.back()   = NaFSel+aeNaFVALnegNumSel.back()+NaFRinnerInvMass;
        aeNaFVALnegDenomSel.back() = NaFSel+aeNaFVALnegDenomSel.back();
    }
    for(int i=0; i<clCutValues.size(); ++i){
        //classifier
        clAglVALposNumSel.push_back("&& scores>"+to_string(clCutValues.at(i)));
        clAglVALposDenomSel.push_back("&& scores>"+to_string(clCutValues.at(i)));
        clAglVALnegNumSel.push_back("&& scores>"+to_string(clCutValues.at(i)));
        clAglVALnegDenomSel.push_back("&& scores>"+to_string(clCutValues.at(i)));
        clAglTESTposNumSel.push_back("&& scores>"+to_string(clCutValues.at(i)));
        clAglTESTposDenomSel.push_back("&& scores>"+to_string(clCutValues.at(i)));
        clAglTESTnegNumSel.push_back("&& scores>"+to_string(clCutValues.at(i)));
        clAglTESTnegDenomSel.push_back("&& scores>"+to_string(clCutValues.at(i)));

        clNaFVALposNumSel.push_back("&& scores>"+to_string(clCutValues.at(i)));
        clNaFVALposDenomSel.push_back("&& scores>"+to_string(clCutValues.at(i)));
        clNaFVALnegNumSel.push_back("&& scores>"+to_string(clCutValues.at(i)));
        clNaFVALnegDenomSel.push_back("&& scores>"+to_string(clCutValues.at(i)));
        clNaFTESTposNumSel.push_back("&& scores>"+to_string(clCutValues.at(i)));
        clNaFTESTposDenomSel.push_back("&& scores>"+to_string(clCutValues.at(i)));
        clNaFTESTnegNumSel.push_back("&& scores>"+to_string(clCutValues.at(i)));
        clNaFTESTnegDenomSel.push_back("&& scores>"+to_string(clCutValues.at(i)));
        
        if(useSafetyFactor) {
            clAglTESTposNumSel.back()   = AglSel+clAglTESTposNumSel.back()+isAbIGRFpos+AglRinnerInvMass;
            clAglTESTposDenomSel.back() = AglSel+clAglTESTposDenomSel.back()+isAbIGRFpos;
            clAglTESTnegNumSel.back()   = AglSel+clAglTESTnegNumSel.back()+isAbIGRFneg+AglRinnerInvMass;
            clAglTESTnegDenomSel.back() = AglSel+clAglTESTnegDenomSel.back()+isAbIGRFneg;

            clNaFTESTposNumSel.back()   = NaFSel+clNaFTESTposNumSel.back()+isAbIGRFpos+NaFRinnerInvMass;
            clNaFTESTposDenomSel.back() = NaFSel+clNaFTESTposDenomSel.back()+isAbIGRFpos;
            clNaFTESTnegNumSel.back()   = NaFSel+clNaFTESTnegNumSel.back()+isAbIGRFneg+NaFRinnerInvMass;
            clNaFTESTnegDenomSel.back() = NaFSel+clNaFTESTnegDenomSel.back()+isAbIGRFneg;
        }
        if(useCutOff){
            clAglTESTposNumSel.back()   = AglSel+clAglTESTposNumSel.back()+IGRFpos+AglRinnerInvMass;
            clAglTESTposDenomSel.back() = AglSel+clAglTESTposDenomSel.back()+IGRFpos;
            clAglTESTnegNumSel.back()   = AglSel+clAglTESTnegNumSel.back()+IGRFneg+AglRinnerInvMass;
            clAglTESTnegDenomSel.back() = AglSel+clAglTESTnegDenomSel.back()+IGRFneg;

            clNaFTESTposNumSel.back()   = NaFSel+clNaFTESTposNumSel.back()+IGRFpos+NaFRinnerInvMass;
            clNaFTESTposDenomSel.back() = NaFSel+clNaFTESTposDenomSel.back()+IGRFpos;
            clNaFTESTnegNumSel.back()   = NaFSel+clNaFTESTnegNumSel.back()+IGRFneg+NaFRinnerInvMass;
            clNaFTESTnegDenomSel.back() = NaFSel+clNaFTESTnegDenomSel.back()+IGRFneg;
        }
        if(!useCutOff && !useSafetyFactor) {
            clAglTESTposNumSel.back()   = AglSel+clAglTESTposNumSel.back()+AglRinnerInvMass;
            clAglTESTposDenomSel.back() = AglSel+clAglTESTposDenomSel.back();
            clAglTESTnegNumSel.back()   = AglSel+clAglTESTnegNumSel.back()+AglRinnerInvMass;
            clAglTESTnegDenomSel.back() = AglSel+clAglTESTnegDenomSel.back();

            clNaFTESTposNumSel.back()   = NaFSel+clNaFTESTposNumSel.back()+NaFRinnerInvMass;
            clNaFTESTposDenomSel.back() = NaFSel+clNaFTESTposDenomSel.back();
            clNaFTESTnegNumSel.back()   = NaFSel+clNaFTESTnegNumSel.back()+NaFRinnerInvMass;
            clNaFTESTnegDenomSel.back() = NaFSel+clNaFTESTnegDenomSel.back();
        }

        clAglVALposNumSel.back()   = AglSel+clAglVALposNumSel.back()+AglRinnerInvMass;
        clAglVALposDenomSel.back() = AglSel+clAglVALposDenomSel.back();
        clAglVALnegNumSel.back()   = AglSel+clAglVALnegNumSel.back()+AglRinnerInvMass;
        clAglVALnegDenomSel.back() = AglSel+clAglVALnegDenomSel.back();

        clNaFVALposNumSel.back()   = NaFSel+clNaFVALposNumSel.back()+NaFRinnerInvMass;
        clNaFVALposDenomSel.back() = NaFSel+clNaFVALposDenomSel.back();
        clNaFVALnegNumSel.back()   = NaFSel+clNaFVALnegNumSel.back()+NaFRinnerInvMass;
        clNaFVALnegDenomSel.back() = NaFSel+clNaFVALnegDenomSel.back();
    }
    for(int i=0; i<aeCutValues.size(); ++i){
        for(int j=0; j<clCutValues.size(); ++j){
            combAglVALposNumSel.push_back("&& MC.anomaly_score<"+to_string(aeCutValues.at(i))+" && scores>"+to_string(clCutValues.at(j)));
            combAglVALposDenomSel.push_back("&& MC.anomaly_score<"+to_string(aeCutValues.at(i))+" && scores>"+to_string(clCutValues.at(j)));
            combAglVALnegNumSel.push_back("&& MCneg.anomaly_score<"+to_string(aeCutValues.at(i))+" && scores>"+to_string(clCutValues.at(j)));
            combAglVALnegDenomSel.push_back("&& MCneg.anomaly_score<"+to_string(aeCutValues.at(i))+"&& scores>"+to_string(clCutValues.at(j)));

            combNaFVALposNumSel.push_back("&& MC.anomaly_score<"+to_string(aeCutValues.at(i))+" && scores>"+to_string(clCutValues.at(j)));
            combNaFVALposDenomSel.push_back("&& MC.anomaly_score<"+to_string(aeCutValues.at(i))+" && scores>"+to_string(clCutValues.at(j)));
            combNaFVALnegNumSel.push_back("&& MCneg.anomaly_score<"+to_string(aeCutValues.at(i))+" && scores>"+to_string(clCutValues.at(j)));
            combNaFVALnegDenomSel.push_back("&& MCneg.anomaly_score<"+to_string(aeCutValues.at(i))+" && scores>"+to_string(clCutValues.at(j)));

            combAglTESTposNumSel.push_back("&& ISS.anomaly_score<"+to_string(aeCutValues.at(i))+" && scores>"+to_string(clCutValues.at(j)));
            combAglTESTposDenomSel.push_back("&& ISS.anomaly_score<"+to_string(aeCutValues.at(i))+" && scores>"+to_string(clCutValues.at(j)));
            combAglTESTnegNumSel.push_back("&& ISSneg.anomaly_score<"+to_string(aeCutValues.at(i))+" && scores>"+to_string(clCutValues.at(j)));
            combAglTESTnegDenomSel.push_back("&& ISSneg.anomaly_score<"+to_string(aeCutValues.at(i))+"&& scores>"+to_string(clCutValues.at(j)));

            combNaFTESTposNumSel.push_back("&& ISS.anomaly_score<"+to_string(aeCutValues.at(i))+" && scores>"+to_string(clCutValues.at(j)));
            combNaFTESTposDenomSel.push_back("&& ISS.anomaly_score<"+to_string(aeCutValues.at(i))+" && scores>"+to_string(clCutValues.at(j)));
            combNaFTESTnegNumSel.push_back("&& ISSneg.anomaly_score<"+to_string(aeCutValues.at(i))+" && scores>"+to_string(clCutValues.at(j)));
            combNaFTESTnegDenomSel.push_back("&& ISSneg.anomaly_score<"+to_string(aeCutValues.at(i))+" && scores>"+to_string(clCutValues.at(j)));

            if(useSafetyFactor) {
                combAglTESTposNumSel.back()   = AglSel+combAglTESTposNumSel.back()+isAbIGRFpos+AglRinnerInvMass;
                combAglTESTposDenomSel.back() = AglSel+combAglTESTposDenomSel.back()+isAbIGRFpos;
                combAglTESTnegNumSel.back()   = AglSel+combAglTESTnegNumSel.back()+isAbIGRFneg+AglRinnerInvMass;
                combAglTESTnegDenomSel.back() = AglSel+combAglTESTnegDenomSel.back()+isAbIGRFneg;

                combNaFTESTposNumSel.back()   = NaFSel+combNaFTESTposNumSel.back()+isAbIGRFpos+NaFRinnerInvMass;
                combNaFTESTposDenomSel.back() = NaFSel+combNaFTESTposDenomSel.back()+isAbIGRFpos;
                combNaFTESTnegNumSel.back()   = NaFSel+combNaFTESTnegNumSel.back()+isAbIGRFneg+NaFRinnerInvMass;
                combNaFTESTnegDenomSel.back() = NaFSel+combNaFTESTnegDenomSel.back()+isAbIGRFneg;
            }
            else if(useCutOff){
                combAglTESTposNumSel.back()   = AglSel+combAglTESTposNumSel.back()+IGRFpos+AglRinnerInvMass;
                combAglTESTposDenomSel.back() = AglSel+combAglTESTposDenomSel.back()+IGRFpos;
                combAglTESTnegNumSel.back()   = AglSel+combAglTESTnegNumSel.back()+IGRFneg+AglRinnerInvMass;
                combAglTESTnegDenomSel.back() = AglSel+combAglTESTnegDenomSel.back()+IGRFneg;

                combNaFTESTposNumSel.back()   = NaFSel+combNaFTESTposNumSel.back()+IGRFpos+NaFRinnerInvMass;
                combNaFTESTposDenomSel.back() = NaFSel+combNaFTESTposDenomSel.back()+IGRFpos;
                combNaFTESTnegNumSel.back()   = NaFSel+combNaFTESTnegNumSel.back()+IGRFneg+NaFRinnerInvMass;
                combNaFTESTnegDenomSel.back() = NaFSel+combNaFTESTnegDenomSel.back()+IGRFneg;
            }
            else {
                combAglTESTposNumSel.back()   = AglSel+combAglTESTposNumSel.back()+AglRinnerInvMass;
                combAglTESTposDenomSel.back() = AglSel+combAglTESTposDenomSel.back();
                combAglTESTnegNumSel.back()   = AglSel+combAglTESTnegNumSel.back()+AglRinnerInvMass;
                combAglTESTnegDenomSel.back() = AglSel+combAglTESTnegDenomSel.back();

                combNaFTESTposNumSel.back()   = NaFSel+combNaFTESTposNumSel.back()+NaFRinnerInvMass;
                combNaFTESTposDenomSel.back() = NaFSel+combNaFTESTposDenomSel.back();
                combNaFTESTnegNumSel.back()   = NaFSel+combNaFTESTnegNumSel.back()+NaFRinnerInvMass;
                combNaFTESTnegDenomSel.back() = NaFSel+combNaFTESTnegDenomSel.back();
            }
            combAglVALposNumSel.back()   = AglSel+combAglVALposNumSel.back()+AglRinnerInvMass;
            combAglVALposDenomSel.back() = AglSel+combAglVALposDenomSel.back();
            combAglVALnegNumSel.back()   = AglSel+combAglVALnegNumSel.back()+AglRinnerInvMass;
            combAglVALnegDenomSel.back() = AglSel+combAglVALnegDenomSel.back();

            combNaFVALposNumSel.back()   = NaFSel+combNaFVALposNumSel.back()+NaFRinnerInvMass;
            combNaFVALposDenomSel.back() = NaFSel+combNaFVALposDenomSel.back();
            combNaFVALnegNumSel.back()   = NaFSel+combNaFVALnegNumSel.back()+NaFRinnerInvMass;
            combNaFVALnegDenomSel.back() = NaFSel+combNaFVALnegDenomSel.back();
        }
    }

    //Binning
    const float mass_MAX = 1., mass_MIN = 1./100.;
    const int nbinsMass = 198;
    const float minRin   = -1., maxRin = 3.5;
    const int nbinsRin = 100;
    const float minCS = 0.,  maxCS = 1.;
    const int nbinsScore = 100;
    const float minAS = 0.49, maxAS = 1.01;
    const int nbinsAS = 52;

    //Drawing
    TString to_draw;
    vector<TString> drawclR, drawclRTEST, drawaeR, drawaeRTEST; 
    vector<TString> drawclRCS, drawclRCSTEST, drawaeRAS, drawaeRASTEST;
    vector<TString> drawCSinvMass, drawCSinvMassTEST, drawASinvMass, drawASinvMassTEST;
    vector<TString> drawclRinvMass;

    //Organizing the output file
    for(int i=0; i<TotTree; i++){
        f_out->cd();
        f_out->mkdir(DirName.at(i).Data());
        TString subDir;
        for(int j=0; j<TotRig; j++){
            subDir = DirName.at(i)+"/"+NameSub.at(j);
            f_out->mkdir(subDir.Data());
            for(int k=0; k<TotRig; ++k){
                subDir=DirName.at(i)+"/"+NameSub.at(j)+"/"+NameR.at(k);
                f_out->mkdir(subDir.Data());
            }
        }
    }

    //Getting trees
    //CLtrees->[MC_pos, MC_neg, ISS_pos, ISS_neg]
    for(int i_file=0; i_file<TotFiles; i_file++){  // loop on files
        if (gSystem->AccessPathName(PathToFile.at(i_file), kFileExists)) {
            printf("The file %s does not exist.\n", PathToFile.at(i_file).Data());
            return;
        }
        printf("The file %s exists.\n", PathToFile.at(i_file).Data());
        files.at(i_file) = new TFile(PathToFile.at(i_file), "READ");
        if(!files.at(i_file)){
            printf("ERROR!! \n File %s not found\n", PathToFile.at(i_file).Data());
            return;
        }
        files.at(i_file)->cd();
        printf("Reading file ==> %s\n\n", PathToFile.at(i_file).Data());
        for(int i_tree=0; i_tree<TotTree; i_tree++){
            if(i_file==0 && i_tree<2){ //CL MC
                CLtrees.at(i_tree) = (TTree*)files.at(i_file)->Get(DirName.at(i_tree).Data());
                if(!CLtrees.at(i_file)) {
                    printf("%s tree not found in the file %s\n", DirName.at(i_tree).Data(), PathToFile.at(i_file).Data());
                    return;
                }
            } // end CL MC
            if(i_file==1 && i_tree>=2){ //CL ISS data
                CLtrees.at(i_tree) = (TTree*)files.at(i_file)->Get(DirName.at(i_tree).Data());
                if(!CLtrees.at(i_file)) {
                    printf("%s tree not found in the file %s\n", DirName.at(i_tree).Data(), PathToFile.at(i_file).Data());
                    return;
                }
            } // end CL MC
        } //end loop on CLtrees

        if(i_file==2){ // AE MC
            AEtrees.at(0) = (TTree*)files.at(i_file)->Get("MC");
            AEtrees.at(1) = (TTree*)files.at(i_file)->Get("MCneg");
            if(!AEtrees.at(0) || !AEtrees.at(1)){
                printf("AE trees not found in the file %s\n", PathToFile.at(i_file).Data());
                return;
            }
            for(int i=0; i<2; ++i) {
                AEtrees.at(i)->BuildIndex("EvRun", "EvNum");
                CLtrees.at(i)->AddFriend(AEtrees.at(i));
            }
        } //end AE MC
        if(i_file==3){ // AE ISS
            AEtrees.at(2) = (TTree*)files.at(i_file)->Get("ISS");
            AEtrees.at(3) = (TTree*)files.at(i_file)->Get("ISSneg");
            if(!AEtrees.at(2) || !AEtrees.at(3)){
                printf("AE trees not found in the file %s\n", PathToFile.at(i_file).Data());
                return;
            }
            for(int i=2; i<4; ++i) {
                AEtrees.at(i)->BuildIndex("EvRun", "EvNum");
                CLtrees.at(i)->AddFriend(AEtrees.at(i));
            }
        } //end AE ISS
    } // end loop on files

    f_out->cd();
    //Initialize the histograms
    for(int i_tree=0; i_tree<2; i_tree++){ //loop on CLtrees
        TString ncl,    nclTEST,  nae,  naeTEST;
        TString tcl,    tclTEST,  tae,  taeTEST;
        TString nRCS,   nRCSTEST, nRAS, nRASTEST;
        TString tRCS,   tRCSTEST, tRAS, tRASTEST;
        //NaF
        TString nCSinvMassNaF,  nCSinvMassNaFTEST,  nASinvMassNaF,  nASinvMassNaFTEST;
        TString tCSinvMassNaF,  tCSinvMassNaFTEST,  tASinvMassNaF,  tASinvMassNaFTEST;
        TString nclRinvMassNaF, nclRinvMassNaFTEST, naeRinvMassNaF, naeRinvMassNaFTEST;
        TString tclRinvMassNaF, tclRinvMassNaFTEST, taeRinvMassNaF, taeRinvMassNaFTEST;
        //AGL
        TString nCSinvMassAgl, nCSinvMassAglTEST,   nASinvMassAgl,  nASinvMassAglTEST;
        TString tCSinvMassAgl, tCSinvMassAglTEST,   tASinvMassAgl,  tASinvMassAglTEST;
        TString nclRinvMassAgl, nclRinvMassAglTEST, naeRinvMassAgl, naeRinvMassAglTEST;
        TString tclRinvMassAgl, tclRinvMassAglTEST, taeRinvMassAgl, taeRinvMassAglTEST;
    
        //Event counts in mass window AGL
        H1_CSinvMassWAgl.push_back( new TH1D(nCSinvMassWAglISS.at(i_tree).Data(), tCSinvMassW.Data(), nbinsScore, minCS, maxCS));
        H1_ASinvMassWAgl.push_back(new TH1D(nASinvMassWAglISS.at(i_tree).Data(), tASinvMassW.Data(), nbinsAS, minAS, maxAS));
        H1_TEST_CSinvMassWAgl.push_back( new TH1D(nCSinvMassWAglTEST.at(i_tree).Data(), tCSinvMassW.Data(), nbinsScore, minCS, maxCS));
        H1_TEST_ASinvMassWAgl.push_back(new TH1D(nASinvMassWAglTEST.at(i_tree).Data(), tASinvMassW.Data(), nbinsAS, minAS, maxAS));
        H2_combinvMassWAgl.push_back( new TH2D(ncombinvMassWAglISS.at(i_tree).Data(), tcombinvMassW.Data(), nbinsAS, minAS, maxAS, nbinsScore, minCS, maxCS));
        H2_combinvMassWAglN.push_back( new TH2D(ncombinvMassWAglISSN.at(i_tree).Data(), tcombinvMassWN.Data(), nbinsAS, minAS, maxAS, nbinsScore, minCS, maxCS));
        //NaF
        H1_CSinvMassWNaF.push_back( new TH1D(nCSinvMassWNaFISS.at(i_tree).Data(), tCSinvMassW.Data(), nbinsScore, minCS, maxCS));
        H1_ASinvMassWNaF.push_back( new TH1D(nASinvMassWNaFISS.at(i_tree).Data(), tASinvMassW.Data(), nbinsAS, minAS, maxAS));
        H1_TEST_CSinvMassWNaF.push_back( new TH1D(nCSinvMassWNaFTEST.at(i_tree).Data(), tCSinvMassW.Data(), nbinsScore, minCS, maxCS));
        H1_TEST_ASinvMassWNaF.push_back( new TH1D(nASinvMassWNaFTEST.at(i_tree).Data(), tASinvMassW.Data(), nbinsAS, minAS, maxAS));
        H2_combinvMassWNaF.push_back( new TH2D(ncombinvMassWNaFISS.at(i_tree).Data(), tcombinvMassW.Data(), nbinsAS, minAS, maxAS, nbinsScore, minCS, maxCS));
        H2_combinvMassWNaFN.push_back( new TH2D(ncombinvMassWNaFISSN.at(i_tree).Data(), tcombinvMassWN.Data(), nbinsAS, minAS, maxAS, nbinsScore, minCS, maxCS));

        for(int j_rig=0; j_rig<TotRig; j_rig++){ //loop on rig
            if(i_tree%2==0) { // Only positives
                ncl   = "CL_"+NameR.at(j_rig)+"Pos";
                nae   = "AE_"+NameR.at(j_rig)+"Pos";
                nclTEST = "CL_"+NameR.at(j_rig)+"ISSPos";
                naeTEST   = "AE_"+NameR.at(j_rig)+"ISSPos";

                RSel.push_back(NameR.at(j_rig)+">0");
                RSel.back().Prepend("("); RSel.back().Append(")*weight");

                nRCS   = "CL_"+NameR.at(j_rig)+"CSPos";
                nRAS   = "AE_"+NameR.at(j_rig)+"ASPos";
                nRCSTEST = "CL_"+NameR.at(j_rig)+"CSISSPos";
                nRASTEST = "AE_"+NameR.at(j_rig)+"ASISSPos";
                
                drawclRCS.push_back("scores:log10(abs("+NameR.at(j_rig)+"))>>+"+nRCS);
                drawaeRAS.push_back("MC.anomaly_score:log10(abs("+NameR.at(j_rig)+"))>>+"+nRAS);
                drawclRCSTEST.push_back("scores:log10(abs("+NameR.at(j_rig)+"))>>+"+nRCSTEST);
                drawaeRASTEST.push_back("ISS.anomaly_score:log10(abs("+NameR.at(j_rig)+"))>>+"+nRASTEST);
                
                nCSinvMassNaF = "CL_CSInvMass_NaF_"+NameR.at(j_rig)+"Pos";
                nASinvMassNaF = "AE_ASInvMass_NaF_"+NameR.at(j_rig)+"Pos";
                nCSinvMassNaFTEST = "CL_CSInvMass_NaF_"+NameR.at(j_rig)+"ISSPos";
                nASinvMassNaFTEST = "AE_ASInvMass_NaF_"+NameR.at(j_rig)+"ISSPos";

                nclRinvMassNaF  = "CL_RinvMass_NaF_"+NameR.at(j_rig)+"Pos";
                naeRinvMassNaF  = "AE_RinvMass_NaF_"+NameR.at(j_rig)+"Pos";
                nclRinvMassNaFTEST  = "CL_RinvMass_NaF_"+NameR.at(j_rig)+"ISSPos";
                naeRinvMassNaFTEST  = "AE_RinvMass_NaF_"+NameR.at(j_rig)+"ISSPos";

                nCSinvMassAgl = "CL_CSInvMass_Agl_"+NameR.at(j_rig)+"Pos";
                nASinvMassAgl = "AE_ASInvMass_Agl_"+NameR.at(j_rig)+"Pos";
                nCSinvMassAglTEST = "CL_CSInvMass_Agl_"+NameR.at(j_rig)+"ISSPos";
                nASinvMassAglTEST = "AE_ASInvMass_Agl_"+NameR.at(j_rig)+"ISSPos";

                nclRinvMassAgl  = "CL_RinvMass_Agl_"+NameR.at(j_rig)+"Pos";
                naeRinvMassAgl  = "AE_RinvMass_Agl_"+NameR.at(j_rig)+"Pos";
                nclRinvMassAglTEST  = "CL_RinvMass_Agl_"+NameR.at(j_rig)+"ISSPos";
                naeRinvMassAglTEST  = "AE_RinvMass_Agl_"+NameR.at(j_rig)+"ISSPos";

                drawCSinvMass.push_back("scores:1./((2.*abs("+NameR.at(j_rig)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))");
                drawASinvMass.push_back("MC.anomaly_score:1./((2.*abs("+NameR.at(j_rig)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))");
                drawCSinvMassTEST.push_back("ISS_pos.scores:1./((2.*abs("+NameR.at(j_rig)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))");
                drawASinvMassTEST.push_back("ISS.anomaly_score:1./((2.*abs("+NameR.at(j_rig)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))");
            }
            else {
                ncl = "CL_"+NameR.at(j_rig)+"Neg";
                nae = "AE_"+NameR.at(j_rig)+"Neg";
                nclTEST = "CL_"+NameR.at(j_rig)+"ISSNeg";
                naeTEST = "AE_"+NameR.at(j_rig)+"ISSNeg";

                RSel.push_back(NameR.at(j_rig)+"<0");
                RSel.back().Prepend("("); RSel.back().Append(")*weight");

                nRCS = "CL_"+NameR.at(j_rig)+"CSNeg";
                nRAS = "AE_"+NameR.at(j_rig)+"ASNeg";
                nRCSTEST = "CL_"+NameR.at(j_rig)+"CSISSNeg";
                nRASTEST = "AE_"+NameR.at(j_rig)+"ASISSNeg";

                drawclRCS.push_back("scores:log10(abs("+NameR.at(j_rig)+"))>>+"+nRCS);
                drawaeRAS.push_back("MCneg.anomaly_score:log10(abs("+NameR.at(j_rig)+"))>>+"+nRAS);
                drawclRCSTEST.push_back("scores:log10(abs("+NameR.at(j_rig)+"))>>+"+nRCSTEST);
                drawaeRASTEST.push_back("ISSneg.anomaly_score:log10(abs("+NameR.at(j_rig)+"))>>+"+nRASTEST);

                nCSinvMassNaF = "CL_CSInvMass_NaF_"+NameR.at(j_rig)+"Neg";
                nASinvMassNaF = "AE_ASInvMass_NaF_"+NameR.at(j_rig)+"Neg";
                nCSinvMassNaFTEST = "CL_CSInvMass_NaF_"+NameR.at(j_rig)+"ISSNeg";
                nASinvMassNaFTEST = "AE_ASInvMass_NaF_"+NameR.at(j_rig)+"ISSNeg";

                nclRinvMassNaF  = "CL_RinvMass_NaF_"+NameR.at(j_rig)+"Neg";
                naeRinvMassNaF  = "AE_RinvMass_NaF_"+NameR.at(j_rig)+"Neg";
                nclRinvMassNaFTEST  = "CL_RinvMass_NaF_"+NameR.at(j_rig)+"ISSNeg";
                naeRinvMassNaFTEST  = "AE_RinvMass_NaF_"+NameR.at(j_rig)+"ISSNeg";

                nCSinvMassAgl = "CL_CSInvMass_Agl_"+NameR.at(j_rig)+"Neg";
                nASinvMassAgl = "AE_ASInvMass_Agl_"+NameR.at(j_rig)+"Neg";
                nCSinvMassAglTEST = "CL_CSInvMass_Agl_"+NameR.at(j_rig)+"ISSNeg";
                nASinvMassAglTEST = "AE_ASInvMass_Agl_"+NameR.at(j_rig)+"ISSNeg";

                nclRinvMassAgl  = "CL_RinvMass_Agl_"+NameR.at(j_rig)+"Neg";
                naeRinvMassAgl  = "AE_RinvMass_Agl_"+NameR.at(j_rig)+"Neg";
                naeRinvMassAglTEST  = "AE_RinvMass_Agl_"+NameR.at(j_rig)+"ISSNeg";
                nclRinvMassAglTEST  = "CL_RinvMass_Agl_"+NameR.at(j_rig)+"ISSNeg";

                drawCSinvMass.push_back("scores:1./((2.*abs("+NameR.at(j_rig)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))");
                drawASinvMass.push_back("MCneg.anomaly_score:1./((2.*abs("+NameR.at(j_rig)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))");
                drawCSinvMassTEST.push_back("ISS_neg.scores:1./((2.*abs("+NameR.at(j_rig)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))");
                drawASinvMassTEST.push_back("ISSneg.anomaly_score:1./((2.*abs("+NameR.at(j_rig)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))");
            }
            
            //Rigidities
            drawclR.push_back("log10(abs("+NameR.at(j_rig)+"))>>+"+ncl);
            drawaeR.push_back("log10(abs("+NameR.at(j_rig)+"))>>+"+nae);
            drawclRTEST.push_back("log10(abs("+NameR.at(j_rig)+"))>>+"+nclTEST);
            drawaeRTEST.push_back("log10(abs("+NameR.at(j_rig)+"))>>+"+naeTEST);

            tcl = ncl+"; [log_{10}(GV)]; Events";
            tae = nae+"; [log_{10}(GV)]; Events";
            tclTEST = nclTEST+"; [log_{10}(GV)]; Events";
            taeTEST = naeTEST+"; [log_{10}(GV)]; Events";

            H1cl_R.push_back(new TH1D(ncl.Data(),tcl.Data(), nbinsRin, minRin, maxRin));
            H1ae_R.push_back(new TH1D(nae.Data(),tae.Data(), nbinsRin, minRin, maxRin));
            H1cl_TEST_R.push_back(new TH1D(nclTEST.Data(),tclTEST.Data(), nbinsRin, minRin, maxRin));
            H1ae_TEST_R.push_back(new TH1D(naeTEST.Data(),taeTEST.Data(), nbinsRin, minRin, maxRin));

            H1cl_R.back()->GetXaxis()->SetTitleOffset(1.2);      H1ae_R.back()->GetXaxis()->SetTitleOffset(1.2);
            H1cl_TEST_R.back()->GetXaxis()->SetTitleOffset(1.2); H1ae_TEST_R.back()->GetXaxis()->SetTitleOffset(1.2);
           
                //R vs scores
            if(j_rig==1) RCSSel.push_back(RichSel+InL1Sel.at(i_tree));
            else RCSSel.push_back(RichSel);
            RCSSel.back().Prepend("("); RCSSel.back().Append(")*weight");

            tRCS = nRCS+"; [log_{10}(GV)]; Classifier scores";
            tRAS = nRAS+"; [log_{10}(GV)]; Anomaly scores";
            tRCSTEST = nRCSTEST+"; [log_{10}(GV)]; Classifier scores";
            tRASTEST = nRASTEST+"; [log_{10}(GV)]; Anomaly scores";

            H2_RCS.push_back(new TH2D(nRCS.Data(), tRCS.Data(), nbinsRin, minRin, maxRin, nbinsScore, minCS, maxCS));
            H2_RAS.push_back(new TH2D(nRAS.Data(), tRAS.Data(), nbinsRin, minRin, maxRin, nbinsScore, minAS, maxAS));
            H2_TEST_RCS.push_back(new TH2D(nRCSTEST.Data(), tRCSTEST.Data(), nbinsRin, minRin, maxRin, nbinsScore, minCS, maxCS));
            H2_TEST_RAS.push_back(new TH2D(nRASTEST.Data(), tRASTEST.Data(), nbinsRin, minRin, maxRin, nbinsScore, minAS, maxAS));
            H2_RCS.back()->GetXaxis()->SetTitleOffset(1.2);
            H2_RAS.back()->GetXaxis()->SetTitleOffset(1.2);
            H2_TEST_RCS.back()->GetXaxis()->SetTitleOffset(1.2);
            H2_TEST_RAS.back()->GetXaxis()->SetTitleOffset(1.2);


            //NaF
                //CS vs Inv. Mass (NaF)
            if(j_rig==1) invMassNaFSel.push_back(NaFSel+InL1Sel.at(i_tree));
            else invMassNaFSel.push_back(NaFSel);
            invMassNaFSel.back().Prepend("("); invMassNaFSel.back().Append(")*weight");

            tCSinvMassNaF = nCSinvMassNaF+"; RICH NaF #frac{1}{m} [1/a.m.u.]; Classifier scores";
            tASinvMassNaF = nASinvMassNaF+"; RICH NaF #frac{1}{m} [1/a.m.u.]; Anomaly scores";
            tCSinvMassNaFTEST = nCSinvMassNaFTEST+"; RICH NaF #frac{1}{m} [1/a.m.u.]; Classifier scores";
            tASinvMassNaFTEST = nASinvMassNaFTEST+"; RICH NaF #frac{1}{m} [1/a.m.u.]; Anomaly scores";

            H2_CSinvMassNaF.push_back(new TH2D(nCSinvMassNaF.Data(), tCSinvMassNaF.Data(), nbinsMass, mass_MIN, mass_MAX, nbinsScore, minCS, maxCS));
            H2_ASinvMassNaF.push_back(new TH2D(nASinvMassNaF.Data(), tASinvMassNaF.Data(), nbinsMass, mass_MIN, mass_MAX, nbinsScore, minAS, maxAS));
            H2_TEST_CSinvMassNaF.push_back(new TH2D(nCSinvMassNaFTEST.Data(), tCSinvMassNaFTEST.Data(), nbinsMass, mass_MIN, mass_MAX, nbinsScore, minCS, maxCS));
            H2_TEST_ASinvMassNaF.push_back(new TH2D(nASinvMassNaFTEST.Data(), tASinvMassNaFTEST.Data(), nbinsMass, mass_MIN, mass_MAX, nbinsScore, minAS, maxAS));
            H2_CSinvMassNaF.back()->GetXaxis()->SetTitleOffset(1.2);      H2_ASinvMassNaF.back()->GetXaxis()->SetTitleOffset(1.2);
            H2_TEST_CSinvMassNaF.back()->GetXaxis()->SetTitleOffset(1.2); H2_TEST_ASinvMassNaF.back()->GetXaxis()->SetTitleOffset(1.2);

                //R vs Inv. Mass (NaF)
            drawclRinvMass.push_back("1./((2.*abs("+NameR.at(j_rig)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315)):log10(abs("+NameR.at(j_rig)+"))");
            tclRinvMassNaF = nclRinvMassNaF+" ; log_{10}(|R|) [log_{10}(GV)] ; RICH NaF #frac{1}{m} [1/a.m.u.] ";
            taeRinvMassNaF = naeRinvMassNaF+" ; log_{10}(|R|) [log_{10}(GV)] ; RICH NaF #frac{1}{m} [1/a.m.u.] ";
            tclRinvMassNaFTEST = nclRinvMassNaFTEST+" ; log_{10}(|R|) [log_{10}(GV)] ; RICH NaF #frac{1}{m} [1/a.m.u.] ";
            taeRinvMassNaFTEST = naeRinvMassNaFTEST+" ; log_{10}(|R|) [log_{10}(GV)] ; RICH NaF #frac{1}{m} [1/a.m.u.] ";
            H2cl_RinvMassNaF.push_back(new TH2D(nclRinvMassNaF.Data(), tclRinvMassNaF.Data(), nbinsRin, minRin, maxRin, nbinsMass, mass_MIN, mass_MAX));
            H2ae_RinvMassNaF.push_back(new TH2D(naeRinvMassNaF.Data(), taeRinvMassNaF.Data(), nbinsRin, minRin, maxRin, nbinsMass, mass_MIN, mass_MAX));
            H2cl_TEST_RinvMassNaF.push_back(new TH2D(nclRinvMassNaFTEST.Data(), tclRinvMassNaFTEST.Data(), nbinsRin, minRin, maxRin, nbinsMass, mass_MIN, mass_MAX));
            H2ae_TEST_RinvMassNaF.push_back(new TH2D(naeRinvMassNaFTEST.Data(), taeRinvMassNaFTEST.Data(), nbinsRin, minRin, maxRin, nbinsMass, mass_MIN, mass_MAX));
            H2cl_RinvMassNaF.back()->GetXaxis()->SetTitleOffset(1.2);
            H2ae_RinvMassNaF.back()->GetXaxis()->SetTitleOffset(1.2);
            H2cl_TEST_RinvMassNaF.back()->GetXaxis()->SetTitleOffset(1.2);
            H2ae_TEST_RinvMassNaF.back()->GetXaxis()->SetTitleOffset(1.2);

            //AGL
                //CS vs Inv. Mass (Agl)
            if(j_rig==1) invMassAglSel.push_back(AglSel+InL1Sel.at(i_tree));
            else invMassAglSel.push_back(AglSel);
            invMassAglSel.back().Prepend("("); invMassAglSel.back().Append(")*weight");

            tCSinvMassAgl = nCSinvMassAgl+"; RICH Agl #frac{1}{m} [1/a.m.u.]; Classifier scores";
            tASinvMassAgl = nASinvMassAgl+"; RICH Agl #frac{1}{m} [1/a.m.u.]; Anomaly scores";
            tCSinvMassAglTEST = nCSinvMassAglTEST+"; RICH Agl #frac{1}{m} [1/a.m.u.]; Classifier scores";
            tASinvMassAglTEST = nASinvMassAglTEST+"; RICH Agl #frac{1}{m} [1/a.m.u.]; Anomaly scores";
            H2_CSinvMassAgl.push_back(new TH2D(nCSinvMassAgl.Data(), tCSinvMassAgl.Data(), nbinsMass, mass_MIN, mass_MAX, nbinsScore, minCS, maxCS));
            H2_ASinvMassAgl.push_back(new TH2D(nASinvMassAgl.Data(), tASinvMassAgl.Data(), nbinsMass, mass_MIN, mass_MAX, nbinsScore, minAS, maxAS));
            H2_TEST_CSinvMassAgl.push_back(new TH2D(nCSinvMassAglTEST.Data(), tCSinvMassAglTEST.Data(), nbinsMass, mass_MIN, mass_MAX, nbinsScore, minCS, maxCS));
            H2_TEST_ASinvMassAgl.push_back(new TH2D(nASinvMassAglTEST.Data(), tASinvMassAglTEST.Data(), nbinsMass, mass_MIN, mass_MAX, nbinsScore, minAS, maxAS));
            H2_CSinvMassAgl.back()->GetXaxis()->SetTitleOffset(1.2);
            H2_ASinvMassAgl.back()->GetXaxis()->SetTitleOffset(1.2);
            H2_TEST_CSinvMassAgl.back()->GetXaxis()->SetTitleOffset(1.2);
            H2_TEST_ASinvMassAgl.back()->GetXaxis()->SetTitleOffset(1.2);
            //R vs Inv. Mass (Agl)
            tclRinvMassAgl = nclRinvMassAgl+" ; log_{10}(|R|) [log_{10}(GV)] ; RICH Agl #frac{1}{m} [1/a.m.u.] ";
            taeRinvMassAgl = naeRinvMassAgl+" ; log_{10}(|R|) [log_{10}(GV)] ; RICH Agl #frac{1}{m} [1/a.m.u.] ";
            tclRinvMassAglTEST = nclRinvMassAglTEST+" ; log_{10}(|R|) [log_{10}(GV)] ; RICH Agl #frac{1}{m} [1/a.m.u.] ";
            taeRinvMassAglTEST = naeRinvMassAglTEST+" ; log_{10}(|R|) [log_{10}(GV)] ; RICH Agl #frac{1}{m} [1/a.m.u.] ";
            H2cl_RinvMassAgl.push_back(new TH2D(nclRinvMassAgl.Data(), tclRinvMassAgl.Data(), nbinsRin, minRin, maxRin, nbinsMass, mass_MIN, mass_MAX));
            H2ae_RinvMassAgl.push_back(new TH2D(naeRinvMassAgl.Data(), taeRinvMassAgl.Data(), nbinsRin, minRin, maxRin, nbinsMass, mass_MIN, mass_MAX));
            H2cl_TEST_RinvMassAgl.push_back(new TH2D(nclRinvMassAglTEST.Data(), tclRinvMassAglTEST.Data(), nbinsRin, minRin, maxRin, nbinsMass, mass_MIN, mass_MAX));
            H2ae_TEST_RinvMassAgl.push_back(new TH2D(naeRinvMassAglTEST.Data(), taeRinvMassAglTEST.Data(), nbinsRin, minRin, maxRin, nbinsMass, mass_MIN, mass_MAX));
            H2cl_RinvMassAgl.back()->GetXaxis()->SetTitleOffset(1.2);
            H2ae_RinvMassAgl.back()->GetXaxis()->SetTitleOffset(1.2);
            H2cl_TEST_RinvMassAgl.back()->GetXaxis()->SetTitleOffset(1.2);
            H2ae_TEST_RinvMassAgl.back()->GetXaxis()->SetTitleOffset(1.2);
        } //end loop on rig
    } // end loop on CLtrees

    printf("So far so good 1\n");

    //Filling the histos
    //CLtrees->[MC_pos, MC_neg, ISS_pos, ISS_neg]
    TString TESTSel;
    for(int i=0; i<H1cl_R.size(); i++){  // loop on histos
        f_out->cd();
        if(i<TotRig){ // POSITIVES +
            //Rigidities
            CLtrees.at(0)->Draw(drawclR.at(i).Data(),RSel.at(i).Data(),"goff");
            H1cl_R.at(i) = (LogToLin1D((TH1D*)gDirectory->Get(H1cl_R.at(i)->GetName())));
            CLtrees.at(0)->Draw(drawaeR.at(i).Data(),RSel.at(i).Data(),"goff");
            H1ae_R.at(i) = (LogToLin1D((TH1D*)gDirectory->Get(H1ae_R.at(i)->GetName())));

            TESTSel = RSel.at(i);
            TESTSel.ReplaceAll(")*weight","");
            if(useSafetyFactor ) TESTSel = TESTSel + isAbIGRFpos;
            if(useCutOff) TESTSel = TESTSel + IGRFpos;
            TESTSel.Append(")");
            CLtrees.at(2)->Draw(drawclRTEST.at(i).Data(),TESTSel.Data(),"goff");
            H1cl_TEST_R.at(i) = (LogToLin1D((TH1D*)gDirectory->Get(H1cl_TEST_R.at(i)->GetName())));
            CLtrees.at(2)->Draw(drawaeRTEST.at(i).Data(),TESTSel.Data(),"goff");
            H1ae_TEST_R.at(i) = (LogToLin1D((TH1D*)gDirectory->Get(H1ae_TEST_R.at(i)->GetName())));
            
            f_out->cd(DirName.at(0).Data());
            H1cl_R.at(i)->Write();
            H1ae_R.at(i)->Write();
            H1cl_TEST_R.at(i)->Write();
            H1ae_TEST_R.at(i)->Write();
            f_out->cd();
                // R vs CS
            CLtrees.at(0)->Draw(drawclRCS.at(i).Data(), RCSSel.at(i).Data(), "goff");
            CLtrees.at(0)->Draw(drawaeRAS.at(i).Data(), RCSSel.at(i).Data(), "goff");
            TESTSel = RCSSel.at(i);
            TESTSel.ReplaceAll(")*weight","");
            if(useSafetyFactor) TESTSel = TESTSel + isAbIGRFpos;
            if(useCutOff) TESTSel = TESTSel + IGRFpos;
            TESTSel.Append(")");
            CLtrees.at(2)->Draw(drawclRCSTEST.at(i).Data(), TESTSel.Data(), "goff");
            CLtrees.at(2)->Draw(drawaeRASTEST.at(i).Data(), TESTSel.Data(), "goff");

            //NaF
                //CS vs Inv. Mass
            to_draw = drawCSinvMass.at(i)+">>+"+H2_CSinvMassNaF.at(i)->GetName();
            CLtrees.at(0)->Draw(to_draw.Data(), invMassNaFSel.at(i).Data(), "goff");
            to_draw = drawASinvMass.at(i)+">>+"+H2_ASinvMassNaF.at(i)->GetName();
            CLtrees.at(0)->Draw(to_draw.Data(), invMassNaFSel.at(i).Data(), "goff");

            to_draw = drawCSinvMassTEST.at(i)+">>+"+H2_TEST_CSinvMassNaF.at(i)->GetName();
            TESTSel =  invMassNaFSel.at(i);
            TESTSel.ReplaceAll(")*weight","");
            if(useSafetyFactor) TESTSel = TESTSel + isAbIGRFpos;
            if(useCutOff) TESTSel = TESTSel + IGRFpos;
            TESTSel.Append(")");
            CLtrees.at(2)->Draw(to_draw.Data(), TESTSel.Data(), "goff");
            to_draw = drawASinvMassTEST.at(i)+">>+"+H2_TEST_ASinvMassNaF.at(i)->GetName();
            CLtrees.at(2)->Draw(to_draw.Data(), TESTSel.Data(), "goff");
                //R vs Inv. Mass
            to_draw = drawclRinvMass.at(i)+">>+"+H2cl_RinvMassNaF.at(i)->GetName();
            CLtrees.at(0)->Draw(to_draw.Data(), invMassNaFSel.at(i).Data(), "goff");
            to_draw = drawclRinvMass.at(i)+">>+"+H2ae_RinvMassNaF.at(i)->GetName();
            CLtrees.at(0)->Draw(to_draw.Data(), invMassNaFSel.at(i).Data(), "goff");
            to_draw = drawclRinvMass.at(i)+">>+"+H2cl_TEST_RinvMassNaF.at(i)->GetName();
            CLtrees.at(2)->Draw(to_draw.Data(), TESTSel.Data(), "goff");
            to_draw = drawclRinvMass.at(i)+">>+"+H2ae_TEST_RinvMassNaF.at(i)->GetName();
            CLtrees.at(2)->Draw(to_draw.Data(), TESTSel.Data(), "goff");
            //Agl
                //CS vs Inv. Mass
            to_draw = drawCSinvMass.at(i)+">>+"+H2_CSinvMassAgl.at(i)->GetName();
            CLtrees.at(0)->Draw(to_draw.Data(), invMassAglSel.at(i).Data(), "goff");
            to_draw = drawASinvMass.at(i)+">>+"+H2_ASinvMassAgl.at(i)->GetName();
            CLtrees.at(0)->Draw(to_draw.Data(), invMassAglSel.at(i).Data(), "goff");

            to_draw = drawCSinvMassTEST.at(i)+">>+"+H2_TEST_CSinvMassAgl.at(i)->GetName();
            TESTSel = invMassAglSel.at(i);
            TESTSel.ReplaceAll(")*weight","");
            if(useSafetyFactor) TESTSel = TESTSel + isAbIGRFpos;
            if(useCutOff) TESTSel = TESTSel + IGRFpos;
            TESTSel.Append(")");
            CLtrees.at(2)->Draw(to_draw.Data(), TESTSel.Data(), "goff");
            to_draw = drawASinvMassTEST.at(i)+">>+"+H2_TEST_ASinvMassAgl.at(i)->GetName();
            CLtrees.at(2)->Draw(to_draw.Data(), TESTSel.Data(), "goff");
                //R vs Inv. Mass
            to_draw = drawclRinvMass.at(i)+">>+"+H2cl_RinvMassAgl.at(i)->GetName();
            CLtrees.at(0)->Draw(to_draw.Data(), invMassAglSel.at(i).Data(), "goff");
            to_draw = drawclRinvMass.at(i)+">>+"+H2ae_RinvMassAgl.at(i)->GetName();
            CLtrees.at(0)->Draw(to_draw.Data(), invMassAglSel.at(i).Data(), "goff");
            to_draw = drawclRinvMass.at(i)+">>+"+H2cl_TEST_RinvMassAgl.at(i)->GetName();
            CLtrees.at(2)->Draw(to_draw.Data(), TESTSel.Data(), "goff");
            to_draw = drawclRinvMass.at(i)+">>+"+H2ae_TEST_RinvMassAgl.at(i)->GetName();
            CLtrees.at(2)->Draw(to_draw.Data(), TESTSel.Data(), "goff");
        }
        else{ // ATIVES -
            //Rigidities
            CLtrees.at(1)->Draw(drawclR.at(i).Data(),RSel.at(i).Data(),"goff");
            H1cl_R.at(i) = (LogToLin1D((TH1D*)gDirectory->Get(H1cl_R.at(i)->GetName())));
            CLtrees.at(1)->Draw(drawaeR.at(i).Data(),RSel.at(i).Data(),"goff");
            H1ae_R.at(i) = (LogToLin1D((TH1D*)gDirectory->Get(H1ae_R.at(i)->GetName())));

            TESTSel = RSel.at(i);
            TESTSel.ReplaceAll(")*weight","");
            if(useSafetyFactor ) TESTSel = TESTSel + isAbIGRFneg;
            if(useCutOff) TESTSel = TESTSel + IGRFneg;
            TESTSel.Append(")");
            CLtrees.at(3)->Draw(drawclRTEST.at(i).Data(),TESTSel.Data(),"goff");
            H1cl_TEST_R.at(i) = (LogToLin1D((TH1D*)gDirectory->Get(H1cl_TEST_R.at(i)->GetName())));
            CLtrees.at(3)->Draw(drawaeRTEST.at(i).Data(),TESTSel.Data(),"goff");
            H1ae_TEST_R.at(i) = (LogToLin1D((TH1D*)gDirectory->Get(H1ae_TEST_R.at(i)->GetName())));

            f_out->cd(DirName.at(1).Data());
            H1cl_R.at(i)->Write();
            H1ae_R.at(i)->Write();
            H1cl_TEST_R.at(i)->Write();
            H1ae_TEST_R.at(i)->Write();
            f_out->cd();
                // R vs CS
            CLtrees.at(1)->Draw(drawclRCS.at(i).Data(), RCSSel.at(i).Data(), "goff");
            CLtrees.at(1)->Draw(drawaeRAS.at(i).Data(), RCSSel.at(i).Data(), "goff");

            TESTSel = RCSSel.at(i);
            TESTSel.ReplaceAll(")*weight","");
            if(useSafetyFactor) TESTSel = TESTSel + isAbIGRFneg;
            if(useCutOff) TESTSel = TESTSel + IGRFneg;
            TESTSel.Append(")");
            CLtrees.at(3)->Draw(drawclRCSTEST.at(i).Data(), TESTSel.Data(), "goff");
            CLtrees.at(3)->Draw(drawaeRASTEST.at(i).Data(), TESTSel.Data(), "goff");

            //NaF
                //CS vs Inv. Mass
            to_draw = drawCSinvMass.at(i)+">>+"+H2_CSinvMassNaF.at(i)->GetName();
            CLtrees.at(1)->Draw(to_draw.Data(), invMassNaFSel.at(i).Data(), "goff");
            to_draw = drawASinvMass.at(i)+">>+"+H2_ASinvMassNaF.at(i)->GetName();
            CLtrees.at(1)->Draw(to_draw.Data(), invMassNaFSel.at(i).Data(), "goff");

            TESTSel = invMassNaFSel.at(i);
            TESTSel.ReplaceAll(")*weight","");
            if(useSafetyFactor) TESTSel = TESTSel + isAbIGRFneg;
            if(useCutOff) TESTSel = TESTSel + IGRFneg;
            TESTSel.Append(")");
            to_draw = drawCSinvMassTEST.at(i)+">>+"+H2_TEST_CSinvMassNaF.at(i)->GetName();
            CLtrees.at(3)->Draw(to_draw.Data(), TESTSel.Data(), "goff");
            to_draw = drawASinvMassTEST.at(i)+">>+"+H2_TEST_ASinvMassNaF.at(i)->GetName();
            CLtrees.at(3)->Draw(to_draw.Data(), TESTSel.Data(), "goff");
                //R vs Inv. Mass
            to_draw = drawclRinvMass.at(i)+">>+"+H2cl_RinvMassNaF.at(i)->GetName();
            CLtrees.at(1)->Draw(to_draw.Data(), invMassNaFSel.at(i).Data(), "goff");
            to_draw = drawclRinvMass.at(i)+">>+"+H2ae_RinvMassNaF.at(i)->GetName();
            CLtrees.at(1)->Draw(to_draw.Data(), invMassNaFSel.at(i).Data(), "goff");
            to_draw = drawclRinvMass.at(i)+">>+"+H2cl_TEST_RinvMassNaF.at(i)->GetName();
            CLtrees.at(3)->Draw(to_draw.Data(), TESTSel.Data(), "goff");
            to_draw = drawclRinvMass.at(i)+">>+"+H2ae_TEST_RinvMassNaF.at(i)->GetName();
            CLtrees.at(3)->Draw(to_draw.Data(), TESTSel.Data(), "goff");

            //Agl
                //CS vs Inv. Mass
            to_draw = drawCSinvMass.at(i)+">>+"+H2_CSinvMassAgl.at(i)->GetName();
            CLtrees.at(1)->Draw(to_draw.Data(), invMassAglSel.at(i).Data(), "goff");
            to_draw = drawASinvMass.at(i)+">>+"+H2_ASinvMassAgl.at(i)->GetName();
            CLtrees.at(1)->Draw(to_draw.Data(), invMassAglSel.at(i).Data(), "goff");

            TESTSel = invMassAglSel.at(i);
            TESTSel.ReplaceAll(")*weight","");
            if(useSafetyFactor) TESTSel = TESTSel + isAbIGRFneg;
            if(useCutOff) TESTSel = TESTSel + IGRFneg;
            TESTSel.Append(")");
            to_draw = drawCSinvMassTEST.at(i)+">>+"+H2_TEST_CSinvMassAgl.at(i)->GetName();
            CLtrees.at(3)->Draw(to_draw.Data(), TESTSel.Data(), "goff");
            to_draw = drawASinvMassTEST.at(i)+">>+"+H2_TEST_ASinvMassAgl.at(i)->GetName();
            CLtrees.at(3)->Draw(to_draw.Data(), TESTSel.Data(), "goff");
                //R vs Inv. Mass
            to_draw = drawclRinvMass.at(i)+">>+"+H2cl_RinvMassAgl.at(i)->GetName();
            CLtrees.at(1)->Draw(to_draw.Data(), invMassAglSel.at(i).Data(), "goff");
            to_draw = drawclRinvMass.at(i)+">>+"+H2ae_RinvMassAgl.at(i)->GetName();
            CLtrees.at(1)->Draw(to_draw.Data(), invMassAglSel.at(i).Data(), "goff");
            to_draw = drawclRinvMass.at(i)+">>+"+H2cl_TEST_RinvMassAgl.at(i)->GetName();
            CLtrees.at(3)->Draw(to_draw.Data(), TESTSel.Data(), "goff");
            to_draw = drawclRinvMass.at(i)+">>+"+H2ae_TEST_RinvMassAgl.at(i)->GetName();
            CLtrees.at(3)->Draw(to_draw.Data(), TESTSel.Data(), "goff");
        }
        H2_RCS.at(i)            = (LogToLin2D((TH2D*)gDirectory->Get(H2_RCS.at(i)->GetName())));
        H2_RAS.at(i)            = (LogToLin2D((TH2D*)gDirectory->Get(H2_RAS.at(i)->GetName())));
        H2_TEST_RCS.at(i)         = (LogToLin2D((TH2D*)gDirectory->Get(H2_TEST_RCS.at(i)->GetName())));
        H2_TEST_RAS.at(i)            = (LogToLin2D((TH2D*)gDirectory->Get(H2_TEST_RAS.at(i)->GetName())));

        H2cl_RinvMassNaF.at(i)    = (LogToLin2D((TH2D*)gDirectory->Get(H2cl_RinvMassNaF.at(i)->GetName())));
        H2ae_RinvMassNaF.at(i)    = (LogToLin2D((TH2D*)gDirectory->Get(H2ae_RinvMassNaF.at(i)->GetName())));
        H2cl_TEST_RinvMassNaF.at(i) = (LogToLin2D((TH2D*)gDirectory->Get(H2cl_TEST_RinvMassNaF.at(i)->GetName())));
        H2ae_TEST_RinvMassNaF.at(i)    = (LogToLin2D((TH2D*)gDirectory->Get(H2ae_TEST_RinvMassNaF.at(i)->GetName())));

        H2_CSinvMassNaF.at(i)     = (TH2D*)gDirectory->Get(H2_CSinvMassNaF.at(i)->GetName());
        H2_ASinvMassNaF.at(i)     = (TH2D*)gDirectory->Get(H2_ASinvMassNaF.at(i)->GetName());
        H2cl_TEST_RinvMassAgl.at(i) = (TH2D*)gDirectory->Get(H2cl_TEST_RinvMassAgl.at(i)->GetName());
        H2_TEST_ASinvMassNaF.at(i)  = (TH2D*)gDirectory->Get(H2_TEST_ASinvMassNaF.at(i)->GetName());
        
        H2cl_RinvMassAgl.at(i)    = (LogToLin2D((TH2D*)gDirectory->Get(H2cl_RinvMassAgl.at(i)->GetName())));
        H2ae_RinvMassAgl.at(i)    = (LogToLin2D((TH2D*)gDirectory->Get(H2ae_RinvMassAgl.at(i)->GetName())));
        H2cl_TEST_RinvMassAgl.at(i) = (LogToLin2D((TH2D*)gDirectory->Get(H2cl_TEST_RinvMassAgl.at(i)->GetName())));
        H2ae_TEST_RinvMassAgl.at(i) = (LogToLin2D((TH2D*)gDirectory->Get(H2ae_TEST_RinvMassAgl.at(i)->GetName())));

        H2_CSinvMassAgl.at(i)    = (TH2D*)gDirectory->Get(H2_CSinvMassAgl.at(i)->GetName());
        H2_ASinvMassAgl.at(i)    = (TH2D*)gDirectory->Get(H2_ASinvMassAgl.at(i)->GetName());
        H2_TEST_CSinvMassAgl.at(i) = (TH2D*)gDirectory->Get(H2_TEST_CSinvMassAgl.at(i)->GetName());
        H2_TEST_ASinvMassAgl.at(i) = (TH2D*)gDirectory->Get(H2_TEST_ASinvMassAgl.at(i)->GetName());

        // MC POSITIVES +
        if(i<TotRig) {
            TemName = DirName.at(0)+"/"+NameSub.at(0)+"/"+NameR.at(i%2);
            f_out->cd(TemName.Data());
        }
        //MC ATIVES -
        else{
            TemName = DirName.at(1)+"/"+NameSub.at(0)+"/"+NameR.at(i%2);
            f_out->cd(TemName.Data());
        }
        H2_RCS.at(i)->Write();
        H2_CSinvMassNaF.at(i)->Write();
        H2cl_RinvMassNaF.at(i)->Write();
        H2_CSinvMassAgl.at(i)->Write();
        H2cl_RinvMassAgl.at(i)->Write();

        // MC POSITIVES +
        if(i<TotRig) {
            TemName = DirName.at(0)+"/"+NameSub.at(1)+"/"+NameR.at(i%2);
            f_out->cd(TemName.Data());
        }
        //MC ATIVES -
        else{
            TemName = DirName.at(1)+"/"+NameSub.at(1)+"/"+NameR.at(i%2);
            f_out->cd(TemName.Data());
        }
        H2_RAS.at(i)->Write();
        H2_ASinvMassNaF.at(i)->Write();
        H2ae_RinvMassNaF.at(i)->Write();
        H2_ASinvMassAgl.at(i)->Write();
        H2ae_RinvMassAgl.at(i)->Write();

        // TEST POSITIVES +
        if(i<TotRig) {
            TemName = DirName.at(2)+"/"+NameSub.at(0)+"/"+NameR.at(i%2);
            f_out->cd(TemName.Data());
        }
        // TEST ATIVES -
        else{
            TemName = DirName.at(3)+"/"+NameSub.at(0)+"/"+NameR.at(i%2);
            f_out->cd(TemName.Data());
        }
        H2_TEST_RCS.at(i)->Write();
        H2_TEST_CSinvMassNaF.at(i)->Write();
        H2cl_TEST_RinvMassNaF.at(i)->Write();
        H2_TEST_CSinvMassAgl.at(i)->Write();
        H2cl_TEST_RinvMassAgl.at(i)->Write();

        // MC POSITIVES +
        if(i<TotRig) {
            TemName = DirName.at(2)+"/"+NameSub.at(1)+"/"+NameR.at(i%2);
            f_out->cd(TemName.Data());
        }
        //MC ATIVES -
        else{
            TemName = DirName.at(3)+"/"+NameSub.at(1)+"/"+NameR.at(i%2);
            f_out->cd(TemName.Data());
        }
        H2_TEST_RAS.at(i)->Write();
        H2_TEST_ASinvMassNaF.at(i)->Write();
        H2ae_TEST_RinvMassNaF.at(i)->Write();
        H2_TEST_ASinvMassAgl.at(i)->Write();
        H2ae_TEST_RinvMassAgl.at(i)->Write();
    } // end loop on histos

    f_out->cd();
    printf("So far so good2\n");

    //CLtrees->[MC_pos, MC_neg, ISS_pos, ISS_neg]
    for(int i=0; i<aeCutValues.size(); i++){   
        double VALnum, VALdenom, TESTnum, TESTdenom;
        double VALfrac, VALerr, TESTfrac, TESTerr;   
        //MC positives AGL
        VALnum = CLtrees.at(0)->GetEntries(aeAglVALposNumSel.at(i).Data())*1.;
        VALdenom = CLtrees.at(0)->GetEntries(aeAglVALposDenomSel.at(i).Data());
        (VALdenom!=0) ? VALfrac = VALnum/VALdenom : VALfrac = 0;
        // (VALdenom!=0 && VALnum!=0) ? VALerr = VALfrac*sqrt(1/VALnum + 1/VALdenom) : VALerr  = 0;
        (VALdenom!=0 && VALnum!=0) ? VALerr = sqrt(VALfrac*(1-VALfrac)/VALdenom) : VALerr  = 0;
        int ibin = H1_ASinvMassWAgl.at(0)->Fill(aeCutValues.at(i), VALfrac);
        H1_ASinvMassWAgl.at(0)->SetBinError(ibin, VALerr);

        //NaF
        VALnum = CLtrees.at(0)->GetEntries(aeNaFVALposNumSel.at(i).Data())*1.;
        VALdenom = CLtrees.at(0)->GetEntries(aeNaFVALposDenomSel.at(i).Data());
        (VALdenom!=0) ? VALfrac = VALnum/VALdenom : VALfrac = 0;
        // (VALdenom!=0 && VALnum!=0) ? VALerr = VALfrac*sqrt(1/VALnum + 1/VALdenom) : VALerr  = 0;
        (VALdenom!=0 && VALnum!=0) ? VALerr = sqrt(VALfrac*(1-VALfrac)/VALdenom) : VALerr  = 0;
        ibin = H1_ASinvMassWNaF.at(0)->Fill(aeCutValues.at(i), VALfrac);
        H1_ASinvMassWNaF.at(0)->SetBinError(ibin, VALerr);
        //MC negatives AGL       
        VALnum = CLtrees.at(1)->GetEntries(aeAglVALnegNumSel.at(i).Data())*1.;
        VALdenom = CLtrees.at(1)->GetEntries(aeAglVALnegDenomSel.at(i).Data());
        (VALdenom!=0) ? VALfrac = VALnum/VALdenom : VALfrac = 0;
        // (VALdenom!=0 && VALnum!=0) ? VALerr = VALfrac*sqrt(1/VALnum + 1/VALdenom) : VALerr  = 0;
        (VALdenom!=0 && VALnum!=0) ? VALerr = sqrt(VALfrac*(1-VALfrac)/VALdenom) : VALerr  = 0;
        ibin = H1_ASinvMassWAgl.at(1)->Fill(aeCutValues.at(i), VALfrac);
        H1_ASinvMassWAgl.at(1)->SetBinError(ibin, VALerr);
        //NaF
        VALnum = CLtrees.at(1)->GetEntries(aeNaFVALnegNumSel.at(i).Data())*1.;
        VALdenom = CLtrees.at(1)->GetEntries(aeNaFVALnegDenomSel.at(i).Data());
        (VALdenom!=0) ? VALfrac = VALnum/VALdenom : VALfrac = 0;
        // (VALdenom!=0 && VALnum!=0) ? VALerr = VALfrac*sqrt(1/VALnum + 1/VALdenom) : VALerr  = 0;
        (VALdenom!=0 && VALnum!=0) ? VALerr = sqrt(VALfrac*(1-VALfrac)/VALdenom) : VALerr  = 0;
        ibin = H1_ASinvMassWNaF.at(1)->Fill(aeCutValues.at(i), VALfrac);
        H1_ASinvMassWNaF.at(1)->SetBinError(ibin, VALerr);

        //TEST
        //positive AGL
        TESTnum = CLtrees.at(2)->GetEntries(aeAglTESTposNumSel.at(i).Data())*1.;
        TESTdenom = CLtrees.at(2)->GetEntries(aeAglTESTposDenomSel.at(i).Data());
        (TESTdenom!=0) ? TESTfrac = TESTnum/TESTdenom : TESTfrac = 0;
        (TESTdenom!=0 && TESTnum!=0) ? TESTerr = sqrt(TESTfrac*(1-TESTfrac)/TESTdenom) : TESTerr  = 0;
        ibin = H1_TEST_ASinvMassWAgl.at(0)->Fill(aeCutValues.at(i), TESTfrac);
        H1_TEST_ASinvMassWAgl.at(0)->SetBinError(ibin, TESTerr);
        //NaF
        TESTnum = CLtrees.at(2)->GetEntries(aeNaFTESTposNumSel.at(i).Data())*1.;
        TESTdenom = CLtrees.at(2)->GetEntries(aeNaFTESTposDenomSel.at(i).Data());
        (TESTdenom!=0) ? TESTfrac = TESTnum/TESTdenom : TESTfrac = 0;
        (TESTdenom!=0 && TESTnum!=0) ? TESTerr = sqrt(TESTfrac*(1-TESTfrac)/TESTdenom) : TESTerr  = 0;
        ibin = H1_TEST_ASinvMassWNaF.at(0)->Fill(aeCutValues.at(i), TESTfrac);
        H1_TEST_ASinvMassWNaF.at(0)->SetBinError(ibin, TESTerr);

        // negatives AGL       
        TESTnum = CLtrees.at(3)->GetEntries(aeAglTESTnegNumSel.at(i).Data())*1.;
        TESTdenom = CLtrees.at(3)->GetEntries(aeAglTESTnegDenomSel.at(i).Data());
        (TESTdenom!=0) ? TESTfrac = TESTnum/TESTdenom : TESTfrac = 0;
        (TESTdenom!=0 && TESTnum!=0) ? TESTerr = sqrt(TESTfrac*(1-TESTfrac)/TESTdenom) : TESTerr  = 0;
        ibin = H1_TEST_ASinvMassWAgl.at(1)->Fill(aeCutValues.at(i), TESTfrac);
        H1_TEST_ASinvMassWAgl.at(1)->SetBinError(ibin, TESTerr);
        //NaF
        TESTnum = CLtrees.at(3)->GetEntries(aeNaFTESTnegNumSel.at(i).Data())*1.;
        TESTdenom = CLtrees.at(3)->GetEntries(aeNaFTESTnegDenomSel.at(i).Data());
        (TESTdenom!=0) ? TESTfrac = TESTnum/TESTdenom : TESTfrac = 0;
        (TESTdenom!=0 && TESTnum!=0) ? TESTerr = sqrt(TESTfrac*(1-TESTfrac)/TESTdenom) : TESTerr  = 0;
        ibin = H1_TEST_ASinvMassWNaF.at(1)->Fill(aeCutValues.at(i), TESTfrac);
        H1_TEST_ASinvMassWNaF.at(1)->SetBinError(ibin, TESTerr);

        for(int j=0; j<clCutValues.size(); ++j){
            if(i==0){
                //MC positives AGL
                VALnum = CLtrees.at(0)->GetEntries(clAglVALposNumSel.at(j).Data())*1.;
                VALdenom = CLtrees.at(0)->GetEntries(clAglVALposDenomSel.at(j).Data());
                (VALnum!=0) ? VALfrac = VALnum/VALdenom : VALfrac = 0;
                // (VALdenom!=0 && VALnum!=0) ? VALerr = VALfrac*sqrt(1/VALnum + 1/VALdenom) : VALerr  = 0;
                (VALdenom!=0 && VALnum!=0) ? VALerr = sqrt(VALfrac*(1-VALfrac)/VALdenom) : VALerr  = 0;
                int ibin = H1_CSinvMassWAgl.at(0)->Fill(clCutValues.at(j), VALfrac);
                H1_CSinvMassWAgl.at(0)->SetBinError(ibin, VALerr);
                //NaF
                VALnum = CLtrees.at(0)->GetEntries(clNaFVALposNumSel.at(j).Data())*1.;
                VALdenom = CLtrees.at(0)->GetEntries(clNaFVALposDenomSel.at(j).Data());
                (VALnum!=0) ? VALfrac = VALnum/VALdenom : VALfrac = 0;
                // (VALdenom!=0 && VALnum!=0) ? VALerr = VALfrac*sqrt(1/VALnum + 1/VALdenom) : VALerr  = 0;
                (VALdenom!=0 && VALnum!=0) ? VALerr = sqrt(VALfrac*(1-VALfrac)/VALdenom) : VALerr  = 0;
                ibin = H1_CSinvMassWNaF.at(0)->Fill(clCutValues.at(j), VALfrac);
                H1_CSinvMassWNaF.at(0)->SetBinError(ibin, VALerr);
                //MC negatives AGL       
                VALnum = CLtrees.at(1)->GetEntries(clAglVALnegNumSel.at(j).Data())*1.;
                VALdenom = CLtrees.at(1)->GetEntries(clAglVALnegDenomSel.at(j).Data());
                (VALnum!=0) ? VALfrac = VALnum/VALdenom : VALfrac = 0;
                // (VALdenom!=0 && VALnum!=0) ? VALerr = VALfrac*sqrt(1/VALnum + 1/VALdenom) : VALerr  = 0;
                (VALdenom!=0 && VALnum!=0) ? VALerr = sqrt(VALfrac*(1-VALfrac)/VALdenom) : VALerr  = 0;
                ibin = H1_CSinvMassWAgl.at(1)->Fill(clCutValues.at(j), VALfrac);
                H1_CSinvMassWAgl.at(1)->SetBinError(ibin, VALerr);
                //NaF
                VALnum = CLtrees.at(1)->GetEntries(clNaFVALnegNumSel.at(j).Data())*1.;
                VALdenom = CLtrees.at(1)->GetEntries(clNaFVALnegDenomSel.at(j).Data());
                (VALnum!=0) ? VALfrac = VALnum/VALdenom : VALfrac = 0;
                // (VALdenom!=0 && VALnum!=0) ? VALerr = VALfrac*sqrt(1/VALnum + 1/VALdenom) : VALerr  = 0;
                (VALdenom!=0 && VALnum!=0) ? VALerr = sqrt(VALfrac*(1-VALfrac)/VALdenom) : VALerr  = 0;
                ibin = H1_CSinvMassWNaF.at(1)->Fill(clCutValues.at(j), VALfrac);
                H1_CSinvMassWNaF.at(1)->SetBinError(ibin, VALerr);
                
                //TEST positives AGL
                TESTnum = CLtrees.at(2)->GetEntries(clAglVALposNumSel.at(j).Data())*1.;
                TESTdenom = CLtrees.at(2)->GetEntries(clAglVALposDenomSel.at(j).Data());
                (TESTnum!=0) ? TESTfrac = TESTnum/TESTdenom : TESTfrac = 0;
                (TESTdenom!=0 && TESTnum!=0) ? TESTerr = sqrt(TESTfrac*(1-TESTfrac)/TESTdenom) : TESTerr  = 0;
                ibin = H1_TEST_CSinvMassWAgl.at(0)->Fill(clCutValues.at(j), TESTfrac);
                H1_TEST_CSinvMassWAgl.at(0)->SetBinError(ibin, TESTerr);
                //NaF
                TESTnum = CLtrees.at(2)->GetEntries(clNaFVALposNumSel.at(j).Data())*1.;
                TESTdenom = CLtrees.at(2)->GetEntries(clNaFVALposDenomSel.at(j).Data());
                (TESTnum!=0) ? TESTfrac = TESTnum/TESTdenom : TESTfrac = 0;
                (TESTdenom!=0 && TESTnum!=0) ? TESTerr = sqrt(TESTfrac*(1-TESTfrac)/TESTdenom) : TESTerr  = 0;
                ibin = H1_TEST_CSinvMassWNaF.at(0)->Fill(clCutValues.at(j), TESTfrac);
                H1_TEST_CSinvMassWNaF.at(0)->SetBinError(ibin, TESTerr);
                //TEST negatives
                TESTnum = CLtrees.at(3)->GetEntries(clAglVALnegNumSel.at(j).Data())*1.;
                TESTdenom = CLtrees.at(3)->GetEntries(clAglVALnegDenomSel.at(j).Data());
                (TESTnum!=0) ? TESTfrac = TESTnum/TESTdenom : TESTfrac = 0;
                (TESTdenom!=0 && TESTnum!=0) ? TESTerr = sqrt(TESTfrac*(1-TESTfrac)/TESTdenom) : TESTerr  = 0;
                ibin = H1_TEST_CSinvMassWAgl.at(1)->Fill(clCutValues.at(j), TESTfrac);
                H1_TEST_CSinvMassWAgl.at(1)->SetBinError(ibin, TESTerr);
                //NaF
                TESTnum = CLtrees.at(3)->GetEntries(clNaFVALnegNumSel.at(j).Data())*1.;
                TESTdenom = CLtrees.at(3)->GetEntries(clNaFVALnegDenomSel.at(j).Data());
                (TESTnum!=0) ? TESTfrac = TESTnum/TESTdenom : TESTfrac = 0;
                (TESTdenom!=0 && TESTnum!=0) ? TESTerr = sqrt(TESTfrac*(1-TESTfrac)/TESTdenom) : TESTerr  = 0;
                ibin = H1_TEST_CSinvMassWNaF.at(1)->Fill(clCutValues.at(j), TESTfrac);
                H1_TEST_CSinvMassWNaF.at(1)->SetBinError(ibin, TESTerr);
            }
            //MC positives AGL
            VALnum = CLtrees.at(0)->GetEntries(combAglVALposNumSel.at((i*clCutValues.size())+j).Data())*1.;
            VALdenom = CLtrees.at(0)->GetEntries(combAglVALposDenomSel.at((i*clCutValues.size())+j).Data());
            (VALdenom!=0) ? VALfrac = VALnum/VALdenom : VALfrac = 0;
            // (VALdenom!=0 && VALnum!=0) ? VALerr = VALfrac*sqrt(1/VALnum + 1/VALdenom) : VALerr  = 0;
            (VALdenom!=0 && VALnum!=0) ? VALerr = sqrt(VALfrac*(1-VALfrac)/VALdenom) : VALerr  = 0;
            int ibin = H2_combinvMassWAgl.at(0)->Fill(aeCutValues.at(i), clCutValues.at(j), VALfrac);
            H2_combinvMassWAglN.at(0)->Fill(aeCutValues.at(i),clCutValues.at(j),VALnum);
            H2_combinvMassWAgl.at(0)->SetBinError(ibin, VALerr);
            //NaF
            VALnum = CLtrees.at(0)->GetEntries(combNaFVALposNumSel.at((i*clCutValues.size())+j).Data())*1.;
            VALdenom = CLtrees.at(0)->GetEntries(combNaFVALposDenomSel.at((i*clCutValues.size())+j).Data());
            (VALdenom!=0) ? VALfrac = VALnum/VALdenom : VALfrac = 0;
            // (VALdenom!=0 && VALnum!=0) ? VALerr = VALfrac*sqrt(1/VALnum + 1/VALdenom) : VALerr  = 0;
            (VALdenom!=0 && VALnum!=0) ? VALerr = sqrt(VALfrac*(1-VALfrac)/VALdenom) : VALerr  = 0;
            ibin = H2_combinvMassWNaF.at(0)->Fill(aeCutValues.at(i), clCutValues.at(j), VALfrac);
            H2_combinvMassWNaFN.at(0)->Fill(aeCutValues.at(i), clCutValues.at(j), VALnum);
            H2_combinvMassWNaF.at(0)->SetBinError(ibin, VALerr);
            //MC negatives AGL       
            VALnum = CLtrees.at(1)->GetEntries(combAglVALnegNumSel.at((i*clCutValues.size())+j).Data())*1.;
            VALdenom = CLtrees.at(1)->GetEntries(combAglVALnegDenomSel.at((i*clCutValues.size())+j).Data());
            (VALdenom!=0) ? VALfrac = VALnum/VALdenom : VALfrac = 0;
            // (VALdenom!=0 && VALnum!=0) ? VALerr = VALfrac*sqrt(1/VALnum + 1/VALdenom) : VALerr  = 0;
            (VALdenom!=0 && VALnum!=0) ? VALerr = sqrt(VALfrac*(1-VALfrac)/VALdenom) : VALerr  = 0;
            ibin = H2_combinvMassWAgl.at(1)->Fill(aeCutValues.at(i), clCutValues.at(j), VALfrac);
            H2_combinvMassWAglN.at(1)->Fill(aeCutValues.at(i), clCutValues.at(j), VALnum);
            H2_combinvMassWAgl.at(1)->SetBinError(ibin, VALerr);
            //NaF
            VALnum = CLtrees.at(1)->GetEntries(combNaFVALnegNumSel.at((i*clCutValues.size())+j).Data())*1.;
            VALdenom = CLtrees.at(1)->GetEntries(combNaFVALnegDenomSel.at((i*clCutValues.size())+j).Data());
            (VALdenom!=0) ? VALfrac = VALnum/VALdenom : VALfrac = 0;
            // (VALdenom!=0 && VALnum!=0) ? VALerr = VALfrac*sqrt(1/VALnum + 1/VALdenom) : VALerr  = 0;
            (VALdenom!=0 && VALnum!=0) ? VALerr = sqrt(VALfrac*(1-VALfrac)/VALdenom) : VALerr  = 0;
            ibin = H2_combinvMassWNaF.at(1)->Fill(aeCutValues.at(i), clCutValues.at(j), VALfrac);
            H2_combinvMassWNaFN.at(1)->Fill(aeCutValues.at(i), clCutValues.at(j), VALnum);
            H2_combinvMassWNaF.at(1)->SetBinError(ibin, VALerr);

            //ISS positives AGL
            TESTnum = CLtrees.at(2)->GetEntries(combAglTESTposNumSel.at((i*clCutValues.size())+j).Data())*1.;
            TESTdenom = CLtrees.at(2)->GetEntries(combAglTESTposDenomSel.at((i*clCutValues.size())+j).Data());
            (TESTdenom!=0) ? TESTfrac = TESTnum/TESTdenom : TESTfrac = 0;
            (TESTdenom!=0 && TESTnum!=0) ? TESTerr = sqrt(TESTfrac*(1-TESTfrac)/TESTdenom) : TESTerr  = 0;
            ibin = H2_combinvMassWAgl.at(0)->Fill(aeCutValues.at(i), clCutValues.at(j), TESTfrac);
            H2_combinvMassWAglN.at(0)->Fill(aeCutValues.at(i),clCutValues.at(j),TESTnum);
            H2_combinvMassWAgl.at(0)->SetBinError(ibin, TESTerr);
            //NaF
            TESTnum = CLtrees.at(2)->GetEntries(combNaFTESTposNumSel.at((i*clCutValues.size())+j).Data())*1.;
            TESTdenom = CLtrees.at(2)->GetEntries(combNaFTESTposDenomSel.at((i*clCutValues.size())+j).Data());
            (TESTdenom!=0) ? TESTfrac = TESTnum/TESTdenom : TESTfrac = 0;
            (TESTdenom!=0 && TESTnum!=0) ? TESTerr = sqrt(TESTfrac*(1-TESTfrac)/TESTdenom) : TESTerr  = 0;
            ibin = H2_combinvMassWNaF.at(0)->Fill(aeCutValues.at(i), clCutValues.at(j), TESTfrac);
            H2_combinvMassWNaFN.at(0)->Fill(aeCutValues.at(i), clCutValues.at(j), TESTnum);
            H2_combinvMassWNaF.at(0)->SetBinError(ibin, TESTerr);
            //ISS negatives AGL       
            TESTnum = CLtrees.at(3)->GetEntries(combAglTESTnegNumSel.at((i*clCutValues.size())+j).Data())*1.;
            TESTdenom = CLtrees.at(3)->GetEntries(combAglTESTnegDenomSel.at((i*clCutValues.size())+j).Data());
            (TESTdenom!=0) ? TESTfrac = TESTnum/TESTdenom : TESTfrac = 0;
            (TESTdenom!=0 && TESTnum!=0) ? TESTerr = sqrt(TESTfrac*(1-TESTfrac)/TESTdenom) : TESTerr  = 0;
            ibin = H2_combinvMassWAgl.at(1)->Fill(aeCutValues.at(i), clCutValues.at(j), TESTfrac);
            H2_combinvMassWAglN.at(1)->Fill(aeCutValues.at(i), clCutValues.at(j), TESTnum);
            H2_combinvMassWAgl.at(1)->SetBinError(ibin, TESTerr);
            //NaF
            TESTnum = CLtrees.at(3)->GetEntries(combNaFTESTnegNumSel.at((i*clCutValues.size())+j).Data())*1.;
            TESTdenom = CLtrees.at(3)->GetEntries(combNaFTESTnegDenomSel.at((i*clCutValues.size())+j).Data());
            (TESTdenom!=0) ? TESTfrac = TESTnum/TESTdenom : TESTfrac = 0;
            (TESTdenom!=0 && TESTnum!=0) ? TESTerr = sqrt(TESTfrac*(1-TESTfrac)/TESTdenom) : TESTerr  = 0;
            ibin = H2_combinvMassWNaF.at(1)->Fill(aeCutValues.at(i), clCutValues.at(j), TESTfrac);
            H2_combinvMassWNaFN.at(1)->Fill(aeCutValues.at(i), clCutValues.at(j), TESTnum);
            H2_combinvMassWNaF.at(1)->SetBinError(ibin, TESTerr);
        }
        if(i==0)printf("Progress:\n");
        printf("%.2f\r",i*100./aeCutValues.size());
        fflush(stdout);
    }

    for(int i=0; i<H1_CSinvMassWAgl.size(); ++i){
        H1_CSinvMassWAgl.at(i)->Write();
        H1_TEST_CSinvMassWAgl.at(i)->Write();
        H1_ASinvMassWAgl.at(i)->Write();
        H1_TEST_ASinvMassWAgl.at(i)->Write();
        H2_combinvMassWAgl.at(i)->Write();
        H2_combinvMassWAglN.at(i)->Write();

        H1_CSinvMassWNaF.at(i)->Write();
        H1_TEST_CSinvMassWNaF.at(i)->Write();
        H1_ASinvMassWNaF.at(i)->Write();
        H1_TEST_ASinvMassWNaF.at(i)->Write();
        H2_combinvMassWNaF.at(i)->Write();
        H2_combinvMassWNaFN.at(i)->Write();
    }



    //How to select survirors using TEventList
    // TotSelection = "hasRich==1 && beta_rich>0.953 && isNaF==0 && hasGoodImpact==1 && kprob_rich>0.01 &&"
    //                "1<charge2_rich && charge2_rich<4 && ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.8 &&"
    //                "scores>0.06 && (anomaly_score!=1) && -Rinner<50 && abs(SigmaUpLow)<=0.5 &&"
    //                "ACC_AntiCounter==0 && InnerHit==8 && isAbIGRFneg";
    // NO cutOff
    // TotSelection = "hasRich==1 && beta_rich>0.953 && isNaF==0 && hasGoodImpact==1 && kprob_rich>0.01 &&"
    //                 "1<charge2_rich && charge2_rich<4 && ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.8 &&"
    //                 "-Rinner<100 && abs(SigmaUpLow)<0.5 && ACC_AntiCounter==0 && InnerHit==8 && anomaly_score!=1 && scores>0.09";
    TotSelection = "hasRich==1 && beta_rich>0.953 && beta_rich < 0.999 && isNaF==0 && hasGoodImpact==1 && kprob_rich>0.01 &&"
                    "1<charge2_rich && charge2_rich<4 && ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4 &&"
                    "scores>=0.3 && abs(SigmaUpLow)<=0.4 && anomaly_score<=0.8";

    

    printf("----> NO GM cutoff\n");
    printf("Selection for survivors: %s\n", TotSelection.Data());

    printf(" ISS data (Rinner<0) \n");
    to_draw = ">>MyEventListNoGMcutoff";
    int MyEntries = CLtrees.at(3)->Draw(to_draw.Data(),TotSelection.Data());
    TEventList *elistNoGMcutoff = (TEventList*)gDirectory->Get("MyEventListNoGMcutoff");
    CLtrees.at(3)->SetEventList(elistNoGMcutoff);
    CLtrees.at(3)->Scan("((2.*abs(Rinner)*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315)):scores:anomaly_score",TotSelection.Data());
    TH1D* h1 = new TH1D("ISS_data_noGMcutoff","ISS data (Rinner<=0) no GM cutoff; Mass AGL [a.m.u.]; Events", 100, 0, 10);
    CLtrees.at(3)->Draw("((2.*abs(Rinner)*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))>>ISS_data_noGMcutoff",TotSelection.Data(),"goff");
    h1 = (TH1D*)gDirectory->Get("ISS_data_noGMcutoff");
    h1->Write();
    delete h1;
    unsigned int EvNum=0, EvRun=0;
    
    double_t MCevNumD=0, MCevRunD=0;
    unsigned int  MCevNumU=0, MCevRunU=0;

    CLtrees.at(3)->SetBranchAddress("EvNum", &EvNum);
    CLtrees.at(3)->SetBranchAddress("EvRun", &EvRun);
    
    for(int ie=0; ie<MyEntries; ie++){
        CLtrees.at(3)->GetEntry(elistNoGMcutoff->GetEntry(ie));
        printf("Run number: %u, Event number: %u\n", EvRun, EvNum);
    }

    printf("\n MC He4 (Rinner<0) \n");
    to_draw = ">>MyEventListMC";
    MyEntries = 0;
    MyEntries = CLtrees.at(1)->Draw(to_draw.Data(),TotSelection.Data());
    TEventList *elistMC = (TEventList*)gDirectory->Get("MyEventListMC");
    CLtrees.at(1)->SetEventList(elistMC);
    CLtrees.at(1)->Scan("((2.*abs(Rinner)*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315)):scores:anomaly_score:weight",TotSelection.Data());
    TH1D* h1MC = new TH1D("MC_He4_negative","MC He4 (Rinner<=0); Mass AGL [a.m.u.]; Weights", 100, 0, 10);
    TotSelection.Prepend("("); TotSelection.Append(")*weight");
    CLtrees.at(1)->Draw("((2.*abs(Rinner)*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))>>MC_He4_negative",TotSelection.Data(),"goff");
    h1MC =  (TH1D*)gDirectory->Get("MC_He4_negative");
    h1MC->Write();
    delete h1MC;

    TString EvNumMCType(CLtrees.at(1)->GetLeaf("EvNum")->GetTypeName());
    TString EvRunMCType(CLtrees.at(1)->GetLeaf("EvRun")->GetTypeName());
    bool isSame = false;
    if(EvNumMCType.CompareTo(EvRunMCType) == 0) isSame = true;
    if(isSame){
        if(EvNumMCType.CompareTo("Double_t")==0) {
            CLtrees.at(1)->SetBranchAddress("EvNum", &MCevNumD);
            CLtrees.at(1)->SetBranchAddress("EvRun", &MCevRunD);
        }
        else{ //if not double then unsigned int
            CLtrees.at(1)->SetBranchAddress("EvNum", &MCevNumU);
            CLtrees.at(1)->SetBranchAddress("EvRun", &MCevRunU);
        }
    }
    else{
        if(EvNumMCType.CompareTo("Double_t")==0)  CLtrees.at(1)->SetBranchAddress("EvNum", &MCevNumD);
        else CLtrees.at(1)->SetBranchAddress("EvNum", &MCevNumU);
        if(EvRunMCType.CompareTo("Double_t")==0) CLtrees.at(1)->SetBranchAddress("EvRun", &MCevRunD);
        else CLtrees.at(1)->SetBranchAddress("EvRun", &MCevRunU);
    }
        
    for(int ie=0; ie<MyEntries; ie++){
        CLtrees.at(1)->GetEntry(elistMC->GetEntry(ie));
        if(isSame){
            if(EvNumMCType.CompareTo("Double_t")==0) printf("Run number: %f, Event number: %f\n", MCevRunD, MCevNumD);
            else printf("Run number: %u, Event number: %u\n", MCevRunU, MCevNumU);
        }
        else{
            if(EvNumMCType.CompareTo("Double_t")==0) printf("Run number: %f, Event number: %u\n", MCevRunD, MCevNumU);
            else printf("Run number: %u, Event number: %f\n", MCevRunU, MCevNumD);
        }
    }


    printf("\n----> WITH GM cutoff\n");
    TotSelection = "hasRich==1 && beta_rich>0.953 && isNaF==0 && hasGoodImpact==1 && kprob_rich>0.01 &&"
                    "1<charge2_rich && charge2_rich<4 && ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.8 &&"
                    "-Rinner<100 && abs(SigmaUpLow)<0.5 && ACC_AntiCounter==0 && InnerHit==8 && anomaly_score!=1 && scores>0.09 && isAbIGRFneg";
    // TotSelection = "hasRich==1 && beta_rich>0.953 && isNaF==0 && hasGoodImpact==1 && kprob_rich>0.01 &&"
    //                 "1<charge2_rich && charge2_rich<4 && ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.8 &&"
    //                 "-Rinner<100 && abs(SigmaUpLow)<0.5 && ACC_AntiCounter==0 && InnerHit==8 && anomaly_score!=1 && scores>0.09 && isAbIGRFneg";
    printf("Selection for survivors: %s\n", TotSelection.Data());

    
    to_draw = ">>MyEventListGMcutoff";
    MyEntries = CLtrees.at(3)->Draw(to_draw.Data(),TotSelection.Data());
    TEventList *elistGMcutoff = (TEventList*)gDirectory->Get("MyEventListGMcutoff");
    CLtrees.at(3)->SetEventList(elistGMcutoff);
    CLtrees.at(3)->Scan("((2.*abs(Rinner)*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315)):scores:anomaly_score",TotSelection.Data());
    TH1D* h1GM = new TH1D("ISS_data_GMcutoff","ISS data (Rinner<=0) GM cutoff; Mass AGL [a.m.u.]; Events", 100, 0, 10);
    CLtrees.at(3)->Draw("((2.*abs(Rinner)*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))>>ISS_data_GMcutoff",TotSelection.Data(),"goff");
    h1GM =  (TH1D*)gDirectory->Get("ISS_data_GMcutoff");
    h1GM->Write();
    delete h1GM;

    CLtrees.at(3)->SetBranchAddress("EvNum", &EvNum);
    CLtrees.at(3)->SetBranchAddress("EvRun", &EvRun);
    
    for(int ie=0; ie<MyEntries; ie++){
        CLtrees.at(3)->GetEntry(elistGMcutoff->GetEntry(ie));
        printf("Run number: %u, Event number: %u\n", EvRun, EvNum);
    }
    
    
    return;
}

//Log(R) -> to R in log scale
TH2D * LogToLin2D(TH2D* h2){

    TString Name = h2->GetName();
    TString Title = h2->GetTitle();

    //Projection
    TH1 *h1X = h2->ProjectionX();
    TH1 *h1Y = h2->ProjectionY();

    //Rigidity axis
    const int nBinsX = h1X->GetNbinsX();
    float minR = pow(10, h1X->GetBinLowEdge(1));
    float maxR = pow(10, h1X->GetBinLowEdge(nBinsX+1));
    //Mass - CS axis
    const int nBinsY = h1Y->GetNbinsX();
    float minY = h1Y->GetBinLowEdge(1);
    float maxY = h1Y->GetBinLowEdge(nBinsY+1);
    TString yTitle = h2->GetYaxis()->GetTitle();

    //bin edges for the new histogram
    double xbins[nBinsX+1], ybins[nBinsY+1];
    for(int ii=1; ii<=nBinsX+1; ii++) xbins[ii-1] = pow(10, h1X->GetBinLowEdge(ii));
    for(int jj=1; jj<=nBinsY+1; jj++) ybins[jj-1] = h1Y->GetBinLowEdge(jj);
    
    TH2D *h2_new = new TH2D("h2_new", "h2_new", nBinsX, xbins, nBinsY, ybins);
    for(int xx=0; xx<=nBinsX+1; xx++){
        for(int yy=0; yy<=nBinsY+1; yy++){
            if(h2->GetBinContent(xx, yy)==0) continue;
            h2_new->SetBinContent(xx, yy, h2->GetBinContent(xx, yy));
        }
    }
    delete h2;

    h2_new->SetName(Name.Data());
    h2_new->SetTitle(Title.Data());
    h2_new->GetXaxis()->SetTitle("|R| [GV]");
    h2_new->GetXaxis()->SetTitleOffset(1.2);
    h2_new->GetXaxis()->SetLabelSize(0.045);
    h2_new->GetXaxis()->SetTitleSize(0.05);
    h2_new->GetYaxis()->SetTitle(yTitle.Data());
    h2_new->GetYaxis()->SetLabelSize(0.045);
    h2_new->GetYaxis()->SetTitleSize(0.05);
    return h2_new;
}

TH1D *LogToLin1D(TH1D *h1){
    TString Name = h1->GetName();
    TString Title = h1->GetTitle();

    //Rigidity axis
    const int nBinsX = h1->GetNbinsX();
    float minR = pow(10, h1->GetBinLowEdge(1));
    float maxR = pow(10, h1->GetBinLowEdge(nBinsX+1));
    //Y axis
    TString yTitle = h1->GetYaxis()->GetTitle();

    //bin edges for the new histogram
    double xbins[nBinsX+1];
    for(int ii=1; ii<=nBinsX+1; ii++) xbins[ii-1] = pow(10, h1->GetBinLowEdge(ii));
    
    TH1D *h1_new = new TH1D("h1_new", "h1_new", nBinsX, xbins);
    for(int xx=0; xx<=nBinsX+1; xx++){
        if(h1->GetBinContent(xx)==0) continue;
        h1_new->SetBinContent(xx, h1->GetBinContent(xx));
    }
    delete h1;

    h1_new->SetName(Name.Data());
    h1_new->SetTitle(Title.Data());
    h1_new->GetXaxis()->SetTitle("|R| [GV]");
    h1_new->GetXaxis()->SetTitleOffset(1.2);
    h1_new->GetXaxis()->SetLabelSize(0.045);
    h1_new->GetXaxis()->SetTitleSize(0.05);
    h1_new->GetYaxis()->SetTitle(yTitle.Data());
    h1_new->GetYaxis()->SetLabelSize(0.045);
    h1_new->GetYaxis()->SetTitleSize(0.05);
    return h1_new;
}
