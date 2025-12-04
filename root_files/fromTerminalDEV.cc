/*
----->
To be used only on models trained on MC
----->

Allow to visualize interactively the
trees obtained from the Validation and 
testing. 

Usage:
    root -l

    std::vector<TTree*> ISS, MC;
    .L fromTerminal_MC.cc+

    ==> on LXPLUS
        // Training with OLD (wrong) Loss weights
    ISS = fromTerminalDEV(false, "2025_03_13", "2025_03_13", "2025_03_13", "2025_03_13", true, true, true);
    MC  = fromTerminalDEV(true,  "2025_03_13", "2025_03_13", "2025_03_13", "2025_03_13", true, true, true);
*/

#include <iostream>
#include <fstream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <array>
#include <vector>

#include <TString.h>
#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>
#include <TStyle.h>
#include <TH1.h>

using namespace std;

const int TotTree  = 4; // Number of CL trees
const int TotFiles = 4; // Number of files
const int LW = 3;
const int nRigbins = 11; // Number of RIG bins as AMS-02 He flux
array<float,nRigbins> Rbins{1.92, 4.02, 6.47, 9.26, 13.0,
                            18.0, 24.7, 31.1, 36.1, 41.9,
                            48.5};
/*
const int nRigbins = 22; // Number of RIG bins as AMS-02 He flux
array<float,nRigbins> Rbins{1.92, 2.97, 4.02, 5.37, 6.47, 7.76, 9.26, 11.0,
                            13.0, 15.3, 18.0, 21.1, 24.7, 28.8, 31.1, 33.5,
                            36.1, 38.9, 41.9, 45.1, 48.5, 52.2};
*/

void IntializeCL2_TH1(array<TH1F*,nRigbins> &harra, TString &myXtitle,
                   TString &myYtitle, TString &myTitle,
                   TString &myName);
void IntializeAE_TH1(array<TH1F*,nRigbins> &harrAE, TString &myXtitle,
                   TString &myYtitle, TString &myTitle,
                   TString &myName);
void GetTh1CL(TString &Det, TString &Sample, TFile *f, array<TH1F*, nRigbins> &TH1sig,
              array<TH1F*, nRigbins> &TH1bkg, TTree *myT, TString &Sel,
              bool isMC, bool mkDir);
void GetTh1AE(TString &Det, TString &Sample, TFile *f, array<TH1F*, TotTree> &TH1my,
              TTree *myT, TString &Sel,
              bool isMC);
void GetMyScores(vector<TTree*> &myTrees, 
                array<TString,3> &Selections, array<TString, TotTree> Samples,
                bool onlyNNs, bool amImc);

std::vector<TTree*> fromTerminalDEV(bool useMC=false,
                                    TString CLtrainOnMCFile="000",
                                    TString CLvalOnISSFile="000",
                                    TString AEtrainOnMCFile="000",
                                    TString AEvalOnISSFile="000",
                                    bool GetScores=false,
                                    bool FirstNNs=false,
                                    bool verbose=false,
                                    vector<TTree*> vTrees = {}
                                ) {
    gStyle->SetPalette(55);//kRainBow palette
    gStyle->SetOptStat(0);
    
    const bool isLocal = false;
    
    bool UseLossWeights; // Loss weights have been used in the training
    (useMC) ? UseLossWeights = true : UseLossWeights = false;
    TString inPath;
    (isLocal) ? inPath ="/home/franz/AMS/output/organized_output/scores/" : inPath="/eos/home-f/frrossi/AMS/output/organized_output/scores/"; 
    
    const float he3 = 2.81, he4 = 3.72; // GeV/c2
    const float bNaF_thL = 0.75,  bNaF_thH = 0.993;
    const float bAGL_thL = 0.953, bAGL_thH = 0.998;
    //NaF
    const float r3NaF_thL = he3 * bNaF_thL * 1./sqrt(1-pow(bNaF_thL,2)) / 2;
    ostringstream RvalNaF; RvalNaF  << fixed << setprecision(3) << r3NaF_thL;
    //AGL
    const float r3AGL_thL = he3 * bAGL_thL * 1./sqrt(1-pow(bAGL_thL,2)) / 2;
    ostringstream RvalAGL; RvalAGL  << fixed << setprecision(3) << r3AGL_thL;

    array<TString, TotFiles> WithWeightsName{"_trainOnMC_withW_2CL_MC_scores.root",     "_trainOnMC_withW_2CL_ISS_scores.root",
                                             "_trainOnMC_withW_AE_MC_scores.root",      "_trainOnMC_withW_AE_ISS_scores.root"};
    array<TString, TotFiles> NoWeightsName{"_trainOnMC_2CL_MC_scores.root",             "_trainOnMC_2CL_ISS_scores.root",
                                           "_trainOnMC_AE_MC_scores.root",               "_trainOnMC_AE_ISS_scores.root"};
    array<TString, TotFiles> ISStrainName{"_trainOnISS_2CL_MC_scores.root",             "_trainOnISS_2CL_ISS_scores.root",
                                           "_trainOnISS_AE_MC_scores.root",               "_trainOnISS_AE_ISS_scores.root"};

    array<TString, TotFiles> PathToFile{
        inPath + CLtrainOnMCFile + "/" + CLtrainOnMCFile, inPath + CLvalOnISSFile + "/" + CLvalOnISSFile,
        inPath + AEtrainOnMCFile + "/" + AEtrainOnMCFile, inPath + AEvalOnISSFile + "/" + AEvalOnISSFile};
    for(int i=0; i<TotFiles; ++i){
        if(useMC){
            (UseLossWeights) ? PathToFile.at(i) += WithWeightsName.at(i) : PathToFile.at(i) += NoWeightsName.at(i);
        }
        else PathToFile.at(i) += ISStrainName.at(i);
    }

    array<TFile*, TotFiles> files;
    array<TTree*, TotTree> CLtrees;
    array<TTree*, TotTree> AEtrees;
    array<TString, TotTree> DirName{"MC_pos", "MC_neg", "ISS_pos", "ISS_neg"};

    // Getting trees
    // CLtrees->[MC_pos, MC_neg, ISS_pos, ISS_neg]
    for (int i_file = 0; i_file < TotFiles; i_file++) {  // Loop on files
        if (gSystem->AccessPathName(PathToFile.at(i_file), kFileExists)) {
            printf("The file %s does not exist.\n", PathToFile.at(i_file).Data());
            return {};
        }
        if(verbose) printf("The file %s exists.\n", PathToFile.at(i_file).Data());
        files.at(i_file) = new TFile(PathToFile.at(i_file), "READ");
        if (!files.at(i_file)) {
            printf("ERROR!! \n File %s not found\n", PathToFile.at(i_file).Data());
            return {};
        }
        files.at(i_file)->cd();
        printf("Reading file ==> %s\n\n", PathToFile.at(i_file).Data());
        for (int i_tree = 0; i_tree < TotTree; i_tree++) {
            if (i_file == 0 && i_tree < 2) { // CL MC
                CLtrees.at(i_tree) = (TTree*)files.at(i_file)->Get(DirName.at(i_tree).Data());
                if (!CLtrees.at(i_tree)) {
                    printf("%s tree not found in the file %s\n", DirName.at(i_tree).Data(), PathToFile.at(i_file).Data());
                    return {};
                }
                CLtrees.at(i_tree)->BuildIndex("EvRun","EvNum");
            } // End CL MC
            if (i_file == 1 && i_tree >= 2) { // CL ISS data
                CLtrees.at(i_tree) = (TTree*)files.at(i_file)->Get(DirName.at(i_tree).Data());
                if (!CLtrees.at(i_tree)) {
                    printf("%s tree not found in the file %s\n", DirName.at(i_tree).Data(), PathToFile.at(i_file).Data());
                    return {};
                }
                CLtrees.at(i_tree)->BuildIndex("EvRun","EvNum");
            } // End CL ISS
        } // End loop on CLtrees

        if (i_file == 2) { // AE MC
            AEtrees.at(0) = (TTree*)files.at(i_file)->Get("MC");
            AEtrees.at(1) = (TTree*)files.at(i_file)->Get("MCneg");
            if (!AEtrees.at(0) || !AEtrees.at(1)) {
                printf("AE trees not found in the file %s\n", PathToFile.at(i_file).Data());
                return {};
            }
            for (int i = 0; i < 2; ++i) {
                AEtrees.at(i)->BuildIndex("EvRun", "EvNum");
                CLtrees.at(i)->AddFriend(AEtrees.at(i));
            }
        } // End AE MC
        if (i_file == 3) { // AE ISS
            AEtrees.at(2) = (TTree*)files.at(i_file)->Get("ISS");
            AEtrees.at(3) = (TTree*)files.at(i_file)->Get("ISSneg");
            if (!AEtrees.at(2) || !AEtrees.at(3)) {
                printf("AE trees not found in the file %s\n", PathToFile.at(i_file).Data());
                return {};
            }
            for (int i = 2; i < 4; ++i) {
                AEtrees.at(i)->BuildIndex("EvRun", "EvNum");
                CLtrees.at(i)->AddFriend(AEtrees.at(i));
            }
        } // End AE ISS
    } // End loop on files

    // Convert the array to a vector
    for(int i=0; i<int(CLtrees.size()); ++i) vTrees.push_back(CLtrees.at(i));

    array<TString,3> MySel;
    TString RichSel, TotSelection;

    if(!FirstNNs){
        //TOF
        MySel.at(0) = "abs(Rinner)>=1.92 &&"
                    "chi2Coo_tof < 4   && chi2Time_tof < 10 &&"
                    "beta_tof>0.72     && beta_tof<=0.96";
        //NaF
        MySel.at(1) = "abs(Rinner)>=1.92 && hasRich==1 && isNaF==1 &&"
                 "isBorder_rich==0  && kprob_rich>0.01 &&"
                 "1<charge2_rich    && charge2_rich<4 &&"
                 "ringPMTs2_rich>5  && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4 &&"
                 "beta_rich>=0.80   && beta_rich<=0.994";
        //AGL
        MySel.at(2) = "abs(Rinner)>="+TString(RvalAGL.str())+" && hasRich==1 && isNaF==0 &&"
                 "hasGoodImpact==1 && kprob_rich>0.01 &&"
                 "1<charge2_rich   && charge2_rich<4 &&" 
                 "ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4 &&"
                 "beta_rich>=0.96  && beta_rich<=0.999";      
        ///OTHERS   
        RichSel = "abs(Rinner)>=1.92 && hasRich==1 &&"
                  "hasGoodImpact==1  && kprob_rich>0.01 &&"
                  "1<charge2_rich    && charge2_rich<4 &&"
                  "ringPMTs2_rich>5  && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4 &&"
                  "beta_rich>0.75";
        TotSelection = "hasRich==1 && beta_rich>=0.953 && beta_rich <= 0.999 && isNaF==0 && hasGoodImpact==1 && kprob_rich>0.01"
                            "1<charge2_rich && charge2_rich<4 && ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4 &&"
                            "scores>=0.3 && abs(SigmaUpLow)<=0.4 && anomaly_score<=0.8";
    }
    else{
        //TOF
        MySel.at(0)  = "NTrTrack<3 && ACC_AntiCounter<2 && abs(Rinner)>1.2 && "
                        "chi2Time_tof < 10 && "
                        "(hasRich==0 || (hasRich==1 && beta_rich<1 && (ringHits_dir_rich+ringHits_ref_rich)>=4 && beta_consistencyTOF<0.06)) &&"
                        "(beta_tof<0.96 || hasRich == 1) ";
        //NaF
        MySel.at(1)  = "NTrTrack<3 && ACC_AntiCounter<2 && abs(Rinner)>1.2 && "
                        "chi2Time_tof < 10 && "
                        "(hasRich==1 && isNaF==1 && beta_rich<1 && (ringHits_dir_rich+ringHits_ref_rich)>=4 && beta_consistencyTOF<0.06) &&"
                        "(beta_tof<0.96 || hasRich == 1) ";

        //AGL
        MySel.at(2)  = "NTrTrack<3 && ACC_AntiCounter<2 && abs(Rinner)>1.2 && "
                        "chi2Time_tof < 10 && "
                        "(hasRich==1 && isNaF==0 && beta_rich<1 && (ringHits_dir_rich+ringHits_ref_rich)>=4 && beta_consistencyTOF<0.06) &&"
                        "(beta_tof<0.96 || hasRich == 1) ";
        //OTHERS
        RichSel = "abs(Rinner)>1.92 && hasRich==1";
        TotSelection = "scores>=0.3 && abs(SigmaUpLow)<=0.4 && anomaly_score<=0.8 &&"
                        "hasRich==1 && beta_rich>=0.953 && beta_rich <= 0.999 && isNaF==0 && hasGoodImpact==1 && kprob_rich>0.01 &&"
                        "1<charge2_rich && charge2_rich<4 && ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4";
    }
    


    (useMC) ? printf("\n\n ===> \n Models are trained on MC data with UseLossWeights : %d \n ===> \n\n", UseLossWeights) : printf("\n\n ===> \n Models are trained on ISS data \n ===> \n\n");
    
    printf("====>\n");
    printf("Common selections TOF - NaF and AGL:\n");
    printf("====>\n");
    for(int i=0; i<int(MySel.size()); ++i) printf("Selection %d: %s\n", i, MySel.at(i).Data());

    if(verbose) {
        printf("\t RICH selection:\n");
        printf("%s\n", RichSel.Data());
        printf("\t Tighter selection no GM cutoff:\n");
        printf("%s\n\n", TotSelection.Data());
    }
    
    if(GetScores) GetMyScores(vTrees,
                              MySel,    DirName,
                              FirstNNs, useMC);

    printf("Returning the trees vector, N. of trees: %zu\n", vTrees.size());
    printf("Sequence of the trees: \n");
    printf("\t 0: MC_pos \t 1: MC_neg \t 2: ISS_pos \t 3: ISS_neg\n");
    
    return vTrees;
}



void GetTh1CL(TString &Det, TString &Sample, TFile *f, array<TH1F*, nRigbins> &TH1sig,
              array<TH1F*, nRigbins> &TH1bkg, TTree *myT, TString &Sel,
              bool isMC, bool mkDir){
    TString to_draw, path, sel;
    path = Det+"/CL2/"+Sample;
    if(mkDir) f->mkdir(Det.Data());
    f->mkdir(path.Data());
    for(int i=0; i<int(TH1sig.size()); ++i){
        f->cd();
        to_draw = "scores>>+";
        to_draw += TH1sig.at(i)->GetName();
        sel = Sel; sel.Prepend("(");
        ostringstream Rlow, Rhigh;
        Rlow   << fixed << setprecision(3) << Rbins.at(i);
        if(i<int(TH1sig.size())-1){
            Rhigh  << fixed << setprecision(3) << Rbins.at(i+1);
            if(isMC) sel.Append("&& abs(Rinner)>="+TString(Rlow.str())+" && abs(Rinner)<"+TString(Rhigh.str())+" && RigLabel==1)*weight");
            else     sel.Append("&& abs(Rinner)>="+TString(Rlow.str())+" && abs(Rinner)<"+TString(Rhigh.str())+" && RigLabel==1)");
        }
        else{
            Rhigh  << fixed << setprecision(3) << Rbins.at(i);
            if(isMC) sel.Append("&& abs(Rinner)>="+TString(Rhigh.str())+" && RigLabel==1)*weight");
            else     sel.Append("&& abs(Rinner)>="+TString(Rhigh.str())+" && RigLabel==1)");
        }
        myT->Draw(to_draw.Data(),sel.Data(),"goff");
        TH1sig.at(i) = (TH1F*)gDirectory->Get(TH1sig.at(i)->GetName());

        to_draw = "scores>>+";
        to_draw += TH1bkg.at(i)->GetName();
        sel = Sel; sel.Prepend("(");
        if(i<int(TH1sig.size())-1){
            if(isMC) sel.Append("&& abs(Rinner)>="+TString(Rlow.str())+" && abs(Rinner)<"+TString(Rhigh.str())+" && RigLabel==0)*weight");
            else     sel.Append("&& abs(Rinner)>="+TString(Rlow.str())+" && abs(Rinner)<"+TString(Rhigh.str())+" && RigLabel==0)");
        }
        else{
            if(isMC) sel.Append("&& abs(Rinner)>="+TString(Rhigh.str())+" && RigLabel==0)*weight");
            else     sel.Append("&& abs(Rinner)>="+TString(Rhigh.str())+" && RigLabel==0)");
        }
        myT->Draw(to_draw.Data(),sel.Data(),"goff");
        TH1bkg.at(i) = (TH1F*)gDirectory->Get(TH1bkg.at(i)->GetName());

        TH1sig.at(i)->SetLineWidth(LW); TH1sig.at(i)->SetLineColor(kGreen+1);
        TH1sig.at(i)->GetXaxis()->CenterTitle(true); TH1sig.at(i)->GetYaxis()->CenterTitle(true);
        TH1bkg.at(i)->SetLineWidth(LW); TH1bkg.at(i)->SetLineColor(kRed);
        TH1bkg.at(i)->GetXaxis()->CenterTitle(true); TH1bkg.at(i)->GetYaxis()->CenterTitle(true);

        f->cd(path.Data());
        TH1sig.at(i)->Write();
        TH1bkg.at(i)->Write();
    }
    f->cd();
    printf("Done CL2 det %s for sample %s\n", Det.Data(), Sample.Data());
    return;
}



void GetTh1AE(TString &Det, TString &Sample, TFile *f, array<TH1F*, nRigbins> &TH1my,
              TTree *myT, TString &Sel,
              bool isMC){
    TString to_draw, path, sel;
    path = Det+"/AE/"+Sample;
    f->mkdir(path.Data());
    for(int i=0; i<int(TH1my.size()); ++i){
        f->cd();
        to_draw = "anomaly_score>>+";
        to_draw += TH1my.at(i)->GetName();
        sel = Sel; sel.Prepend("(");
        ostringstream Rlow, Rhigh;
        Rlow   << fixed << setprecision(3) << Rbins.at(i);
        if(i<int(TH1my.size())-1){
            Rhigh  << fixed << setprecision(3) << Rbins.at(i+1);
            if(isMC) sel.Append("&& abs(Rinner)>="+TString(Rlow.str())+" && abs(Rinner)<"+TString(Rhigh.str())+")*weight");
            else     sel.Append("&& abs(Rinner)>="+TString(Rlow.str())+" && abs(Rinner)<"+TString(Rhigh.str())+")");
        }
        else{
            Rhigh  << fixed << setprecision(3) << Rbins.at(i);
            if(isMC) sel.Append("&& abs(Rinner)>="+TString(Rhigh.str())+")*weight");
            else     sel.Append("&& abs(Rinner)>="+TString(Rhigh.str())+")");
        }
        myT->Draw(to_draw.Data(),sel.Data(),"goff");
        TH1my.at(i) = (TH1F*)gDirectory->Get(TH1my.at(i)->GetName());

        TH1my.at(i)->SetLineWidth(LW); TH1my.at(i)->SetLineColor(kGreen+1);
        TH1my.at(i)->GetXaxis()->CenterTitle(true); TH1my.at(i)->GetYaxis()->CenterTitle(true);

        f->cd(path.Data());
        TH1my.at(i)->Write();
    }
    f->cd();
    printf("Done AE det %s for sample %s\n", Det.Data(), Sample.Data());
    return;
}


void IntializeCL2_TH1(array<TH1F*,nRigbins> &harra, TString &myXtitle,
                    TString &myYtitle, TString &myTitle,
                    TString &myName){
    for(int i=0; i<nRigbins; ++i){
        if(harra.at(i)!=nullptr) delete harra.at(i);
        harra.at(i) = nullptr;
        TString title =myTitle+";"+myXtitle+";"+myYtitle;
        TString name  =myName+"_bin"+to_string(i);
        harra.at(i) = new TH1F(name.Data(), title.Data(), 200,-0.005,1.005);
    }
    return;
}

void IntializeAE_TH1(array<TH1F*,nRigbins> &harrAE, TString &myXtitle,
                    TString &myYtitle, TString &myTitle,
                    TString &myName){
    for(int i=0; i<nRigbins; ++i){
        if(harrAE.at(i)!=nullptr) delete harrAE.at(i);
        harrAE.at(i) = nullptr;
        TString title=myTitle+";"+myXtitle+";"+myYtitle;
        TString name=myName+"_bin"+to_string(i);
        harrAE.at(i) = new TH1F(name.Data(), title.Data(), 102,0.495,1.005);
    }
    return;
}

void GetMyScores(vector<TTree*> &myTrees, 
                array<TString,3> &Selections, array<TString, TotTree> Samples,
                bool onlyNNs, bool amImc){

    array<TString,3>DetName{"TOF","NaF","AGL"};
    //Creating scores plots for each det and NN
    TString outFileName;
    if(amImc) {
        (onlyNNs) ? outFileName = "scores_TrainOnMC_NNs_NEW.root" : outFileName = "scores_TrainOnMC_beta.root";
    }
    else{
        (onlyNNs) ? outFileName = "scores_TrainOnISS_NNs_NEW.root" : outFileName = "scores_TrainOnISS_beta.root";
    }
    TFile *f_out = new TFile(outFileName.Data(),"RECREATE");
    printf("Creating outfile %s ...\n", outFileName.Data());


    //CL2 hist
    array<TH1F*, nRigbins> hCL2Sig{}, hCL2Bkg{};

    printf("Setting TH1 for CL2 and AE\n");
    array<TString,TotTree> Xtitle{"CL_{ISS} output", "CL_{ISS} output", "CL_{ISS} output", "CL_{ISS} output"};
    if(amImc) for(int i=0; i<TotTree; ++i) Xtitle.at(i)="CL_{MC} output";
    array<TString,TotTree> Ytitle{"Normalised weights", "Normalised weights", "Normalised events", "Normalised events"};
    array <TString,TotTree> SigTitle{"Good reco. (R_{INNER}>0)","Good reco. (R_{INNER}<0)",
                              "Good reco. (R_{INNER}>0)","Good reco. (R_{INNER}<0)"};
    array <TString,TotTree> BkgTitle{"Bad reco. (R_{INNER}>0)","Bad reco. (R_{INNER}<0)",
                              "Bad reco. (R_{INNER}>0)","Bad reco. (R_{INNER}<0)"};

    array<TString, TotTree> CL2SigNameTOF{"He4_MC_RposRig_TOFLabel1", "He4_MC_RnegRig_TOFLabel1", "ISS_RposRig_TOFLabel1", "ISS_RnegRig_TOFLabel1"};
    array<TString, TotTree> CL2BkgNameTOF{"He4_MC_RposRig_TOFLabel0", "He4_MC_RnegRig_TOFLabel0", "ISS_RposRig_TOFLabel0", "ISS_RnegRig_TOFLabel0"};

    array<TString, nRigbins> CL2SigNameNaF{"He4_MC_RposRig_NaFLabel1", "He4_MC_RnegRig_NaFLabel1", "ISS_RposRig_NaFLabel1", "ISS_RnegRig_NaFLabel1"};
    array<TString, nRigbins> CL2BkgNameNaF{"He4_MC_RposRig_NaFLabel0", "He4_MC_RnegRig_NaFLabel0", "ISS_RposRig_NaFLabel0", "ISS_RnegRig_NaFLabel0"};
    
    array<TString, nRigbins> CL2SigNameAGL{"He4_MC_RposRig_AGLLabel1", "He4_MC_RnegRig_AGLLabel1", "ISS_RposRig_AGLLabel1", "ISS_RnegRig_AGLLabel1"};
    array<TString, nRigbins> CL2BkgNameAGL{"He4_MC_RposRig_AGLLabel0", "He4_MC_RnegRig_AGLLabel0", "ISS_RposRig_AGLLabel0", "ISS_RnegRig_AGLLabel0"};


    //AE hist
    array<TH1F*, nRigbins> hAE{};

    array<TString,TotTree> AEXtitle{"Anomaly score (AE_{ISS} output)", "Anomaly score (AE_{ISS} output)",
                                    "Anomaly score (AE_{ISS} output)", "Anomaly score (AE_{ISS} output)"};
    if(amImc) for(int i=0; i<TotTree; ++i) AEXtitle.at(i)="Anomaly score (AE_{MC} output)"; 
    array<TString,TotTree> AETitle{"^{4}He MC (R_{INNER}>0)","^{4}He MC (R_{INNER}<0)",
                              "ISS data (R_{INNER}>0)","ISS data (R_{INNER}<0)"};

    array<TString, nRigbins> AENameTOF{"He4_MC_Rpos_TOF", "He4_MC_Rneg_TOF",
                                       "ISS_Rpos_TOF",    "ISS_Rneg_TOF"};
    array<TString, nRigbins> AENameNaF{"He4_MC_Rpos_NaF", "He4_MC_Rneg_NaF",
                                       "ISS_Rpos_NaF",    "ISS_Rneg_NaF"};
    array<TString, nRigbins> AENameAGL{"He4_MC_Rpos_AGL", "He4_MC_Rneg_AGL",
                                       "ISS_Rpos_AGL",    "ISS_Rneg_AGL"};

    //getting TH1
    bool MCtree = true;
    bool CreateDir = true;
    for(int i=0; i<int(myTrees.size()); i++){
        if(i!=0) CreateDir = false;
        if(i>=2) MCtree = false;
        //TOF CL2
        IntializeCL2_TH1(hCL2Sig,Xtitle.at(i),
                        Ytitle.at(i), SigTitle.at(i),
                        CL2SigNameTOF.at(i));
        IntializeCL2_TH1(hCL2Bkg,Xtitle.at(i),
                        Ytitle.at(i), BkgTitle.at(i),
                        CL2BkgNameTOF.at(i));
        GetTh1CL(DetName.at(0),Samples.at(i),f_out, hCL2Sig,
                hCL2Bkg, myTrees.at(i), Selections.at(0),
                MCtree, CreateDir);
        //TOF AE
        IntializeAE_TH1(hAE,AEXtitle.at(i),
                        Ytitle.at(i), AETitle.at(i),
                        AENameTOF.at(i));
        GetTh1AE(DetName.at(0),Samples.at(i),f_out,hAE,
                    myTrees.at(i), Selections.at(0), MCtree);

        //NaF CL2
        IntializeCL2_TH1(hCL2Sig,Xtitle.at(i),
                        Ytitle.at(i), SigTitle.at(i),
                        CL2SigNameNaF.at(i));
        IntializeCL2_TH1(hCL2Bkg,Xtitle.at(i),
                        Ytitle.at(i), BkgTitle.at(i),
                        CL2BkgNameNaF.at(i));
        GetTh1CL(DetName.at(1),Samples.at(i),f_out,hCL2Sig,
                hCL2Bkg, myTrees.at(i), Selections.at(1),
                MCtree, CreateDir);
        //NaF CL2
        IntializeAE_TH1(hAE,AEXtitle.at(i),
                        Ytitle.at(i), AETitle.at(i),
                        AENameNaF.at(i));
        GetTh1AE(DetName.at(1),Samples.at(i),f_out,hAE,
                    myTrees.at(i), Selections.at(1), MCtree);
        
        //AGL CL2
        IntializeCL2_TH1(hCL2Sig,Xtitle.at(i),
                        Ytitle.at(i), SigTitle.at(i),
                        CL2SigNameAGL.at(i));
        IntializeCL2_TH1(hCL2Bkg,Xtitle.at(i),
                        Ytitle.at(i), BkgTitle.at(i),
                        CL2BkgNameAGL.at(i));
        GetTh1CL(DetName.at(2),Samples.at(i),f_out,hCL2Sig,
                hCL2Bkg, myTrees.at(i), Selections.at(2),
                MCtree, CreateDir); 
        //AGL AE
        IntializeAE_TH1(hAE,AEXtitle.at(i),
                        Ytitle.at(i), AETitle.at(i),
                        AENameAGL.at(i));
        GetTh1AE(DetName.at(2),Samples.at(i),f_out,hAE,
                    myTrees.at(i), Selections.at(2), MCtree);
    }
   
    f_out->Close();

    printf("TH1 plots availables here: %s\n", outFileName.Data());

}
