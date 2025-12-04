// Author: F.Rossi
// Created on: 24.01.2025
// Last modified on: 13.02.2025
// Revision 1.0 -> adding the MC
// Revision 2.0 -> adding the AE
/*
 To be used on models trained on DATA
 Plot inverse mass vs scores
 -------------------------------------------------------
    How to use it:

    give the folders date as input parameters and
    just run this command:
        root -l 'FullMass.cc("CLdataFile", "CLmcFile", "AEdataFile", "AEmcFile")'
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

void trainOnISS_mass(TString CLdataFile="000", TString CLmcFile="000", TString AEdataFile="000", TString AEmcFile="000"){
    gStyle->SetPalette(55);//kRainBow palette
    const bool useSafetyFactor = true; //Safety factor = 1.2
    const bool useCutOff       = false; // Value of the cutoff (no safety factor)
    if(useSafetyFactor) printf("---> Using GM-cutoff with a safety fafctor of 1.2\n");
    if(useCutOff) printf("---> Using Cutoff with a safety fafctor of 1.\n");
    const float uma    = 0.9315; //GeV/c^2
    const float LabelSize = 0.045, TitleSize = 0.05;

    const int TotFiles = 4; //number of files
    const int TotTree  = 4; //n. of CLtrees
    const int TotRig   = 2; //n. of rigidities
    const int TotDet   = 2; //n. of detectors

    const int Ncut     = 10; // n. of interval on scores

    array<TString, TotFiles> PathToFile{"/eos/home-f/frrossi/AMS/output/organized_output/scores/"+CLdataFile+"/"+CLdataFile+"_trainOnISS_2CL_ISS_scores.root",
                                        "/eos/home-f/frrossi/AMS/output/organized_output/scores/"+CLmcFile+"/"+CLmcFile+"_trainOnISS_2CL_MC_scores.root",
                                        "/eos/home-f/frrossi/AMS/output/organized_output/scores/"+AEdataFile+"/"+AEdataFile+"_trainOnISS_2CL_AE_ISS_scores.root",
                                        "/eos/home-f/frrossi/AMS/output/organized_output/scores/"+AEmcFile+"/"+AEmcFile+"_trainOnISS_2CL_AE_MC_scores.root",
                                    };
    array<TFile*, TotFiles> files;
    TFile *f_out = new TFile("/eos/home-f/frrossi/AMS/ams_network/root_files/TrainOnData/ISS_MC_mass.root", "RECREATE");
    array<TTree*, TotTree> CLtrees;
    array<TTree*, TotTree> AEtrees;
    //TH1 vector
    vector<TH1*> H1_CSinvMassWAgl, H1_MC_CSinvMassWAgl, H1_ASinvMassWAgl, H1_MC_ASinvMassWAgl;
    vector<TH1*> H1_CSinvMassWNaF, H1_MC_CSinvMassWNaF, H1_ASinvMassWNaF, H1_MC_ASinvMassWNaF;
    vector<TH1*> H1cl_R,           H1cl_MC_R,           H1ae_R,            H1ae_MC_R;
    //TH2 vectors
    vector<TH2D*> H2_combinvMassWAgl, H2_combinvMassWNaF;
    vector<TH2D*> H2_combinvMassWAglN, H2_combinvMassWNaFN;
    vector<TH2D*> H2_CSinvMassNaF,  H2_MC_CSinvMassNaF,  H2_ASinvMassNaF,  H2_MC_ASinvMassNaF; 
    vector<TH2D*> H2cl_RinvMassNaF, H2cl_MC_RinvMassNaF, H2ae_RinvMassNaF, H2ae_MC_RinvMassNaF;
    vector<TH2D*> H2_CSinvMassAgl,  H2_MC_CSinvMassAgl,  H2_ASinvMassAgl,  H2_MC_ASinvMassAgl;
    vector<TH2D*> H2cl_RinvMassAgl, H2cl_MC_RinvMassAgl, H2ae_RinvMassAgl, H2ae_MC_RinvMassAgl;
    vector<TH2D*> H2_RCS,           H2_MC_RCS,           H2_RAS,           H2_MC_RAS;
    array<double, Ncut> clCutValues; // from 0 to 1
    for(int i=0; i<clCutValues.size(); ++i) clCutValues.at(i) = 1./clCutValues.size()-0.005+(i*1./clCutValues.size());
    array<double, Ncut> aeCutValues; // from 1 to 0.5
    for(int i=0; i<aeCutValues.size(); ++i) aeCutValues.at(i) = 0.995 - (i*0.5/aeCutValues.size());
    printf("Check on last CS cut: %f\n", clCutValues.at(clCutValues.size()-1));
    printf("Check on last AS cut: %f\n", aeCutValues.at(aeCutValues.size()-1));


    array<TString, TotRig> NameSub{"CL","AE"};
    array<TString, TotRig> NameR{"Rinner", "RinnerL1"};
    array<TString, TotTree> DirName{"ISS_pos", "ISS_neg", "MC_pos", "MC_neg"};
    //CLassifier score
    array<TString, TotTree/2> nCSinvMassWAglISS{"CSinvMassWPosAgl",  "CSinvMassWNegAgl"};
    array<TString, TotTree/2> nCSinvMassWNaFISS{"CSinvMassWPosNaF",  "CSinvMassWNegNaF"};
    array<TString, TotTree/2> nCSinvMassWAglMC{"CSinvMassWMCPosAgl", "CSinvMassWMCNegAgl"};
    array<TString, TotTree/2> nCSinvMassWNaFMC{"CSinvMassWMCPosNaF", "CSinvMassWMCNegNaF"};
    //Anomaly score
    array<TString, TotTree/2> nASinvMassWAglISS{"ASinvMassWPosAgl",  "ASinvMassWNegAgl"};
    array<TString, TotTree/2> nASinvMassWNaFISS{"ASinvMassWPosNaF",  "ASinvMassWNegNaF"};
    array<TString, TotTree/2> nASinvMassWAglMC{"ASinvMassWMCPosAgl", "ASinvMassWMCNegAgl"};
    array<TString, TotTree/2> nASinvMassWNaFMC{"ASinvMassWMCPosNaF", "ASinvMassWMCNegNaF"};
    //Combination
    array<TString, TotTree/2> ncombinvMassWAglISS{"combinvMassWPosAgl",  "combinvMassWNegAgl"};
    array<TString, TotTree/2> ncombinvMassWNaFISS{"combinvMassWPosNaF",  "combinvMassWNegNaF"};
    array<TString, TotTree/2> ncombinvMassWAglISSN{"combinvMassWPosAglN",  "combinvMassWNegAglN"};
    array<TString, TotTree/2> ncombinvMassWNaFISSN{"combinvMassWPosNaFN",  "combinvMassWNegNaFN"};
    array<TString, TotTree/2> ncombinvMassWAglMC{"combinvMassWMCPosAgl", "combinvMassWMCNegAgl"};
    array<TString, TotTree/2> ncombinvMassWNaFMC{"combinvMassWMCPosNaF", "combinvMassWMCNegNaF"};
    TString tCSinvMassW    = ";Classifier score cut value; ISS event fraction m^{Agl}#in[2, 5] [a.m.u.]";
    TString tASinvMassW    = ";Anomaly score cut value; ISS event fraction m^{Agl}#in[2, 5] [a.m.u.]";
    TString tcombinvMassW  = ";Anomaly score cut value; Classifier score cut value; ISS event fraction m^{Agl}#in[2, 5] [a.m.u.]";
    TString tcombinvMassWN = ";Anomaly score cut value; Classifier score cut value; ISS event number m^{Agl}#in[2, 5] [a.m.u.]";

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
    vector<TString> clAglISSposNumSel, clAglISSposDenomSel, clAglISSnegNumSel, clAglISSnegDenomSel, clAglMCposNumSel, clAglMCposDenomSel, clAglMCnegNumSel, clAglMCnegDenomSel;
    vector<TString> clNaFISSposNumSel, clNaFISSposDenomSel, clNaFISSnegNumSel, clNaFISSnegDenomSel, clNaFMCposNumSel, clNaFMCposDenomSel, clNaFMCnegNumSel, clNaFMCnegDenomSel;
    //ae
    vector<TString> aeAglISSposNumSel, aeAglISSposDenomSel, aeAglISSnegNumSel, aeAglISSnegDenomSel, aeAglMCposNumSel, aeAglMCposDenomSel, aeAglMCnegNumSel, aeAglMCnegDenomSel;
    vector<TString> aeNaFISSposNumSel, aeNaFISSposDenomSel, aeNaFISSnegNumSel, aeNaFISSnegDenomSel, aeNaFMCposNumSel, aeNaFMCposDenomSel, aeNaFMCnegNumSel, aeNaFMCnegDenomSel;
    //comb
    vector<TString> combAglISSposNumSel, combAglISSposDenomSel, combAglISSnegNumSel, combAglISSnegDenomSel, combAglMCposNumSel, combAglMCposDenomSel, combAglMCnegNumSel, combAglMCnegDenomSel;
    vector<TString> combNaFISSposNumSel, combNaFISSposDenomSel, combNaFISSnegNumSel, combNaFISSnegDenomSel, combNaFMCposNumSel, combNaFMCposDenomSel, combNaFMCnegNumSel, combNaFMCnegDenomSel;
    //initialize the event count selection
    for(int i=0; i<aeCutValues.size(); ++i){
        //ae
        aeAglISSposNumSel.push_back("&& ISS.anomaly_score<"+to_string(aeCutValues.at(i)));
        aeAglISSposDenomSel.push_back("&& ISS.anomaly_score<"+to_string(aeCutValues.at(i)));
        aeAglISSnegNumSel.push_back("&& ISSneg.NEGanomaly_score<"+to_string(aeCutValues.at(i)));
        aeAglISSnegDenomSel.push_back("&& ISSneg.NEGanomaly_score<"+to_string(aeCutValues.at(i)));

        aeNaFISSposNumSel.push_back("&& ISS.anomaly_score<"+to_string(aeCutValues.at(i)));
        aeNaFISSposDenomSel.push_back("&& ISS.anomaly_score<"+to_string(aeCutValues.at(i)));
        aeNaFISSnegNumSel.push_back("&& ISSneg.NEGanomaly_score<"+to_string(aeCutValues.at(i)));
        aeNaFISSnegDenomSel.push_back("&& ISSneg.NEGanomaly_score<"+to_string(aeCutValues.at(i)));
        if(useSafetyFactor) {
            //ae
            aeAglISSposNumSel.back()   = AglSel+aeAglISSposNumSel.back()+isAbIGRFpos+AglRinnerInvMass;
            aeAglISSposDenomSel.back() = AglSel+aeAglISSposDenomSel.back()+isAbIGRFpos;
            aeAglISSnegNumSel.back()   = AglSel+aeAglISSnegNumSel.back()+isAbIGRFneg+AglRinnerInvMass;
            aeAglISSnegDenomSel.back() = AglSel+aeAglISSnegDenomSel.back()+isAbIGRFneg;

            aeNaFISSposNumSel.back()   = NaFSel+aeNaFISSposNumSel.back()+isAbIGRFpos+NaFRinnerInvMass;
            aeNaFISSposDenomSel.back() = NaFSel+aeNaFISSposDenomSel.back()+isAbIGRFpos;
            aeNaFISSnegNumSel.back()   = NaFSel+aeNaFISSnegNumSel.back()+isAbIGRFneg+NaFRinnerInvMass;
            aeNaFISSnegDenomSel.back() = NaFSel+aeNaFISSnegDenomSel.back()+isAbIGRFneg;
        }
        //ae
        if(useCutOff){
            aeAglISSposNumSel.back()   = AglSel+aeAglISSposNumSel.back()+IGRFpos+AglRinnerInvMass;
            aeAglISSposDenomSel.back() = AglSel+aeAglISSposDenomSel.back()+IGRFpos;
            aeAglISSnegNumSel.back()   = AglSel+aeAglISSnegNumSel.back()+IGRFneg+AglRinnerInvMass;
            aeAglISSnegDenomSel.back() = AglSel+aeAglISSnegDenomSel.back()+IGRFneg;

            aeNaFISSposNumSel.back()   = NaFSel+aeNaFISSposNumSel.back()+IGRFpos+NaFRinnerInvMass;
            aeNaFISSposDenomSel.back() = NaFSel+aeNaFISSposDenomSel.back()+IGRFpos;
            aeNaFISSnegNumSel.back()   = NaFSel+aeNaFISSnegNumSel.back()+IGRFneg+NaFRinnerInvMass;
            aeNaFISSnegDenomSel.back() = NaFSel+aeNaFISSnegDenomSel.back()+IGRFneg;
        }

        //ae
        if(!useCutOff && !useSafetyFactor) {
            aeAglISSposNumSel.back()   = AglSel+aeAglISSposNumSel.back()+AglRinnerInvMass;
            aeAglISSposDenomSel.back() = AglSel+aeAglISSposDenomSel.back();
            aeAglISSnegNumSel.back()   = AglSel+aeAglISSnegNumSel.back()+AglRinnerInvMass;
            aeAglISSnegDenomSel.back() = AglSel+aeAglISSnegDenomSel.back();

            aeNaFISSposNumSel.back()   = NaFSel+aeNaFISSposNumSel.back()+NaFRinnerInvMass;
            aeNaFISSposDenomSel.back() = NaFSel+aeNaFISSposDenomSel.back();
            aeNaFISSnegNumSel.back()   = NaFSel+aeNaFISSnegNumSel.back()+NaFRinnerInvMass;
            aeNaFISSnegDenomSel.back() = NaFSel+aeNaFISSnegDenomSel.back();
        }
    }
    for(int i=0; i<clCutValues.size(); ++i){
        //classifier
        clAglISSposNumSel.push_back("&& scores_pos>"+to_string(clCutValues.at(i)));
        clAglISSposDenomSel.push_back("&& scores_pos>"+to_string(clCutValues.at(i)));
        clAglISSnegNumSel.push_back("&& scores_neg>"+to_string(clCutValues.at(i)));
        clAglISSnegDenomSel.push_back("&& scores_neg>"+to_string(clCutValues.at(i)));
        clAglMCposNumSel.push_back("&& scores_pos>"+to_string(clCutValues.at(i)));
        clAglMCposDenomSel.push_back("&& scores_pos>"+to_string(clCutValues.at(i)));
        clAglMCnegNumSel.push_back("&& scores_neg>"+to_string(clCutValues.at(i)));
        clAglMCnegDenomSel.push_back("&& scores_neg>"+to_string(clCutValues.at(i)));

        clNaFISSposNumSel.push_back("&& scores_pos>"+to_string(clCutValues.at(i)));
        clNaFISSposDenomSel.push_back("&& scores_pos>"+to_string(clCutValues.at(i)));
        clNaFISSnegNumSel.push_back("&& scores_neg>"+to_string(clCutValues.at(i)));
        clNaFISSnegDenomSel.push_back("&& scores_neg>"+to_string(clCutValues.at(i)));
        clNaFMCposNumSel.push_back("&& scores_pos>"+to_string(clCutValues.at(i)));
        clNaFMCposDenomSel.push_back("&& scores_pos>"+to_string(clCutValues.at(i)));
        clNaFMCnegNumSel.push_back("&& scores_neg>"+to_string(clCutValues.at(i)));
        clNaFMCnegDenomSel.push_back("&& scores_neg>"+to_string(clCutValues.at(i)));
        
        if(useSafetyFactor) {
            //classifier
            clAglISSposNumSel.back()   = AglSel+clAglISSposNumSel.back()+isAbIGRFpos+AglRinnerInvMass;
            clAglISSposDenomSel.back() = AglSel+clAglISSposDenomSel.back()+isAbIGRFpos;
            clAglISSnegNumSel.back()   = AglSel+clAglISSnegNumSel.back()+isAbIGRFneg+AglRinnerInvMass;
            clAglISSnegDenomSel.back() = AglSel+clAglISSnegDenomSel.back()+isAbIGRFneg;

            clNaFISSposNumSel.back()   = NaFSel+clNaFISSposNumSel.back()+isAbIGRFpos+NaFRinnerInvMass;
            clNaFISSposDenomSel.back() = NaFSel+clNaFISSposDenomSel.back()+isAbIGRFpos;
            clNaFISSnegNumSel.back()   = NaFSel+clNaFISSnegNumSel.back()+isAbIGRFneg+NaFRinnerInvMass;
            clNaFISSnegDenomSel.back() = NaFSel+clNaFISSnegDenomSel.back()+isAbIGRFneg;
        }
        if(useCutOff){
            //classifier
            clAglISSposNumSel.back()   = AglSel+clAglISSposNumSel.back()+IGRFpos+AglRinnerInvMass;
            clAglISSposDenomSel.back() = AglSel+clAglISSposDenomSel.back()+IGRFpos;
            clAglISSnegNumSel.back()   = AglSel+clAglISSnegNumSel.back()+IGRFneg+AglRinnerInvMass;
            clAglISSnegDenomSel.back() = AglSel+clAglISSnegDenomSel.back()+IGRFneg;

            clNaFISSposNumSel.back()   = NaFSel+clNaFISSposNumSel.back()+IGRFpos+NaFRinnerInvMass;
            clNaFISSposDenomSel.back() = NaFSel+clNaFISSposDenomSel.back()+IGRFpos;
            clNaFISSnegNumSel.back()   = NaFSel+clNaFISSnegNumSel.back()+IGRFneg+NaFRinnerInvMass;
            clNaFISSnegDenomSel.back() = NaFSel+clNaFISSnegDenomSel.back()+IGRFneg;
        }
        if(!useCutOff && !useSafetyFactor) {
            //classifier
            clAglISSposNumSel.back()   = AglSel+clAglISSposNumSel.back()+AglRinnerInvMass;
            clAglISSposDenomSel.back() = AglSel+clAglISSposDenomSel.back();
            clAglISSnegNumSel.back()   = AglSel+clAglISSnegNumSel.back()+AglRinnerInvMass;
            clAglISSnegDenomSel.back() = AglSel+clAglISSnegDenomSel.back();

            clNaFISSposNumSel.back()   = NaFSel+clNaFISSposNumSel.back()+NaFRinnerInvMass;
            clNaFISSposDenomSel.back() = NaFSel+clNaFISSposDenomSel.back();
            clNaFISSnegNumSel.back()   = NaFSel+clNaFISSnegNumSel.back()+NaFRinnerInvMass;
            clNaFISSnegDenomSel.back() = NaFSel+clNaFISSnegDenomSel.back();
        }

        clAglMCposNumSel.back()   = AglSel+clAglMCposNumSel.back()+AglRinnerInvMass;
        clAglMCposDenomSel.back() = AglSel+clAglMCposDenomSel.back();
        clAglMCnegNumSel.back()   = AglSel+clAglMCnegNumSel.back()+AglRinnerInvMass;
        clAglMCnegDenomSel.back() = AglSel+clAglMCnegDenomSel.back();

        clNaFMCposNumSel.back()   = NaFSel+clNaFMCposNumSel.back()+NaFRinnerInvMass;
        clNaFMCposDenomSel.back() = NaFSel+clNaFMCposDenomSel.back();
        clNaFMCnegNumSel.back()   = NaFSel+clNaFMCnegNumSel.back()+NaFRinnerInvMass;
        clNaFMCnegDenomSel.back() = NaFSel+clNaFMCnegDenomSel.back();
    }
    for(int i=0; i<aeCutValues.size(); ++i){
        for(int j=0; j<clCutValues.size(); ++j){
            combAglISSposNumSel.push_back("&& ISS.anomaly_score<"+to_string(aeCutValues.at(i))+" && scores_pos>"+to_string(clCutValues.at(j)));
            combAglISSposDenomSel.push_back("&& ISS.anomaly_score<"+to_string(aeCutValues.at(i))+" && scores_pos>"+to_string(clCutValues.at(j)));
            combAglISSnegNumSel.push_back("&& ISSneg.NEGanomaly_score<"+to_string(aeCutValues.at(i))+" && scores_neg>"+to_string(clCutValues.at(j)));
            combAglISSnegDenomSel.push_back("&& ISSneg.NEGanomaly_score<"+to_string(aeCutValues.at(i))+"&& scores_neg>"+to_string(clCutValues.at(j)));

            combNaFISSposNumSel.push_back("&& ISS.anomaly_score<"+to_string(aeCutValues.at(i))+" && scores_pos>"+to_string(clCutValues.at(j)));
            combNaFISSposDenomSel.push_back("&& ISS.anomaly_score<"+to_string(aeCutValues.at(i))+" && scores_pos>"+to_string(clCutValues.at(j)));
            combNaFISSnegNumSel.push_back("&& ISSneg.NEGanomaly_score<"+to_string(aeCutValues.at(i))+" && scores_neg>"+to_string(clCutValues.at(j)));
            combNaFISSnegDenomSel.push_back("&& ISSneg.NEGanomaly_score<"+to_string(aeCutValues.at(i))+" && scores_neg>"+to_string(clCutValues.at(j)));

            if(useSafetyFactor) {
                combAglISSposNumSel.back()   = AglSel+combAglISSposNumSel.back()+isAbIGRFpos+AglRinnerInvMass;
                combAglISSposDenomSel.back() = AglSel+combAglISSposDenomSel.back()+isAbIGRFpos;
                combAglISSnegNumSel.back()   = AglSel+combAglISSnegNumSel.back()+isAbIGRFneg+AglRinnerInvMass;
                combAglISSnegDenomSel.back() = AglSel+combAglISSnegDenomSel.back()+isAbIGRFneg;

                combNaFISSposNumSel.back()   = NaFSel+combNaFISSposNumSel.back()+isAbIGRFpos+NaFRinnerInvMass;
                combNaFISSposDenomSel.back() = NaFSel+combNaFISSposDenomSel.back()+isAbIGRFpos;
                combNaFISSnegNumSel.back()   = NaFSel+combNaFISSnegNumSel.back()+isAbIGRFneg+NaFRinnerInvMass;
                combNaFISSnegDenomSel.back() = NaFSel+combNaFISSnegDenomSel.back()+isAbIGRFneg;
            }
            else if(useCutOff){
                combAglISSposNumSel.back()   = AglSel+combAglISSposNumSel.back()+IGRFpos+AglRinnerInvMass;
                combAglISSposDenomSel.back() = AglSel+combAglISSposDenomSel.back()+IGRFpos;
                combAglISSnegNumSel.back()   = AglSel+combAglISSnegNumSel.back()+IGRFneg+AglRinnerInvMass;
                combAglISSnegDenomSel.back() = AglSel+combAglISSnegDenomSel.back()+IGRFneg;

                combNaFISSposNumSel.back()   = NaFSel+combNaFISSposNumSel.back()+IGRFpos+NaFRinnerInvMass;
                combNaFISSposDenomSel.back() = NaFSel+combNaFISSposDenomSel.back()+IGRFpos;
                combNaFISSnegNumSel.back()   = NaFSel+combNaFISSnegNumSel.back()+IGRFneg+NaFRinnerInvMass;
                combNaFISSnegDenomSel.back() = NaFSel+combNaFISSnegDenomSel.back()+IGRFneg;
            }
            else {
                combAglISSposNumSel.back()   = AglSel+combAglISSposNumSel.back()+AglRinnerInvMass;
                combAglISSposDenomSel.back() = AglSel+combAglISSposDenomSel.back();
                combAglISSnegNumSel.back()   = AglSel+combAglISSnegNumSel.back()+AglRinnerInvMass;
                combAglISSnegDenomSel.back() = AglSel+combAglISSnegDenomSel.back();

                combNaFISSposNumSel.back()   = NaFSel+combNaFISSposNumSel.back()+NaFRinnerInvMass;
                combNaFISSposDenomSel.back() = NaFSel+combNaFISSposDenomSel.back();
                combNaFISSnegNumSel.back()   = NaFSel+combNaFISSnegNumSel.back()+NaFRinnerInvMass;
                combNaFISSnegDenomSel.back() = NaFSel+combNaFISSnegDenomSel.back();
            }
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
    vector<TString> drawclR,       drawclRMC,       drawaeR,       drawaeRMC; 
    vector<TString> drawclRCS,     drawclRCSMC,     drawaeRAS,     drawaeRASMC;
    vector<TString> drawclRinvMass, drawCSinvMass,  drawASinvMass;

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
    //CLtrees->[ISS_pos, ISS_neg, MC_pos, MC_neg]
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
            if(i_file==0 && i_tree<2){ //CL ISS data
                CLtrees.at(i_tree) = (TTree*)files.at(i_file)->Get(DirName.at(i_tree).Data());
                if(!CLtrees.at(i_file)) {
                    printf("%s tree not found in the file %s\n", DirName.at(i_tree).Data(), PathToFile.at(i_file).Data());
                    return;
                }
            } // end CL ISS data
            if(i_file==1 && i_tree>=2){ //MC
                CLtrees.at(i_tree) = (TTree*)files.at(i_file)->Get(DirName.at(i_tree).Data());
                if(!CLtrees.at(i_file)) {
                    printf("%s tree not found in the file %s\n", DirName.at(i_tree).Data(), PathToFile.at(i_file).Data());
                    return;
                }
            } // end CL MC
        } //end loop on CLtrees

        if(i_file==2){ // AE ISS data
            AEtrees.at(0) = (TTree*)files.at(i_file)->Get("ISS");
            AEtrees.at(1) = (TTree*)files.at(i_file)->Get("ISSneg");
            if(!AEtrees.at(0) || !AEtrees.at(1)){
                printf("AE trees not found in the file %s\n", PathToFile.at(i_file).Data());
                return;
            }
            for(int i=0; i<2; ++i) {
                AEtrees.at(i)->BuildIndex("EvRun", "EvNum");
                CLtrees.at(i)->AddFriend(AEtrees.at(i));
            }
        } //end AE ISS data
        if(i_file==3){ // AE MC
            AEtrees.at(2) = (TTree*)files.at(i_file)->Get("MC");
            AEtrees.at(3) = (TTree*)files.at(i_file)->Get("MCneg");
            if(!AEtrees.at(2) || !AEtrees.at(3)){
                printf("AE trees not found in the file %s\n", PathToFile.at(i_file).Data());
                return;
            }
            for(int i=2; i<4; ++i) {
                AEtrees.at(i)->BuildIndex("EvRun", "EvNum");
                CLtrees.at(i)->AddFriend(AEtrees.at(i));
            }
        } //end AE MC
    } // end loop on files

    f_out->cd();
    //Initialize the histograms
    for(int i_tree=0; i_tree<2; i_tree++){ //loop on CLtrees
        TString ncl,    nclMC,  nae,  naeMC;
        TString tcl,    tclMC,  tae,  taeMC;
        TString nRCS,   nRCSMC, nRAS, nRASMC;
        TString tRCS,   tRCSMC, tRAS, tRASMC;
        //NaF
        TString nCSinvMassNaF,  nCSinvMassNaFMC,  nASinvMassNaF,  nASinvMassNaFMC;
        TString tCSinvMassNaF,  tCSinvMassNaFMC,  tASinvMassNaF,  tASinvMassNaFMC;
        TString nclRinvMassNaF, nclRinvMassNaFMC, naeRinvMassNaF, naeRinvMassNaFMC;
        TString tclRinvMassNaF, tclRinvMassNaFMC, taeRinvMassNaF, taeRinvMassNaFMC;
        //AGL
        TString nCSinvMassAgl, nCSinvMassAglMC,   nASinvMassAgl,  nASinvMassAglMC;
        TString tCSinvMassAgl, tCSinvMassAglMC,   tASinvMassAgl,  tASinvMassAglMC;
        TString nclRinvMassAgl, nclRinvMassAglMC, naeRinvMassAgl, naeRinvMassAglMC;
        TString tclRinvMassAgl, tclRinvMassAglMC, taeRinvMassAgl, taeRinvMassAglMC;
    
        //Event counts in mass window AGL
        H1_CSinvMassWAgl.push_back( new TH1D(nCSinvMassWAglISS.at(i_tree).Data(), tCSinvMassW.Data(), nbinsScore, minCS, maxCS));
        H1_ASinvMassWAgl.push_back(new TH1D(nASinvMassWAglISS.at(i_tree).Data(), tASinvMassW.Data(), nbinsAS, minAS, maxAS));
        H1_MC_CSinvMassWAgl.push_back( new TH1D(nCSinvMassWAglMC.at(i_tree).Data(), tCSinvMassW.Data(), nbinsScore, minCS, maxCS));
        H2_combinvMassWAgl.push_back( new TH2D(ncombinvMassWAglISS.at(i_tree).Data(), tcombinvMassW.Data(), nbinsAS, minAS, maxAS, nbinsScore, minCS, maxCS));
        H2_combinvMassWAglN.push_back( new TH2D(ncombinvMassWAglISSN.at(i_tree).Data(), tcombinvMassWN.Data(), nbinsAS, minAS, maxAS, nbinsScore, minCS, maxCS));
        //NaF
        H1_CSinvMassWNaF.push_back( new TH1D(nCSinvMassWNaFISS.at(i_tree).Data(), tCSinvMassW.Data(), nbinsScore, minCS, maxCS));
        H1_ASinvMassWNaF.push_back( new TH1D(nASinvMassWNaFISS.at(i_tree).Data(), tASinvMassW.Data(), nbinsAS, minAS, maxAS));
        H1_MC_CSinvMassWNaF.push_back( new TH1D(nCSinvMassWNaFMC.at(i_tree).Data(), tCSinvMassW.Data(), nbinsScore, minCS, maxCS));
        H2_combinvMassWNaF.push_back( new TH2D(ncombinvMassWNaFISS.at(i_tree).Data(), tcombinvMassW.Data(), nbinsAS, minAS, maxAS, nbinsScore, minCS, maxCS));
        H2_combinvMassWNaFN.push_back( new TH2D(ncombinvMassWNaFISSN.at(i_tree).Data(), tcombinvMassWN.Data(), nbinsAS, minAS, maxAS, nbinsScore, minCS, maxCS));

        for(int j_rig=0; j_rig<TotRig; j_rig++){ //loop on rig
            if(i_tree%2==0) { // Only positives
                ncl   = "CL_"+NameR.at(j_rig)+"Pos";
                nae   = "AE_"+NameR.at(j_rig)+"Pos";
                nclMC = "CL_"+NameR.at(j_rig)+"MCPos";
                nae   = "AE_"+NameR.at(j_rig)+"MCPos";

                RSel.push_back(NameR.at(j_rig)+">0");
                if(useSafetyFactor) RSel.back() = RSel.back()+isAbIGRFpos;
                if(useCutOff) RSel.back() = RSel.back()+IGRFpos;

                nRCS    = "CL_"+NameR.at(j_rig)+"CSPos";
                nRAS    = "AE_"+NameR.at(j_rig)+"ASPos";
                nRCSMC  = "CL_"+NameR.at(j_rig)+"CSMCPos";
                nRASMC  = "AE_"+NameR.at(j_rig)+"ASMCPos";
                
                drawclRCS.push_back("scores_pos:log10(abs("+NameR.at(j_rig)+"))>>+"+nRCS);
                drawaeRAS.push_back("ISS.anomaly_score:log10(abs("+NameR.at(j_rig)+"))>>+"+nRAS);
                drawclRCSMC.push_back("scores_pos:log10(abs("+NameR.at(j_rig)+"))>>+"+nRCSMC);
                drawaeRASMC.push_back("anomaly_score:log10(abs("+NameR.at(j_rig)+"))>>+"+nRASMC);
                
                nCSinvMassNaF = "CL_CSInvMass_NaF_"+NameR.at(j_rig)+"Pos";
                nASinvMassNaF = "AE_ASInvMass_NaF_"+NameR.at(j_rig)+"Pos";
                nCSinvMassNaFMC = "CL_CSInvMass_NaF_"+NameR.at(j_rig)+"MCPos";
                nASinvMassNaFMC = "AE_ASInvMass_NaF_"+NameR.at(j_rig)+"MCPos";

                nclRinvMassNaF  = "CL_RinvMass_NaF_"+NameR.at(j_rig)+"Pos";
                naeRinvMassNaF  = "AE_RinvMass_NaF_"+NameR.at(j_rig)+"Pos";
                nclRinvMassNaFMC  = "CL_RinvMass_NaF_"+NameR.at(j_rig)+"MCPos";
                naeRinvMassNaFMC  = "AE_RinvMass_NaF_"+NameR.at(j_rig)+"MCPos";

                nCSinvMassAgl = "CL_CSInvMass_Agl_"+NameR.at(j_rig)+"Pos";
                nASinvMassAgl = "AE_ASInvMass_Agl_"+NameR.at(j_rig)+"Pos";
                nCSinvMassAglMC = "CL_CSInvMass_Agl_"+NameR.at(j_rig)+"MCPos";
                nASinvMassAglMC = "AE_ASInvMass_Agl_"+NameR.at(j_rig)+"MCPos";

                nclRinvMassAgl  = "CL_RinvMass_Agl_"+NameR.at(j_rig)+"Pos";
                naeRinvMassAgl  = "AE_RinvMass_Agl_"+NameR.at(j_rig)+"Pos";
                nclRinvMassAglMC  = "CL_RinvMass_Agl_"+NameR.at(j_rig)+"MCPos";
                naeRinvMassAglMC  = "AE_RinvMass_Agl_"+NameR.at(j_rig)+"MCPos";

                drawCSinvMass.push_back("scores_pos:1./((2.*abs("+NameR.at(j_rig)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))");
                drawASinvMass.push_back("anomaly_score:1./((2.*abs("+NameR.at(j_rig)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))");
            }
            else {
                ncl = "CL_"+NameR.at(j_rig)+"Neg";
                nae = "AE_"+NameR.at(j_rig)+"Neg";
                nclMC = "CL_"+NameR.at(j_rig)+"MCNeg";
                naeMC = "AE_"+NameR.at(j_rig)+"MCNeg";

                RSel.push_back(NameR.at(j_rig)+"<0");
                if(useSafetyFactor) RSel.back() = RSel.back()+isAbIGRFneg;
                if(useCutOff) RSel.back() = RSel.back()+isAbIGRFneg;

                nRCS = "CL_"+NameR.at(j_rig)+"CSNeg";
                nRAS = "AE_"+NameR.at(j_rig)+"ASNeg";
                nRCSMC = "CL_"+NameR.at(j_rig)+"CSMCNeg";
                nRASMC = "AE_"+NameR.at(j_rig)+"ASMCNeg";

                drawclRCS.push_back("scores_neg:log10(abs("+NameR.at(j_rig)+"))>>+"+nRCS);
                drawaeRAS.push_back("ISSneg.NEGanomaly_score:log10(abs("+NameR.at(j_rig)+"))>>+"+nRAS);
                drawclRCSMC.push_back("scores_neg:log10(abs("+NameR.at(j_rig)+"))>>+"+nRCSMC);
                drawaeRASMC.push_back("NEGanomaly_score:log10(abs("+NameR.at(j_rig)+"))>>+"+nRASMC);

                nCSinvMassNaF = "CL_CSInvMass_NaF_"+NameR.at(j_rig)+"Neg";
                nASinvMassNaF = "AE_ASInvMass_NaF_"+NameR.at(j_rig)+"Neg";
                nCSinvMassNaFMC = "CL_CSInvMass_NaF_"+NameR.at(j_rig)+"MCNeg";
                nASinvMassNaFMC = "AE_ASInvMass_NaF_"+NameR.at(j_rig)+"MCNeg";

                nclRinvMassNaF  = "CL_RinvMass_NaF_"+NameR.at(j_rig)+"Neg";
                naeRinvMassNaF  = "AE_RinvMass_NaF_"+NameR.at(j_rig)+"Neg";
                nclRinvMassNaFMC  = "CL_RinvMass_NaF_"+NameR.at(j_rig)+"MCNeg";
                naeRinvMassNaFMC  = "AE_RinvMass_NaF_"+NameR.at(j_rig)+"MCNeg";

                nCSinvMassAgl = "CL_CSInvMass_Agl_"+NameR.at(j_rig)+"Neg";
                nASinvMassAgl = "AE_ASInvMass_Agl_"+NameR.at(j_rig)+"Neg";
                nCSinvMassAglMC = "CL_CSInvMass_Agl_"+NameR.at(j_rig)+"MCNeg";
                nASinvMassAglMC = "AE_ASInvMass_Agl_"+NameR.at(j_rig)+"MCNeg";

                nclRinvMassAgl  = "CL_RinvMass_Agl_"+NameR.at(j_rig)+"Neg";
                naeRinvMassAgl  = "AE_RinvMass_Agl_"+NameR.at(j_rig)+"Neg";
                nclRinvMassAglMC  = "CL_RinvMass_Agl_"+NameR.at(j_rig)+"MCNeg";
                naeRinvMassAglMC  = "AE_RinvMass_Agl_"+NameR.at(j_rig)+"MCNeg";

                drawCSinvMass.push_back("scores_neg:1./((2.*abs("+NameR.at(j_rig)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))");
                drawASinvMass.push_back("NEGanomaly_score:1./((2.*abs("+NameR.at(j_rig)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))");
            }
            
            //Rigidities
            drawclR.push_back("log10(abs("+NameR.at(j_rig)+"))>>+"+ncl);
            drawaeR.push_back("log10(abs("+NameR.at(j_rig)+"))>>+"+nae);
            drawclRMC.push_back("log10(abs("+NameR.at(j_rig)+"))>>+"+nclMC);
            drawaeRMC.push_back("log10(abs("+NameR.at(j_rig)+"))>>+"+naeMC);

            tcl = ncl+"; [log_{10}(GV)]; Events";
            tae = nae+"; [log_{10}(GV)]; Events";
            tclMC = nclMC+"; [log_{10}(GV)]; Events";
            taeMC = naeMC+"; [log_{10}(GV)]; Events";

            H1cl_R.push_back(new TH1D(ncl.Data(),tcl.Data(), nbinsRin, minRin, maxRin));
            H1ae_R.push_back(new TH1D(nae.Data(),tae.Data(), nbinsRin, minRin, maxRin));
            H1cl_MC_R.push_back(new TH1D(nclMC.Data(),tclMC.Data(), nbinsRin, minRin, maxRin));
            H1ae_MC_R.push_back(new TH1D(naeMC.Data(),taeMC.Data(), nbinsRin, minRin, maxRin));

            H1cl_R.back()->GetXaxis()->SetTitleOffset(1.2);    H1ae_R.back()->GetXaxis()->SetTitleOffset(1.2);
            H1cl_MC_R.back()->GetXaxis()->SetTitleOffset(1.2); H1ae_MC_R.back()->GetXaxis()->SetTitleOffset(1.2);

                //R vs scores
            if(j_rig==1) RCSSel.push_back(RichSel+InL1Sel.at(i_tree));
            else RCSSel.push_back(RichSel);
            if(useSafetyFactor && i_tree%2==0) RCSSel.back() = RCSSel.back()+isAbIGRFpos;
            if(useCutOff && i_tree%2==0) RCSSel.back() = RCSSel.back()+isAbIGRFpos;
            if(useSafetyFactor && i_tree%2==1) RCSSel.back() = RCSSel.back()+isAbIGRFneg;
            if(useCutOff && i_tree%2==1) RCSSel.back() = RCSSel.back()+isAbIGRFneg;

            tRCS = nRCS+"; [log_{10}(GV)]; Classifier scores";
            tRAS = nRAS+"; [log_{10}(GV)]; Anomaly scores";
            tRCSMC = nRCSMC+"; [log_{10}(GV)]; Classifier scores";
            tRASMC = nRASMC+"; [log_{10}(GV)]; Anomaly scores";

            H2_RCS.push_back(new TH2D(nRCS.Data(), tRCS.Data(), nbinsRin, minRin, maxRin, nbinsScore, minCS, maxCS));
            H2_RAS.push_back(new TH2D(nRAS.Data(), tRAS.Data(), nbinsRin, minRin, maxRin, nbinsScore, minAS, maxAS));
            H2_MC_RCS.push_back(new TH2D(nRCSMC.Data(), tRCSMC.Data(), nbinsRin, minRin, maxRin, nbinsScore, minCS, maxCS));
            H2_MC_RAS.push_back(new TH2D(nRASMC.Data(), tRASMC.Data(), nbinsRin, minRin, maxRin, nbinsScore, minAS, maxAS));
            H2_RCS.back()->GetXaxis()->SetTitleOffset(1.2);    H2_RAS.back()->GetXaxis()->SetTitleOffset(1.2);
            H2_MC_RCS.back()->GetXaxis()->SetTitleOffset(1.2); H2_MC_RAS.back()->GetXaxis()->SetTitleOffset(1.2);

            //NaF
                //CS vs Inv. Mass (NaF)
            if(j_rig==1) invMassNaFSel.push_back(NaFSel+InL1Sel.at(i_tree));
            else invMassNaFSel.push_back(NaFSel);
            if(useSafetyFactor && i_tree%2==0) invMassNaFSel.back() = invMassNaFSel.back()+isAbIGRFpos;
            if(useCutOff && i_tree%2==0) invMassNaFSel.back() = invMassNaFSel.back()+isAbIGRFpos;
            if(useSafetyFactor && i_tree%2==1) invMassNaFSel.back() = invMassNaFSel.back()+isAbIGRFneg;
            if(useCutOff && i_tree%2==1) invMassNaFSel.back() = invMassNaFSel.back()+isAbIGRFneg;

            tCSinvMassNaF = nCSinvMassNaF+"; RICH NaF #frac{1}{m} [1/a.m.u.]; Classifier scores";
            tASinvMassNaF = nASinvMassNaF+"; RICH NaF #frac{1}{m} [1/a.m.u.]; Anomaly scores";
            tCSinvMassNaFMC = nCSinvMassNaFMC+"; RICH NaF #frac{1}{m} [1/a.m.u.]; Classifier scores";
            tASinvMassNaFMC = nASinvMassNaFMC+"; RICH NaF #frac{1}{m} [1/a.m.u.]; Anomaly scores";

            H2_CSinvMassNaF.push_back(new TH2D(nCSinvMassNaF.Data(), tCSinvMassNaF.Data(), nbinsMass, mass_MIN, mass_MAX, nbinsScore, minCS, maxCS));
            H2_ASinvMassNaF.push_back(new TH2D(nASinvMassNaF.Data(), tASinvMassNaF.Data(), nbinsMass, mass_MIN, mass_MAX, nbinsScore, minAS, maxAS));
            H2_MC_CSinvMassNaF.push_back(new TH2D(nCSinvMassNaFMC.Data(), tCSinvMassNaFMC.Data(), nbinsMass, mass_MIN, mass_MAX, nbinsScore, minCS, maxCS));
            H2_MC_ASinvMassNaF.push_back(new TH2D(nASinvMassNaFMC.Data(), tASinvMassNaFMC.Data(), nbinsMass, mass_MIN, mass_MAX, nbinsScore, minAS, maxAS));
            H2_CSinvMassNaF.back()->GetXaxis()->SetTitleOffset(1.2);    H2_ASinvMassNaF.back()->GetXaxis()->SetTitleOffset(1.2);
            H2_MC_CSinvMassNaF.back()->GetXaxis()->SetTitleOffset(1.2); H2_MC_ASinvMassNaF.back()->GetXaxis()->SetTitleOffset(1.2);

                //R vs Inv. Mass (NaF)
            drawclRinvMass.push_back("1./((2.*abs("+NameR.at(j_rig)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315)):log10(abs("+NameR.at(j_rig)+"))");
            tclRinvMassNaF = nclRinvMassNaF+" ; log_{10}(|R|) [log_{10}(GV)] ; RICH NaF #frac{1}{m} [1/a.m.u.] ";
            taeRinvMassNaF = naeRinvMassNaF+" ; log_{10}(|R|) [log_{10}(GV)] ; RICH NaF #frac{1}{m} [1/a.m.u.] ";
            tclRinvMassNaFMC = nclRinvMassNaFMC+" ; log_{10}(|R|) [log_{10}(GV)] ; RICH NaF #frac{1}{m} [1/a.m.u.] ";
            taeRinvMassNaFMC = naeRinvMassNaFMC+" ; log_{10}(|R|) [log_{10}(GV)] ; RICH NaF #frac{1}{m} [1/a.m.u.] ";
            H2cl_RinvMassNaF.push_back(new TH2D(nclRinvMassNaF.Data(), tclRinvMassNaF.Data(), nbinsRin, minRin, maxRin, nbinsMass, mass_MIN, mass_MAX));
            H2ae_RinvMassNaF.push_back(new TH2D(naeRinvMassNaF.Data(), taeRinvMassNaF.Data(), nbinsRin, minRin, maxRin, nbinsMass, mass_MIN, mass_MAX));
            H2cl_MC_RinvMassNaF.push_back(new TH2D(nclRinvMassNaFMC.Data(), tclRinvMassNaFMC.Data(), nbinsRin, minRin, maxRin, nbinsMass, mass_MIN, mass_MAX));
            H2ae_MC_RinvMassNaF.push_back(new TH2D(naeRinvMassNaFMC.Data(), taeRinvMassNaFMC.Data(), nbinsRin, minRin, maxRin, nbinsMass, mass_MIN, mass_MAX));
            H2cl_RinvMassNaF.back()->GetXaxis()->SetTitleOffset(1.2);    H2ae_RinvMassNaF.back()->GetXaxis()->SetTitleOffset(1.2);
            H2cl_MC_RinvMassNaF.back()->GetXaxis()->SetTitleOffset(1.2); H2ae_MC_RinvMassNaF.back()->GetXaxis()->SetTitleOffset(1.2);

            //AGL
                //CS vs Inv. Mass (Agl)
            if(j_rig==1) invMassAglSel.push_back(AglSel+InL1Sel.at(i_tree));
            else invMassAglSel.push_back(AglSel);
            if(useSafetyFactor && i_tree%2==0) invMassAglSel.back() = invMassAglSel.back()+isAbIGRFpos;
            if(useCutOff && i_tree%2==0) invMassAglSel.back() = invMassAglSel.back()+isAbIGRFpos;
            if(useSafetyFactor && i_tree%2==1) invMassAglSel.back() = invMassAglSel.back()+isAbIGRFneg;
            if(useCutOff && i_tree%2==1) invMassAglSel.back() = invMassAglSel.back()+isAbIGRFneg;

            tCSinvMassAgl = nCSinvMassAgl+"; RICH Agl #frac{1}{m} [1/a.m.u.]; Classifier scores";
            tASinvMassAgl = nASinvMassAgl+"; RICH Agl #frac{1}{m} [1/a.m.u.]; Anomaly scores";
            tCSinvMassAglMC = nCSinvMassAglMC+"; RICH Agl #frac{1}{m} [1/a.m.u.]; Classifier scores";
            tASinvMassAglMC = nASinvMassAglMC+"; RICH Agl #frac{1}{m} [1/a.m.u.]; Anomaly scores";
            H2_CSinvMassAgl.push_back(new TH2D(nCSinvMassAgl.Data(), tCSinvMassAgl.Data(), nbinsMass, mass_MIN, mass_MAX, nbinsScore, minCS, maxCS));
            H2_ASinvMassAgl.push_back(new TH2D(nASinvMassAgl.Data(), tASinvMassAgl.Data(), nbinsMass, mass_MIN, mass_MAX, nbinsScore, minAS, maxAS));
            H2_MC_CSinvMassAgl.push_back(new TH2D(nCSinvMassAglMC.Data(), tCSinvMassAglMC.Data(), nbinsMass, mass_MIN, mass_MAX, nbinsScore, minCS, maxCS));
            H2_MC_ASinvMassAgl.push_back(new TH2D(nASinvMassAglMC.Data(), tASinvMassAglMC.Data(), nbinsMass, mass_MIN, mass_MAX, nbinsScore, minAS, maxAS));
            H2_CSinvMassAgl.back()->GetXaxis()->SetTitleOffset(1.2);    H2_ASinvMassAgl.back()->GetXaxis()->SetTitleOffset(1.2);
            H2_MC_CSinvMassAgl.back()->GetXaxis()->SetTitleOffset(1.2); H2_MC_ASinvMassAgl.back()->GetXaxis()->SetTitleOffset(1.2);
                //R vs Inv. Mass (Agl)
            tclRinvMassAgl = nclRinvMassAgl+" ; log_{10}(|R|) [log_{10}(GV)] ; RICH Agl #frac{1}{m} [1/a.m.u.] ";
            taeRinvMassAgl = naeRinvMassAgl+" ; log_{10}(|R|) [log_{10}(GV)] ; RICH Agl #frac{1}{m} [1/a.m.u.] ";
            tclRinvMassAglMC = nclRinvMassAglMC+" ; log_{10}(|R|) [log_{10}(GV)] ; RICH Agl #frac{1}{m} [1/a.m.u.] ";
            taeRinvMassAglMC = naeRinvMassAglMC+" ; log_{10}(|R|) [log_{10}(GV)] ; RICH Agl #frac{1}{m} [1/a.m.u.] ";
            H2cl_RinvMassAgl.push_back(new TH2D(nclRinvMassAgl.Data(), tclRinvMassAgl.Data(), nbinsRin, minRin, maxRin, nbinsMass, mass_MIN, mass_MAX));
            H2ae_RinvMassAgl.push_back(new TH2D(naeRinvMassAgl.Data(), taeRinvMassAgl.Data(), nbinsRin, minRin, maxRin, nbinsMass, mass_MIN, mass_MAX));
            H2cl_MC_RinvMassAgl.push_back(new TH2D(nclRinvMassAglMC.Data(), tclRinvMassAglMC.Data(), nbinsRin, minRin, maxRin, nbinsMass, mass_MIN, mass_MAX));
            H2ae_MC_RinvMassAgl.push_back(new TH2D(naeRinvMassAglMC.Data(), taeRinvMassAglMC.Data(), nbinsRin, minRin, maxRin, nbinsMass, mass_MIN, mass_MAX));
            H2cl_RinvMassAgl.back()->GetXaxis()->SetTitleOffset(1.2);    H2ae_RinvMassAgl.back()->GetXaxis()->SetTitleOffset(1.2);
            H2cl_MC_RinvMassAgl.back()->GetXaxis()->SetTitleOffset(1.2); H2ae_MC_RinvMassAgl.back()->GetXaxis()->SetTitleOffset(1.2);
        } //end loop on rig
    } // end loop on CLtrees


    //Filling the histos
    //CLtrees->[ISS_pos, ISS_neg, MC_pos, MC_neg]
    TString mcSel;
    for(int i=0; i<H1cl_R.size(); i++){  // loop on histos
        f_out->cd();
        if(i<TotRig){ // POSITIVES +
            //Rigidities
            CLtrees.at(0)->Draw(drawclR.at(i).Data(),RSel.at(i).Data(),"goff");
            H1cl_R.at(i) = (LogToLin1D((TH1D*)gDirectory->Get(H1cl_R.at(i)->GetName())));
            CLtrees.at(0)->Draw(drawaeR.at(i).Data(),RSel.at(i).Data(),"goff");
            H1ae_R.at(i) = (LogToLin1D((TH1D*)gDirectory->Get(H1ae_R.at(i)->GetName())));

            mcSel = RSel.at(i);
            if(useSafetyFactor ) mcSel.ReplaceAll(isAbIGRFpos.Data(),"");
            if(useCutOff) mcSel.ReplaceAll(IGRFpos.Data(),"");
            mcSel.Prepend("("); mcSel.Append(")*weight");
            CLtrees.at(2)->Draw(drawclRMC.at(i).Data(),mcSel.Data(),"goff");
            H1cl_MC_R.at(i) = (LogToLin1D((TH1D*)gDirectory->Get(H1cl_MC_R.at(i)->GetName())));
            CLtrees.at(2)->Draw(drawaeRMC.at(i).Data(),mcSel.Data(),"goff");
            H1ae_MC_R.at(i) = (LogToLin1D((TH1D*)gDirectory->Get(H1ae_MC_R.at(i)->GetName())));
            
            f_out->cd(DirName.at(0).Data());
            H1cl_R.at(i)->Write();    H1ae_R.at(i)->Write();
            H1cl_MC_R.at(i)->Write(); H1ae_MC_R.at(i)->Write();
            f_out->cd();
                // R vs CS
            CLtrees.at(0)->Draw(drawclRCS.at(i).Data(), RCSSel.at(i).Data(), "goff");
            CLtrees.at(0)->Draw(drawaeRAS.at(i).Data(), RCSSel.at(i).Data(), "goff");
            mcSel = RCSSel.at(i);
            if(useSafetyFactor) mcSel.ReplaceAll(isAbIGRFpos.Data(),"");
            if(useCutOff) mcSel.ReplaceAll(IGRFpos.Data(),"");
            mcSel.Prepend("("); mcSel.Append(")*weight");
            CLtrees.at(2)->Draw(drawclRCSMC.at(i).Data(), mcSel.Data(), "goff");
            CLtrees.at(2)->Draw(drawaeRASMC.at(i).Data(), mcSel.Data(), "goff");

            //NaF
                //CS vs Inv. Mass
            to_draw = drawCSinvMass.at(i)+">>+"+H2_CSinvMassNaF.at(i)->GetName();
            CLtrees.at(0)->Draw(to_draw.Data(), invMassNaFSel.at(i).Data(), "goff");
            to_draw = drawASinvMass.at(i)+">>+"+H2_ASinvMassNaF.at(i)->GetName();
            CLtrees.at(0)->Draw(to_draw.Data(), invMassNaFSel.at(i).Data(), "goff");
            
            mcSel =  invMassNaFSel.at(i);
            if(useSafetyFactor) mcSel.ReplaceAll(isAbIGRFpos.Data(),"");
            if(useCutOff) mcSel.ReplaceAll(IGRFpos.Data(),"");
            mcSel.Prepend("("); mcSel.Append(")*weight");
            to_draw = drawCSinvMass.at(i)+">>+"+H2_MC_CSinvMassNaF.at(i)->GetName();
            CLtrees.at(2)->Draw(to_draw.Data(), mcSel.Data(), "goff");
            to_draw = drawASinvMass.at(i)+">>+"+H2_MC_ASinvMassNaF.at(i)->GetName();
            CLtrees.at(2)->Draw(to_draw.Data(), mcSel.Data(), "goff");
                //R vs Inv. Mass
            to_draw = drawclRinvMass.at(i)+">>+"+H2cl_RinvMassNaF.at(i)->GetName();
            CLtrees.at(0)->Draw(to_draw.Data(), invMassNaFSel.at(i).Data(), "goff");
            to_draw = drawclRinvMass.at(i)+">>+"+H2ae_RinvMassNaF.at(i)->GetName();
            CLtrees.at(0)->Draw(to_draw.Data(), invMassNaFSel.at(i).Data(), "goff");

            to_draw = drawclRinvMass.at(i)+">>+"+H2cl_MC_RinvMassNaF.at(i)->GetName();
            CLtrees.at(2)->Draw(to_draw.Data(), mcSel.Data(), "goff");
            to_draw = drawclRinvMass.at(i)+">>+"+H2ae_MC_RinvMassNaF.at(i)->GetName();
            CLtrees.at(2)->Draw(to_draw.Data(), mcSel.Data(), "goff");

            //Agl
                //CS vs Inv. Mass
            to_draw = drawCSinvMass.at(i)+">>+"+H2_CSinvMassAgl.at(i)->GetName();
            CLtrees.at(0)->Draw(to_draw.Data(), invMassAglSel.at(i).Data(), "goff");
            to_draw = drawASinvMass.at(i)+">>+"+H2_ASinvMassAgl.at(i)->GetName();
            CLtrees.at(0)->Draw(to_draw.Data(), invMassAglSel.at(i).Data(), "goff");
            
            mcSel = invMassAglSel.at(i);
            if(useSafetyFactor)mcSel.ReplaceAll(isAbIGRFpos.Data(),"");
            if(useCutOff)mcSel.ReplaceAll(IGRFpos.Data(),"");
            mcSel.Prepend("("); mcSel.Append(")*weight");
            to_draw = drawCSinvMass.at(i)+">>+"+H2_MC_CSinvMassAgl.at(i)->GetName();
            CLtrees.at(2)->Draw(to_draw.Data(), mcSel.Data(), "goff");
            to_draw = drawASinvMass.at(i)+">>+"+H2_MC_ASinvMassAgl.at(i)->GetName();
            CLtrees.at(2)->Draw(to_draw.Data(), mcSel.Data(), "goff");
                //R vs Inv. Mass
            to_draw = drawclRinvMass.at(i)+">>+"+H2cl_RinvMassAgl.at(i)->GetName();
            CLtrees.at(0)->Draw(to_draw.Data(), invMassAglSel.at(i).Data(), "goff");
            to_draw = drawclRinvMass.at(i)+">>+"+H2ae_RinvMassAgl.at(i)->GetName();
            CLtrees.at(0)->Draw(to_draw.Data(), invMassAglSel.at(i).Data(), "goff");
            to_draw = drawclRinvMass.at(i)+">>+"+H2cl_MC_RinvMassAgl.at(i)->GetName();
            CLtrees.at(2)->Draw(to_draw.Data(), mcSel.Data(), "goff");
            to_draw = drawclRinvMass.at(i)+">>+"+H2ae_MC_RinvMassAgl.at(i)->GetName();
            CLtrees.at(2)->Draw(to_draw.Data(), mcSel.Data(), "goff");
        }
        else{ // NEGATIVES -
            //Rigidities
            CLtrees.at(1)->Draw(drawclR.at(i).Data(),RSel.at(i).Data(),"goff");
            H1cl_R.at(i) = (LogToLin1D((TH1D*)gDirectory->Get(H1cl_R.at(i)->GetName())));
            CLtrees.at(1)->Draw(drawaeR.at(i).Data(),RSel.at(i).Data(),"goff");
            H1ae_R.at(i) = (LogToLin1D((TH1D*)gDirectory->Get(H1ae_R.at(i)->GetName())));

            mcSel =RSel.at(i);
            if(useSafetyFactor) mcSel.ReplaceAll(isAbIGRFneg.Data(),"");
            if(useCutOff) mcSel.ReplaceAll(IGRFneg.Data(),"");
            mcSel.Prepend("("); mcSel.Append(")*weight");
            CLtrees.at(3)->Draw(drawclRMC.at(i).Data(),mcSel.Data(),"goff");
            H1cl_MC_R.at(i) = (LogToLin1D((TH1D*)gDirectory->Get(H1cl_MC_R.at(i)->GetName())));
            CLtrees.at(3)->Draw(drawaeRMC.at(i).Data(),mcSel.Data(),"goff");
            H1ae_MC_R.at(i) = (LogToLin1D((TH1D*)gDirectory->Get(H1ae_MC_R.at(i)->GetName())));
           
            f_out->cd(DirName.at(1).Data());
            H1cl_R.at(i)->Write();    H1ae_R.at(i)->Write();
            H1cl_MC_R.at(i)->Write(); H1ae_MC_R.at(i)->Write();
            f_out->cd();
                // R vs CS
            CLtrees.at(1)->Draw(drawclRCS.at(i).Data(), RCSSel.at(i).Data(), "goff");
            CLtrees.at(1)->Draw(drawaeRAS.at(i).Data(), RCSSel.at(i).Data(), "goff");

            mcSel = RCSSel.at(i);
            if(useSafetyFactor) mcSel.ReplaceAll(isAbIGRFneg.Data(),"");
            if(useCutOff) mcSel.ReplaceAll(IGRFneg.Data(),"");
            mcSel.Prepend("("); mcSel.Append(")*weight");
            CLtrees.at(3)->Draw(drawclRCSMC.at(i).Data(), mcSel.Data(), "goff");
            CLtrees.at(3)->Draw(drawaeRASMC.at(i).Data(), mcSel.Data(), "goff");

            //NaF
                //CS vs Inv. Mass
            to_draw = drawCSinvMass.at(i)+">>+"+H2_CSinvMassNaF.at(i)->GetName();
            CLtrees.at(1)->Draw(to_draw.Data(), invMassNaFSel.at(i).Data(), "goff");
            to_draw = drawASinvMass.at(i)+">>+"+H2_ASinvMassNaF.at(i)->GetName();
            CLtrees.at(1)->Draw(to_draw.Data(), invMassNaFSel.at(i).Data(), "goff");

            mcSel = invMassNaFSel.at(i);
            if(useSafetyFactor) mcSel.ReplaceAll(isAbIGRFneg.Data(),"");
            if(useCutOff) mcSel.ReplaceAll(IGRFneg.Data(),"");
            mcSel.Prepend("("); mcSel.Append(")*weight");
            to_draw = drawCSinvMass.at(i)+">>+"+H2_MC_CSinvMassNaF.at(i)->GetName();
            CLtrees.at(3)->Draw(to_draw.Data(), mcSel.Data(), "goff");
            to_draw = drawASinvMass.at(i)+">>+"+H2_MC_ASinvMassNaF.at(i)->GetName();
            CLtrees.at(3)->Draw(to_draw.Data(), mcSel.Data(), "goff");
                //R vs Inv. Mass
            to_draw = drawclRinvMass.at(i)+">>+"+H2cl_RinvMassNaF.at(i)->GetName();
            CLtrees.at(1)->Draw(to_draw.Data(), invMassNaFSel.at(i).Data(), "goff");
            to_draw = drawclRinvMass.at(i)+">>+"+H2ae_RinvMassNaF.at(i)->GetName();
            CLtrees.at(1)->Draw(to_draw.Data(), invMassNaFSel.at(i).Data(), "goff");
            to_draw = drawclRinvMass.at(i)+">>+"+H2cl_MC_RinvMassNaF.at(i)->GetName();
            CLtrees.at(3)->Draw(to_draw.Data(), mcSel.Data(), "goff");
            to_draw = drawclRinvMass.at(i)+">>+"+H2ae_MC_RinvMassNaF.at(i)->GetName();
            CLtrees.at(3)->Draw(to_draw.Data(), mcSel.Data(), "goff");
            
            //Agl
                //CS vs Inv. Mass
            to_draw = drawCSinvMass.at(i)+">>+"+H2_CSinvMassAgl.at(i)->GetName();
            CLtrees.at(1)->Draw(to_draw.Data(), invMassAglSel.at(i).Data(), "goff");
            to_draw = drawASinvMass.at(i)+">>+"+H2_ASinvMassAgl.at(i)->GetName();
            CLtrees.at(1)->Draw(to_draw.Data(), invMassAglSel.at(i).Data(), "goff");

            mcSel = invMassAglSel.at(i);
            if(useSafetyFactor) mcSel.ReplaceAll(isAbIGRFneg.Data(),"");
            if(useCutOff) mcSel.ReplaceAll(IGRFneg.Data(),"");
            mcSel.Prepend("("); mcSel.Append(")*weight");
            to_draw = drawCSinvMass.at(i)+">>+"+H2_MC_CSinvMassAgl.at(i)->GetName();
            CLtrees.at(3)->Draw(to_draw.Data(), mcSel.Data(), "goff");
            to_draw = drawASinvMass.at(i)+">>+"+H2_MC_ASinvMassAgl.at(i)->GetName();
            CLtrees.at(3)->Draw(to_draw.Data(), mcSel.Data(), "goff");
                //R vs Inv. Mass
            to_draw = drawclRinvMass.at(i)+">>+"+H2cl_RinvMassAgl.at(i)->GetName();
            CLtrees.at(1)->Draw(to_draw.Data(), invMassAglSel.at(i).Data(), "goff");
            to_draw = drawclRinvMass.at(i)+">>+"+H2ae_RinvMassAgl.at(i)->GetName();
            CLtrees.at(1)->Draw(to_draw.Data(), invMassAglSel.at(i).Data(), "goff");
            to_draw = drawclRinvMass.at(i)+">>+"+H2cl_MC_RinvMassAgl.at(i)->GetName();
            CLtrees.at(3)->Draw(to_draw.Data(), mcSel.Data(), "goff");
            to_draw = drawclRinvMass.at(i)+">>+"+H2ae_MC_RinvMassAgl.at(i)->GetName();
            CLtrees.at(1)->Draw(to_draw.Data(), mcSel.Data(), "goff");
        }
        H2_RCS.at(i)            = (LogToLin2D((TH2D*)gDirectory->Get(H2_RCS.at(i)->GetName())));
        H2_RAS.at(i)            = (LogToLin2D((TH2D*)gDirectory->Get(H2_RAS.at(i)->GetName())));
        H2_MC_RCS.at(i)         = (LogToLin2D((TH2D*)gDirectory->Get(H2_MC_RCS.at(i)->GetName())));
        H2_MC_RAS.at(i)            = (LogToLin2D((TH2D*)gDirectory->Get(H2_MC_RAS.at(i)->GetName())));

        H2cl_RinvMassNaF.at(i)    = (LogToLin2D((TH2D*)gDirectory->Get(H2cl_RinvMassNaF.at(i)->GetName())));
        H2ae_RinvMassNaF.at(i)    = (LogToLin2D((TH2D*)gDirectory->Get(H2ae_RinvMassNaF.at(i)->GetName())));
        H2cl_MC_RinvMassNaF.at(i) = (LogToLin2D((TH2D*)gDirectory->Get(H2cl_MC_RinvMassNaF.at(i)->GetName())));
        H2ae_MC_RinvMassNaF.at(i)    = (LogToLin2D((TH2D*)gDirectory->Get(H2ae_MC_RinvMassNaF.at(i)->GetName())));

        H2_CSinvMassNaF.at(i)     = (TH2D*)gDirectory->Get(H2_CSinvMassNaF.at(i)->GetName());
        H2_ASinvMassNaF.at(i)     = (TH2D*)gDirectory->Get(H2_ASinvMassNaF.at(i)->GetName());
        H2cl_MC_RinvMassAgl.at(i) = (TH2D*)gDirectory->Get(H2cl_MC_RinvMassAgl.at(i)->GetName());
        H2_MC_ASinvMassNaF.at(i)     = (TH2D*)gDirectory->Get(H2_MC_ASinvMassNaF.at(i)->GetName());
        
        H2cl_RinvMassAgl.at(i)    = (LogToLin2D((TH2D*)gDirectory->Get(H2cl_RinvMassAgl.at(i)->GetName())));
        H2ae_RinvMassAgl.at(i)    = (LogToLin2D((TH2D*)gDirectory->Get(H2ae_RinvMassAgl.at(i)->GetName())));
        H2cl_MC_RinvMassAgl.at(i) = (LogToLin2D((TH2D*)gDirectory->Get(H2cl_MC_RinvMassAgl.at(i)->GetName())));
        H2ae_MC_RinvMassAgl.at(i) = (LogToLin2D((TH2D*)gDirectory->Get(H2ae_MC_RinvMassAgl.at(i)->GetName())));

        H2_CSinvMassAgl.at(i)    = (TH2D*)gDirectory->Get(H2_CSinvMassAgl.at(i)->GetName());
        H2_ASinvMassAgl.at(i)    = (TH2D*)gDirectory->Get(H2_ASinvMassAgl.at(i)->GetName());
        H2_MC_CSinvMassAgl.at(i) = (TH2D*)gDirectory->Get(H2_MC_CSinvMassAgl.at(i)->GetName());
        H2_MC_ASinvMassAgl.at(i) = (TH2D*)gDirectory->Get(H2_MC_ASinvMassAgl.at(i)->GetName());

        //CL2
        if(i<TotRig) { // ISS POSITIVES +
            TemName = DirName.at(0)+"/"+NameSub.at(0)+"/"+NameR.at(i%2);
            f_out->cd(TemName.Data());
        }
        else{ //ISS NEGATIVES -
            TemName = DirName.at(1)+"/"+NameSub.at(0)+"/"+NameR.at(i%2);
            f_out->cd(TemName.Data());
        }
        H2_RCS.at(i)->Write();
        H2_CSinvMassNaF.at(i)->Write();
        H2cl_RinvMassNaF.at(i)->Write();
        H2_CSinvMassAgl.at(i)->Write();
        H2cl_RinvMassAgl.at(i)->Write();

        //AE
        if(i<TotRig) { // ISS POSITIVES +
            TemName = DirName.at(0)+"/"+NameSub.at(1)+"/"+NameR.at(i%2);
            f_out->cd(TemName.Data());
        }
        else{ //ISS NEGATIVES -
            TemName = DirName.at(1)+"/"+NameSub.at(1)+"/"+NameR.at(i%2);
            f_out->cd(TemName.Data());
        }
        H2_RAS.at(i)->Write();
        H2_ASinvMassNaF.at(i)->Write();
        H2ae_RinvMassNaF.at(i)->Write();
        H2_ASinvMassAgl.at(i)->Write();
        H2ae_RinvMassAgl.at(i)->Write();

        //CL
        if(i<TotRig) { // MC POSITIVES +
            TemName = DirName.at(2)+"/"+NameSub.at(0)+"/"+NameR.at(i%2);
            f_out->cd(TemName.Data());
        }
        else{  // MC NEGATIVES -
            TemName = DirName.at(3)+"/"+NameSub.at(0)+"/"+NameR.at(i%2);
            f_out->cd(TemName.Data());
        }
        H2_MC_RCS.at(i)->Write();
        H2_MC_CSinvMassNaF.at(i)->Write();
        H2cl_MC_RinvMassNaF.at(i)->Write();
        H2_MC_CSinvMassAgl.at(i)->Write();
        H2cl_MC_RinvMassAgl.at(i)->Write();

        //AE
        if(i<TotRig) { // MC POSITIVES +
            TemName = DirName.at(2)+"/"+NameSub.at(1)+"/"+NameR.at(i%2);
            f_out->cd(TemName.Data());
        }
        else{  // MC NEGATIVES -
            TemName = DirName.at(3)+"/"+NameSub.at(1)+"/"+NameR.at(i%2);
            f_out->cd(TemName.Data());
        }
        H2_MC_RAS.at(i)->Write();
        H2_MC_ASinvMassNaF.at(i)->Write();
        H2ae_MC_RinvMassNaF.at(i)->Write();
        H2_MC_ASinvMassAgl.at(i)->Write();
        H2ae_MC_RinvMassAgl.at(i)->Write();
    } // end loop on histos

    f_out->cd();
    printf("So far so good\n");

    for(int i=0; i<aeCutValues.size(); i++){   
        double ISSnum, ISSdenom;
        double ISSfrac, ISSerr;   
        //ISS positives AGL
        ISSnum = CLtrees.at(0)->GetEntries(aeAglISSposNumSel.at(i).Data())*1.;
        ISSdenom = CLtrees.at(0)->GetEntries(aeAglISSposDenomSel.at(i).Data());
        (ISSdenom!=0) ? ISSfrac = ISSnum/ISSdenom : ISSfrac = 0;
        // (ISSdenom!=0 && ISSnum!=0) ? ISSerr = ISSfrac*sqrt(1/ISSnum + 1/ISSdenom) : ISSerr  = 0;
        (ISSdenom!=0 && ISSnum!=0) ? ISSerr = sqrt(ISSfrac*(1-ISSfrac)/ISSdenom) : ISSerr  = 0;
        int ibin = H1_ASinvMassWAgl.at(0)->Fill(aeCutValues.at(i), ISSfrac);
        H1_ASinvMassWAgl.at(0)->SetBinError(ibin, ISSerr);

        //NaF
        ISSnum = CLtrees.at(0)->GetEntries(aeNaFISSposNumSel.at(i).Data())*1.;
        ISSdenom = CLtrees.at(0)->GetEntries(aeNaFISSposDenomSel.at(i).Data());
        (ISSdenom!=0) ? ISSfrac = ISSnum/ISSdenom : ISSfrac = 0;
        // (ISSdenom!=0 && ISSnum!=0) ? ISSerr = ISSfrac*sqrt(1/ISSnum + 1/ISSdenom) : ISSerr  = 0;
        (ISSdenom!=0 && ISSnum!=0) ? ISSerr = sqrt(ISSfrac*(1-ISSfrac)/ISSdenom) : ISSerr  = 0;
        ibin = H1_ASinvMassWNaF.at(0)->Fill(aeCutValues.at(i), ISSfrac);
        H1_ASinvMassWNaF.at(0)->SetBinError(ibin, ISSerr);
        //ISS negatives AGL       
        ISSnum = CLtrees.at(1)->GetEntries(aeAglISSnegNumSel.at(i).Data())*1.;
        ISSdenom = CLtrees.at(1)->GetEntries(aeAglISSnegDenomSel.at(i).Data());
        (ISSdenom!=0) ? ISSfrac = ISSnum/ISSdenom : ISSfrac = 0;
        // (ISSdenom!=0 && ISSnum!=0) ? ISSerr = ISSfrac*sqrt(1/ISSnum + 1/ISSdenom) : ISSerr  = 0;
        (ISSdenom!=0 && ISSnum!=0) ? ISSerr = sqrt(ISSfrac*(1-ISSfrac)/ISSdenom) : ISSerr  = 0;
        ibin = H1_ASinvMassWAgl.at(1)->Fill(aeCutValues.at(i), ISSfrac);
        H1_ASinvMassWAgl.at(1)->SetBinError(ibin, ISSerr);
        //NaF
        ISSnum = CLtrees.at(1)->GetEntries(aeNaFISSnegNumSel.at(i).Data())*1.;
        ISSdenom = CLtrees.at(1)->GetEntries(aeNaFISSnegDenomSel.at(i).Data());
        (ISSdenom!=0) ? ISSfrac = ISSnum/ISSdenom : ISSfrac = 0;
        // (ISSdenom!=0 && ISSnum!=0) ? ISSerr = ISSfrac*sqrt(1/ISSnum + 1/ISSdenom) : ISSerr  = 0;
        (ISSdenom!=0 && ISSnum!=0) ? ISSerr = sqrt(ISSfrac*(1-ISSfrac)/ISSdenom) : ISSerr  = 0;
        ibin = H1_ASinvMassWNaF.at(1)->Fill(aeCutValues.at(i), ISSfrac);
        H1_ASinvMassWNaF.at(1)->SetBinError(ibin, ISSerr);

        for(int j=0; j<clCutValues.size(); ++j){
            if(i==0){
                double MCnum, MCdenom, MCfrac, MCerr;
                //ISS positives AGL
                ISSnum = CLtrees.at(0)->GetEntries(clAglISSposNumSel.at(j).Data())*1.;
                ISSdenom = CLtrees.at(0)->GetEntries(clAglISSposDenomSel.at(j).Data());
                (ISSnum!=0) ? ISSfrac = ISSnum/ISSdenom : ISSfrac = 0;
                // (ISSdenom!=0 && ISSnum!=0) ? ISSerr = ISSfrac*sqrt(1/ISSnum + 1/ISSdenom) : ISSerr  = 0;
                (ISSdenom!=0 && ISSnum!=0) ? ISSerr = sqrt(ISSfrac*(1-ISSfrac)/ISSdenom) : ISSerr  = 0;
                int ibin = H1_CSinvMassWAgl.at(0)->Fill(clCutValues.at(j), ISSfrac);
                H1_CSinvMassWAgl.at(0)->SetBinError(ibin, ISSerr);
                //NaF
                ISSnum = CLtrees.at(0)->GetEntries(clNaFISSposNumSel.at(j).Data())*1.;
                ISSdenom = CLtrees.at(0)->GetEntries(clNaFISSposDenomSel.at(j).Data());
                (ISSnum!=0) ? ISSfrac = ISSnum/ISSdenom : ISSfrac = 0;
                // (ISSdenom!=0 && ISSnum!=0) ? ISSerr = ISSfrac*sqrt(1/ISSnum + 1/ISSdenom) : ISSerr  = 0;
                (ISSdenom!=0 && ISSnum!=0) ? ISSerr = sqrt(ISSfrac*(1-ISSfrac)/ISSdenom) : ISSerr  = 0;
                ibin = H1_CSinvMassWNaF.at(0)->Fill(clCutValues.at(j), ISSfrac);
                H1_CSinvMassWNaF.at(0)->SetBinError(ibin, ISSerr);
                //ISS negatives AGL       
                ISSnum = CLtrees.at(1)->GetEntries(clAglISSnegNumSel.at(j).Data())*1.;
                ISSdenom = CLtrees.at(1)->GetEntries(clAglISSnegDenomSel.at(j).Data());
                (ISSnum!=0) ? ISSfrac = ISSnum/ISSdenom : ISSfrac = 0;
                // (ISSdenom!=0 && ISSnum!=0) ? ISSerr = ISSfrac*sqrt(1/ISSnum + 1/ISSdenom) : ISSerr  = 0;
                (ISSdenom!=0 && ISSnum!=0) ? ISSerr = sqrt(ISSfrac*(1-ISSfrac)/ISSdenom) : ISSerr  = 0;
                ibin = H1_CSinvMassWAgl.at(1)->Fill(clCutValues.at(j), ISSfrac);
                H1_CSinvMassWAgl.at(1)->SetBinError(ibin, ISSerr);
                //NaF
                ISSnum = CLtrees.at(1)->GetEntries(clNaFISSnegNumSel.at(j).Data())*1.;
                ISSdenom = CLtrees.at(1)->GetEntries(clNaFISSnegDenomSel.at(j).Data());
                (ISSnum!=0) ? ISSfrac = ISSnum/ISSdenom : ISSfrac = 0;
                // (ISSdenom!=0 && ISSnum!=0) ? ISSerr = ISSfrac*sqrt(1/ISSnum + 1/ISSdenom) : ISSerr  = 0;
                (ISSdenom!=0 && ISSnum!=0) ? ISSerr = sqrt(ISSfrac*(1-ISSfrac)/ISSdenom) : ISSerr  = 0;
                ibin = H1_CSinvMassWNaF.at(1)->Fill(clCutValues.at(j), ISSfrac);
                H1_CSinvMassWNaF.at(1)->SetBinError(ibin, ISSerr);
                
                //MC positives AGL
                MCnum = CLtrees.at(2)->GetEntries(clAglMCposNumSel.at(j).Data())*1.;
                MCdenom = CLtrees.at(2)->GetEntries(clAglMCposDenomSel.at(j).Data());
                (MCnum!=0) ? MCfrac = MCnum/MCdenom : MCfrac = 0;
                // (MCdenom!=0 && MCnum!=0) ? MCerr = MCfrac*sqrt(1/MCnum + 1/MCdenom) : MCerr  = 0;
                (MCdenom!=0 && MCnum!=0) ? MCerr = sqrt(MCfrac*(1-MCfrac)/MCdenom) : MCerr  = 0;
                ibin = H1_MC_CSinvMassWAgl.at(0)->Fill(clCutValues.at(j), MCfrac);
                H1_MC_CSinvMassWAgl.at(0)->SetBinError(ibin, MCerr);
                //NaF
                MCnum = CLtrees.at(2)->GetEntries(clNaFMCposNumSel.at(j).Data())*1.;
                MCdenom = CLtrees.at(2)->GetEntries(clNaFMCposDenomSel.at(j).Data());
                (MCnum!=0) ? MCfrac = MCnum/MCdenom : MCfrac = 0;
                // (MCdenom!=0 && MCnum!=0) ? MCerr = MCfrac*sqrt(1/MCnum + 1/MCdenom) : MCerr  = 0;
                (MCdenom!=0 && MCnum!=0) ? MCerr = sqrt(MCfrac*(1-MCfrac)/MCdenom) : MCerr  = 0;
                ibin = H1_MC_CSinvMassWNaF.at(0)->Fill(clCutValues.at(j), MCfrac);
                H1_MC_CSinvMassWNaF.at(0)->SetBinError(ibin, MCerr);
                //MC negatives
                MCnum = CLtrees.at(3)->GetEntries(clAglMCnegNumSel.at(j).Data())*1.;
                MCdenom = CLtrees.at(3)->GetEntries(clAglMCnegDenomSel.at(j).Data());
                (MCnum!=0) ? MCfrac = MCnum/MCdenom : MCfrac = 0;
                // (MCdenom!=0 && MCnum!=0) ? MCerr = MCfrac*sqrt(1/MCnum + 1/MCdenom) : MCerr  = 0;
                (MCdenom!=0 && MCnum!=0) ? MCerr = sqrt(MCfrac*(1-MCfrac)/MCdenom) : MCerr  = 0;
                ibin = H1_MC_CSinvMassWAgl.at(1)->Fill(clCutValues.at(j), MCfrac);
                H1_MC_CSinvMassWAgl.at(1)->SetBinError(ibin, MCerr);
                //NaF
                MCnum = CLtrees.at(3)->GetEntries(clNaFMCnegNumSel.at(j).Data())*1.;
                MCdenom = CLtrees.at(3)->GetEntries(clNaFMCnegDenomSel.at(j).Data());
                (MCnum!=0) ? MCfrac = MCnum/MCdenom : MCfrac = 0;
                // (MCdenom!=0 && MCnum!=0) ? MCerr = MCfrac*sqrt(1/MCnum + 1/MCdenom) : MCerr  = 0;
                (MCdenom!=0 && MCnum!=0) ? MCerr = sqrt(MCfrac*(1-MCfrac)/MCdenom) : MCerr  = 0;
                ibin = H1_MC_CSinvMassWNaF.at(1)->Fill(clCutValues.at(j), MCfrac);
                H1_MC_CSinvMassWNaF.at(1)->SetBinError(ibin, MCerr);
            }
            //ISS positives AGL
            ISSnum = CLtrees.at(0)->GetEntries(combAglISSposNumSel.at((i*clCutValues.size())+j).Data())*1.;
            ISSdenom = CLtrees.at(0)->GetEntries(combAglISSposDenomSel.at((i*clCutValues.size())+j).Data());
            (ISSdenom!=0) ? ISSfrac = ISSnum/ISSdenom : ISSfrac = 0;
            // (ISSdenom!=0 && ISSnum!=0) ? ISSerr = ISSfrac*sqrt(1/ISSnum + 1/ISSdenom) : ISSerr  = 0;
            (ISSdenom!=0 && ISSnum!=0) ? ISSerr = sqrt(ISSfrac*(1-ISSfrac)/ISSdenom) : ISSerr  = 0;
            int ibin = H2_combinvMassWAgl.at(0)->Fill(aeCutValues.at(i), clCutValues.at(j), ISSfrac);
            H2_combinvMassWAglN.at(0)->Fill(aeCutValues.at(i),clCutValues.at(j),ISSnum);
            H2_combinvMassWAgl.at(0)->SetBinError(ibin, ISSerr);

            //NaF
            ISSnum = CLtrees.at(0)->GetEntries(combNaFISSposNumSel.at((i*clCutValues.size())+j).Data())*1.;
            ISSdenom = CLtrees.at(0)->GetEntries(combNaFISSposDenomSel.at((i*clCutValues.size())+j).Data());
            (ISSdenom!=0) ? ISSfrac = ISSnum/ISSdenom : ISSfrac = 0;
            // (ISSdenom!=0 && ISSnum!=0) ? ISSerr = ISSfrac*sqrt(1/ISSnum + 1/ISSdenom) : ISSerr  = 0;
            (ISSdenom!=0 && ISSnum!=0) ? ISSerr = sqrt(ISSfrac*(1-ISSfrac)/ISSdenom) : ISSerr  = 0;
            ibin = H2_combinvMassWNaF.at(0)->Fill(aeCutValues.at(i), clCutValues.at(j), ISSfrac);
            H2_combinvMassWNaFN.at(0)->Fill(aeCutValues.at(i), clCutValues.at(j), ISSnum);
            H2_combinvMassWNaF.at(0)->SetBinError(ibin, ISSerr);
            //ISS negatives AGL       
            ISSnum = CLtrees.at(1)->GetEntries(combAglISSnegNumSel.at((i*clCutValues.size())+j).Data())*1.;
            ISSdenom = CLtrees.at(1)->GetEntries(combAglISSnegDenomSel.at((i*clCutValues.size())+j).Data());
            (ISSdenom!=0) ? ISSfrac = ISSnum/ISSdenom : ISSfrac = 0;
            // (ISSdenom!=0 && ISSnum!=0) ? ISSerr = ISSfrac*sqrt(1/ISSnum + 1/ISSdenom) : ISSerr  = 0;
            (ISSdenom!=0 && ISSnum!=0) ? ISSerr = sqrt(ISSfrac*(1-ISSfrac)/ISSdenom) : ISSerr  = 0;
            ibin = H2_combinvMassWAgl.at(1)->Fill(aeCutValues.at(i), clCutValues.at(j), ISSfrac);
            H2_combinvMassWAglN.at(1)->Fill(aeCutValues.at(i), clCutValues.at(j), ISSnum);
            H2_combinvMassWAgl.at(1)->SetBinError(ibin, ISSerr);
            //NaF
            ISSnum = CLtrees.at(1)->GetEntries(combNaFISSnegNumSel.at((i*clCutValues.size())+j).Data())*1.;
            ISSdenom = CLtrees.at(1)->GetEntries(combNaFISSnegDenomSel.at((i*clCutValues.size())+j).Data());
            (ISSdenom!=0) ? ISSfrac = ISSnum/ISSdenom : ISSfrac = 0;
            // (ISSdenom!=0 && ISSnum!=0) ? ISSerr = ISSfrac*sqrt(1/ISSnum + 1/ISSdenom) : ISSerr  = 0;
            (ISSdenom!=0 && ISSnum!=0) ? ISSerr = sqrt(ISSfrac*(1-ISSfrac)/ISSdenom) : ISSerr  = 0;
            ibin = H2_combinvMassWNaF.at(1)->Fill(aeCutValues.at(i), clCutValues.at(j), ISSfrac);
            H2_combinvMassWNaFN.at(1)->Fill(aeCutValues.at(i), clCutValues.at(j), ISSnum);
            H2_combinvMassWNaF.at(1)->SetBinError(ibin, ISSerr);
        }
        if(i==0)printf("Progress:\n");
        printf("%.2f\r",i*100./aeCutValues.size());
        fflush(stdout);
    }

    for(int i=0; i<H1_CSinvMassWAgl.size(); ++i){
        H1_CSinvMassWAgl.at(i)->Write();
        H1_MC_CSinvMassWAgl.at(i)->Write();
        H1_ASinvMassWAgl.at(i)->Write();
        H1_MC_ASinvMassWAgl.at(i)->Write();
        H2_combinvMassWAgl.at(i)->Write();
        H2_combinvMassWAglN.at(i)->Write();

        H1_CSinvMassWNaF.at(i)->Write();
        H1_MC_CSinvMassWNaF.at(i)->Write();
        H1_ASinvMassWNaF.at(i)->Write();
        H1_MC_ASinvMassWNaF.at(i)->Write();
        H2_combinvMassWNaF.at(i)->Write();
        H2_combinvMassWNaFN.at(i)->Write();
    }



    //How to select survirors using TEventList
    // NO cutOff
    
    
    // TotSelection = "hasRich==1 && beta_rich>0.953 && isNaF==0 && hasGoodImpact==1 && kprob_rich>0.01 &&"
    //                "1<charge2_rich && charge2_rich<4 && ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.8 &&"
    //                "scores_neg>0.06 && (NEGanomaly_score!=1) && -Rinner<50 && abs(SigmaUpLow)<=0.5 &&"
    //                "ACC_AntiCounter==0 && InnerHit==8 && isAbIGRFneg";
    
    // TotSelection = "hasRich==1 && beta_rich>0.953 && isNaF==0 && hasGoodImpact==1 && kprob_rich>0.01 &&"
    //                 "1<charge2_rich && charge2_rich<4 && ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.8 &&"
    //                 "-Rinner<100 && abs(SigmaUpLow)<0.5 && ACC_AntiCounter==0 && InnerHit==8 && NEGanomaly_score!=1 && scores_neg>0.09";

    //Most strong selection so far
    // TotSelection = "hasRich==1 && beta_rich>0.953 && beta_rich<=0.999 && isNaF==0 && hasGoodImpact==1 && kprob_rich>0.01 &&"
    //                 "1<charge2_rich && charge2_rich<4 && ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4 && "
    //                 "scores_neg>=0.3 && abs(SigmaUpLow)<=0.4 && NEGanomaly_score<=0.8 &&"
    //                 "log10(invRerr_IN/(1./abs(Rinner)))<=-0.91 && log10(beta_consistency_rich)<-2 &&"
    //                 "abs(Rinner)+(3*pow(Rinner,2)*invRerr_IN) > 4.4194597 && TRK_MINchXYonTrackPlane4>1.5 && abs(Rinner)>abs(IGRFneg)";

    // TotSelection = "hasRich==1 && beta_rich>0.953 && isNaF==0 && hasGoodImpact==1 && kprob_rich>0.01 &&"
    //                 "1<charge2_rich && charge2_rich<4 && ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4 &&"
    //                 "scores_neg>=0.3 && abs(SigmaUpLow)<=0.4 && NEGanomaly_score<=0.8";
    TotSelection = "hasRich==1 && beta_rich>0.953 && beta_rich < 0.999 && isNaF==0 && hasGoodImpact==1 && kprob_rich>0.01 &&"
                    "1<charge2_rich && charge2_rich<4 && ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4 &&"
                    "scores_neg>=0.3 && abs(SigmaUpLow)<=0.4 && NEGanomaly_score<=0.8";
    

    printf("----> NO GM cutoff\n");
    printf("Selection for survivors: %s\n", TotSelection.Data());

    to_draw = ">>MyEventListNoGMcutoff";
    int MyEntries = CLtrees.at(1)->Draw(to_draw.Data(),TotSelection.Data());
    TEventList *elistNoGMcutoff = (TEventList*)gDirectory->Get("MyEventListNoGMcutoff");
    CLtrees.at(1)->SetEventList(elistNoGMcutoff);
    CLtrees.at(1)->Scan("((2.*abs(Rinner)*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))",TotSelection.Data());
    unsigned int EvNum=0, EvRun=0;

    CLtrees.at(1)->SetBranchAddress("EvNum", &EvNum);
    CLtrees.at(1)->SetBranchAddress("EvRun", &EvRun);
    
    for(int ie=0; ie<MyEntries; ie++){
        CLtrees.at(1)->GetEntry(elistNoGMcutoff->GetEntry(ie));
        printf("Run number: %u, Event number: %u\n", EvRun, EvNum);
    }


    printf("\n----> WITH GM cutoff\n");
    // TotSelection = "hasRich==1 && beta_rich>0.953 && isNaF==0 && hasGoodImpact==1 && kprob_rich>0.01 &&"
    //                 "1<charge2_rich && charge2_rich<4 && ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.8 &&"
    //                 "-Rinner<100 && abs(SigmaUpLow)<0.5 && ACC_AntiCounter==0 && InnerHit==8 && NEGanomaly_score!=1 && scores_neg>0.09 && isAbIGRFneg";

    TotSelection = "hasRich==1 && beta_rich>0.953 && beta_rich < 0.999 && isNaF==0 && hasGoodImpact==1 && kprob_rich>0.01 &&"
                    "1<charge2_rich && charge2_rich<4 && ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4 &&"
                    "scores_neg>=0.3 && abs(SigmaUpLow)<=0.4 && NEGanomaly_score<=0.8 && isAbIGRFneg";
    printf("Selection for survivors: %s\n", TotSelection.Data());

    
    to_draw = ">>MyEventListGMcutoff";
    MyEntries = CLtrees.at(1)->Draw(to_draw.Data(),TotSelection.Data());
    TEventList *elistGMcutoff = (TEventList*)gDirectory->Get("MyEventListGMcutoff");
    CLtrees.at(1)->SetEventList(elistGMcutoff);

    CLtrees.at(1)->SetBranchAddress("EvNum", &EvNum);
    CLtrees.at(1)->SetBranchAddress("EvRun", &EvRun);
    
    for(int ie=0; ie<MyEntries; ie++){
        CLtrees.at(1)->GetEntry(elistGMcutoff->GetEntry(ie));
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
    int nBinsX = h1X->GetNbinsX();
    float minR = pow(10, h1X->GetBinLowEdge(1));
    float maxR = pow(10, h1X->GetBinLowEdge(nBinsX+1));
    //Mass - CS axis
    int nBinsY = h1Y->GetNbinsX();
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
    int nBinsX = h1->GetNbinsX();
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
