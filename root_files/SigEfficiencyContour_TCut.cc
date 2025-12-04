/*
----->
Working both on MC and ISS-trained scores
----->
No GM request on data

Visualize 2d plot of scores in rig bins
The graphical cut based on custom TF1 
are currently commented

Usage:
    ===> Locally (true = TrainOnMC; false = trainOnISS)
    root -l -b -q 'SigEfficiencyContour.cc(false, "2025_05_27","2025_05_27","2025_05_27","2025_05_27")' 

    ==> on LXPLUS
    root -l -b -q 'SigEfficiencyContour.cc(false, "2025_05_25","2025_05_25","2025_05_25","2025_05_25")' 
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
#include <TCutG.h>

using namespace std;

void SigEfficiencyContour(bool MCorData=true, TString CLtrainOnMCFile="000", TString CLvalOnISSFile="000", TString AEtrainOnMCFile="000", TString AEvalOnISSFile="000") {
    gStyle->SetPalette(55); //kRainBow palette
    const bool isMC = MCorData; //True for training on MC, False for training on ISS 
    const bool isLocal = false; //True for debugging
    // Loss weights have been used in the training
    // Safety factor = 1.2
    // cutoff (no safety factor)
    bool UseLossWeights, useSafetyFactor, useCutOff;
    TString isAbIGRFpos,isAbIGRFneg,IGRFpos, IGRFneg;
    if(isMC) {
        UseLossWeights = true;
        useSafetyFactor = false;
        useCutOff = false;
        isAbIGRFpos  = "&& isAbIGRFpos==1"; isAbIGRFneg  = "&& isAbIGRFneg==1";
        IGRFpos = "&& IGRFpos<Rinner"; IGRFneg = "&& abs(IGRFneg)<abs(Rinner)";
    }
    else{
        UseLossWeights = false;
        useSafetyFactor = false;
        useCutOff       = false;
        if(useSafetyFactor) printf("---> Using GM-cutoff with a safety fafctor of 1.2\n");
        if(useCutOff) printf("---> Using Cutoff with a safety fafctor of 1.\n");
    }

    const int TotTree  = 4; // Number of CL trees
    const int TotFiles = 4; // Number of files
    const int nRigbins_ams02He = 39; // Number of RIG bins as AMS-02 He flux
    array<float,nRigbins_ams02He> Rbins{1.92, 2.15, 2.40, 2.67, 2.97, 3.29, 3.64, 4.02, 4.43, 4.88, 5.37, 5.90, 6.47, 7.09, 7.76,
                                        8.48, 9.26, 10.1, 11.0, 12.0, 13.0, 14.1, 15.3, 16.6, 18.0, 19.5, 21.1, 22.8, 24.7, 26.7,
                                        28.8, 31.1, 33.5, 36.1, 38.9, 41.9, 45.1, 48.5, 52.2};
    ostringstream ossLow, ossHigh, ossPerL, ossPerU;

    const int NQuant   = 1000; // Number of quantiles
    // To compute the right index:
    //  percentile * 10 - 1 ----> 5% = 49
    const int PerL = 49; // array index for loweer quantile (hist -> PerL+1)
    const int PerU = 949; // array index for upper quantile (hist -> PerU+1)
    //Preparing the quantiles
    double xp[NQuant], p[NQuant]; //Positions of the quantiles and corresponding prob.
    for(int i=0; i<NQuant; ++i) p[i] = 0.001+(i*0.001);

    TString inPath;
    (isLocal) ? inPath ="/home/franz/AMS/output/organized_output/scores/" : inPath="/eos/home-f/frrossi/AMS/output/organized_output/scores/"; 

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
        if(isMC){
            (UseLossWeights) ? PathToFile.at(i) += WithWeightsName.at(i) : PathToFile.at(i) += NoWeightsName.at(i);
        }
        else PathToFile.at(i) += ISStrainName.at(i);
    }
    TString outPath("/eos/home-f/frrossi/AMS/ams_network/root_files/scores_");
    if(isLocal) outPath = "/home/franz/AMS/ams_network/root_files/";
    (isMC) ? outPath += "TrainOnMC_" : outPath += "TrainOnISS";
    if (isMC) (UseLossWeights) ? outPath += "withW.root" : outPath += ".root";
    else outPath += ".root";
    

    array<TFile*,  TotFiles> files;
    array<TTree*,  TotTree> CLtrees;
    array<TTree*,  TotTree> AEtrees;
    array<TString, TotTree> DirName{"MC_pos", "MC_neg", "ISS_pos", "ISS_neg"};
    vector<double> Lperc, Uperc;
    array<TLegend*, nRigbins_ams02He> leg;
    

    // MC positves
    array<TCanvas*, nRigbins_ams02He> c, cM;
    array<TLine*, nRigbins_ams02He> lineLB, lineUB;
    array<TH1F*, nRigbins_ams02He> h1Mass2Pos;  // mass squared (RICH AGL)
    array<TH1F*, nRigbins_ams02He> h1Mass2PosQ; // quantiles for squared mass (RICH AGL)
    array<TH2F*, nRigbins_ams02He> h2ScoresPos; // AS vs CS
    
        //Selection, Titles and Names for 1D plot on MASS^2 and quantiles
    array<TString, nRigbins_ams02He>SelM2, M2Name, M2NameQ, M2Title;
    array<TString, nRigbins_ams02He>SelScores, ScoresName, ScoresTitle;
        //Selection, Titles and Names for 2D plot on scores
    // array<TF1*,  nRigbins_ams02He> funcLB;   // Lower bound
    // array<TF1*,  nRigbins_ams02He> funcUB;   // Upper bound
    // array<TString, nRigbins_ams02He> funcLBName, funcUBName;
    // array<TString, nRigbins_ams02He> funcLBexp{"(0.7-0.2)/(0.5)*x-0.3", "(0.5-0.15)/0.5*x-0.2",
    //                                   "(0.3-0.12)/0.5*x-0.06", "(0.2-0.07)/0.5*x-0.06"};
    // array<TString, nRigbins_ams02He> funcUBexp{"",                      "(0.9-1.)/0.5*x+1.1",
    //                                   "(0.9-1.)/0.5*x+1.1",    "(0.75-0.9)/0.5*x+1.05"};

    // MC negatives
    array<TCanvas*, nRigbins_ams02He> cNeg;
    array<TH1F*, nRigbins_ams02He>  h1Mass2Neg;
    array<TH2F*, nRigbins_ams02He> h2ScoresNeg, h2ScoresNegCut; 
    array<TString, nRigbins_ams02He> M2NameNeg;
    array<TString, nRigbins_ams02He> ScoresNameNeg;
    // array<TCutG*, nRigbins_ams02He> cutGL, cutGU; // CutG for lower and upper bounds
    // array<TString, nRigbins_ams02He> cutLname, cutUname;

    // ISS positives
    array<TCanvas*, nRigbins_ams02He> cISS;
    array<TH1F*, nRigbins_ams02He>  h1Mass2ISS;
    array<TH2F*, nRigbins_ams02He> h2ScoresISS, h2ScoresISSCut; 
    array<TString, nRigbins_ams02He> M2NameISS;
    array<TString, nRigbins_ams02He> ScoresNameISS;
    // array<TCutG*, nRigbins_ams02He> cutGLISS, cutGUISS; // CutG for lower and upper bounds
    // array<TString, nRigbins_ams02He> cutLnameISS, cutUnameISS;
    
    //ISS negatives
    array<TCanvas*, nRigbins_ams02He> cISSn;
    array<TH1F*, nRigbins_ams02He>  h1Mass2ISSn;
    array<TH2F*, nRigbins_ams02He> h2ScoresISSn, h2ScoresISSnCut; 
    array<TString, nRigbins_ams02He> M2NameISSn;
    array<TString, nRigbins_ams02He> ScoresNameISSn;

    //Setting canvas
    for(int i=0; i<nRigbins_ams02He; ++i) {
        c.at(i) = new TCanvas(Form("cPos_%d", i), Form("cPos_%d", i), 600, 600);
        c.at(i)->SetLogz();
        c.at(i)->SetGrid();

        cM.at(i) = new TCanvas(Form("cM_%d", i), Form("cM_%d", i), 600, 600);
        cM.at(i)->SetLogy();
        cM.at(i)->SetGrid();

        cNeg.at(i) = new TCanvas(Form("cNeg_%d", i), Form("cNeg_%d", i), 600, 600);
        cNeg.at(i)->SetLogz();
        cNeg.at(i)->SetGrid();

        cISS.at(i) = new TCanvas(Form("cISS_%d", i), Form("cISS_%d", i), 600, 600);
        cISS.at(i)->SetLogz();
        cISS.at(i)->SetGrid();

        cISSn.at(i) = new TCanvas(Form("cISSn_%d", i), Form("cISSn_%d", i), 600, 600);
        cISSn.at(i)->SetLogz();
        cISSn.at(i)->SetGrid();
    }

    // Getting trees
    // CLtrees->[MC_pos, MC_neg, ISS_pos, ISS_neg]
    for (int i_file = 0; i_file < TotFiles; i_file++) {  // Loop on files
        if (gSystem->AccessPathName(PathToFile.at(i_file), kFileExists)) {
            printf("The file %s does not exist.\n", PathToFile.at(i_file).Data());
            return;
        }
        printf("The file %s exists.\n", PathToFile.at(i_file).Data());
        files.at(i_file) = new TFile(PathToFile.at(i_file), "READ");
        if (!files.at(i_file)) {
            printf("ERROR!! \n File %s not found\n", PathToFile.at(i_file).Data());
            return;
        }
        files.at(i_file)->cd();
        printf("Reading file ==> %s\n\n", PathToFile.at(i_file).Data());
        for (int i_tree = 0; i_tree < TotTree; i_tree++) {
            if (i_file == 0 && i_tree < 2) { // CL MC
                CLtrees.at(i_tree) = (TTree*)files.at(i_file)->Get(DirName.at(i_tree).Data());
                if (!CLtrees.at(i_tree)) {
                    printf("%s tree not found in the file %s\n", DirName.at(i_tree).Data(), PathToFile.at(i_file).Data());
                    return;
                }
            } // End CL MC
            if (i_file == 1 && i_tree >= 2) { // CL ISS data
                CLtrees.at(i_tree) = (TTree*)files.at(i_file)->Get(DirName.at(i_tree).Data());
                if (!CLtrees.at(i_tree)) {
                    printf("%s tree not found in the file %s\n", DirName.at(i_tree).Data(), PathToFile.at(i_file).Data());
                    return;
                }
            } // End CL ISS
        } // End loop on CLtrees

        if (i_file == 2) { // AE MC
            AEtrees.at(0) = (TTree*)files.at(i_file)->Get("MC");
            AEtrees.at(1) = (TTree*)files.at(i_file)->Get("MCneg");
            if (!AEtrees.at(0) || !AEtrees.at(1)) {
                printf("AE trees not found in the file %s\n", PathToFile.at(i_file).Data());
                return;
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
                return;
            }
            for (int i = 2; i < 4; ++i) {
                AEtrees.at(i)->BuildIndex("EvRun", "EvNum");
                CLtrees.at(i)->AddFriend(AEtrees.at(i));
            }
        } // End AE ISS
    } // End loop on files

    TString RichSel = "hasRich==1 && beta_rich>0.75 && isBorder_rich==1 && "
                      "kprob_rich>0.01 && 1<charge2_rich && charge2_rich<4 && "
                      "ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4 &&"
                      "beta_rich<=0.997";
    TString NaFSel = "hasRich==1 && beta_rich>0.75 && isNaF==1 && "
                     "isBorder_rich==1 && kprob_rich>0.01 && 1<charge2_rich && "
                     "charge2_rich<4 && ringPMTs2_rich>10 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.45 &&"
                     "beta_rich<=0.992";
    TString AglSel = "hasRich==1 && beta_rich>0.953 && isNaF==0 && "
                     "isBorder_rich==1 && kprob_rich>0.01 && 1<charge2_rich && "
                     "charge2_rich<4 && ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4 &&"
                     "beta_rich<=0.997";
    TString TotSelection = "hasRich==1 && beta_rich>0.953 && isNaF==0 && isBorder_rich==1 && kprob_rich>0.01 &&"
                   "1<charge2_rich && charge2_rich<4 && ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4 && beta_rich < 0.997 &&"
                   "-Rinner<100 && abs(SigmaUpLow)<0.5 && ACC_AntiCounter==0 && anomaly_score<=0.9 && scores>0.1";

    TString TotSel, to_draw;
    //MC positives
    for(int i=0; i<nRigbins_ams02He-1; ++i) {
        ossLow.str(""); ossHigh.str(""); ossPerL.str(""); ossPerU.str("");
        ossLow  << fixed << setprecision(2) << Rbins.at(i);
        ossHigh << fixed << setprecision(2) << Rbins.at(i+1); 
        //Getting 1D histograms for:
        //  - AGL mass^2
        //  - AGL mass^2 quantiles
        SelM2.at(i)      = AglSel +" && abs(Rinner)>"+TString(ossLow.str())+" && abs(Rinner)<"+TString(ossHigh.str()) ;
        SelM2.at(i).Prepend("(");  SelM2.at(i).Append(")*weight");
        //MC positives
        M2Name.at(i)  = "h1Mass2Pos_"+to_string(i);
        M2NameQ.at(i)  = "h1Mass2PosQ_"+to_string(i);
        M2Title.at(i) = "R_{INNER}#in("+TString(ossLow.str())+","+TString(ossHigh.str())+") [GV]; Mass^{2}_{AGL} [a.m.u.^2]";
        h1Mass2Pos.at(i) = new TH1F(M2Name.at(i).Data(), M2Title.at(i).Data(), 160, -80, 80);
        to_draw = "pow(2.*Rinner,2)*(1-pow(beta_rich,2))/pow(beta_rich*0.9315,2)>>+"+M2Name.at(i);
        CLtrees.at(0)->Draw(to_draw.Data(), SelM2.at(i).Data(),"goff");
        h1Mass2Pos.at(i) = (TH1F*)gDirectory->Get(h1Mass2Pos.at(i)->GetName());
        h1Mass2Pos.at(i)->GetQuantiles(NQuant, xp, p);
        ossPerL << fixed << setprecision(2) << xp[PerL];
        ossPerU << fixed << setprecision(2) << xp[PerU];
        h1Mass2PosQ.at(i) = new TH1F(M2NameQ.at(i).Data(), M2Title.at(i).Data(), NQuant-1, xp);
        for(int ib=1; ib<=h1Mass2PosQ.at(i)->GetNbinsX(); ++ib) h1Mass2PosQ.at(i)->SetBinContent(h1Mass2PosQ.at(i)->FindFixBin(xp[ib-1]), p[ib-1]);

        //Getting 2D scores histos
        SelScores.at(i) = AglSel +" && abs(Rinner)>"+TString(ossLow.str())+" && abs(Rinner)<"+TString(ossHigh.str())+
                            " && pow(2.*Rinner,2)*(1-pow(beta_rich,2))/pow(beta_rich*0.9315,2)>"+TString(ossPerL.str())+
                            " && pow(2.*Rinner,2)*(1-pow(beta_rich,2))/pow(beta_rich*0.9315,2)<"+TString(ossPerU.str());
        SelScores.at(i).Prepend("("); SelScores.at(i).Append(")*weight");
        ScoresName.at(i) = "h2ScoresPos_"+to_string(i);
        ScoresTitle.at(i) = "R_{INNER}#in("+TString(ossLow.str())+","+TString(ossHigh.str())+") [GV]; Anomaly score; Classifier score";
        h2ScoresPos.at(i) = new TH2F(ScoresName.at(i).Data(), ScoresTitle.at(i).Data(), 52, 0.49, 1.01, 102, -0.01, 1.01);
        to_draw = "scores:anomaly_score>>+"+ScoresName.at(i);
        CLtrees.at(0)->Draw(to_draw.Data(), SelScores.at(i).Data(),"goff");
        h2ScoresPos.at(i) = (TH2F*)gDirectory->Get(h2ScoresPos.at(i)->GetName());

        // funcLBName.at(i) = "funcLB_"+to_string(i)+"_"+to_string(i);
        // funcLB.at(i) = new TF1(funcLBName.at(i).Data(),funcLBexp.at(i).Data(), 0.49, 1.01);
        // funcLB.at(i)->SetLineWidth(6); funcLB.at(i)->SetLineStyle(2);

        c.at(i)->cd();
        h2ScoresPos.at(i)->Draw("colz");
        // funcLB.at(i)->Draw("same");

        // if(i!=0){ //no UB for first rigidity bin
            // funcUBName.at(i) = "funcUB_"+to_string(i)+"_"+to_string(i);
            // funcUB.at(i) = new TF1(funcUBName.at(i).Data(),funcUBexp.at(i).Data(), 0.49, 1.01);
            // funcUB.at(i)->SetLineWidth(6); funcUB.at(i)->SetLineStyle(2);
            // funcUB.at(i)->Draw("same");
        // }
        Lperc.push_back(xp[PerL]); Uperc.push_back(xp[PerU]);
    }
    
    //MC negatives
    for(int i=0; i<nRigbins_ams02He-1; ++i){
        ossLow.str(""); ossHigh.str(""); ossPerL.str(""); ossPerU.str("");
        ossLow  << fixed << setprecision(2) << Rbins.at(i);
        ossHigh << fixed << setprecision(2) << Rbins.at(i+1); 
        //Getting 2D scores histos
        SelScores.at(i) = AglSel +" && abs(Rinner)>"+TString(ossLow.str())+" && abs(Rinner)<"+TString(ossHigh.str());
        SelScores.at(i).Prepend("("); SelScores.at(i).Append(")*weight");
        ScoresNameNeg.at(i) = "h2ScoresNeg_"+to_string(i);
        h2ScoresNeg.at(i) = new TH2F(ScoresNameNeg.at(i).Data(), ScoresTitle.at(i).Data(), 52, 0.49, 1.01, 102, -0.01, 1.01);
        to_draw = "scores:anomaly_score>>+"+ScoresNameNeg.at(i);
        CLtrees.at(1)->Draw(to_draw.Data(), SelScores.at(i).Data(),"goff");
        h2ScoresNeg.at(i) = (TH2F*)gDirectory->Get(h2ScoresNeg.at(i)->GetName());
        ScoresNameNeg.at(i) = "h2ScoresNegCut_"+to_string(i);
        h2ScoresNegCut.at(i) = new TH2F(ScoresNameNeg.at(i).Data(), ScoresTitle.at(i).Data(), 52, 0.49, 1.01, 102, -0.01, 1.01);

        //Cutting using the functions
        // TH1D*hx = h2ScoresNeg.at(i)->ProjectionX();
        // TH1D*hy = h2ScoresNeg.at(i)->ProjectionY();
        // cutLname.at(i) = "Lcut_"+to_string(i);
        // cutUname.at(i) = "Ucut_"+to_string(i);
        // cutGL.at(i) = new TCutG(cutLname.at(i).Data(), hx->GetNbinsX()+3);
        // cutGL.at(i)->SetVarX("anomaly_score");
        // cutGL.at(i)->SetVarY("scores");
        // if(i!=0) {
        //     cutGU.at(i) = new TCutG(cutUname.at(i).Data(), hx->GetNbinsX()+3);
        //     cutGU.at(i)->SetVarX("anomaly_score");
        //     cutGU.at(i)->SetVarY("scores");
        // }
        
        // double InitX = hx->GetBinCenter(1), InitYL = funcLB.at(i)->Eval(InitX);
        // double InitYU;
        // if (i!=0) InitYU = funcUB.at(i)->Eval(InitX);
        // for(int ibx=1; ibx<=hx->GetNbinsX(); ++ibx){
        //     double x   = hx->GetBinCenter(ibx);
        //     double fxL = funcLB.at(i)->Eval(x);
        //     cutGL.at(i)->SetPoint(ibx-1, x, fxL);
        //     if(i!=0) cutGU.at(i)->SetPoint(ibx-1, x, funcUB.at(i)->Eval(x));
        //     for(int iby=1; iby<=hy->GetNbinsX(); ++iby){
        //         double y = hy->GetBinCenter(iby);
        //         if(y>fxL){
        //             if(i==0 && h2ScoresNeg.at(i)->GetBinContent(ibx,iby)!=0)
        //                 h2ScoresNegCut.at(i)->SetBinContent(ibx, iby, h2ScoresNeg.at(i)->GetBinContent(ibx, iby));
        //             if (i!=0 && y<funcUB.at(i)->Eval(x) && h2ScoresNeg.at(i)->GetBinContent(ibx,iby)!=0)
        //                 h2ScoresNegCut.at(i)->SetBinContent(ibx, iby, h2ScoresNeg.at(i)->GetBinContent(ibx, iby));
        //         }
        //     }
        // }
        // cutGL.at(i)->SetPoint(hx->GetNbinsX(), hx->GetBinCenter(hx->GetNbinsX()), hy->GetBinCenter(hy->GetNbinsX()));
        // cutGL.at(i)->SetPoint(hx->GetNbinsX()+1, InitX, hy->GetBinCenter(hy->GetNbinsX()));
        // cutGL.at(i)->SetPoint(hx->GetNbinsX()+2, InitX, InitYL);
        // if(i!=0){
        //     cutGU.at(i)->SetPoint(hx->GetNbinsX(), hx->GetBinCenter(hx->GetNbinsX()), hy->GetBinCenter(1));
        //     cutGU.at(i)->SetPoint(hx->GetNbinsX()+1, InitX, hy->GetBinCenter(1));
        //     cutGU.at(i)->SetPoint(hx->GetNbinsX()+2, InitX, InitYU);
        // }

        // SelM2.at(i) = AglSel +" && abs(Rinner)>"+TString(ossLow.str())+" && abs(Rinner)<"+TString(ossHigh.str()) + "&& "+cutLname.at(i);
        // if(i!=0) SelM2.at(i) += " && "+cutUname.at(i);
        SelM2.at(i) = AglSel +" && abs(Rinner)>"+TString(ossLow.str())+" && abs(Rinner)<"+TString(ossHigh.str());
        SelM2.at(i).Prepend("("); SelM2.at(i).Append(")*weight");
        M2NameNeg.at(i)  = "h1Mass2Neg_"+to_string(i);
        h1Mass2Neg.at(i) = new TH1F(M2NameNeg.at(i).Data(), M2Title.at(i).Data(), 160, -80, 80);
        to_draw = "pow(2.*Rinner,2)*(1-pow(beta_rich,2))/pow(beta_rich*0.9315,2)>>+"+M2NameNeg.at(i);
        CLtrees.at(1)->Draw(to_draw.Data(), SelM2.at(i).Data(),"goff");
        h1Mass2Neg.at(i) = (TH1F*)gDirectory->Get(h1Mass2Neg.at(i)->GetName());
        
        cNeg.at(i)->cd();
        h2ScoresNeg.at(i)->Draw("colz");
        // funcLB.at(i)->Draw("same");
        // if(i!=0) funcUB.at(i)->Draw("same");
    }

    //ISS positives
    for(int i=0; i<nRigbins_ams02He-1; ++i){
        ossLow.str(""); ossHigh.str(""); ossPerL.str(""); ossPerU.str("");
        ossLow  << fixed << setprecision(2) << Rbins.at(i);
        ossHigh << fixed << setprecision(2) << Rbins.at(i+1); 
        //Getting 2D scores histos
        SelScores.at(i) = AglSel +" && abs(Rinner)>"+TString(ossLow.str())+" && abs(Rinner)<"+TString(ossHigh.str());
        if(useSafetyFactor) SelScores.at(i) += isAbIGRFpos;
        if(useCutOff) SelScores.at(i) += IGRFpos;
        ScoresNameISS.at(i) = "h2ScoresISS_"+to_string(i);
        h2ScoresISS.at(i) = new TH2F(ScoresNameISS.at(i).Data(), ScoresTitle.at(i).Data(), 52, 0.49, 1.01, 102, -0.01, 1.01);
        to_draw = "scores:anomaly_score>>+"+ScoresNameISS.at(i);
        CLtrees.at(2)->Draw(to_draw.Data(), SelScores.at(i).Data(),"goff");
        h2ScoresISS.at(i) = (TH2F*)gDirectory->Get(h2ScoresISS.at(i)->GetName());
        ScoresNameISS.at(i) = "h2ScoresISSCut_"+to_string(i);
        h2ScoresISSCut.at(i) = new TH2F(ScoresNameISS.at(i).Data(), ScoresTitle.at(i).Data(), 52, 0.49, 1.01, 102, -0.01, 1.01);

        //Cutting using the functions
        // TH1D*hx = h2ScoresISS.at(i)->ProjectionX();
        // TH1D*hy = h2ScoresISS.at(i)->ProjectionY();
        // cutLnameISS.at(i) = "LcutISS_"+to_string(i);
        // cutUnameISS.at(i) = "UcutISS_"+to_string(i);
        // cutGLISS.at(i) = new TCutG(cutLnameISS.at(i).Data(), hx->GetNbinsX()+3);
        // cutGLISS.at(i)->SetVarX("anomaly_score");
        // cutGLISS.at(i)->SetVarY("scores");
        // if(i!=0) {
        //     cutGUISS.at(i) = new TCutG(cutUnameISS.at(i).Data(), hx->GetNbinsX()+3);
        //     cutGUISS.at(i)->SetVarX("anomaly_score");
        //     cutGUISS.at(i)->SetVarY("scores");
        // }
        
        // double InitX = hx->GetBinCenter(1), InitYL = funcLB.at(i)->Eval(InitX);
        // double InitYU;
        // if (i!=0) InitYU = funcUB.at(i)->Eval(InitX);
        // for(int ibx=1; ibx<=hx->GetNbinsX(); ++ibx){
        //     double x   = hx->GetBinCenter(ibx);
        //     // double fxL = funcLB.at(i)->Eval(x);
        //     cutGLISS.at(i)->SetPoint(ibx-1, x, fxL);
        //     if(i!=0) cutGUISS.at(i)->SetPoint(ibx-1, x, funcUB.at(i)->Eval(x));
        //     for(int iby=1; iby<=hy->GetNbinsX(); ++iby){
        //         double y = hy->GetBinCenter(iby);
        //         if(y>fxL){
        //             if(i==0 && h2ScoresISS.at(i)->GetBinContent(ibx,iby)!=0)
        //                 h2ScoresISSCut.at(i)->SetBinContent(ibx, iby, h2ScoresISS.at(i)->GetBinContent(ibx, iby));
        //             if (i!=0 && y<funcUB.at(i)->Eval(x) && h2ScoresISS.at(i)->GetBinContent(ibx,iby)!=0)
        //                 h2ScoresISSCut.at(i)->SetBinContent(ibx, iby, h2ScoresISS.at(i)->GetBinContent(ibx, iby));
        //         }
        //     }
        // }
        // cutGLISS.at(i)->SetPoint(hx->GetNbinsX(), hx->GetBinCenter(hx->GetNbinsX()), hy->GetBinCenter(hy->GetNbinsX()));
        // cutGLISS.at(i)->SetPoint(hx->GetNbinsX()+1, InitX, hy->GetBinCenter(hy->GetNbinsX()));
        // cutGLISS.at(i)->SetPoint(hx->GetNbinsX()+2, InitX, InitYL);
        // if(i!=0){
        //     cutGUISS.at(i)->SetPoint(hx->GetNbinsX(), hx->GetBinCenter(hx->GetNbinsX()), hy->GetBinCenter(1));
        //     cutGUISS.at(i)->SetPoint(hx->GetNbinsX()+1, InitX, hy->GetBinCenter(1));
        //     cutGUISS.at(i)->SetPoint(hx->GetNbinsX()+2, InitX, InitYU);
        // }

        // SelM2.at(i) = AglSel +" && abs(Rinner)>"+TString(ossLow.str())+" && abs(Rinner)<"+TString(ossHigh.str()) + "&& "+cutLnameISS.at(i);
        // if(i!=0) SelM2.at(i) += " && "+cutUnameISS.at(i);
        SelM2.at(i) = AglSel +" && abs(Rinner)>"+TString(ossLow.str())+" && abs(Rinner)<"+TString(ossHigh.str());
        if(useSafetyFactor) SelM2.at(i) += isAbIGRFpos;
        if(useCutOff) SelM2.at(i) += IGRFpos;
        M2NameISS.at(i)  = "h1Mass2ISS_"+to_string(i);
        h1Mass2ISS.at(i) = new TH1F(M2NameISS.at(i).Data(), M2Title.at(i).Data(), 160, -80, 80);
        to_draw = "pow(2.*Rinner,2)*(1-pow(beta_rich,2))/pow(beta_rich*0.9315,2)>>+"+M2NameISS.at(i);
        CLtrees.at(2)->Draw(to_draw.Data(), SelM2.at(i).Data(),"goff");
        h1Mass2ISS.at(i) = (TH1F*)gDirectory->Get(h1Mass2ISS.at(i)->GetName());
        
        cISS.at(i)->cd();
        h2ScoresISS.at(i)->Draw("colz");
        // funcLB.at(i)->Draw("same");
        // if(i!=0) funcUB.at(i)->Draw("same");
    }

    //ISS negatives
    for(int i=0; i<nRigbins_ams02He-1; ++i){
        ossLow.str(""); ossHigh.str(""); ossPerL.str(""); ossPerU.str("");
        ossLow  << fixed << setprecision(2) << Rbins.at(i);
        ossHigh << fixed << setprecision(2) << Rbins.at(i+1); 
        //Getting 2D scores histos
        SelScores.at(i) = AglSel +" && abs(Rinner)>"+TString(ossLow.str())+" && abs(Rinner)<"+TString(ossHigh.str());
        if(useSafetyFactor) SelScores.at(i) += isAbIGRFpos;
        if(useCutOff) SelScores.at(i) += IGRFpos;
        ScoresNameISSn.at(i) = "h2ScoresISSn_"+to_string(i);
        h2ScoresISSn.at(i) = new TH2F(ScoresNameISSn.at(i).Data(), ScoresTitle.at(i).Data(), 52, 0.49, 1.01, 102, -0.01, 1.01);
        to_draw = "scores:anomaly_score>>+"+ScoresNameISSn.at(i);
        CLtrees.at(3)->Draw(to_draw.Data(), SelScores.at(i).Data(),"goff");
        h2ScoresISSn.at(i) = (TH2F*)gDirectory->Get(h2ScoresISSn.at(i)->GetName());
        ScoresNameISSn.at(i) = "h2ScoresISSnCut_"+to_string(i);
        h2ScoresISSnCut.at(i) = new TH2F(ScoresNameISSn.at(i).Data(), ScoresTitle.at(i).Data(), 52, 0.49, 1.01, 102, -0.01, 1.01);

        //Cutting using the functions
        // TH1D*hx = h2ScoresISSn.at(i)->ProjectionX();
        // TH1D*hy = h2ScoresISSn.at(i)->ProjectionY();
        // for(int ibx=1; ibx<=hx->GetNbinsX(); ++ibx){
        //     double x   = hx->GetBinCenter(ibx);
        //     // double fxL = funcLB.at(i)->Eval(x);
        //     for(int iby=1; iby<=hy->GetNbinsX(); ++iby){
        //         double y = hy->GetBinCenter(iby);
        //         if(y>fxL){
        //             if(i==0 && h2ScoresISSn.at(i)->GetBinContent(ibx,iby)!=0)
        //                 h2ScoresISSnCut.at(i)->SetBinContent(ibx, iby, h2ScoresISSn.at(i)->GetBinContent(ibx, iby));
        //             if (i!=0 && y<funcUB.at(i)->Eval(x) && h2ScoresISSn.at(i)->GetBinContent(ibx,iby)!=0)
        //                 h2ScoresISSnCut.at(i)->SetBinContent(ibx, iby, h2ScoresISSn.at(i)->GetBinContent(ibx, iby));
        //         }
        //     }
        // }

        // SelM2.at(i) = AglSel +" && abs(Rinner)>"+TString(ossLow.str())+" && abs(Rinner)<"+TString(ossHigh.str()) + "&& "+cutLname.at(i);
        // if(i!=0) SelM2.at(i) += " && "+cutUname.at(i);
        SelM2.at(i) = AglSel +" && abs(Rinner)>"+TString(ossLow.str())+" && abs(Rinner)<"+TString(ossHigh.str());
        if(useSafetyFactor) SelM2.at(i) += isAbIGRFpos;
        if(useCutOff) SelM2.at(i) += IGRFpos;
        M2NameISSn.at(i)  = "h1Mass2ISSn_"+to_string(i);
        h1Mass2ISSn.at(i) = new TH1F(M2NameISSn.at(i).Data(), M2Title.at(i).Data(), 160, -80, 80);
        to_draw = "pow(2.*Rinner,2)*(1-pow(beta_rich,2))/pow(beta_rich*0.9315,2)>>+"+M2NameISSn.at(i);
        CLtrees.at(3)->Draw(to_draw.Data(), SelM2.at(i).Data(),"goff");
        h1Mass2ISSn.at(i) = (TH1F*)gDirectory->Get(h1Mass2ISSn.at(i)->GetName());
        
        cISSn.at(i)->cd();
        h2ScoresISSn.at(i)->Draw("colz");
        // funcLB.at(i)->Draw("same");
        // if(i!=0) funcUB.at(i)->Draw("same");
    }
    
    
    printf("====>\n");
    printf("Common selections:\n");
    printf("====>\n");

    printf("\t RICH selection:\n\n");
    printf("%s\n\n\n", RichSel.Data());
    printf("\t RICH-NaF selection:\n\n");
    printf("%s\n\n\n", NaFSel.Data());
    printf("\t RICH-Agl selection:\n\n");
    printf("%s\n\n\n", AglSel.Data());
    printf("\t Tighter selection no GM cutoff:\n\n");
    printf("%s\n\n\n", TotSelection.Data());

    TFile *fOut = new TFile(outPath.Data(),"RECREATE");
    const int NDir = 4, NSubDirPos = 5, NSubDirNeg = 3, NSubDirISS = 3, NSubDirISSn = 3;
    array<TString, NDir> OutDir{"MCpos", "MCneg", "ISSpos", "ISSneg"};
    array<TString, NSubDirPos> SubDirPos{"MCscores", "MCscoresCutFunc","Mass2","Mass2Quantiles", "Canvas"};
    for(int i=0; i<NSubDirPos; ++i) SubDirPos.at(i).Prepend(OutDir.at(0)+"/");
    array<TString, NSubDirNeg> SubDirNeg{"MCscores","Mass2", "Canvas"};
    for(int i=0; i<NSubDirNeg; ++i) SubDirNeg.at(i).Prepend(OutDir.at(1)+"/");
    array<TString, NSubDirISS> SubDirISS{"ISSscores","Mass2", "Canvas"};
    for(int i=0; i<NSubDirISS; ++i) SubDirISS.at(i).Prepend(OutDir.at(2)+"/");
    array<TString, NSubDirISSn> SubDirISSn{"ISSscores","Mass2", "Canvas"};
    for(int i=0; i<NSubDirISSn; ++i) SubDirISSn.at(i).Prepend(OutDir.at(3)+"/");
    for(int iDir=0; iDir<NDir; ++iDir){
        fOut->mkdir(OutDir.at(iDir).Data());
        switch (iDir){
            case 0:
                for(int isub=0; isub<NSubDirPos; ++isub) {
                    fOut->mkdir(SubDirPos.at(isub).Data());
                    fOut->cd(SubDirPos.at(isub).Data());
                    for(int cc=0; cc<nRigbins_ams02He-1; ++cc){
                        switch(isub){
                            case 0:
                                h2ScoresPos.at(cc)->Write();
                                break;
                            case 1:
                                // funcLB.at(cc)->Write();
                                // if(cc!=0) funcUB.at(cc)->Write();
                                break;
                            case 2:
                                h1Mass2Pos.at(cc)->Write();
                                break;
                            case 3:
                                h1Mass2PosQ.at(cc)->Write();
                                break;
                            case 4:
                                c.at(cc)->Write();
                                break;
                        }
                    }
                }
                break;

            case 1:
                for(int isub=0; isub<NSubDirNeg; ++isub) {
                    fOut->mkdir(SubDirNeg.at(isub).Data());
                    fOut->cd(SubDirNeg.at(isub).Data());
                    for(int cc=0; cc<nRigbins_ams02He-1; ++cc){
                        switch(isub){
                            case 0:
                                h2ScoresNeg.at(cc)->Write();
                                h2ScoresNegCut.at(cc)->Write();
                                break;
                            case 1:
                                h1Mass2Neg.at(cc)->Write();
                                break;
                            case 2:
                                cNeg.at(cc)->Write();
                                break;
                        }
                    }
                }
                break;

            case 2:
                for(int isub=0; isub<NSubDirISS; ++isub) {
                    fOut->mkdir(SubDirISS.at(isub).Data());
                    fOut->cd(SubDirISS.at(isub).Data());
                    for(int cc=0; cc<nRigbins_ams02He-1; ++cc){
                        switch(isub){
                            case 0:
                                h2ScoresISS.at(cc)->Write();
                                h2ScoresISSCut.at(cc)->Write();
                                break;
                            case 1:
                                h1Mass2ISS.at(cc)->Write();
                                break;
                            case 2:
                                cISS.at(cc)->Write();
                                break;
                        }
                    }
                }
                break;
            
                case 3:
                    for(int isub=0; isub<NSubDirISSn; ++isub) {
                        fOut->mkdir(SubDirISSn.at(isub).Data());
                        fOut->cd(SubDirISSn.at(isub).Data());
                        for(int cc=0; cc<nRigbins_ams02He-1; ++cc){
                            switch(isub){
                                case 0:
                                    h2ScoresISSn.at(cc)->Write();
                                    h2ScoresISSnCut.at(cc)->Write();
                                    break;
                                case 1:
                                    h1Mass2ISSn.at(cc)->Write();
                                    break;
                                case 2:
                                    cISSn.at(cc)->Write();
                                    break;
                            }
                        }
                    }
                    break;
        }
        fOut->cd();
    }

    for(int i=0; i<nRigbins_ams02He-1; ++i){
        cM.at(i)->cd();

        h1Mass2ISS.at(i)->Scale(1.e+3); //taking into account the prescaling
        h1Mass2ISS.at(i)->SetLineColor(kTeal+3); h1Mass2ISS.at(i)->SetMarkerColor(kTeal+3); h1Mass2ISS.at(i)->SetMarkerStyle(22);
        h1Mass2ISS.at(i)->Rebin(2);
        h1Mass2ISS.at(i)->GetYaxis()->SetRangeUser(0.1, h1Mass2ISS.at(i)->GetMaximum()*1.1);
        h1Mass2ISS.at(i)->Draw("e ");
        h1Mass2ISSn.at(i)->SetLineColor(kRed); h1Mass2ISSn.at(i)->SetMarkerColor(kRed); h1Mass2ISSn.at(i)->SetMarkerStyle(22);
        h1Mass2ISSn.at(i)->Rebin(2);
        h1Mass2ISSn.at(i)->Draw("e same");

        // h1Mass2Pos.at(i)->Scale(h1Mass2ISS.at(i)->Integral()/h1Mass2Pos.at(i)->Integral());
        h1Mass2Pos.at(i)->SetLineColor(kGreen+1); h1Mass2Pos.at(i)->SetFillColor(kGreen);
        h1Mass2Pos.at(i)->SetFillStyle(3003);
        h1Mass2Pos.at(i)->Rebin(2);
        h1Mass2Pos.at(i)->Draw("hist same");
        //h1Mass2Pos.at(i)->GetYaxis()->SetRangeUser(0.1, h1Mass2Pos.at(i)->GetMaximum()*1.1);
        
        // h1Mass2Neg.at(i)->Scale(h1Mass2ISSn.at(i)->Integral()/h1Mass2Neg.at(i)->Integral());
        h1Mass2Neg.at(i)->SetLineColor(kPink); h1Mass2Neg.at(i)->SetFillColor(kRed);
        h1Mass2Neg.at(i)->SetFillStyle(3004);
        h1Mass2Neg.at(i)->Rebin(2);
        h1Mass2Neg.at(i)->Draw("hist same");
        

        lineLB.at(i) = new TLine(Lperc.at(i), 0.1, Lperc.at(i), h1Mass2ISS.at(i)->GetMaximum()*1.1);
        lineLB.at(i)->SetLineWidth(6); lineLB.at(i)->SetLineStyle(2); lineLB.at(i)->SetLineColor(kMagenta);
        lineLB.at(i)->Draw("same");
        lineUB.at(i) = new TLine(Uperc.at(i), 0.1, Uperc.at(i), h1Mass2ISS.at(i)->GetMaximum()*1.1);
        lineUB.at(i)->SetLineWidth(6); lineUB.at(i)->SetLineStyle(2); lineUB.at(i)->SetLineColor(kMagenta);
        lineUB.at(i)->Draw("same");

        TString tForLeg = "R_{INNER}#in["+TString(ossLow.str())+","+TString(ossHigh.str())+"]";
        leg.at(i) = new TLegend(0.1,0.7,0.48,0.9);
        leg.at(i)->SetHeader(tForLeg.Data(),"C"); // option "C" allows to center the header
        // leg.at(i)->SetNColumns(2);
        leg.at(i)->AddEntry(h1Mass2Pos.at(i),"MC ^{4}He (R>0)","l");
        leg.at(i)->AddEntry(h1Mass2Neg.at(i),"MC ^{4}He (R<0)","l");
        leg.at(i)->AddEntry(h1Mass2ISS.at(i),"ISS data (R>0)","lep");
        leg.at(i)->AddEntry(h1Mass2ISSn.at(i),"ISS data (R>0)","lep");
        leg.at(i)->Draw("same");
        cM.at(i)->Write();
    }
    fOut->mkdir("GraphCuts");
    fOut->cd("GraphCuts");
    // for(int i=0; i<nRigbins_ams02He-1; ++i){
    //     cutGL.at(i)->Write();
    //     cutGLISS.at(i)->Write();
    //     if(i!=0) {
    //         cutGU.at(i)->Write();
    //         cutGUISS.at(i)->Write();
    //     }
    // }
    printf("Output file in %s!\n", outPath.Data());
    return;
}
