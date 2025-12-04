/*
----->
To be used only on models trained on MC
----->

Allow to visualize interactively the
trees obtained from the Validation and 
testing. 

Usage:
    root -l

    std::vector<TTree*> clTrees;
    .L fromTerminal_MC.cc+

    ==> on LXPLUS
        // Training with OLD (wrong) Loss weights
    clTrees = fromTerminal_MC("2025_03_13", "2025_03_13", "2025_03_13", "2025_03_13");
        // Training with Loss weights
    clTrees = fromTerminal_MC("2025_03_23", "2025_03_23", "2025_03_23", "2025_03_23");
        // Training without Loss weights
    clTrees = fromTerminal_MC("2025_03_17", "2025_03_17", "2025_03_23", "2025_03_23");

    ==> Locally
    clTrees = fromTerminal_MC("2025_02_27", "2025_03_03", "2025_02_28", "2025_03_06");
*/

#include <vector>
#include <array>
#include <TString.h>
#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>

using namespace std;

std::vector<TTree*> fromTerminal(bool isMC=false,
                                    TString CLtrainOnMCFile="000",
                                    TString CLvalOnISSFile="000",
                                    TString AEtrainOnMCFile="000",
                                    TString AEvalOnISSFile="000",
                                    bool GetScores=false,
                                    bool FirstNNs=false,
                                    bool verbose=false,
                                    vector<TTree*> myTrees = {}
                                ) {
    gStyle->SetPalette(55);//kRainBow palette
    gStyle->SetOptStat(0);
    const int TotTree  = 4; // Number of CL trees
    const int TotFiles = 4; // Number of files
    const bool isLocal = false;
    bool UseLossWeights; // Loss weights have been used in the training
    (isMC) ? UseLossWeights = true : UseLossWeights = false;
    TString inPath;
    (isLocal) ? inPath ="/home/franz/AMS/output/organized_output/scores/" : inPath="/eos/home-f/frrossi/AMS/output/organized_output/scores/"; 
    
    const int LW = 3;
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
                                           "_trainOnMC_AE_MCscores.root",               "_trainOnMC_AE_ISS_scores.root"};
    array<TString, TotFiles> ISStrainName{"_trainOnISS_2CL_MC_scores.root",             "_trainOnISS_2CL_ISS_scores.root",
                                           "_trainOnISS_AE_MC_scores.root",               "_trainOnISS_AE_ISS_scores.root"};

    array<TString, TotFiles> PathToFile{
        inPath + CLtrainOnMCFile + "/" + CLtrainOnMCFile, inPath + CLvalOnISSFile + "/" + CLvalOnISSFile,
        inPath + AEtrainOnMCFile + "/" + AEtrainOnMCFile, inPath + AEvalOnISSFile + "/" + AEvalOnISSFile};
    for(int i=0; i<TotFiles; ++i){
        if(isMC){
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
            } // End CL MC
            if (i_file == 1 && i_tree >= 2) { // CL ISS data
                CLtrees.at(i_tree) = (TTree*)files.at(i_file)->Get(DirName.at(i_tree).Data());
                if (!CLtrees.at(i_tree)) {
                    printf("%s tree not found in the file %s\n", DirName.at(i_tree).Data(), PathToFile.at(i_file).Data());
                    return {};
                }
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
    for(int i=0; i<CLtrees.size(); ++i) myTrees.push_back(CLtrees.at(i));

    TString TOFSel, RichSel, NaFSel, AglSel, TotSelection;

    if(!FirstNNs){
        TOFSel  = "abs(Rinner)>=1.92 &&"
                  "chi2Coo_tof < 4   && chi2Time_tof < 10 &&"
                  "beta_tof>0.72     && beta_tof<=0.96";
        RichSel = "abs(Rinner)>=1.92 && hasRich==1 &&"
                  "hasGoodImpact==1  && kprob_rich>0.01 &&"
                  "1<charge2_rich    && charge2_rich<4 &&"
                  "ringPMTs2_rich>5  && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4 &&"
                  "beta_rich>0.75";
        NaFSel = "abs(Rinner)>=1.92 && hasRich==1 && isNaF==1 &&"
                 "isBorder_rich==0  && kprob_rich>0.01 &&"
                 "1<charge2_rich    && charge2_rich<4 &&"
                 "ringPMTs2_rich>5  && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4 &&"
                 "beta_rich>=0.80   && beta_rich<=0.994";
        AglSel = "abs(Rinner)>="+TString(RvalAGL.str())+" && hasRich==1 && isNaF==0 &&"
                 "hasGoodImpact==1 && kprob_rich>0.01 &&"
                 "1<charge2_rich   && charge2_rich<4 &&" 
                 "ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4 &&"
                 "beta_rich>=0.96  && beta_rich<=0.999";            
        TotSelection = "hasRich==1 && beta_rich>=0.953 && beta_rich <= 0.999 && isNaF==0 && hasGoodImpact==1 && kprob_rich>0.01"
                            "1<charge2_rich && charge2_rich<4 && ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4 &&"
                            "scores_neg>=0.3 && abs(SigmaUpLow)<=0.4 && NEGanomaly_score<=0.8";
        // TString TotSelection = "hasRich==1 && beta_rich>0.953 && beta_rich <0.998 && isNaF==0 && hasGoodImpact==1 && kprob_rich>0.01 &&"
        //                 "1<charge2_rich && charge2_rich<4 && ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.8 &&"
        //                 "-Rinner<100 && abs(SigmaUpLow)<0.5 && ACC_AntiCounter==0 && InnerHit==8 && NEGanomaly_score!=1 && scores_neg>0.09";

    }
    else{
        TOFSel  = "abs(Rinner)>1.92";
        RichSel = "abs(Rinner)>1.92 && hasRich==1";
        NaFSel  = "abs(Rinner)>1.92 && hasRich==1 && isNaF==1";
        AglSel  = "abs(Rinner)>"+TString(RvalAGL.str())+" && hasRich==1 && isNaF==0";
        TotSelection = "scores_neg>=0.3 && abs(SigmaUpLow)<=0.4 && NEGanomaly_score<=0.8 &&"
                        "hasRich==1 && beta_rich>=0.953 && beta_rich <= 0.999 && isNaF==0 && hasGoodImpact==1 && kprob_rich>0.01 &&"
                        "1<charge2_rich && charge2_rich<4 && ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4";
    }
    


    (isMC) ? printf("\n\n ===> \n Models are trained on MC data with UseLossWeights : %d \n ===> \n\n", UseLossWeights) : printf("\n\n ===> \n Models are trained on ISS data \n ===> \n\n");
    
    printf("====>\n");
    printf("Common selections:\n");
    printf("====>\n");

    printf("\t TOF selection:\n ");
    printf("%s\n\n", TOFSel.Data());
    if(verbose) {
        printf("\t RICH selection:\n");
        printf("%s\n", RichSel.Data());
    }
    printf("\t RICH-NaF selection:\n");
    printf("%s\n\n", NaFSel.Data());
    printf("\t RICH-Agl selection:\n");
    printf("%s\n\n", AglSel.Data());
    if(verbose) {
        printf("\t Tighter selection no GM cutoff:\n");
        printf("%s\n\n", TotSelection.Data());
    }

    if(GetScores){
        printf("Creating TH1 for CL2 ...\n");
        //Creating scores plots for each det and NN
        TString outFileName;
        if(isMC) {
            (FirstNNs) ? outFileName = "scores_TrainOnMC_NNs.root" : outFileName = "scores_TrainOnMC_beta.root";
        }
        else{
            (FirstNNs) ? outFileName = "scores_TrainOnISS_NNs.root" : outFileName = "scores_TrainOnISS_beta.root";
        }
        TFile *f_out = new TFile(outFileName.Data(),"RECREATE");
        TString to_draw, sel;

        //CL2 hist
        array<TH1F*, TotTree> hCL2SigTOF, hCL2BkgTOF;
        array<TH1F*, TotTree> hCL2SigNaF, hCL2BkgNaF;
        array<TH1F*, TotTree> hCL2SigAGL, hCL2BkgAGL;
        array<TString, TotTree> CL2SigTrainOnMCTitle{"Good reco. (R_{INNER}>0); CL_{MC} output; Arbitrary units",
                                                    "Good reco. (R_{INNER}<0); CL_{MC} output; Arbitrary units",
                                                    "Good reco. (R_{INNER}>0); CL_{MC} output; Arbitrary units",
                                                    "Good reco. (R_{INNER}>0); CL_{MC} output; Arbitrary units"};
        array<TString, TotTree> CL2BkgTrainOnMCTitle{"Bad reco. (R_{INNER}>0); CL_{MC} output; Arbitrary units",
                                                    "Bad reco. (R_{INNER}<0); CL_{MC} output; Arbitrary units",
                                                    "Bad reco. (R_{INNER}>0); CL_{MC} output; Arbitrary units",
                                                    "Bad reco. (R_{INNER}>0); CL_{MC} output; Arbitrary units"};
        array<TString, TotTree> CL2SigTrainOnISSTitle{"Good reco. (R_{INNER}>0); CL_{ISS} output; Arbitrary units",
                                                    "Good reco. (R_{INNER}<0); CL_{ISS} output; Arbitrary units",
                                                    "Good reco. (R_{INNER}>0); CL_{ISS} output; Arbitrary units",
                                                    "Good reco. (R_{INNER}>0); CL_{ISS} output; Arbitrary units"};
        array<TString, TotTree> CL2BkgTrainOnISSTitle{"Bad reco. (R_{INNER}>0);  CL_{ISS} output; Arbitrary units",
                                                    "Bad reco. (R_{INNER}<0);  CL_{ISS} output; Arbitrary units",
                                                    "Bad reco. (R_{INNER}>0);  CL_{ISS} output; Arbitrary units",
                                                    "bad reco. (R_{INNER}>0);  CL_{ISS} output; Arbitrary units"};

        array<TString, TotTree> CL2SigNameTOF{"He4_MC_RposRig_TOFLabel1", "He4_MC_RnegRig_TOFLabel1", "ISS_RposRig_TOFLabel1", "ISS_RnegRig_TOFLabel1"};
        array<TString, TotTree> CL2SigNameNaF{"He4_MC_RposRig_NaFLabel1", "He4_MC_RnegRig_NaFLabel1", "ISS_RposRig_NaFLabel1", "ISS_RnegRig_NaFLabel1"};
        array<TString, TotTree> CL2SigNameAGL{"He4_MC_RposRig_AGLLabel1", "He4_MC_RnegRig_AGLLabel1", "ISS_RposRig_AGLLabel1", "ISS_RnegRig_AGLLabel1"};
        
        array<TString, TotTree> CL2BkgNameTOF{"He4_MC_RposRig_TOFLabel0", "He4_MC_RnegRig_TOFLabel0", "ISS_RposRig_TOFLabel0", "ISS_RnegRig_TOFLabel0"};
        array<TString, TotTree> CL2BkgNameNaF{"He4_MC_RposRig_NaFLabel0", "He4_MC_RnegRig_NaFLabel0", "ISS_RposRig_NaFLabel0", "ISS_RnegRig_NaFLabel0"};
        array<TString, TotTree> CL2BkgNameAGL{"He4_MC_RposRig_AGLLabel0", "He4_MC_RnegRig_AGLLabel0", "ISS_RposRig_AGLLabel0", "ISS_RnegRig_AGLLabel0"};

        array<TString, TotTree> CL2SigTitle, CL2BkgTitle;
        for(int i=0; i<TotTree; i++) {
            if(isMC){
                CL2SigTitle.at(i) = CL2SigTrainOnMCTitle.at(i);
                CL2BkgTitle.at(i) = CL2BkgTrainOnMCTitle.at(i);
            }
            else{
                CL2SigTitle.at(i) = CL2SigTrainOnISSTitle.at(i);
                CL2BkgTitle.at(i) = CL2BkgTrainOnISSTitle.at(i);
            }
        }

        //Defining TH1 for CL2
        for(int i=0; i<TotTree; i++){
            hCL2SigTOF.at(i) = new TH1F(CL2SigNameTOF.at(i).Data(), CL2SigTitle.at(i).Data(), 102, -0.01, 1.01);
            hCL2BkgTOF.at(i) = new TH1F(CL2BkgNameTOF.at(i).Data(), CL2BkgTitle.at(i).Data(), 102, -0.01, 1.01);
            
            hCL2SigNaF.at(i) = new TH1F(CL2SigNameNaF.at(i).Data(), CL2SigTitle.at(i).Data(), 102, -0.01, 1.01);
            hCL2BkgNaF.at(i) = new TH1F(CL2BkgNameNaF.at(i).Data(), CL2BkgTitle.at(i).Data(), 102, -0.01, 1.01);

            hCL2SigAGL.at(i) = new TH1F(CL2SigNameAGL.at(i).Data(), CL2SigTitle.at(i).Data(), 102, -0.01, 1.01);
            hCL2BkgAGL.at(i) = new TH1F(CL2BkgNameAGL.at(i).Data(), CL2BkgTitle.at(i).Data(), 102, -0.01, 1.01);
        }


        //TOF
        f_out->mkdir("TOF"); f_out->mkdir("TOF/CL2"); 
        for(int i=0; i<TotTree; i++){
            f_out->cd();
            //MC
            if(i<2) {
                if(i%2==0) to_draw = "scores_pos>>+";
                else     to_draw = "scores_neg>>+";
                to_draw += hCL2SigTOF.at(i)->GetName();
                sel     = TOFSel; sel.Prepend("("); sel.Append(" && RigLabel==1)*weight");
                myTrees.at(i)->Draw(to_draw.Data(),sel.Data(),"goff");
                hCL2SigTOF.at(i) = (TH1F*)gDirectory->Get(hCL2SigTOF.at(i)->GetName());

                if(i%2==0) to_draw = "scores_pos>>+";
                else     to_draw = "scores_neg>>+";
                to_draw += hCL2BkgTOF.at(i)->GetName();
                sel     = TOFSel; sel.Prepend("("); sel.Append(" && RigLabel==0)*weight");
                myTrees.at(i)->Draw(to_draw.Data(),sel.Data(),"goff");
                hCL2BkgTOF.at(i) = (TH1F*)gDirectory->Get(hCL2BkgTOF.at(i)->GetName());
            }
            //ISS
            else{
                if(i%2==0) to_draw = "scores_pos>>+";
                else     to_draw = "scores_neg>>+";
                to_draw += hCL2SigTOF.at(i)->GetName();
                sel     = TOFSel; sel.Prepend("("); sel.Append(" && RigLabel==1)");
                myTrees.at(i)->Draw(to_draw.Data(),sel.Data(),"goff");
                hCL2SigTOF.at(i) = (TH1F*)gDirectory->Get(hCL2SigTOF.at(i)->GetName());

                if(i%2==0) to_draw = "scores_pos>>+";
                else     to_draw = "scores_neg>>+";
                to_draw += hCL2BkgTOF.at(i)->GetName();
                sel     = TOFSel; sel.Prepend("("); sel.Append(" && RigLabel==0)");
                myTrees.at(i)->Draw(to_draw.Data(),sel.Data(),"goff");
                hCL2BkgTOF.at(i) = (TH1F*)gDirectory->Get(hCL2BkgTOF.at(i)->GetName());
            }

            hCL2SigTOF.at(i)->SetLineWidth(LW); hCL2SigTOF.at(i)->SetLineColor(kGreen+1);
            hCL2SigTOF.at(i)->GetXaxis()->CenterTitle(true); hCL2SigTOF.at(i)->GetYaxis()->CenterTitle(true);
            hCL2BkgTOF.at(i)->SetLineWidth(LW); hCL2BkgTOF.at(i)->SetLineColor(kRed);
            hCL2BkgTOF.at(i)->GetXaxis()->CenterTitle(true); hCL2BkgTOF.at(i)->GetYaxis()->CenterTitle(true);
            f_out->cd("TOF/CL2");
            hCL2SigTOF.at(i)->Write();
            hCL2BkgTOF.at(i)->Write();
        }
        f_out->cd();
        printf("TOF done!\n");
        

        //NaF
        f_out->mkdir("NaF"); f_out->mkdir("NaF/CL2"); 
        for(int i=0; i<TotTree; i++){
            f_out->cd();
            //MC
            if(i<2) {
                if(i%2==0) to_draw = "scores_pos>>+";
                else     to_draw = "scores_neg>>+";
                to_draw += hCL2SigNaF.at(i)->GetName();
                sel     = NaFSel; sel.Prepend("("); sel.Append(" && RigLabel==1)*weight");
                myTrees.at(i)->Draw(to_draw.Data(),sel.Data(),"goff");
                hCL2SigNaF.at(i) = (TH1F*)gDirectory->Get(hCL2SigNaF.at(i)->GetName());

                if(i%2==0) to_draw = "scores_pos>>+";
                else     to_draw = "scores_neg>>+";
                to_draw += hCL2BkgNaF.at(i)->GetName();
                sel     = NaFSel; sel.Prepend("("); sel.Append(" && RigLabel==0)*weight");
                myTrees.at(i)->Draw(to_draw.Data(),sel.Data(),"goff");
                hCL2BkgNaF.at(i) = (TH1F*)gDirectory->Get(hCL2BkgNaF.at(i)->GetName());
            }
            //ISS
            else{
                if(i%2==0) to_draw = "scores_pos>>+";
                else     to_draw = "scores_neg>>+";
                to_draw += hCL2SigNaF.at(i)->GetName();
                sel     = NaFSel; sel.Prepend("("); sel.Append(" && RigLabel==1)");
                myTrees.at(i)->Draw(to_draw.Data(),sel.Data(),"goff");
                hCL2SigNaF.at(i) = (TH1F*)gDirectory->Get(hCL2SigNaF.at(i)->GetName());

                if(i%2==0) to_draw = "scores_pos>>+";
                else     to_draw = "scores_neg>>+";
                to_draw += hCL2BkgNaF.at(i)->GetName();
                sel     = NaFSel; sel.Prepend("("); sel.Append(" && RigLabel==0)");
                myTrees.at(i)->Draw(to_draw.Data(),sel.Data(),"goff");
                hCL2BkgNaF.at(i) = (TH1F*)gDirectory->Get(hCL2BkgNaF.at(i)->GetName());
            }
            hCL2SigNaF.at(i)->SetLineWidth(LW); hCL2SigNaF.at(i)->SetLineColor(kGreen+1);
            hCL2SigNaF.at(i)->GetXaxis()->CenterTitle(true); hCL2SigNaF.at(i)->GetYaxis()->CenterTitle(true);
            hCL2BkgNaF.at(i)->SetLineWidth(LW); hCL2BkgNaF.at(i)->SetLineColor(kRed);
            hCL2BkgNaF.at(i)->GetXaxis()->CenterTitle(true); hCL2BkgNaF.at(i)->GetYaxis()->CenterTitle(true);
            f_out->cd("NaF/CL2");
            hCL2SigNaF.at(i)->Write();
            hCL2BkgNaF.at(i)->Write();
        }
        f_out->cd();
        printf("NaF done!\n");


        //AGL
        f_out->mkdir("AGL"); f_out->mkdir("AGL/CL2"); 
        for(int i=0; i<TotTree; i++){
            f_out->cd();
            //MC
            if(i<2) {
                if(i%2==0) to_draw = "scores_pos>>+";
                else     to_draw = "scores_neg>>+";
                to_draw += hCL2SigAGL.at(i)->GetName();
                sel     = AglSel; sel.Prepend("("); sel.Append(" && RigLabel==1)*weight");
                myTrees.at(i)->Draw(to_draw.Data(),sel.Data(),"goff");
                hCL2SigAGL.at(i) = (TH1F*)gDirectory->Get(hCL2SigAGL.at(i)->GetName());

                if(i%2==0) to_draw = "scores_pos>>+";
                else     to_draw = "scores_neg>>+";
                to_draw += hCL2BkgAGL.at(i)->GetName();
                sel     = AglSel; sel.Prepend("("); sel.Append(" && RigLabel==0)*weight");
                myTrees.at(i)->Draw(to_draw.Data(),sel.Data(),"goff");
                hCL2BkgAGL.at(i) = (TH1F*)gDirectory->Get(hCL2BkgAGL.at(i)->GetName());
            }
            //ISS
            else{
                if(i%2==0) to_draw = "scores_pos>>+";
                else     to_draw = "scores_neg>>+";
                to_draw += hCL2SigAGL.at(i)->GetName();
                sel     = AglSel; sel.Prepend("("); sel.Append(" && RigLabel==1)");
                myTrees.at(i)->Draw(to_draw.Data(),sel.Data(),"goff");
                hCL2SigAGL.at(i) = (TH1F*)gDirectory->Get(hCL2SigAGL.at(i)->GetName());

                if(i%2==0) to_draw = "scores_pos>>+";
                else     to_draw = "scores_neg>>+";
                to_draw += hCL2BkgAGL.at(i)->GetName();
                sel     = AglSel; sel.Prepend("("); sel.Append(" && RigLabel==0)");
                myTrees.at(i)->Draw(to_draw.Data(),sel.Data(),"goff");
                hCL2BkgAGL.at(i) = (TH1F*)gDirectory->Get(hCL2BkgAGL.at(i)->GetName());
            }    
            hCL2SigAGL.at(i)->SetLineWidth(LW); hCL2SigAGL.at(i)->SetLineColor(kGreen+1);
            hCL2SigAGL.at(i)->GetXaxis()->CenterTitle(true); hCL2SigAGL.at(i)->GetYaxis()->CenterTitle(true);
            hCL2BkgAGL.at(i)->SetLineWidth(LW); hCL2BkgAGL.at(i)->SetLineColor(kRed);
            hCL2BkgAGL.at(i)->GetXaxis()->CenterTitle(true); hCL2BkgAGL.at(i)->GetYaxis()->CenterTitle(true);
            f_out->cd("AGL/CL2");
            hCL2SigAGL.at(i)->Write();
            hCL2BkgAGL.at(i)->Write();
        }
        printf("AGL done!\n");
        f_out->cd();




        printf("Creating TH1 for AE ...\n");
        //AE hist
        array<TH1F*, TotTree/2> hAERposTOF, hAERnegTOF;
        array<TH1F*, TotTree/2> hAERposNaF, hAERnegNaF;
        array<TH1F*, TotTree/2> hAERposAGL, hAERnegAGL;

        array<TString, TotTree/2> AERposTrainOnMCTitle{"^{4}He MC (R_{INNER}>0); Anomaly score (AE_{MC} output); Arbitrary units",
                                                    "ISS data (R_{INNER}>0);  Anomaly score (AE_{MC} output); Arbitrary units"};
        array<TString, TotTree/2> AERnegTrainOnMCTitle{"^{4}He MC (R_{INNER}<0); Anomaly score (AE_{MC} output); Arbitrary units",
                                                    "ISS data (R_{INNER}<0);  Anomaly score (AE_{MC} output); Arbitrary units"};
        array<TString, TotTree/2> AERposTrainOnISSTitle{"^{4}He MC (R_{INNER}>0); Anomaly score (AE_{ISS} output); Arbitrary units",
                                                        "ISS data (R_{INNER}>0);  Anomaly score (AE_{ISS} output); Arbitrary units"};
        array<TString, TotTree/2> AERnegTrainOnISSTitle{"^{4}He MC (R_{INNER}<0); Anomaly score (AE_{ISS} output); Arbitrary units",
                                                        "ISS data (R_{INNER}<0);  Anomaly score (AE_{ISS} output); Arbitrary units"};

        array<TString, TotTree/2> AERposNameTOF{"He4_MC_Rpos_TOF", "ISS_Rpos_TOF"};
        array<TString, TotTree/2> AERposNameNaF{"He4_MC_Rpos_NaF", "ISS_Rpos_NaF"};
        array<TString, TotTree/2> AERposNameAGL{"He4_MC_Rpos_AGL", "ISS_Rpos_AGL"};

        array<TString, TotTree/2> AERnegNameTOF{"He4_MC_Rneg_TOF", "ISS_Rneg_TOF"};
        array<TString, TotTree/2> AERnegNameNaF{"He4_MC_Rneg_NaF", "ISS_Rneg_NaF"};
        array<TString, TotTree/2> AERnegNameAGL{"He4_MC_Rneg_AGL", "ISS_Rneg_AGL"};

        array<TString, TotTree/2> AERposTitle, AERnegTitle;
        for(int i=0; i<TotTree/2; i++) {
            if(isMC){
                AERposTitle.at(i) = AERposTrainOnMCTitle.at(i);
                AERnegTitle.at(i) = AERnegTrainOnMCTitle.at(i);
            }
            else{
                AERposTitle.at(i) = AERposTrainOnISSTitle.at(i);
                AERnegTitle.at(i) = AERnegTrainOnISSTitle.at(i);
            }
        }

        //Defining TH1 for AE
        for(int i=0; i<TotTree/2; i++){
            hAERposTOF.at(i) = new TH1F(AERposNameTOF.at(i).Data(), AERposTitle.at(i).Data(), 52, 0.49, 1.01);
            hAERnegTOF.at(i) = new TH1F(AERnegNameTOF.at(i).Data(), AERnegTitle.at(i).Data(), 52, 0.49, 1.01);
            
            hAERposNaF.at(i) = new TH1F(AERposNameNaF.at(i).Data(), AERposTitle.at(i).Data(), 52, 0.49, 1.01);
            hAERnegNaF.at(i) = new TH1F(AERnegNameNaF.at(i).Data(), AERnegTitle.at(i).Data(), 52, 0.49, 1.01);

            hAERposAGL.at(i) = new TH1F(AERposNameAGL.at(i).Data(), AERposTitle.at(i).Data(), 52, 0.49, 1.01);
            hAERnegAGL.at(i) = new TH1F(AERnegNameAGL.at(i).Data(), AERnegTitle.at(i).Data(), 52, 0.49, 1.01);
        }


            //TOF
        f_out->mkdir("TOF/AE");
        for(int i=0; i<TotTree; i++){
            f_out->cd();
            switch(i){
                case 0: //MC POS
                    to_draw = "anomaly_score>>+";
                    to_draw += hAERposTOF.at(0)->GetName();
                    sel     = TOFSel; sel.Prepend("("); sel.Append(")*weight");
                    myTrees.at(i)->Draw(to_draw.Data(),sel.Data(),"goff");
                    hAERposTOF.at(0) = (TH1F*)gDirectory->Get(hAERposTOF.at(0)->GetName());
                    hAERposTOF.at(0)->SetLineWidth(LW); hAERposTOF.at(0)->SetLineColor(kGreen+1);
                    hAERposTOF.at(0)->GetXaxis()->CenterTitle(true); hAERposTOF.at(0)->GetYaxis()->CenterTitle(true);
                    f_out->cd("TOF/AE");
                    hAERposTOF.at(0)->Write();
                    break;
                case 1: //MC NEG
                    to_draw = "NEGanomaly_score>>+";
                    to_draw += hAERnegTOF.at(0)->GetName();
                    sel     = TOFSel; sel.Prepend("("); sel.Append(")*weight");
                    myTrees.at(i)->Draw(to_draw.Data(),sel.Data(),"goff");
                    hAERnegTOF.at(0) = (TH1F*)gDirectory->Get(hAERnegTOF.at(0)->GetName());
                    hAERnegTOF.at(0)->SetLineWidth(LW); hAERnegTOF.at(0)->SetLineColor(kRed);
                    hAERnegTOF.at(0)->GetXaxis()->CenterTitle(true); hAERnegTOF.at(0)->GetYaxis()->CenterTitle(true);
                    f_out->cd("TOF/AE");
                    hAERnegTOF.at(0)->Write();
                    break; 
                case 2: // ISS POS
                    to_draw = "anomaly_score>>+";
                    to_draw += hAERposTOF.at(1)->GetName();
                    sel     = TOFSel; sel.Prepend("("); sel.Append(")");
                    myTrees.at(i)->Draw(to_draw.Data(),sel.Data(),"goff");
                    hAERposTOF.at(1) = (TH1F*)gDirectory->Get(hAERposTOF.at(1)->GetName());
                    hAERposTOF.at(1)->SetLineWidth(LW); hAERposTOF.at(1)->SetLineColor(kGreen+1);
                    hAERposTOF.at(1)->GetXaxis()->CenterTitle(true); hAERposTOF.at(1)->GetYaxis()->CenterTitle(true);
                    f_out->cd("TOF/AE");
                    hAERposTOF.at(1)->Write();
                    break;
                case 3: // ISS neg
                    to_draw = "NEGanomaly_score>>+";
                    to_draw += hAERnegTOF.at(1)->GetName();
                    sel     = TOFSel; sel.Prepend("("); sel.Append(")");
                    myTrees.at(i)->Draw(to_draw.Data(),sel.Data(),"goff");
                    hAERnegTOF.at(1) = (TH1F*)gDirectory->Get(hAERnegTOF.at(1)->GetName());
                    hAERnegTOF.at(1)->SetLineWidth(LW); hAERnegTOF.at(1)->SetLineColor(kRed);
                    hAERnegTOF.at(1)->GetXaxis()->CenterTitle(true); hAERnegTOF.at(1)->GetYaxis()->CenterTitle(true);
                    f_out->cd("TOF/AE");
                    hAERnegTOF.at(1)->Write();
                    break;
            }
        }
        printf("TOF done!\n");

            //NaF
        f_out->mkdir("NaF/AE");
        for(int i=0; i<TotTree; i++){
            f_out->cd();
            switch(i){
                case 0: //MC POS
                    to_draw = "anomaly_score>>+";
                    to_draw += hAERposNaF.at(0)->GetName();
                    sel     = NaFSel; sel.Prepend("("); sel.Append(")*weight");
                    myTrees.at(i)->Draw(to_draw.Data(),sel.Data(),"goff");
                    hAERposNaF.at(0) = (TH1F*)gDirectory->Get(hAERposNaF.at(0)->GetName());
                    hAERposNaF.at(0)->SetLineWidth(LW); hAERposNaF.at(0)->SetLineColor(kGreen+1);
                    hAERposNaF.at(0)->GetXaxis()->CenterTitle(true); hAERposNaF.at(0)->GetYaxis()->CenterTitle(true);
                    f_out->cd("NaF/AE");
                    hAERposNaF.at(0)->Write();
                    break;
                case 1: //MC NEG
                    to_draw = "NEGanomaly_score>>+";
                    to_draw += hAERnegNaF.at(0)->GetName();
                    sel     = NaFSel; sel.Prepend("("); sel.Append(")*weight");
                    myTrees.at(i)->Draw(to_draw.Data(),sel.Data(),"goff");
                    hAERnegNaF.at(0) = (TH1F*)gDirectory->Get(hAERnegNaF.at(0)->GetName());
                    hAERnegNaF.at(0)->SetLineWidth(LW); hAERnegNaF.at(0)->SetLineColor(kRed);
                    hAERnegNaF.at(0)->GetXaxis()->CenterTitle(true); hAERnegNaF.at(0)->GetYaxis()->CenterTitle(true);
                    f_out->cd("NaF/AE");
                    hAERnegNaF.at(0)->Write();
                    break; 
                case 2: // ISS POS
                    to_draw = "anomaly_score>>+";
                    to_draw += hAERposNaF.at(1)->GetName();
                    sel     = NaFSel; sel.Prepend("("); sel.Append(")");
                    myTrees.at(i)->Draw(to_draw.Data(),sel.Data(),"goff");
                    hAERposNaF.at(1) = (TH1F*)gDirectory->Get(hAERposNaF.at(1)->GetName());
                    hAERposNaF.at(1)->SetLineWidth(LW); hAERposNaF.at(1)->SetLineColor(kGreen+1);
                    hAERposNaF.at(1)->GetXaxis()->CenterTitle(true); hAERposNaF.at(1)->GetYaxis()->CenterTitle(true);
                    f_out->cd("NaF/AE");
                    hAERposNaF.at(1)->Write();
                    break;
                case 3: // ISS neg
                    to_draw = "NEGanomaly_score>>+";
                    to_draw += hAERnegNaF.at(1)->GetName();
                    sel     = NaFSel; sel.Prepend("("); sel.Append(")");
                    myTrees.at(i)->Draw(to_draw.Data(),sel.Data(),"goff");
                    hAERnegNaF.at(1) = (TH1F*)gDirectory->Get(hAERnegNaF.at(1)->GetName());
                    hAERnegNaF.at(1)->SetLineWidth(LW); hAERnegNaF.at(1)->SetLineColor(kRed);
                    hAERnegNaF.at(1)->GetXaxis()->CenterTitle(true); hAERnegNaF.at(1)->GetYaxis()->CenterTitle(true);
                    f_out->cd("NaF/AE");
                    hAERnegNaF.at(1)->Write();
                    break;
            }
        }
        printf("NaF done!\n");

            //AGL
        f_out->mkdir("AGL/AE"); 
        for(int i=0; i<TotTree; i++){
            f_out->cd();
            switch(i){
                case 0: //MC POS
                    to_draw = "anomaly_score>>+";
                    to_draw += hAERposAGL.at(0)->GetName();
                    sel     = AglSel; sel.Prepend("("); sel.Append(")*weight");
                    myTrees.at(i)->Draw(to_draw.Data(),sel.Data(),"goff");
                    hAERposAGL.at(0) = (TH1F*)gDirectory->Get(hAERposAGL.at(0)->GetName());
                    hAERposAGL.at(0)->SetLineWidth(LW); hAERposAGL.at(0)->SetLineColor(kGreen+1);
                    hAERposAGL.at(0)->GetXaxis()->CenterTitle(true); hAERposAGL.at(0)->GetYaxis()->CenterTitle(true);
                    f_out->cd("AGL/AE");
                    hAERposAGL.at(0)->Write();
                    break;
                case 1: //MC NEG
                    to_draw = "NEGanomaly_score>>+";
                    to_draw += hAERnegAGL.at(0)->GetName();
                    sel     = AglSel; sel.Prepend("("); sel.Append(")*weight");
                    myTrees.at(i)->Draw(to_draw.Data(),sel.Data(),"goff");
                    hAERnegAGL.at(0) = (TH1F*)gDirectory->Get(hAERnegAGL.at(0)->GetName());
                    hAERnegAGL.at(0)->SetLineWidth(LW); hAERnegAGL.at(0)->SetLineColor(kRed);
                    hAERnegAGL.at(0)->GetXaxis()->CenterTitle(true); hAERnegAGL.at(0)->GetYaxis()->CenterTitle(true);
                    f_out->cd("AGL/AE");
                    hAERnegAGL.at(0)->Write();
                    break; 
                case 2: // ISS POS
                    to_draw = "anomaly_score>>+";
                    to_draw += hAERposAGL.at(1)->GetName();
                    sel     = AglSel; sel.Prepend("("); sel.Append(")");
                    myTrees.at(i)->Draw(to_draw.Data(),sel.Data(),"goff");
                    hAERposAGL.at(1) = (TH1F*)gDirectory->Get(hAERposAGL.at(1)->GetName());
                    hAERposAGL.at(1)->SetLineWidth(LW); hAERposAGL.at(1)->SetLineColor(kGreen+1);
                    hAERposAGL.at(1)->GetXaxis()->CenterTitle(true); hAERposAGL.at(1)->GetYaxis()->CenterTitle(true);
                    f_out->cd("AGL/AE");
                    hAERposAGL.at(1)->Write();
                    break;
                case 3: // ISS neg
                    to_draw = "NEGanomaly_score>>+";
                    to_draw += hAERnegAGL.at(1)->GetName();
                    sel     = AglSel; sel.Prepend("("); sel.Append(")");
                    myTrees.at(i)->Draw(to_draw.Data(),sel.Data(),"goff");
                    hAERnegAGL.at(1) = (TH1F*)gDirectory->Get(hAERnegAGL.at(1)->GetName());
                    hAERnegAGL.at(1)->SetLineWidth(LW); hAERnegAGL.at(1)->SetLineColor(kRed);
                    hAERnegAGL.at(1)->GetXaxis()->CenterTitle(true); hAERnegAGL.at(1)->GetYaxis()->CenterTitle(true);
                    f_out->cd("AGL/AE");
                    hAERnegAGL.at(1)->Write();
                    break;
            }
        }
        f_out->cd();
        printf("AGL done!\n");
        f_out->Close();

        printf("TH1 plots availables here: %s\n", outFileName.Data());

    }
    


    printf("Returning the trees vector, N. of trees: %d\n", TotTree);
    printf("Sequence of the trees: \n");
    printf("\t 0: MC_pos \t 1: MC_neg \t 2: ISS_pos \t 3: ISS_neg\n");
    
    return myTrees;
}
