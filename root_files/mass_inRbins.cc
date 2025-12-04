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

void mass_inRbins(bool MCorData=true, TString CLtrainOnMCFile="000", TString CLvalOnISSFile="000", TString AEtrainOnMCFile="000", TString AEvalOnISSFile="000"){
    gStyle->SetPalette(55);//kRainBow palette

    const bool isMC = MCorData; //True for training on MC, False for training on ISS 
    (isMC) ? printf("Using file trained on MC!!\n\n") : printf("Using file trained on ISS!!\n\n"); 
    const bool UseLossWeights  = true ; // Loss weights have been used in the training
    const bool useSafetyFactor = false; //Safety factor = 1.2
    const bool useCutOff       = false; // Value of the cutoff (no safety factor)
    const bool isLocal         = false;  // True for debugging

    if(useSafetyFactor) printf("---> Using GM-cutoff with a safety fafctor of 1.2\n");
    if(useCutOff) printf("---> Using Cutoff with a safety fafctor of 1.\n");
    const float uma    = 0.9315; //GeV/c^2
    const float LabelSize = 0.045, TitleSize = 0.05;

    const int TotFiles = 4; //number of files
    const int TotTree  = 4; //n. of CLtrees
    const int TotRig   = 2; //n. of rigidities
    const int TotDet   = 3; //n. of detectors

    TString InPath("/eos/home-f/frrossi/AMS/output/organized_output/scores/");
    if(isLocal) InPath= "/home/franz/AMS/output/organized_output/scores/";
    array<TString, TotFiles> WithWeightsName{"_trainOnMC_withW_2CL_MC_scores.root",     "_trainOnMC_withW_2CL_ISS_scores.root",
                                             "_trainOnMC_withW_AE_MC_scores.root",      "_trainOnMC_withW_AE_ISS_scores.root"};
    array<TString, TotFiles> NoWeightsName{"_trainOnMC_2CL_MC_scores.root",             "_trainOnMC_2CL_ISS_scores.root",
                                           "_trainOnMC_AE_MC_scores.root",               "_trainOnMC_AE_ISS_scores.root"};
    array<TString, TotFiles> ISStrainName{"_trainOnISS_2CL_MC_scores.root",             "_trainOnISS_2CL_ISS_scores.root",
                                           "_trainOnISS_AE_MC_scores.root",               "_trainOnISS_AE_ISS_scores.root"};
    array<TString, TotFiles> PathToFile{InPath+CLtrainOnMCFile+"/"+CLtrainOnMCFile,     InPath+CLvalOnISSFile+"/"+CLvalOnISSFile,
                                        InPath+AEtrainOnMCFile+"/"+AEtrainOnMCFile,     InPath+AEvalOnISSFile+"/"+AEvalOnISSFile};

    TString outName("/eos/home-f/frrossi/AMS/ams_network/root_files/");
    if(isLocal) outName = "/home/franz/AMS/ams_network/root_files/";
    (isMC) ? outName += "TrainOnMC/" : outName += "TrainOnData/";
    for(int i=0; i<TotFiles; ++i) {
        if(isMC){
            (UseLossWeights) ? PathToFile.at(i) += WithWeightsName.at(i) : PathToFile.at(i) += NoWeightsName.at(i);
        }
        else PathToFile.at(i) += ISStrainName.at(i);
    } 
    if(isMC) (UseLossWeights) ? outName +=  "mass_RigBins_withW.root" : outName += "mass_RigBins.root";
    else outName +=  "mass_RigBins.root";
    
    
    array<TFile*, TotFiles> files;
    TFile *f_out = new TFile(outName.Data(), "RECREATE");
    array<TTree*, TotTree> CLtrees;
    array<TTree*, TotTree> AEtrees;

    array<TString, TotRig> NameSub{"CL","AE"};
    array<TString, TotRig> NameR{"Rinner", "RinnerL1"};
    array<TString, TotDet> NameDet{"TOF", "NaF", "AGL"};
    array<TString, TotTree> DirName{"MC_pos", "MC_neg", "ISS_pos", "ISS_neg"};


    //TH2 vectors TOF
    vector<TH2D*> H2_MCpos_CSinvMassTOF,  H2_MCneg_CSinvMassTOF,
                  H2_ISSpos_CSinvMassTOF, H2_ISSneg_CSinvMassTOF;
    vector<TH2D*> H2_MCpos_ASinvMassTOF,  H2_MCneg_ASinvMassTOF,
                  H2_ISSpos_ASinvMassTOF, H2_ISSneg_ASinvMassTOF; 
    //TH2 vectors NaF
    vector<TH2D*> H2_MCpos_CSinvMassNaF,  H2_MCneg_CSinvMassNaF,
                  H2_ISSpos_CSinvMassNaF, H2_ISSneg_CSinvMassNaF;
    vector<TH2D*> H2_MCpos_ASinvMassNaF,  H2_MCneg_ASinvMassNaF,
                  H2_ISSpos_ASinvMassNaF, H2_ISSneg_ASinvMassNaF; 
    //TH2 vectors AGL
    vector<TH2D*> H2_MCpos_CSinvMassAgl,  H2_MCneg_CSinvMassAgl,
                  H2_ISSpos_CSinvMassAgl, H2_ISSneg_CSinvMassAgl;
    vector<TH2D*> H2_MCpos_ASinvMassAgl,  H2_MCneg_ASinvMassAgl,
                  H2_ISSpos_ASinvMassAgl, H2_ISSneg_ASinvMassAgl;
    //Drawing CS vs INV mass
    TString to_draw;
    array<TString, TotRig> draw_MCpos_CSinvMass{"scores:1./((2.*abs("+NameR.at(0)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))",
                                                "scores:1./((2.*abs("+NameR.at(1)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))"};
    array<TString, TotRig> draw_MCneg_CSinvMass{"scores:1./((2.*abs("+NameR.at(0)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))",
                                                "scores:1./((2.*abs("+NameR.at(1)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))"};
    array<TString, TotRig> draw_ISSpos_CSinvMass{"ISS_pos.scores:1./((2.*abs("+NameR.at(0)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))",
                                                 "ISS_pos.scores:1./((2.*abs("+NameR.at(1)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))"};
    array<TString, TotRig> draw_ISSneg_CSinvMass{"ISS_neg.scores:1./((2.*abs("+NameR.at(0)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))",
                                                 "ISS_neg.scores:1./((2.*abs("+NameR.at(1)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))"};
    //Drawing AS vs INV mass
    array<TString, TotRig> draw_MCpos_ASinvMass{"MC.anomaly_score:1./((2.*abs("+NameR.at(0)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))",
                                                 "MC.anomaly_score:1./((2.*abs("+NameR.at(1)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))"};
    array<TString, TotRig> draw_MCneg_ASinvMass{"MCneg.anomaly_score:1./((2.*abs("+NameR.at(0)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))",
                                                 "MCneg.anomaly_score:1./((2.*abs("+NameR.at(1)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))"};
    array<TString, TotRig> draw_ISSpos_ASinvMass{"ISS.anomaly_score:1./((2.*abs("+NameR.at(0)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))",
                                                 "ISS.anomaly_score:1./((2.*abs("+NameR.at(1)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))"};
    array<TString, TotRig> draw_ISSneg_ASinvMass{"ISSneg.anomaly_score:1./((2.*abs("+NameR.at(0)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))",
                                                 "ISSneg.anomaly_score:1./((2.*abs("+NameR.at(1)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))"};


    

    //Selections
    TString TOFSel  = "chi2Coo_tof < 4 && chi2Time_tof < 10";
    TString RichSel = "hasRich==1 && beta_rich>0.75 && hasGoodImpact==1 && "
                      "kprob_rich>0.01 && 1<charge2_rich && charge2_rich<=4 && "
                      "ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4 &&"
                      "beta_rich<=0.997";
    TString NaFSel = "hasRich==1 && beta_rich>0.75 && isNaF==1 && "
                     "isBorder_rich==0 && kprob_rich>0.01 && 1<charge2_rich && "
                     "charge2_rich<=4 && ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.45 &&"
                     "beta_rich<=0.992";
    TString AglSel = "hasRich==1 && beta_rich>0.953 && isNaF==0 && "
                     "hasGoodImpact==1 && kprob_rich>0.01 && 1<charge2_rich && "
                     "charge2_rich<=4 && ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4 &&"
                     "beta_rich<=0.997";
    
    TString isAbIGRFpos  = "&& isAbIGRFpos==1";
    TString isAbIGRFneg  = "&& isAbIGRFneg==1";
    TString IGRFpos = "&& IGRFpos<Rinner";
    TString IGRFneg = "&& abs(IGRFneg)<abs(Rinner)";
    array<TString, TotTree> InL1Sel{" && RinnerL1>0", " && RinnerL1<0", " && RinnerL1>0", " && RinnerL1<0",};
    vector<TString> invMassTOFSel, invMassNaFSel, invMassAglSel;
    printf("TOF selection: %s\n", TOFSel.Data());
    printf("NaF selection: %s\n", NaFSel.Data());
    printf("Agl selection: %s\n", AglSel.Data());
    printf("Rich selection: %s\n\n\n", RichSel.Data());
    TString TotSelection, TempPath;

    //Binning
    const float mass_MAX = 1., mass_MIN = 1./100.;
    const int nbinsMass = 198;
    const float minRin   = -1., maxRin = 3.5;
    const int nbinsRin = 100;
    const float minCS = 0.,  maxCS = 1.;
    const int nbinsScore = 100;
    const float minAS = 0.49, maxAS = 1.01;
    const int nbinsAS = 52;
    // const int nRigbins_ams02He = 39;
    // array<float,nRigbins_ams02He> Rbins{1.92, 2.15, 2.40, 2.67, 2.97, 3.29, 3.64, 4.02, 4.43, 4.88, 5.37, 5.90, 6.47, 7.09, 7.76,
    //                                     8.48, 9.26, 10.1, 11.0, 12.0, 13.0, 14.1, 15.3, 16.6, 18.0, 19.5, 21.1, 22.8, 24.7, 26.7,
    //                                     28.8, 31.1, 33.5, 36.1, 38.9, 41.9, 45.1, 48.5, 52.2};
    const int nRigbins_ams02He = 22; // Number of RIG bins as AMS-02 He flux
    array<float,nRigbins_ams02He> Rbins{1.92, 2.97, 4.02, 5.37, 6.47, 7.76, 9.26, 11.0,
                                        13.0, 15.3, 18.0, 21.1, 24.7, 28.8, 31.1, 33.5,
                                        36.1, 38.9, 41.9, 45.1, 48.5, 52.2};

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
                for(int l=0; l<TotDet; ++l){
                    subDir=DirName.at(i)+"/"+NameSub.at(j)+"/"+NameR.at(k)+"/"+NameDet.at(l);
                    f_out->mkdir(subDir.Data());
                }
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
    //TOF
    TString nCSinvMassTOF,  nASinvMassTOF;
    TString tCSinvMassTOF,  tASinvMassTOF;
    //NaF
    TString nCSinvMassNaF,  nASinvMassNaF;
    TString tCSinvMassNaF,  tASinvMassNaF;
    //AGL
    TString nCSinvMassAgl, nASinvMassAgl;
    TString tCSinvMassAgl, tASinvMassAgl;

    for(int j_rig=0; j_rig<TotRig; j_rig++){ //loop on Rinner Rinner-L1 rig
        for(int ibin=0; ibin<nRigbins_ams02He-1; ++ibin){ // loop on rig bins
            ostringstream ossLow, ossHigh;
            ossLow << std::fixed << std::setprecision(2) << Rbins.at(ibin);
            ossHigh << std::fixed << std::setprecision(2) << Rbins.at(ibin+1);

            //TOF
                //CS vs Inv. Mass (TOF)
            // if(j_rig==1) invMassTOFSel.push_back(TOFSel+InL1Sel.at(i_tree)+" && abs(RinnerL1)<"+TString(ossHigh.str())+"&& abs(RinnerL1)>"+TString(ossLow.str()));
            if(j_rig==1) invMassTOFSel.push_back(TOFSel+" && abs(RinnerL1)<"+TString(ossHigh.str())+"&& abs(RinnerL1)>"+TString(ossLow.str()));
            else invMassTOFSel.push_back(TOFSel+" && abs(Rinner)<"+TString(ossHigh.str())+"&& abs(Rinner)>"+TString(ossLow.str()));
            invMassTOFSel.back().Prepend("("); invMassTOFSel.back().Append(")*weight");

            nCSinvMassTOF = "CL_CSInvMass_TOF_"+NameR.at(j_rig)+"Pos_"+TString(to_string(ibin));
            tCSinvMassTOF = nCSinvMassTOF+"; RICH TOF #frac{1}{m} [1/a.m.u.]; Classifier scores";
            H2_MCpos_CSinvMassTOF.push_back(new TH2D(nCSinvMassTOF.Data(), tCSinvMassTOF.Data(),
                                            nbinsMass, mass_MIN, mass_MAX, nbinsScore, minCS, maxCS));
            nCSinvMassTOF = "CL_CSInvMass_TOF_"+NameR.at(j_rig)+"Neg_"+TString(to_string(ibin));
            tCSinvMassTOF = nCSinvMassTOF+"; RICH TOF #frac{1}{m} [1/a.m.u.]; Classifier scores";
            H2_MCneg_CSinvMassTOF.push_back(new TH2D(nCSinvMassTOF.Data(), tCSinvMassTOF.Data(),
                                            nbinsMass, mass_MIN, mass_MAX, nbinsScore, minCS, maxCS));

            nCSinvMassTOF = "CL_CSInvMass_TOF_"+NameR.at(j_rig)+"IssPos_"+TString(to_string(ibin));
            tCSinvMassTOF = nCSinvMassTOF+"; RICH TOF #frac{1}{m} [1/a.m.u.]; Classifier scores";
            H2_ISSpos_CSinvMassTOF.push_back(new TH2D(nCSinvMassTOF.Data(), tCSinvMassTOF.Data(),
                                            nbinsMass, mass_MIN, mass_MAX, nbinsScore, minCS, maxCS));
            nCSinvMassTOF = "CL_CSInvMass_TOF_"+NameR.at(j_rig)+"IssNeg_"+TString(to_string(ibin));
            tCSinvMassTOF = nCSinvMassTOF+"; RICH TOF #frac{1}{m} [1/a.m.u.]; Classifier scores";
            H2_ISSneg_CSinvMassTOF.push_back(new TH2D(nCSinvMassTOF.Data(), tCSinvMassTOF.Data(),
                                            nbinsMass, mass_MIN, mass_MAX, nbinsScore, minCS, maxCS));
                //AS vs Inv. Mass (TOF)
            nASinvMassTOF = "AE_ASInvMass_TOF_"+NameR.at(j_rig)+"Pos_"+TString(to_string(ibin));
            tASinvMassTOF = nASinvMassTOF+"; RICH TOF #frac{1}{m} [1/a.m.u.]; Anomaly scores";
            H2_MCpos_ASinvMassTOF.push_back(new TH2D(nASinvMassTOF.Data(), tASinvMassTOF.Data(),
                                            nbinsMass, mass_MIN, mass_MAX, nbinsScore, minAS, maxAS));
            nASinvMassTOF = "AE_ASInvMass_TOF_"+NameR.at(j_rig)+"Neg_"+TString(to_string(ibin));
            tASinvMassTOF = nASinvMassTOF+"; RICH TOF #frac{1}{m} [1/a.m.u.]; Anomaly scores";
            H2_MCneg_ASinvMassTOF.push_back(new TH2D(nASinvMassTOF.Data(), tASinvMassTOF.Data(),
                                            nbinsMass, mass_MIN, mass_MAX, nbinsScore, minAS, maxAS));
            
            nASinvMassTOF = "AE_ASInvMass_TOF_"+NameR.at(j_rig)+"IssPos_"+TString(to_string(ibin));
            tASinvMassTOF = nASinvMassTOF+"; RICH TOF #frac{1}{m} [1/a.m.u.]; Anomaly scores";
            H2_ISSpos_ASinvMassTOF.push_back(new TH2D(nASinvMassTOF.Data(), tASinvMassTOF.Data(),
                                            nbinsMass, mass_MIN, mass_MAX, nbinsScore, minAS, maxAS));
            nASinvMassTOF = "AE_ASInvMass_TOF_"+NameR.at(j_rig)+"IssNeg_"+TString(to_string(ibin));
            tASinvMassTOF = nASinvMassTOF+"; RICH TOF #frac{1}{m} [1/a.m.u.]; Anomaly scores";
            H2_ISSneg_ASinvMassTOF.push_back(new TH2D(nASinvMassTOF.Data(), tASinvMassTOF.Data(),
                                            nbinsMass, mass_MIN, mass_MAX, nbinsScore, minAS, maxAS));
            
            H2_MCpos_CSinvMassTOF.back()->GetXaxis()->SetTitleOffset(1.2);  H2_MCneg_CSinvMassTOF.back()->GetXaxis()->SetTitleOffset(1.2);
            H2_ISSpos_CSinvMassTOF.back()->GetXaxis()->SetTitleOffset(1.2); H2_ISSneg_CSinvMassTOF.back()->GetXaxis()->SetTitleOffset(1.2);
            H2_MCpos_ASinvMassTOF.back()->GetXaxis()->SetTitleOffset(1.2);  H2_MCneg_ASinvMassTOF.back()->GetXaxis()->SetTitleOffset(1.2);
            H2_ISSpos_ASinvMassTOF.back()->GetXaxis()->SetTitleOffset(1.2); H2_ISSneg_ASinvMassTOF.back()->GetXaxis()->SetTitleOffset(1.2);

            //NaF
                //CS vs Inv. Mass (NaF)
            // if(j_rig==1) invMassNaFSel.push_back(NaFSel+InL1Sel.at(i_tree)+" && abs(RinnerL1)<"+TString(ossHigh.str())+"&& abs(RinnerL1)>"+TString(ossLow.str()));
            if(j_rig==1) invMassNaFSel.push_back(NaFSel+" && abs(RinnerL1)<"+TString(ossHigh.str())+"&& abs(RinnerL1)>"+TString(ossLow.str()));
            else invMassNaFSel.push_back(NaFSel+" && abs(Rinner)<"+TString(ossHigh.str())+"&& abs(Rinner)>"+TString(ossLow.str()));
            invMassNaFSel.back().Prepend("("); invMassNaFSel.back().Append(")*weight");

            nCSinvMassNaF = "CL_CSInvMass_NaF_"+NameR.at(j_rig)+"Pos_"+TString(to_string(ibin));
            tCSinvMassNaF = nCSinvMassNaF+"; RICH NaF #frac{1}{m} [1/a.m.u.]; Classifier scores";
            H2_MCpos_CSinvMassNaF.push_back(new TH2D(nCSinvMassNaF.Data(), tCSinvMassNaF.Data(),
                                            nbinsMass, mass_MIN, mass_MAX, nbinsScore, minCS, maxCS));
            nCSinvMassNaF = "CL_CSInvMass_NaF_"+NameR.at(j_rig)+"Neg_"+TString(to_string(ibin));
            tCSinvMassNaF = nCSinvMassNaF+"; RICH NaF #frac{1}{m} [1/a.m.u.]; Classifier scores";
            H2_MCneg_CSinvMassNaF.push_back(new TH2D(nCSinvMassNaF.Data(), tCSinvMassNaF.Data(),
                                            nbinsMass, mass_MIN, mass_MAX, nbinsScore, minCS, maxCS));

            nCSinvMassNaF = "CL_CSInvMass_NaF_"+NameR.at(j_rig)+"IssPos_"+TString(to_string(ibin));
            tCSinvMassNaF = nCSinvMassNaF+"; RICH NaF #frac{1}{m} [1/a.m.u.]; Classifier scores";
            H2_ISSpos_CSinvMassNaF.push_back(new TH2D(nCSinvMassNaF.Data(), tCSinvMassNaF.Data(),
                                            nbinsMass, mass_MIN, mass_MAX, nbinsScore, minCS, maxCS));
            nCSinvMassNaF = "CL_CSInvMass_NaF_"+NameR.at(j_rig)+"IssNeg_"+TString(to_string(ibin));
            tCSinvMassNaF = nCSinvMassNaF+"; RICH NaF #frac{1}{m} [1/a.m.u.]; Classifier scores";
            H2_ISSneg_CSinvMassNaF.push_back(new TH2D(nCSinvMassNaF.Data(), tCSinvMassNaF.Data(),
                                            nbinsMass, mass_MIN, mass_MAX, nbinsScore, minCS, maxCS));
                //AS vs Inv. Mass (NaF)
            nASinvMassNaF = "AE_ASInvMass_NaF_"+NameR.at(j_rig)+"Pos_"+TString(to_string(ibin));
            tASinvMassNaF = nASinvMassNaF+"; RICH NaF #frac{1}{m} [1/a.m.u.]; Anomaly scores";
            H2_MCpos_ASinvMassNaF.push_back(new TH2D(nASinvMassNaF.Data(), tASinvMassNaF.Data(),
                                            nbinsMass, mass_MIN, mass_MAX, nbinsScore, minAS, maxAS));
            nASinvMassNaF = "AE_ASInvMass_NaF_"+NameR.at(j_rig)+"Neg_"+TString(to_string(ibin));
            tASinvMassNaF = nASinvMassNaF+"; RICH NaF #frac{1}{m} [1/a.m.u.]; Anomaly scores";
            H2_MCneg_ASinvMassNaF.push_back(new TH2D(nASinvMassNaF.Data(), tASinvMassNaF.Data(),
                                            nbinsMass, mass_MIN, mass_MAX, nbinsScore, minAS, maxAS));
            
            nASinvMassNaF = "AE_ASInvMass_NaF_"+NameR.at(j_rig)+"IssPos_"+TString(to_string(ibin));
            tASinvMassNaF = nASinvMassNaF+"; RICH NaF #frac{1}{m} [1/a.m.u.]; Anomaly scores";
            H2_ISSpos_ASinvMassNaF.push_back(new TH2D(nASinvMassNaF.Data(), tASinvMassNaF.Data(),
                                            nbinsMass, mass_MIN, mass_MAX, nbinsScore, minAS, maxAS));
            nASinvMassNaF = "AE_ASInvMass_NaF_"+NameR.at(j_rig)+"IssNeg_"+TString(to_string(ibin));
            tASinvMassNaF = nASinvMassNaF+"; RICH NaF #frac{1}{m} [1/a.m.u.]; Anomaly scores";
            H2_ISSneg_ASinvMassNaF.push_back(new TH2D(nASinvMassNaF.Data(), tASinvMassNaF.Data(),
                                            nbinsMass, mass_MIN, mass_MAX, nbinsScore, minAS, maxAS));
            
            H2_MCpos_CSinvMassNaF.back()->GetXaxis()->SetTitleOffset(1.2);  H2_MCneg_CSinvMassNaF.back()->GetXaxis()->SetTitleOffset(1.2);
            H2_ISSpos_CSinvMassNaF.back()->GetXaxis()->SetTitleOffset(1.2); H2_ISSneg_CSinvMassNaF.back()->GetXaxis()->SetTitleOffset(1.2);
            H2_MCpos_ASinvMassNaF.back()->GetXaxis()->SetTitleOffset(1.2);  H2_MCneg_ASinvMassNaF.back()->GetXaxis()->SetTitleOffset(1.2);
            H2_ISSpos_ASinvMassNaF.back()->GetXaxis()->SetTitleOffset(1.2); H2_ISSneg_ASinvMassNaF.back()->GetXaxis()->SetTitleOffset(1.2);

            //Agl
                //CS vs Inv. Mass (Agl)
            // if(j_rig==1) invMassAglSel.push_back(AglSel+InL1Sel.at(i_tree)+"abs(RinnerL1)<"+ossHigh.str()+"&& abs(RinnerL1)>"+ossLow.str());
            if(j_rig==1) invMassAglSel.push_back(AglSel+" && abs(RinnerL1)<"+TString(ossHigh.str())+"&& abs(RinnerL1)>"+TString(ossLow.str()));
            else invMassAglSel.push_back(AglSel+"&& abs(Rinner)<"+TString(ossHigh.str())+"&& abs(Rinner)>"+TString(ossLow.str()));
            invMassAglSel.back().Prepend("("); invMassAglSel.back().Append(")*weight");

            nCSinvMassAgl = "CL_CSInvMass_Agl_"+NameR.at(j_rig)+"Pos_"+TString(to_string(ibin));
            tCSinvMassAgl = nCSinvMassAgl+"; RICH Agl #frac{1}{m} [1/a.m.u.]; Classifier scores";
            H2_MCpos_CSinvMassAgl.push_back(new TH2D(nCSinvMassAgl.Data(), tCSinvMassAgl.Data(),
                                            nbinsMass, mass_MIN, mass_MAX, nbinsScore, minCS, maxCS));
            nCSinvMassAgl = "CL_CSInvMass_Agl_"+NameR.at(j_rig)+"Neg_"+TString(to_string(ibin));
            tCSinvMassAgl = nCSinvMassAgl+"; RICH Agl #frac{1}{m} [1/a.m.u.]; Classifier scores";
            H2_MCneg_CSinvMassAgl.push_back(new TH2D(nCSinvMassAgl.Data(), tCSinvMassAgl.Data(),
                                            nbinsMass, mass_MIN, mass_MAX, nbinsScore, minCS, maxCS));

            nCSinvMassAgl = "CL_CSInvMass_Agl_"+NameR.at(j_rig)+"IssPos_"+TString(to_string(ibin));
            tCSinvMassAgl = nCSinvMassAgl+"; RICH Agl #frac{1}{m} [1/a.m.u.]; Classifier scores";
            H2_ISSpos_CSinvMassAgl.push_back(new TH2D(nCSinvMassAgl.Data(), tCSinvMassAgl.Data(),
                                            nbinsMass, mass_MIN, mass_MAX, nbinsScore, minCS, maxCS));
            nCSinvMassAgl = "CL_CSInvMass_Agl_"+NameR.at(j_rig)+"IssNeg_"+TString(to_string(ibin));
            tCSinvMassAgl = nCSinvMassAgl+"; RICH Agl #frac{1}{m} [1/a.m.u.]; Classifier scores";
            H2_ISSneg_CSinvMassAgl.push_back(new TH2D(nCSinvMassAgl.Data(), tCSinvMassAgl.Data(),
                                            nbinsMass, mass_MIN, mass_MAX, nbinsScore, minCS, maxCS));
                //AS vs Inv. Mass (Agl)
            nASinvMassAgl = "AE_ASInvMass_Agl_"+NameR.at(j_rig)+"Pos_"+TString(to_string(ibin));
            tASinvMassAgl = nASinvMassAgl+"; RICH Agl #frac{1}{m} [1/a.m.u.]; Anomaly scores";
            H2_MCpos_ASinvMassAgl.push_back(new TH2D(nASinvMassAgl.Data(), tASinvMassAgl.Data(),
                                            nbinsMass, mass_MIN, mass_MAX, nbinsScore, minAS, maxAS));
            nASinvMassAgl = "AE_ASInvMass_Agl_"+NameR.at(j_rig)+"Neg_"+TString(to_string(ibin));
            tASinvMassAgl = nASinvMassAgl+"; RICH Agl #frac{1}{m} [1/a.m.u.]; Anomaly scores";
            H2_MCneg_ASinvMassAgl.push_back(new TH2D(nASinvMassAgl.Data(), tASinvMassAgl.Data(),
                                            nbinsMass, mass_MIN, mass_MAX, nbinsScore, minAS, maxAS));
            
            nASinvMassAgl = "AE_ASInvMass_Agl_"+NameR.at(j_rig)+"IssPos_"+TString(to_string(ibin));
            tASinvMassAgl = nASinvMassAgl+"; RICH Agl #frac{1}{m} [1/a.m.u.]; Anomaly scores";
            H2_ISSpos_ASinvMassAgl.push_back(new TH2D(nASinvMassAgl.Data(), tASinvMassAgl.Data(),
                                            nbinsMass, mass_MIN, mass_MAX, nbinsScore, minAS, maxAS));
            nASinvMassAgl = "AE_ASInvMass_Agl_"+NameR.at(j_rig)+"IssNeg_"+TString(to_string(ibin));
            tASinvMassAgl = nASinvMassAgl+"; RICH Agl #frac{1}{m} [1/a.m.u.]; Anomaly scores";
            H2_ISSneg_ASinvMassAgl.push_back(new TH2D(nASinvMassAgl.Data(), tASinvMassAgl.Data(),
                                            nbinsMass, mass_MIN, mass_MAX, nbinsScore, minAS, maxAS));
            
            H2_MCpos_CSinvMassAgl.back()->GetXaxis()->SetTitleOffset(1.2);  H2_MCneg_CSinvMassAgl.back()->GetXaxis()->SetTitleOffset(1.2);
            H2_ISSpos_CSinvMassAgl.back()->GetXaxis()->SetTitleOffset(1.2); H2_ISSneg_CSinvMassAgl.back()->GetXaxis()->SetTitleOffset(1.2);
            H2_MCpos_ASinvMassAgl.back()->GetXaxis()->SetTitleOffset(1.2);  H2_MCneg_ASinvMassAgl.back()->GetXaxis()->SetTitleOffset(1.2);
            H2_ISSpos_ASinvMassAgl.back()->GetXaxis()->SetTitleOffset(1.2); H2_ISSneg_ASinvMassAgl.back()->GetXaxis()->SetTitleOffset(1.2);
        } // end loop on rig bins
    } //end loop on rig

    printf("So far so good 1\n");

    //Filling the histos
    //CLtrees->[MC_pos, MC_neg, ISS_pos, ISS_neg]
    TString ISSsel;
    for(int i=0; i<H2_MCpos_CSinvMassNaF.size(); i++){  // loop on histos
        f_out->cd();

        //TOF
            //CS vs Inv. Mass
        (i<nRigbins_ams02He-1) ? to_draw = draw_MCpos_CSinvMass.at(0)+">>+"+H2_MCpos_CSinvMassTOF.at(i)->GetName() : 
                                 to_draw = draw_MCpos_CSinvMass.at(1)+">>+"+H2_MCpos_CSinvMassTOF.at(i)->GetName();
        CLtrees.at(0)->Draw(to_draw.Data(), invMassTOFSel.at(i).Data(), "goff");
        H2_MCpos_CSinvMassTOF.at(i)  = (TH2D*)gDirectory->Get(H2_MCpos_CSinvMassTOF.at(i)->GetName());
        (i<nRigbins_ams02He-1) ? to_draw = draw_MCneg_CSinvMass.at(0)+">>+"+H2_MCneg_CSinvMassTOF.at(i)->GetName() : 
                                 to_draw = draw_MCneg_CSinvMass.at(1)+">>+"+H2_MCneg_CSinvMassTOF.at(i)->GetName();
        CLtrees.at(1)->Draw(to_draw.Data(), invMassTOFSel.at(i).Data(), "goff");
        H2_MCneg_CSinvMassTOF.at(i)  = (TH2D*)gDirectory->Get(H2_MCneg_CSinvMassTOF.at(i)->GetName());
        
        ISSsel =  invMassTOFSel.at(i);
        ISSsel.ReplaceAll(")*weight","");
        if(useSafetyFactor) ISSsel = ISSsel + isAbIGRFpos;
        if(useCutOff) ISSsel = ISSsel + IGRFpos;
        ISSsel.Append(")");
        (i<nRigbins_ams02He-1) ? to_draw = draw_ISSpos_CSinvMass.at(0)+">>+"+H2_ISSpos_CSinvMassTOF.at(i)->GetName() : 
                                 to_draw = draw_ISSpos_CSinvMass.at(1)+">>+"+H2_ISSpos_CSinvMassTOF.at(i)->GetName();
        CLtrees.at(2)->Draw(to_draw.Data(), ISSsel.Data(), "goff");
        H2_ISSpos_CSinvMassTOF.at(i) = (TH2D*)gDirectory->Get(H2_ISSpos_CSinvMassTOF.at(i)->GetName());
        (i<nRigbins_ams02He-1) ? to_draw = draw_ISSneg_CSinvMass.at(0)+">>+"+H2_ISSneg_CSinvMassTOF.at(i)->GetName() : 
                                 to_draw = draw_ISSneg_CSinvMass.at(1)+">>+"+H2_ISSneg_CSinvMassTOF.at(i)->GetName();
        CLtrees.at(3)->Draw(to_draw.Data(), ISSsel.Data(), "goff");
        H2_ISSneg_CSinvMassTOF.at(i) = (TH2D*)gDirectory->Get(H2_ISSneg_CSinvMassTOF.at(i)->GetName());

            //AS vs Inv. Mass
        (i<nRigbins_ams02He-1) ? to_draw = draw_MCpos_ASinvMass.at(0)+">>+"+H2_MCpos_ASinvMassTOF.at(i)->GetName() : 
                                 to_draw = draw_MCpos_ASinvMass.at(1)+">>+"+H2_MCpos_ASinvMassTOF.at(i)->GetName();
        CLtrees.at(0)->Draw(to_draw.Data(), invMassTOFSel.at(i).Data(), "goff");
        H2_MCpos_ASinvMassTOF.at(i)  = (TH2D*)gDirectory->Get(H2_MCpos_ASinvMassTOF.at(i)->GetName());
        (i<nRigbins_ams02He-1) ? to_draw = draw_MCneg_ASinvMass.at(0)+">>+"+H2_MCneg_ASinvMassTOF.at(i)->GetName() : 
                                 to_draw = draw_MCneg_ASinvMass.at(1)+">>+"+H2_MCneg_ASinvMassTOF.at(i)->GetName();
        CLtrees.at(1)->Draw(to_draw.Data(), invMassTOFSel.at(i).Data(), "goff");
        H2_MCneg_ASinvMassTOF.at(i)  = (TH2D*)gDirectory->Get(H2_MCneg_ASinvMassTOF.at(i)->GetName());
        
        ISSsel =  invMassTOFSel.at(i);
        ISSsel.ReplaceAll(")*weight","");
        if(useSafetyFactor) ISSsel = ISSsel + isAbIGRFpos;
        if(useCutOff) ISSsel = ISSsel + IGRFpos;
        ISSsel.Append(")");
        (i<nRigbins_ams02He-1) ? to_draw = draw_ISSpos_ASinvMass.at(0)+">>+"+H2_ISSpos_ASinvMassTOF.at(i)->GetName() : 
                                 to_draw = draw_ISSpos_ASinvMass.at(1)+">>+"+H2_ISSpos_ASinvMassTOF.at(i)->GetName();
        CLtrees.at(2)->Draw(to_draw.Data(), ISSsel.Data(), "goff");
        H2_ISSpos_ASinvMassTOF.at(i) = (TH2D*)gDirectory->Get(H2_ISSpos_ASinvMassTOF.at(i)->GetName());
        (i<nRigbins_ams02He-1) ? to_draw = draw_ISSneg_ASinvMass.at(0)+">>+"+H2_ISSneg_ASinvMassTOF.at(i)->GetName() : 
                                 to_draw = draw_ISSneg_ASinvMass.at(1)+">>+"+H2_ISSneg_ASinvMassTOF.at(i)->GetName();
        CLtrees.at(3)->Draw(to_draw.Data(), ISSsel.Data(), "goff");
        H2_ISSneg_ASinvMassTOF.at(i) = (TH2D*)gDirectory->Get(H2_ISSneg_ASinvMassTOF.at(i)->GetName());

        //NaF
            //CS vs Inv. Mass
        (i<nRigbins_ams02He-1) ? to_draw = draw_MCpos_CSinvMass.at(0)+">>+"+H2_MCpos_CSinvMassNaF.at(i)->GetName() : 
                                 to_draw = draw_MCpos_CSinvMass.at(1)+">>+"+H2_MCpos_CSinvMassNaF.at(i)->GetName();
        CLtrees.at(0)->Draw(to_draw.Data(), invMassNaFSel.at(i).Data(), "goff");
        H2_MCpos_CSinvMassNaF.at(i)  = (TH2D*)gDirectory->Get(H2_MCpos_CSinvMassNaF.at(i)->GetName());
        (i<nRigbins_ams02He-1) ? to_draw = draw_MCneg_CSinvMass.at(0)+">>+"+H2_MCneg_CSinvMassNaF.at(i)->GetName() : 
                                 to_draw = draw_MCneg_CSinvMass.at(1)+">>+"+H2_MCneg_CSinvMassNaF.at(i)->GetName();
        CLtrees.at(1)->Draw(to_draw.Data(), invMassNaFSel.at(i).Data(), "goff");
        H2_MCneg_CSinvMassNaF.at(i)  = (TH2D*)gDirectory->Get(H2_MCneg_CSinvMassNaF.at(i)->GetName());
        
        ISSsel =  invMassNaFSel.at(i);
        ISSsel.ReplaceAll(")*weight","");
        if(useSafetyFactor) ISSsel = ISSsel + isAbIGRFpos;
        if(useCutOff) ISSsel = ISSsel + IGRFpos;
        ISSsel.Append(")");
        (i<nRigbins_ams02He-1) ? to_draw = draw_ISSpos_CSinvMass.at(0)+">>+"+H2_ISSpos_CSinvMassNaF.at(i)->GetName() : 
                                 to_draw = draw_ISSpos_CSinvMass.at(1)+">>+"+H2_ISSpos_CSinvMassNaF.at(i)->GetName();
        CLtrees.at(2)->Draw(to_draw.Data(), ISSsel.Data(), "goff");
        H2_ISSpos_CSinvMassNaF.at(i) = (TH2D*)gDirectory->Get(H2_ISSpos_CSinvMassNaF.at(i)->GetName());
        (i<nRigbins_ams02He-1) ? to_draw = draw_ISSneg_CSinvMass.at(0)+">>+"+H2_ISSneg_CSinvMassNaF.at(i)->GetName() : 
                                 to_draw = draw_ISSneg_CSinvMass.at(1)+">>+"+H2_ISSneg_CSinvMassNaF.at(i)->GetName();
        CLtrees.at(3)->Draw(to_draw.Data(), ISSsel.Data(), "goff");
        H2_ISSneg_CSinvMassNaF.at(i) = (TH2D*)gDirectory->Get(H2_ISSneg_CSinvMassNaF.at(i)->GetName());

            //AS vs Inv. Mass
        (i<nRigbins_ams02He-1) ? to_draw = draw_MCpos_ASinvMass.at(0)+">>+"+H2_MCpos_ASinvMassNaF.at(i)->GetName() : 
                                 to_draw = draw_MCpos_ASinvMass.at(1)+">>+"+H2_MCpos_ASinvMassNaF.at(i)->GetName();
        CLtrees.at(0)->Draw(to_draw.Data(), invMassNaFSel.at(i).Data(), "goff");
        H2_MCpos_ASinvMassNaF.at(i)  = (TH2D*)gDirectory->Get(H2_MCpos_ASinvMassNaF.at(i)->GetName());
        (i<nRigbins_ams02He-1) ? to_draw = draw_MCneg_ASinvMass.at(0)+">>+"+H2_MCneg_ASinvMassNaF.at(i)->GetName() : 
                                 to_draw = draw_MCneg_ASinvMass.at(1)+">>+"+H2_MCneg_ASinvMassNaF.at(i)->GetName();
        CLtrees.at(1)->Draw(to_draw.Data(), invMassNaFSel.at(i).Data(), "goff");
        H2_MCneg_ASinvMassNaF.at(i)  = (TH2D*)gDirectory->Get(H2_MCneg_ASinvMassNaF.at(i)->GetName());
        
        ISSsel =  invMassNaFSel.at(i);
        ISSsel.ReplaceAll(")*weight","");
        if(useSafetyFactor) ISSsel = ISSsel + isAbIGRFpos;
        if(useCutOff) ISSsel = ISSsel + IGRFpos;
        ISSsel.Append(")");
        (i<nRigbins_ams02He-1) ? to_draw = draw_ISSpos_ASinvMass.at(0)+">>+"+H2_ISSpos_ASinvMassNaF.at(i)->GetName() : 
                                 to_draw = draw_ISSpos_ASinvMass.at(1)+">>+"+H2_ISSpos_ASinvMassNaF.at(i)->GetName();
        CLtrees.at(2)->Draw(to_draw.Data(), ISSsel.Data(), "goff");
        H2_ISSpos_ASinvMassNaF.at(i) = (TH2D*)gDirectory->Get(H2_ISSpos_ASinvMassNaF.at(i)->GetName());
        (i<nRigbins_ams02He-1) ? to_draw = draw_ISSneg_ASinvMass.at(0)+">>+"+H2_ISSneg_ASinvMassNaF.at(i)->GetName() : 
                                 to_draw = draw_ISSneg_ASinvMass.at(1)+">>+"+H2_ISSneg_ASinvMassNaF.at(i)->GetName();
        CLtrees.at(3)->Draw(to_draw.Data(), ISSsel.Data(), "goff");
        H2_ISSneg_ASinvMassNaF.at(i) = (TH2D*)gDirectory->Get(H2_ISSneg_ASinvMassNaF.at(i)->GetName());




        //Agl
            //CS vs Inv. Mass
        (i<nRigbins_ams02He-1) ? to_draw = draw_MCpos_CSinvMass.at(0)+">>+"+H2_MCpos_CSinvMassAgl.at(i)->GetName() : 
                                 to_draw = draw_MCpos_CSinvMass.at(1)+">>+"+H2_MCpos_CSinvMassAgl.at(i)->GetName();
        CLtrees.at(0)->Draw(to_draw.Data(), invMassAglSel.at(i).Data(), "goff");
        H2_MCpos_CSinvMassAgl.at(i)  = (TH2D*)gDirectory->Get(H2_MCpos_CSinvMassAgl.at(i)->GetName());
        
        (i<nRigbins_ams02He-1) ? to_draw = draw_MCneg_CSinvMass.at(0)+">>+"+H2_MCneg_CSinvMassAgl.at(i)->GetName() : 
                                 to_draw = draw_MCneg_CSinvMass.at(1)+">>+"+H2_MCneg_CSinvMassAgl.at(i)->GetName();
        CLtrees.at(1)->Draw(to_draw.Data(), invMassAglSel.at(i).Data(), "goff");
        H2_MCneg_CSinvMassAgl.at(i)  = (TH2D*)gDirectory->Get(H2_MCneg_CSinvMassAgl.at(i)->GetName());
        

        ISSsel =  invMassAglSel.at(i);
        ISSsel.ReplaceAll(")*weight","");
        if(useSafetyFactor) ISSsel = ISSsel + isAbIGRFpos;
        if(useCutOff) ISSsel = ISSsel + IGRFpos;
        ISSsel.Append(")");
        (i<nRigbins_ams02He-1) ? to_draw = draw_ISSpos_CSinvMass.at(0)+">>+"+H2_ISSpos_CSinvMassAgl.at(i)->GetName() : 
                                 to_draw = draw_ISSpos_CSinvMass.at(1)+">>+"+H2_ISSpos_CSinvMassAgl.at(i)->GetName();
        CLtrees.at(2)->Draw(to_draw.Data(), ISSsel.Data(), "goff");
        H2_ISSpos_CSinvMassAgl.at(i) = (TH2D*)gDirectory->Get(H2_ISSpos_CSinvMassAgl.at(i)->GetName());
        (i<nRigbins_ams02He-1) ? to_draw = draw_ISSneg_CSinvMass.at(0)+">>+"+H2_ISSneg_CSinvMassAgl.at(i)->GetName() : 
                                 to_draw = draw_ISSneg_CSinvMass.at(1)+">>+"+H2_ISSneg_CSinvMassAgl.at(i)->GetName();
        CLtrees.at(3)->Draw(to_draw.Data(), ISSsel.Data(), "goff");
        H2_ISSneg_CSinvMassAgl.at(i) = (TH2D*)gDirectory->Get(H2_ISSneg_CSinvMassAgl.at(i)->GetName());


            //AS vs Inv. Mass
        (i<nRigbins_ams02He-1) ? to_draw = draw_MCpos_ASinvMass.at(0)+">>+"+H2_MCpos_ASinvMassAgl.at(i)->GetName() : 
                                 to_draw = draw_MCpos_ASinvMass.at(1)+">>+"+H2_MCpos_ASinvMassAgl.at(i)->GetName();
        CLtrees.at(0)->Draw(to_draw.Data(), invMassAglSel.at(i).Data(), "goff");
        H2_MCpos_ASinvMassAgl.at(i)  = (TH2D*)gDirectory->Get(H2_MCpos_ASinvMassAgl.at(i)->GetName());
        
        (i<nRigbins_ams02He-1) ? to_draw = draw_MCneg_ASinvMass.at(0)+">>+"+H2_MCneg_ASinvMassAgl.at(i)->GetName() : 
                                 to_draw = draw_MCneg_ASinvMass.at(1)+">>+"+H2_MCneg_ASinvMassAgl.at(i)->GetName();
        CLtrees.at(1)->Draw(to_draw.Data(), invMassAglSel.at(i).Data(), "goff");
        H2_MCneg_ASinvMassAgl.at(i)  = (TH2D*)gDirectory->Get(H2_MCneg_ASinvMassAgl.at(i)->GetName());
        

        ISSsel =  invMassAglSel.at(i);
        ISSsel.ReplaceAll(")*weight","");
        if(useSafetyFactor) ISSsel = ISSsel + isAbIGRFpos;
        if(useCutOff) ISSsel = ISSsel + IGRFpos;
        ISSsel.Append(")");
        (i<nRigbins_ams02He-1) ? to_draw = draw_ISSpos_ASinvMass.at(0)+">>+"+H2_ISSpos_ASinvMassAgl.at(i)->GetName() : 
                                 to_draw = draw_ISSpos_ASinvMass.at(1)+">>+"+H2_ISSpos_ASinvMassAgl.at(i)->GetName();
        CLtrees.at(2)->Draw(to_draw.Data(), ISSsel.Data(), "goff");
        H2_ISSpos_ASinvMassAgl.at(i) = (TH2D*)gDirectory->Get(H2_ISSpos_ASinvMassAgl.at(i)->GetName());
        (i<nRigbins_ams02He-1) ? to_draw = draw_ISSneg_ASinvMass.at(0)+">>+"+H2_ISSneg_ASinvMassAgl.at(i)->GetName() : 
                                 to_draw = draw_ISSneg_ASinvMass.at(1)+">>+"+H2_ISSneg_ASinvMassAgl.at(i)->GetName();
        CLtrees.at(3)->Draw(to_draw.Data(), ISSsel.Data(), "goff");
        H2_ISSneg_ASinvMassAgl.at(i) = (TH2D*)gDirectory->Get(H2_ISSneg_ASinvMassAgl.at(i)->GetName());



        //Writing
        int ibin=0;
        (i<nRigbins_ams02He-1) ? ibin = i : ibin = i-(nRigbins_ams02He-1);
        ostringstream ossLow, ossHigh;
        ossLow << std::fixed << std::setprecision(2) << Rbins.at(ibin);
        ossHigh << std::fixed << std::setprecision(2) << Rbins.at(ibin+1);
        TString Title, Name;

        // MC POSITIVES + 
        (i<nRigbins_ams02He-1) ? TempPath = DirName.at(0)+"/"+NameSub.at(0)+"/"+NameR.at(0) : 
                                 TempPath = DirName.at(0)+"/"+NameSub.at(0)+"/"+NameR.at(1);
        TString CurrentDir = TempPath + "/" + NameDet.at(0); //CL-Rig-TOF
        f_out->cd(CurrentDir.Data());
        Name  = H2_MCpos_CSinvMassTOF.at(i)->GetName();
        Title = TString(H2_MCpos_CSinvMassTOF.at(i)->GetName())+"_"+TString(ossLow.str())+"_"+TString(ossHigh.str());
        H2_MCpos_CSinvMassTOF.at(i)->SetTitle(Title.Data()); H2_MCpos_CSinvMassTOF.at(i)->Write(Name.Data());
        CurrentDir = TempPath + "/" + NameDet.at(1); //CL-Rig-NaF
        f_out->cd(CurrentDir.Data());
        Name  = H2_MCpos_CSinvMassNaF.at(i)->GetName();
        Title = TString(H2_MCpos_CSinvMassNaF.at(i)->GetName())+"_"+TString(ossLow.str())+"_"+TString(ossHigh.str());
        H2_MCpos_CSinvMassNaF.at(i)->SetTitle(Title.Data()); H2_MCpos_CSinvMassNaF.at(i)->Write(Name.Data());
        CurrentDir = TempPath + "/" + NameDet.at(2);         //CL-Rig-Agl
        f_out->cd(CurrentDir.Data());
        Name  = H2_MCpos_CSinvMassAgl.at(i)->GetName();
        Title = TString(H2_MCpos_CSinvMassAgl.at(i)->GetName())+"_"+TString(ossLow.str())+"_"+TString(ossHigh.str());
        H2_MCpos_CSinvMassAgl.at(i)->SetTitle(Title.Data()); H2_MCpos_CSinvMassAgl.at(i)->Write(Name.Data());

        (i<nRigbins_ams02He-1) ? TempPath = DirName.at(0)+"/"+NameSub.at(1)+"/"+NameR.at(0) : 
                                 TempPath = DirName.at(0)+"/"+NameSub.at(1)+"/"+NameR.at(1);
        CurrentDir = TempPath + "/" + NameDet.at(0);         //AE-Rig-TOF
        f_out->cd(CurrentDir.Data());
        Name  = H2_MCpos_ASinvMassTOF.at(i)->GetName();
        Title = TString(H2_MCpos_ASinvMassTOF.at(i)->GetName())+"_"+TString(ossLow.str())+"_"+TString(ossHigh.str());
        H2_MCpos_ASinvMassTOF.at(i)->SetTitle(Title.Data()); H2_MCpos_ASinvMassTOF.at(i)->Write(Name.Data());
        CurrentDir = TempPath + "/" + NameDet.at(1);         //AE-Rig-NaF
        f_out->cd(CurrentDir.Data());
        Name  = H2_MCpos_ASinvMassNaF.at(i)->GetName();
        Title = TString(H2_MCpos_ASinvMassNaF.at(i)->GetName())+"_"+TString(ossLow.str())+"_"+TString(ossHigh.str());
        H2_MCpos_ASinvMassNaF.at(i)->SetTitle(Title.Data()); H2_MCpos_ASinvMassNaF.at(i)->Write(Name.Data());
        CurrentDir = TempPath + "/" + NameDet.at(2);         //AE-Rig-AGL
        f_out->cd(CurrentDir.Data());
        Name  = H2_MCpos_ASinvMassAgl.at(i)->GetName();
        Title = TString(H2_MCpos_ASinvMassAgl.at(i)->GetName())+"_"+TString(ossLow.str())+"_"+TString(ossHigh.str());
        H2_MCpos_ASinvMassAgl.at(i)->SetTitle(Title.Data()); H2_MCpos_ASinvMassAgl.at(i)->Write(Name.Data());

        // MC ATIVE - 
        (i<nRigbins_ams02He-1) ? TempPath = DirName.at(1)+"/"+NameSub.at(0)+"/"+NameR.at(0) : 
                                 TempPath = DirName.at(1)+"/"+NameSub.at(0)+"/"+NameR.at(1);
        CurrentDir = TempPath + "/" + NameDet.at(0); //CL-Rig-TOF
        f_out->cd(CurrentDir.Data());
        Name  = H2_MCneg_CSinvMassTOF.at(i)->GetName();
        Title = TString(H2_MCneg_CSinvMassTOF.at(i)->GetName())+"_"+TString(ossLow.str())+"_"+TString(ossHigh.str());
        H2_MCneg_CSinvMassTOF.at(i)->SetTitle(Title.Data()); H2_MCneg_CSinvMassTOF.at(i)->Write(Name.Data());
        CurrentDir = TempPath + "/" + NameDet.at(1); //CL-Rig-NaF
        f_out->cd(CurrentDir.Data());
        Name  = H2_MCneg_CSinvMassNaF.at(i)->GetName();
        Title = TString(H2_MCneg_CSinvMassNaF.at(i)->GetName())+"_"+TString(ossLow.str())+"_"+TString(ossHigh.str());
        H2_MCneg_CSinvMassNaF.at(i)->SetTitle(Title.Data()); H2_MCneg_CSinvMassNaF.at(i)->Write(Name.Data());
        CurrentDir = TempPath + "/" + NameDet.at(2);         //CL-Rig-Agl
        f_out->cd(CurrentDir.Data());
        Name  = H2_MCneg_CSinvMassAgl.at(i)->GetName();
        Title = TString(H2_MCneg_CSinvMassAgl.at(i)->GetName())+"_"+TString(ossLow.str())+"_"+TString(ossHigh.str());
        H2_MCneg_CSinvMassAgl.at(i)->SetTitle(Title.Data()); H2_MCneg_CSinvMassAgl.at(i)->Write(Name.Data());
        
        (i<nRigbins_ams02He-1) ? TempPath = DirName.at(1)+"/"+NameSub.at(1)+"/"+NameR.at(0) : 
                                 TempPath = DirName.at(1)+"/"+NameSub.at(1)+"/"+NameR.at(1);
        CurrentDir = TempPath + "/" + NameDet.at(0);         //AE-Rig-TOF
        f_out->cd(CurrentDir.Data());
        Name  = H2_MCneg_ASinvMassTOF.at(i)->GetName();
        Title = TString(H2_MCneg_ASinvMassTOF.at(i)->GetName())+"_"+TString(ossLow.str())+"_"+TString(ossHigh.str());
        H2_MCneg_ASinvMassTOF.at(i)->SetTitle(Title.Data()); H2_MCneg_ASinvMassTOF.at(i)->Write(Name.Data());
        CurrentDir = TempPath + "/" + NameDet.at(1);         //AE-Rig-NaF
        f_out->cd(CurrentDir.Data());
        Name  = H2_MCneg_ASinvMassNaF.at(i)->GetName();
        Title = TString(H2_MCneg_ASinvMassNaF.at(i)->GetName())+"_"+TString(ossLow.str())+"_"+TString(ossHigh.str());
        H2_MCneg_ASinvMassNaF.at(i)->SetTitle(Title.Data()); H2_MCneg_ASinvMassNaF.at(i)->Write(Name.Data());
        CurrentDir = TempPath + "/" + NameDet.at(2);         //AE-Rig-NaF
        f_out->cd(CurrentDir.Data());
        Name  = H2_MCneg_ASinvMassAgl.at(i)->GetName();
        Title = TString(H2_MCneg_ASinvMassAgl.at(i)->GetName())+"_"+TString(ossLow.str())+"_"+TString(ossHigh.str());
        H2_MCneg_ASinvMassAgl.at(i)->SetTitle(Title.Data()); H2_MCneg_ASinvMassAgl.at(i)->Write(Name.Data());

        // ISS POSITIVES + 
        (i<nRigbins_ams02He-1) ? TempPath = DirName.at(2)+"/"+NameSub.at(0)+"/"+NameR.at(0) : 
                                 TempPath = DirName.at(2)+"/"+NameSub.at(0)+"/"+NameR.at(1);
        CurrentDir = TempPath + "/" + NameDet.at(0); //CL-Rig-TOF
        f_out->cd(CurrentDir.Data());
        Name  = H2_ISSpos_CSinvMassTOF.at(i)->GetName();
        Title = TString(H2_ISSpos_CSinvMassTOF.at(i)->GetName())+"_"+TString(ossLow.str())+"_"+TString(ossHigh.str());
        H2_ISSpos_CSinvMassTOF.at(i)->SetTitle(Title.Data()); H2_ISSpos_CSinvMassTOF.at(i)->Write(Name.Data());
        CurrentDir = TempPath + "/" + NameDet.at(1); //CL-Rig-NaF
        f_out->cd(CurrentDir.Data());
        Name  = H2_ISSpos_CSinvMassNaF.at(i)->GetName();
        Title = TString(H2_ISSpos_CSinvMassNaF.at(i)->GetName())+"_"+TString(ossLow.str())+"_"+TString(ossHigh.str());
        H2_ISSpos_CSinvMassNaF.at(i)->SetTitle(Title.Data()); H2_ISSpos_CSinvMassNaF.at(i)->Write(Name.Data());
        CurrentDir = TempPath + "/" + NameDet.at(2);         //CL-Rig-Agl
        f_out->cd(CurrentDir.Data());
        Name  = H2_ISSpos_CSinvMassAgl.at(i)->GetName();
        Title = TString(H2_ISSpos_CSinvMassAgl.at(i)->GetName())+"_"+TString(ossLow.str())+"_"+TString(ossHigh.str());
        H2_ISSpos_CSinvMassAgl.at(i)->SetTitle(Title.Data()); H2_ISSpos_CSinvMassAgl.at(i)->Write(Name.Data());

        (i<nRigbins_ams02He-1) ? TempPath = DirName.at(2)+"/"+NameSub.at(1)+"/"+NameR.at(0) : 
                                 TempPath = DirName.at(2)+"/"+NameSub.at(1)+"/"+NameR.at(1);
        CurrentDir = TempPath + "/" + NameDet.at(0);         //AE-Rig-TOF
        f_out->cd(CurrentDir.Data());
        Name  = H2_ISSpos_ASinvMassTOF.at(i)->GetName();
        Title = TString(H2_ISSpos_ASinvMassTOF.at(i)->GetName())+"_"+TString(ossLow.str())+"_"+TString(ossHigh.str());
        H2_ISSpos_ASinvMassTOF.at(i)->SetTitle(Title.Data()); H2_ISSpos_ASinvMassTOF.at(i)->Write(Name.Data());
        CurrentDir = TempPath + "/" + NameDet.at(1);         //AE-Rig-NaF
        f_out->cd(CurrentDir.Data());
        Name  = H2_ISSpos_ASinvMassNaF.at(i)->GetName();
        Title = TString(H2_ISSpos_ASinvMassNaF.at(i)->GetName())+"_"+TString(ossLow.str())+"_"+TString(ossHigh.str());
        H2_ISSpos_ASinvMassNaF.at(i)->SetTitle(Title.Data()); H2_ISSpos_ASinvMassNaF.at(i)->Write(Name.Data());
        CurrentDir = TempPath + "/" + NameDet.at(2);         //AE-Rig-NaF
        f_out->cd(CurrentDir.Data());
        Name  = H2_ISSpos_ASinvMassAgl.at(i)->GetName();
        Title = TString(H2_ISSpos_ASinvMassAgl.at(i)->GetName())+"_"+TString(ossLow.str())+"_"+TString(ossHigh.str());
        H2_ISSpos_ASinvMassAgl.at(i)->SetTitle(Title.Data()); H2_ISSpos_ASinvMassAgl.at(i)->Write(Name.Data());

        // ISS ATIVES - 
        (i<nRigbins_ams02He-1) ? TempPath = DirName.at(3)+"/"+NameSub.at(0)+"/"+NameR.at(0) : 
                                 TempPath = DirName.at(3)+"/"+NameSub.at(0)+"/"+NameR.at(1);
        CurrentDir = TempPath + "/" + NameDet.at(0); //CL-Rig-TOF
        f_out->cd(CurrentDir.Data());
        Name  = H2_ISSneg_CSinvMassTOF.at(i)->GetName();
        Title = TString(H2_ISSneg_CSinvMassTOF.at(i)->GetName())+"_"+TString(ossLow.str())+"_"+TString(ossHigh.str());
        H2_ISSneg_CSinvMassTOF.at(i)->SetTitle(Title.Data()); H2_ISSneg_CSinvMassTOF.at(i)->Write(Name.Data());
        CurrentDir = TempPath + "/" + NameDet.at(1); //CL-Rig-NaF
        f_out->cd(CurrentDir.Data());
        Name  = H2_ISSneg_CSinvMassNaF.at(i)->GetName();
        Title = TString(H2_ISSneg_CSinvMassNaF.at(i)->GetName())+"_"+TString(ossLow.str())+"_"+TString(ossHigh.str());
        H2_ISSneg_CSinvMassNaF.at(i)->SetTitle(Title.Data()); H2_ISSneg_CSinvMassNaF.at(i)->Write(Name.Data());
        CurrentDir = TempPath + "/" + NameDet.at(2);         //CL-Rig-Agl
        f_out->cd(CurrentDir.Data());
        Name  = H2_ISSneg_CSinvMassAgl.at(i)->GetName();
        Title = TString(H2_ISSneg_CSinvMassAgl.at(i)->GetName())+"_"+TString(ossLow.str())+"_"+TString(ossHigh.str());
        H2_ISSneg_CSinvMassAgl.at(i)->SetTitle(Title.Data()); H2_ISSneg_CSinvMassAgl.at(i)->Write(Name.Data());

        (i<nRigbins_ams02He-1) ? TempPath = DirName.at(3)+"/"+NameSub.at(1)+"/"+NameR.at(0) : 
                                 TempPath = DirName.at(3)+"/"+NameSub.at(1)+"/"+NameR.at(1);
        CurrentDir = TempPath + "/" + NameDet.at(0);         //AE-Rig-TOF
        f_out->cd(CurrentDir.Data());
        Name  = H2_ISSneg_ASinvMassTOF.at(i)->GetName();
        Title = TString(H2_ISSneg_ASinvMassTOF.at(i)->GetName())+"_"+TString(ossLow.str())+"_"+TString(ossHigh.str());
        H2_ISSneg_ASinvMassTOF.at(i)->SetTitle(Title.Data()); H2_ISSneg_ASinvMassTOF.at(i)->Write(Name.Data());
        CurrentDir = TempPath + "/" + NameDet.at(0);         //AE-Rig-NaF
        f_out->cd(CurrentDir.Data());
        Name  = H2_ISSneg_ASinvMassNaF.at(i)->GetName();
        Title = TString(H2_ISSneg_ASinvMassNaF.at(i)->GetName())+"_"+TString(ossLow.str())+"_"+TString(ossHigh.str());
        H2_ISSneg_ASinvMassNaF.at(i)->SetTitle(Title.Data()); H2_ISSneg_ASinvMassNaF.at(i)->Write(Name.Data());
        CurrentDir = TempPath + "/" + NameDet.at(1);         //AE-Rig-NaF
        f_out->cd(CurrentDir.Data());
        Name  = H2_ISSneg_ASinvMassAgl.at(i)->GetName();
        Title = TString(H2_ISSneg_ASinvMassAgl.at(i)->GetName())+"_"+TString(ossLow.str())+"_"+TString(ossHigh.str());
        H2_ISSneg_ASinvMassAgl.at(i)->SetTitle(Title.Data()); H2_ISSneg_ASinvMassAgl.at(i)->Write(Name.Data());
    } // end loop on histos

    f_out->cd();
    printf("So far so good2\n");


    //How to select survirors using TEventList
    // TotSelection = "hasRich==1 && beta_rich>0.953 && isNaF==0 && hasGoodImpact==1 && kprob_rich>0.01 &&"
    //                "1<charge2_rich && charge2_rich<4 && ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.8 &&"
    //                "scores>0.06 && (anomaly_score!=1) && -Rinner<50 && abs(SigmaUpLow)<=0.5 &&"
    //                "ACC_AntiCounter==0 && InnerHit==8 && isAbIGRFneg";
    // NO cutOff
    // TotSelection = "hasRich==1 && beta_rich>0.953 && isNaF==0 && hasGoodImpact==1 && kprob_rich>0.01 &&"
    //                "1<charge2_rich && charge2_rich<4 && ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4 && beta_rich < 0.997 &&"
    //                "-Rinner<100 && abs(SigmaUpLow)<0.5 && ACC_AntiCounter==0 && anomaly_score<=0.9 && scores>0.1";
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
    TotSelection = "hasRich==1 && beta_rich>0.953 && beta_rich < 0.999 && isNaF==0 && hasGoodImpact==1 && kprob_rich>0.01 &&"
                   "1<charge2_rich && charge2_rich<4 && ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4 &&"
                   "scores>=0.3 && abs(SigmaUpLow)<=0.4 && anomaly_score<=0.8 && abs(IGRFneg)<abs(Rinner)";
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

    printf("Output file is here: %s\n", outName.Data());
    
    
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
