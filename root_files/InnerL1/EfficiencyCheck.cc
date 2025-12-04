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

#include <iostream>
#include <fstream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <array>
#include <vector>

#include <TDirectory.h>
#include <TString.h>
#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>
#include <TH1.h>

using namespace std;
vector<TTree*> fromTerminal(bool isMC=false,
                                    TString CLtrainOnMCFile="000",
                                    TString CLvalOnISSFile="000",
                                    TString AEtrainOnMCFile="000",
                                    TString AEvalOnISSFile="000",
                                    vector<TTree*> myTrees = {}
                                );
TH1F* DivideHistograms(TH1F* h1, TH1F* h2, TString hName="", TString hTitle="");

void EfficiencyCheck(){
    gStyle->SetPalette(55);//kRainBow palette
    gStyle->SetOptStat(0);
    gROOT->ForceStyle();
    bool isMinSel = true;
    TDirectory *DefDir = gDirectory;
    const int NDet= 3;
    const int LW  = 3;
    const float he3 = 2.81, he4 = 3.72; // GeV/c2
    const float bNaF_thL = 0.75,  bNaF_thH = 0.993;
    const float bAGL_thL = 0.953, bAGL_thH = 0.998;
    //NaF
    const float r3NaF_thL = he3 * bNaF_thL * 1./sqrt(1-pow(bNaF_thL,2)) / 2;
    //AGL
    const float r3AGL_thL = he3 * bAGL_thL * 1./sqrt(1-pow(bAGL_thL,2)) / 2;

    const int ASbins = 52;   const float ASmin = 0.49,   ASmax = 1.01; // Anomaly score
    const int CL2bins = 102; const float CL2min = -0.01, CL2max = 1.01; // CL2 score

    vector<TTree*> ISS, MC;
    ISS = fromTerminal(false, "2025_06_30", "2025_06_30", "2025_06_30", "2025_06_30");
    MC  = fromTerminal(true,  "2025_06_30", "2025_06_30", "2025_06_30", "2025_06_30");
    DefDir->cd();


    ostringstream RvalNaF; RvalNaF  << fixed << setprecision(3) << r3NaF_thL;
    ostringstream RvalAGL; RvalAGL  << fixed << setprecision(3) << r3AGL_thL;
    array<TString, NDet> R3abTH{"&& abs(Rinner)-(3*pow(Rinner,2)*invRerr_IN)>=1.92",
        " && abs(Rinner)-(3*pow(Rinner,2)*invRerr_IN)>="+TString(RvalNaF.str()),
        " && abs(Rinner)-(3*pow(Rinner,2)*invRerr_IN)>="+TString(RvalAGL.str())};

    array<TString, NDet> Sel{
        "abs(Rinner)>=1.92 && "
            "chi2Coo_tof < 4   && chi2Time_tof < 10 && "
            "beta_tof>0.72     && beta_tof<=0.96",
        "abs(Rinner)>=1.92 && hasRich==1 && isNaF==1 && "
            "isBorder_rich==0  && kprob_rich>0.01 && "
            "1<charge2_rich    && charge2_rich<4 && "
            "ringPMTs2_rich>5  && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4 && "
            "beta_rich>=0.80   && beta_rich<=0.994",
        "abs(Rinner)>="+TString(RvalAGL.str())+" && hasRich==1 && isNaF==0 && "
            "hasGoodImpact==1 && kprob_rich>0.01 && "
            "1<charge2_rich   && charge2_rich<4 && " 
            "ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4 && "
            "beta_rich>=0.96  && beta_rich<=0.999"
        };

    array<TString, NDet> MinSel{
        "abs(Rinner)>=1.92",
        "abs(Rinner)>=1.92 && hasRich==1 && isNaF==1",
        "abs(Rinner)>="+TString(RvalAGL.str())+" && hasRich==1 && isNaF==0"
        };
    if(isMinSel) for(int i=0; i<NDet; ++i) Sel.at(i) = MinSel.at(i);
    

    //Scores TH1
    TString ScoreFileName;
    TFile *f_MCs, *f_ISSs;
    if(isMinSel){
        ScoreFileName = "scores_TrainOnMC_NNs.root";  f_MCs  = new TFile(ScoreFileName.Data(),"OPEN");
        ScoreFileName = "scores_TrainOnISS_NNs.root"; f_ISSs = new TFile(ScoreFileName.Data(),"OPEN");
    }
    else{
        ScoreFileName = "scores_TrainOnMC_beta.root";  f_MCs  = new TFile(ScoreFileName.Data(),"OPEN");
        ScoreFileName = "scores_TrainOnISS_beta.root"; f_ISSs = new TFile(ScoreFileName.Data(),"OPEN");
    }
    array<TH1F*, NDet> hCL2SigISS, hCL2SigMC;
    array<TH1F*, NDet> hAESigISS,  hAESigMC;
    array<TString, NDet> dets{"TOF", "NaF", "AGL"};

    array<TString, NDet> CL2SigNameISS {"ISS_RposRig_TOFLabel1",   "ISS_RposRig_NaFLabel1",    "ISS_RposRig_AGLLabel1"};
    array<TString, NDet> CL2SigNameMC {"He4_MC_RposRig_TOFLabel1", "He4_MC_RposRig_NaFLabel1", "He4_MC_RposRig_AGLLabel1"};
    array<TString, NDet> AESigNameISS {"ISS_Rpos_TOF",   "ISS_Rpos_NaF",    "ISS_Rpos_AGL"};
    array<TString, NDet> AESigNameMC {"He4_MC_Rpos_TOF", "He4_MC_Rpos_NaF", "He4_MC_Rpos_AGL"};

    //Efficiency cuts on scores
    const int Ncut = 5;
    double pCL2[Ncut]{1-0.95, 1-0.90, 1-0.85, 1-0.8, 1-0.67}; //1-0.95 percentile -> 95% efficiency on RigLabel=1
    double pAE[Ncut]{0.95, 0.90, 0.85, 0.80, 0.67};//95 percentile -> 90% efficiency on R>0 anomaly score
    double CL2cut[Ncut]{0,0,0,0,0}, AEcut[Ncut]{0,0,0,0,0}; //scores>=CL2cut && anomaly<=AEcut
    array<ostringstream, Ncut> CL2cutISS, CL2cutMC, AEcutISS, AEcutMC;
    ostringstream effCut;
    array<TString, Ncut> ISSasSel, ISScl2Sel, MCasSel, MCcl2Sel;
    

    //Drawings lines
    array<TString, Ncut> ISSas_toDraw, ISScl2_toDraw, MCas_toDraw, MCcl2_toDraw;
    TString DrawAS("anomaly_score>>+"), DrawCL2("scores_pos>>+");

    //Canvas
    const int Nhist = 4;
    array<TCanvas*,Nhist> canva, c_rat;
    array<TLegend*,Nhist> legend, l_rat;
    array<TString, Nhist> cNames{"ISSas", "MCas", "ISScl2", "MCcl2"};
    //Scores histograms
    array<TH1F*, Ncut> hISSas, hISScl2, hMCas, hMCcl2;
    array<TH1F*, Ncut> hISSasR, hISScl2R, hMCasR, hMCcl2R;
    array<TString, NDet> ISSasBase  {"TOF_ISSas",  "NaF_ISSas",  "AGL_ISSas"};
    array<TString, NDet> ISScl2Base{"TOF_ISScl2", "NaF_ISScl2", "AGL_ISScl2"};
    array<TString, NDet> MCasBase  {"TOF_MCas",  "NaF_MCas",  "AGL_MCas"};
    array<TString, NDet> MCcl2Base {"TOF_MCcl2", "NaF_MCcl2", "AGL_MCcl2"};
    array<TString, Ncut> ISSasName, ISScl2Name, MCasName, MCcl2Name;
    array<TString, Ncut> ISSasNameR, ISScl2NameR, MCasNameR, MCcl2NameR;
    TString ISSasTitle (";  ISS anomaly score; Arbitrary units");
    TString ISScl2Title(";  ISS classifier score; Arbitrary units");
    TString MCasTitle  ("; MC anomaly score; Arbitrary units");
    TString MCcl2Title ("; MC classifier score; Arbitrary units");

    TString ISSasTitleR (";  ISS anomaly score; Arbitrary units");
    TString ISScl2TitleR(";  ISS classifier score; Arbitrary units");
    TString MCasTitleR  ("; MC anomaly score; Arbitrary units");
    TString MCcl2TitleR ("; MC classifier score; Arbitrary units");


    TFile *f_out = new TFile("/eos/home-f/frrossi/AMS/EfficiencyCheck_POS.root","RECREATE");
    
    for(int idet=0; idet<NDet; idet++){ //loop on detectors

        TString dir = dets.at(idet)+"/CL2/"+CL2SigNameISS.at(idet);
        hCL2SigISS.at(idet)  = (TH1F*)f_ISSs->Get(dir.Data());
        hCL2SigISS.at(idet)->GetQuantiles(Ncut, CL2cut, pCL2);
        for(int icut=0; icut<Ncut; ++icut) CL2cutISS.at(icut) << fixed << setprecision(5) << CL2cut[icut];

        dir = dets.at(idet)+"/CL2/"+CL2SigNameMC.at(idet);
        hCL2SigMC.at(idet) = (TH1F*)f_MCs->Get(dir.Data());
        hCL2SigMC.at(idet)->GetQuantiles(Ncut, CL2cut, pCL2);      
        for(int icut=0; icut<Ncut; ++icut) CL2cutMC.at(icut)  << fixed << setprecision(5) << CL2cut[icut];

        dir = dets.at(idet)+"/AE/"+AESigNameISS.at(idet);
        hAESigISS.at(idet) = (TH1F*)f_ISSs->Get(dir.Data());
        hAESigISS.at(idet)->GetQuantiles(Ncut, AEcut, pAE);
        for(int icut=0; icut<Ncut; ++icut) AEcutISS.at(icut)  << fixed << setprecision(5) << AEcut[icut];

        dir = dets.at(idet)+"/AE/"+AESigNameMC.at(idet);
        hAESigMC.at(idet) = (TH1F*)f_MCs->Get(dir.Data());
        hAESigMC.at(idet)->GetQuantiles(Ncut, AEcut, pAE);
        for(int icut=0; icut<Ncut; ++icut) AEcutMC.at(icut)   << fixed << setprecision(5) << AEcut[icut];

        printf("CL2: %f, %f, \n AE: %f, %f\n\n",
                hCL2SigISS.at(idet)->GetEntries(),  hCL2SigMC.at(idet)->GetEntries(),
                hAESigISS.at(idet)->GetEntries(),   hAESigMC.at(idet)->GetEntries());
            
        //Canvas
        for(int i=0; i<Nhist; ++i){
            TString c_name = TString("c_")+dets.at(idet)+"_"+cNames.at(i);
            canva.at(i) = new TCanvas(c_name.Data(), c_name.Data(), 600, 600);
            canva.at(i)->cd(); canva.at(i)->SetGrid(); canva.at(i)->SetLogy();
            c_name = TString("c_Ratio_")+dets.at(idet)+"_"+cNames.at(i);
            c_rat.at(i) = new TCanvas(c_name.Data(), c_name.Data(), 600, 600);
            c_rat.at(i)->cd(); c_rat.at(i)->SetGrid(); 
        }
        

        printf("===> %s\n", dets.at(idet).Data());
        printf("Common selection: %s \n", Sel.at(idet).Data()); 


        for(int icut=0; icut<Ncut; ++icut){ //loop on cuts values

            DefDir->cd();
            effCut.str("");
            effCut << fixed << setprecision(2) << pAE[icut];
            TString myTitle("eff. "+effCut.str());
            ISSasName.at(icut) = ISSasBase.at(idet)+"_"+effCut.str(); ISScl2Name.at(icut) = ISScl2Base.at(idet)+"_"+effCut.str();
            MCasName.at(icut)  = MCasBase.at(idet)+"_"+effCut.str();  MCcl2Name.at(icut)  = MCcl2Base.at(idet)+"_"+effCut.str();
            //Definition of score histograms
            hISSas.at(icut)  = new TH1F(ISSasName.at(icut).Data(),  ISSasTitle.Data(),  ASbins, ASmin, ASmax);
            hISSas.at(icut)->SetTitle(myTitle);
            hISScl2.at(icut) = new TH1F(ISScl2Name.at(icut).Data(), ISScl2Title.Data(), CL2bins, CL2min, CL2max);
            hISScl2.at(icut)->SetTitle(myTitle);
            hMCas.at(icut)   = new TH1F(MCasName.at(icut).Data(),   MCasTitle.Data(),   ASbins, ASmin, ASmax);
            hMCas.at(icut)->SetTitle(myTitle);
            hMCcl2.at(icut)  = new TH1F(MCcl2Name.at(icut).Data(),  MCcl2Title.Data(),  CL2bins, CL2min, CL2max);
            hMCcl2.at(icut)->SetTitle(myTitle);

            ISSasNameR.at(icut) = ISSasBase.at(idet)+"_Ratio_"+effCut.str(); ISScl2NameR.at(icut) = ISScl2Base.at(idet)+"_Ratio_"+effCut.str();
            MCasNameR.at(icut)  = MCasBase.at(idet)+"_Ratio_"+effCut.str();  MCcl2NameR.at(icut)  = MCcl2Base.at(idet)+"_Ratio_"+effCut.str();

            //Definition of Event selections
            ISSasSel.at(icut) = Sel.at(idet) +  
                "&& scores_pos>="+TString(CL2cutISS.at(icut).str());
            ISScl2Sel.at(icut) = Sel.at(idet) + 
                "&& anomaly_score<="+TString(AEcutISS.at(icut).str()) + 
                "&& RigLabel==1";
            MCasSel.at(icut)  = Sel.at(idet) + 
                "&& scores_pos>="+TString(CL2cutMC.at(icut).str());
            MCcl2Sel.at(icut)  = Sel.at(idet) + 
                "&& anomaly_score<="+TString(AEcutMC.at(icut).str()) +
                "&& RigLabel==1";
            MCasSel.at(icut).Prepend("("); MCasSel.at(icut).Append(")*weight");
            MCcl2Sel.at(icut).Prepend("("); MCcl2Sel.at(icut).Append(")*weight");
            printf("\tscores cuts: \t eff RigLabel==1 -> %.2f eff R>0 -> %.2f \n\t%s %s\n\t%s %s\n",
                    1-pCL2[icut], pAE[icut],
                    TString("ISS_CL2scores>="+TString(CL2cutISS.at(icut).str())).Data(), TString("ISS_AEscore<="+TString(AEcutISS.at(icut).str())).Data(), 
                    TString("MC_CL2scores>="+TString(CL2cutMC.at(icut).str())).Data(), TString("MC_AEscore<="+TString(AEcutMC.at(icut).str())).Data());


            //Definition of drawing lines
            ISSas_toDraw.at(icut)  = DrawAS+hISSas.at(icut)->GetName();
            ISScl2_toDraw.at(icut) = DrawCL2+hISScl2.at(icut)->GetName();
            MCas_toDraw.at(icut)   = DrawAS+hMCas.at(icut)->GetName();
            MCcl2_toDraw.at(icut)  = DrawCL2+hMCcl2.at(icut)->GetName();

            //Drawings
            ISS.at(2)->Draw(ISSas_toDraw.at(icut).Data(),ISSasSel.at(icut).Data(),"goff");
            hISSas.at(icut) = (TH1F*)gDirectory->Get(hISSas.at(icut)->GetName());
            hISSas.at(icut)->SetLineWidth(LW);
            hISSas.at(icut)->GetXaxis()->CenterTitle(true); hISSas.at(icut)->GetXaxis()->SetLabelFont(62); hISSas.at(icut)->GetXaxis()->SetTitleFont(62); 
            hISSas.at(icut)->GetYaxis()->CenterTitle(true); hISSas.at(icut)->GetYaxis()->SetLabelFont(62); hISSas.at(icut)->GetYaxis()->SetTitleFont(62);
            ISS.at(2)->Draw(ISScl2_toDraw.at(icut).Data(),ISScl2Sel.at(icut).Data(), "goff");
            hISScl2.at(icut) = (TH1F*)gDirectory->Get(hISScl2.at(icut)->GetName());
            hISScl2.at(icut)->SetLineWidth(LW);
            hISScl2.at(icut)->GetXaxis()->CenterTitle(true); hISScl2.at(icut)->GetXaxis()->SetLabelFont(62); hISScl2.at(icut)->GetXaxis()->SetTitleFont(62); 
            hISScl2.at(icut)->GetYaxis()->CenterTitle(true); hISScl2.at(icut)->GetYaxis()->SetLabelFont(62); hISScl2.at(icut)->GetYaxis()->SetTitleFont(62);
            MC.at(0)->Draw(MCas_toDraw.at(icut).Data(),MCasSel.at(icut).Data(), "goff");
            hMCas.at(icut) = (TH1F*)gDirectory->Get(hMCas.at(icut)->GetName());
            hMCas.at(icut)->SetLineWidth(LW); 
            hMCas.at(icut)->GetXaxis()->CenterTitle(true); hMCas.at(icut)->GetXaxis()->SetLabelFont(62); hMCas.at(icut)->GetXaxis()->SetTitleFont(62); 
            hMCas.at(icut)->GetYaxis()->CenterTitle(true); hMCas.at(icut)->GetYaxis()->SetLabelFont(62); hMCas.at(icut)->GetYaxis()->SetTitleFont(62);
            hMCas.at(icut)->GetXaxis()->CenterTitle(true); hISSas.at(icut)->GetXaxis()->SetLabelFont(62); hISSas.at(icut)->GetXaxis()->SetTitleFont(62);
            hMCas.at(icut)->GetYaxis()->CenterTitle(true); hISSas.at(icut)->GetYaxis()->SetLabelFont(62); hISSas.at(icut)->GetYaxis()->SetTitleFont(62);
            MC.at(0)->Draw(MCcl2_toDraw.at(icut).Data(), MCcl2Sel.at(icut).Data(), "goff");
            hMCcl2.at(icut) = (TH1F*)gDirectory->Get(hMCcl2.at(icut)->GetName());
            hMCcl2.at(icut)->SetLineWidth(LW);            
            hMCcl2.at(icut)->GetXaxis()->CenterTitle(true); hMCcl2.at(icut)->GetXaxis()->SetLabelFont(62); hMCcl2.at(icut)->GetXaxis()->SetTitleFont(62); 
            hMCcl2.at(icut)->GetYaxis()->CenterTitle(true); hMCcl2.at(icut)->GetYaxis()->SetLabelFont(62); hMCcl2.at(icut)->GetYaxis()->SetTitleFont(62);


            hISScl2R.at(icut) = DivideHistograms(hCL2SigISS.at(idet),hISScl2.at(icut), ISScl2NameR.at(icut), myTitle);
            hISSasR.at(icut)  = DivideHistograms(hAESigISS.at(idet), hISSas.at(icut),  ISSasNameR.at(icut),  myTitle);

            hMCcl2R.at(icut) = DivideHistograms(hCL2SigMC.at(idet),hMCcl2.at(icut), MCcl2NameR.at(icut), myTitle);
            hMCasR.at(icut)  = DivideHistograms(hAESigMC.at(idet), hMCas.at(icut),  MCasNameR.at(icut),  myTitle);
        } //loop on cuts    

        f_out->cd();
        f_out->mkdir(dets.at(idet).Data());
        TString myDir;
        for(int i=0; i<Nhist; ++i) {
            myDir=dets.at(idet)+"/"+cNames.at(i);
            f_out->mkdir(myDir.Data());
            f_out->cd(myDir.Data());
            
            for(int icut=0; icut<Ncut; icut++){
                switch(i){
                    case 0:
                        canva.at(i)->cd();
                        if(icut==0) hAESigISS.at(idet)->DrawNormalized("hist PLC");
                        hISSas.at(icut)->DrawNormalized("hist PLC same");
                        c_rat.at(i)->cd();
                        if(icut==0) hISSasR.at(icut)->Draw("e PLC");
                        else hISSasR.at(icut)->Draw("e PLC same");
                        hISSasR.at(icut)->Write();
                        break;
                    case 1:
                        canva.at(i)->cd();
                        if(icut==0) hAESigMC.at(idet)->DrawNormalized("hist PLC");
                        hMCas.at(icut)->DrawNormalized("hist PLC same");
                        hMCas.at(icut)->Write();
                        c_rat.at(i)->cd();
                        if(icut==0) hMCasR.at(icut)->Draw("e PLC");
                        else hMCasR.at(icut)->Draw("e PLC same");
                        hMCasR.at(icut)->Write();
                        break;
                    case 2:
                        canva.at(i)->cd();
                        if(icut==0) hCL2SigISS.at(idet)->DrawNormalized("hist PLC");
                        hISScl2.at(icut)->DrawNormalized("hist PLC same");
                        hISScl2.at(icut)->Write();
                        c_rat.at(i)->cd();
                        if(icut==0) hISScl2R.at(icut)->Draw("e PLC");
                        else hISScl2R.at(icut)->Draw("e PLC same");
                        hISScl2R.at(icut)->Write();
                        break;
                    case 3:
                        canva.at(i)->cd();
                        if(icut==0) hCL2SigMC.at(idet)->DrawNormalized("hist PLC");
                        hMCcl2.at(icut)->DrawNormalized("hist PLC same");
                        hMCcl2.at(icut)->Write();
                        c_rat.at(i)->cd();
                        if(icut==0) hMCcl2R.at(icut)->Draw("e PLC");
                        else hMCcl2R.at(icut)->Draw("e PLC same");
                        hMCcl2R.at(icut)->Write();
                        break;
                }
            }
            legend.at(i) = canva.at(i)->BuildLegend();
            legend.at(i)->SetBorderSize(0); legend.at(i)->SetNColumns(3);
            canva.at(i)->Write();

            l_rat.at(i) = c_rat.at(i)->BuildLegend();
            l_rat.at(i)->SetBorderSize(0); l_rat.at(i)->SetNColumns(3);
            c_rat.at(i)->Write();
            delete canva.at(i);  delete c_rat.at(i);
            delete legend.at(i); delete l_rat.at(i);
        }
        for(int i=0; i<Ncut; ++i){
            CL2cutISS.at(i).str("");
            CL2cutMC.at(i).str("");
            AEcutISS.at(i).str("");
            AEcutMC.at(i).str("");
            delete hISSas.at(i);
            delete hISScl2.at(i);
            delete hMCas.at(i);
            delete hMCcl2.at(i);

        }
                
    } //loop on detectors

    return;
}


vector<TTree*> fromTerminal(bool isMC=false,
                                    TString CLtrainOnMCFile="000",
                                    TString CLvalOnISSFile="000",
                                    TString AEtrainOnMCFile="000",
                                    TString AEvalOnISSFile="000",
                                    vector<TTree*> myTrees = {}
                                ) {
    const int TotTree  = 4; // Number of CL trees
    const int TotFiles = 4; // Number of files
    const bool isLocal = false;
    bool UseLossWeights; // Loss weights have been used in the training
    (isMC) ? UseLossWeights = true : UseLossWeights = false;
    TString inPath;
    (isLocal) ? inPath ="/home/franz/AMS/output/organized_output/scores/" : inPath="/eos/home-f/frrossi/AMS/output/organized_output/scores/"; 

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
        //printf("The file %s exists.\n", PathToFile.at(i_file).Data());
        files.at(i_file) = new TFile(PathToFile.at(i_file), "READ");
        if (!files.at(i_file)) {
            printf("ERROR!! \n File %s not found\n", PathToFile.at(i_file).Data());
            return {};
        }
        files.at(i_file)->cd();
        printf("Reading file ==> %s\n", PathToFile.at(i_file).Data());
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


    (isMC) ? printf(" ===> \n Models are trained on MC data with UseLossWeights : %d \n ===> \n", UseLossWeights) : printf(" ===> \n Models are trained on ISS data \n ===> \n");
    
    printf("Returning the trees vector, N. of trees: %d\n", TotTree);
    printf("Sequence of the trees: \n");
    printf("\t 0: MC_pos \t 1: MC_neg \t 2: ISS_pos \t 3: ISS_neg\n\n");
    
    return myTrees;
}


TH1F* DivideHistograms(TH1F* h1, TH1F* h2, TString hName="", TString hTitle="") {
    if (!h1 || !h2 || h1->GetNbinsX() != h2->GetNbinsX()) return nullptr;
    h1->Scale(1./h1->Integral());
    h2->Scale(1./h2->Integral());

    TH1F* hRatio = (TH1F*)h2->Clone(hName.Data());
    hRatio->SetTitle(hTitle.Data());
    hRatio->Reset();

    for (int i = 1; i <= h1->GetNbinsX(); ++i) {
        double A = h1->GetBinContent(i);
        double B = h2->GetBinContent(i);
        if (B == 0) {
            hRatio->SetBinContent(i, 0);
            hRatio->SetBinError(i, 0);
            continue;
        }
        double ratio = A / B;
        // Error propagation: σ_r = r * sqrt( (σ_A/A)^2 + (σ_B/B)^2 )
        double sigmaA = h1->GetBinError(i);
        double sigmaB = h2->GetBinError(i);

        if (A == 0) sigmaA = 1.0; // Poisson assumption for low stats
        if (B == 0) sigmaB = 1.0;

        double error = ratio * sqrt( pow(sigmaA / A,2) + pow(sigmaB / B,2) );

        hRatio->SetBinContent(i, ratio);
        hRatio->SetBinError(i, error);
    }

    return hRatio;
}
