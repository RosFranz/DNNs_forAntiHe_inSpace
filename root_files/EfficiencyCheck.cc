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
#include <algorithm>

#include <TDirectory.h>
#include <TString.h>
#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>
#include <TStyle.h>
#include <TROOT.h>
#include <TH1.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TError.h>

using namespace std;

//Common constants
const int NDet= 3;
const int LW  = 3;
const int Ncanvas = 4;
const int nRigbins = 11; // Number of RIG bins as AMS-02 He flux
array<float,nRigbins> Rbins{1.92, 4.02, 6.47, 9.26, 13.0,
                            18.0, 24.7, 31.1, 36.1, 41.9,
                            48.5};
/*
const int nRigbins = 22; // Number of RIG bins as AMS-02 He flux
array<float,nRigbins> Rbins{1.92, 2.97, 4.02, 5.37, 6.47,
                            7.76, 9.26, 11.0, 13.0, 15.3,
                            18.0, 21.1, 24.7, 28.8, 31.1,
                            33.5, 36.1, 38.9, 41.9, 45.1,
                            48.5, 52.2};
*/
const float he3 = 2.81, he4 = 3.72; // GeV/c2
const float bNaF_thL = 0.75,  bNaF_thH = 0.993;
const float bAGL_thL = 0.953, bAGL_thH = 0.998;
//NaF
const float r3NaF_thL = he3 * bNaF_thL * 1./sqrt(1-pow(bNaF_thL,2)) / 2;
//AGL
const float r3AGL_thL = he3 * bAGL_thL * 1./sqrt(1-pow(bAGL_thL,2)) / 2;

const int ASbins = 26;   const float ASmin = 0.49,   ASmax = 1.01; // Anomaly score
const int CL2bins = 51; const float CL2min = -0.01, CL2max = 1.01; // CL2 score
const float minY = 5e-5, maxY = 1;
const float minYR = 0.5, maxYR = 2;


// At line 72 (prototype)
std::vector<TTree*> fromTerminal(bool isMC=false,
                                 TString CLtrainOnMCFile="000",
                                 TString CLvalOnISSFile="000",
                                 TString AEtrainOnMCFile="000",
                                 TString AEvalOnISSFile="000",
                                 std::vector<TTree*> myTrees = {});
TH1F* DivideHistograms(TH1F* h1, TH1F* h2, TString hName, TString hTitle);
TCanvas* MakeSquareCanvas(const char* name, const char* title, int size);

void SetStyle(TH1F*h);
void Evaluate(vector<TTree*> &ISS, vector<TTree*>MC,
              TDirectory *MyDir,   TFile *fO,
              TFile *f_I,          TFile *f_M,
              TString MySel,       double &MyP,
              array<TCanvas*,Ncanvas> &c,  array<TCanvas*,Ncanvas> &c_r,
              array<TLegend*,Ncanvas> &l,  array<TLegend*,Ncanvas> &l_r,
              array<TString, Ncanvas> &myDir,
              const int ibD,         const int ibR,
              const int ibC);

void EfficiencyCheck(){
    gErrorIgnoreLevel = kWarning;
    //Style
    gStyle->SetOptTitle(0);
    gStyle->SetPalette(55);//kRainBow palette
    gStyle->SetOptStat(0);
    gROOT->ForceStyle();
    bool isMinSel = true;
    TDirectory *DefDir = gDirectory;
    
    vector<TTree*> ISS, MC;
    ISS = fromTerminal(false, "2025_10_07", "2025_10_07", "2025_10_07", "2025_10_07");
    MC  = fromTerminal(true,  "2025_10_07", "2025_10_07", "2025_10_07", "2025_10_07");
    /*
    ISS = fromTerminal(false, "2025_09_12", "2025_09_12", "2025_09_12", "2025_09_12");
    MC  = fromTerminal(true,  "2025_09_12", "2025_09_12", "2025_09_12", "2025_09_12");
    */
    /* INNER no beta rec
    ISS = fromTerminal(false, "2025_08_29", "2025_08_29", "2025_08_29", "2025_08_29");
    MC  = fromTerminal(true, "2025_08_29", "2025_08_29", "2025_08_29", "2025_08_29");
     */
    /* INL1
    ISS = fromTerminal(false, "2025_06_30", "2025_06_30", "2025_06_30", "2025_06_30");
    MC  = fromTerminal(true,  "2025_06_30", "2025_06_30", "2025_06_30", "2025_06_30");
    ISS = fromTerminal(false, "2025_08_17", "2025_08_17", "2025_08_17", "2025_08_17");
    MC  = fromTerminal(true,  "2025_08_17", "2025_08_17", "2025_08_17", "2025_08_17");
    */
    DefDir->cd();

    array<TString, NDet> MyDets{"TOF", "NaF", "AGL"};

    ostringstream RvalNaF; RvalNaF  << fixed << setprecision(3) << r3NaF_thL;
    ostringstream RvalAGL; RvalAGL  << fixed << setprecision(3) << r3AGL_thL;
    // array<TString, NDet> R3abTH{"&& abs(Rinner)-(3*pow(Rinner,2)*invRerr_IN)>=1.92",
    //     " && abs(Rinner)-(3*pow(Rinner,2)*invRerr_IN)>="+TString(RvalNaF.str()),
    //     " && abs(Rinner)-(3*pow(Rinner,2)*invRerr_IN)>="+TString(RvalAGL.str())};

    array<TString, NDet> Sel{
        "NTrTrack<3 && ACC_AntiCounter<2 && abs(Rinner)>1.2 && "
            "chi2Time_tof < 10 && "
            "(hasRich==0 || (hasRich==1 && beta_rich<1 && (ringHits_dir_rich+ringHits_ref_rich)>=4 && beta_consistencyTOF<0.06)) &&"
            "(beta_tof<0.96 || hasRich == 1) ",
        "NTrTrack<3 && ACC_AntiCounter<2 && abs(Rinner)>1.2 && "
            "chi2Time_tof < 10 && "
            "(hasRich==1 && isNaF==1 && beta_rich<1 && (ringHits_dir_rich+ringHits_ref_rich)>=4 && beta_consistencyTOF<0.06) &&"
            "(beta_tof<0.96 || hasRich == 1)",
        "NTrTrack<3 && ACC_AntiCounter<2 && abs(Rinner)>1.2 && "
            "chi2Time_tof < 10 && "
            "(hasRich==0 || (hasRich==1 && isNaF==0 && beta_rich<1 && (ringHits_dir_rich+ringHits_ref_rich)>=4 && beta_consistencyTOF<0.06)) &&"
            "(beta_tof<0.96 || hasRich == 1) "};
   if(!isMinSel){
        array<TString, NDet> BetaQual{
        "&& chi2Coo_tof < 4   && chi2Time_tof < 10 && ",
        "&& isBorder_rich==0  && kprob_rich>0.01 && "
            "1<charge2_rich    && charge2_rich<4 && "
            "ringPMTs2_rich>5  && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4",
        "&& hasGoodImpact==1 && kprob_rich>0.01 && "
            "1<charge2_rich   && charge2_rich<4 && "
            "ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4"
        };
        for(int i=0; i<NDet; ++i) Sel.at(i) += BetaQual.at(i);
    }
    for(int id=0; id<NDet; ++id) printf("Common selection %d: %s \n", id, Sel.at(id).Data()); 

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

    //Efficiency cuts on scores
    const int Ncut = 5;
    double myP[Ncut]{0.95, 0.90, 0.85, 0.80, 0.67};//95 percentile -> 90% efficiency on R>0 anomaly score

    TFile *f_out = new TFile("/eos/home-f/frrossi/AMS/EfficiencyCheck_POS.root","RECREATE");
    array<TString, Ncanvas> cNames{"ISSas", "MCas", "ISScl2", "MCcl2"};
    TString DefPDFpath("/eos/home-f/frrossi/AMS/EfficiencyOnScores/INNER/");
    TString PDFpath;
    
    for(int id=0; id<NDet; id++){ //loop on detectors
        printf("===> %s\n", MyDets.at(id).Data());
        f_out->mkdir(MyDets.at(id).Data());
        for(int ib=0; ib<nRigbins; ++ib){
            if(id==2 && ib<nRigbins-1 && Rbins[ib+1]<r3AGL_thL) continue;
            //Canvas
            array<TLegend*,Ncanvas> legend, l_rat;
            array<TCanvas*, Ncanvas> canva, c_rat;
            TString myDir;
            for(int ic=0; ic<Ncanvas; ++ic){
                myDir = MyDets.at(id)+"/"+cNames.at(ic)+"/RigBin_"+to_string(ib);
                f_out->mkdir(myDir.Data()); f_out->cd(myDir.Data());
                TString c_name = TString("c_")+MyDets.at(id)+"_"+cNames.at(ic)+"_bin"+to_string(ib);
                canva.at(ic) = MakeSquareCanvas(c_name.Data(), c_name.Data(), 800); canva.at(ic)->SetLogy();
                c_name = TString("c_Ratio_")+MyDets.at(id)+"_"+cNames.at(ic)+"_bin"+to_string(ib);
                c_rat.at(ic) = MakeSquareCanvas(c_name.Data(), c_name.Data(), 800);
            }

            for(int icut=0; icut<Ncut; ++icut){ //Loop on cut values
                Evaluate(ISS,      MC,
                         DefDir,     f_out,
                         f_ISSs,     f_MCs,
                         Sel.at(id), myP[icut],
                         canva,      c_rat,
                         legend,     l_rat,
                         cNames,
                         id,         ib,
                         icut);
            } // end loop on cut values
            //Make directory and write
            for(int ic=0; ic<Ncanvas; ++ic){
                myDir = MyDets.at(id)+"/"+cNames.at(ic)+"/RigBin_"+to_string(ib);
                PDFpath = DefPDFpath+MyDets.at(id)+"/"+cNames.at(ic)+"/RigBin_"+to_string(ib)+".pdf";
                f_out->cd(myDir.Data());
                legend.at(ic) = canva.at(ic)->BuildLegend(0.16,0.86,0.84,0.98);
                legend.at(ic)->SetBorderSize(0); legend.at(ic)->SetNColumns(3);

                canva.at(ic)->Write(); canva.at(ic)->SaveAs(PDFpath.Data());

                PDFpath = DefPDFpath+MyDets.at(id)+"/"+cNames.at(ic)+"/RigBin_"+to_string(ib)+"_Ratio.pdf";
                l_rat.at(ic) = c_rat.at(ic)->BuildLegend(0.16,0.86,0.84,0.98);
                l_rat.at(ic)->SetBorderSize(0); l_rat.at(ic)->SetNColumns(3);
                c_rat.at(ic)->Write(); c_rat.at(ic)->SaveAs(PDFpath.Data());
                delete canva.at(ic);  delete c_rat.at(ic);
                delete legend.at(ic); delete l_rat.at(ic);
            }
        }// loop on nRigbins       
    } // loop on detectors
    return;
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

TCanvas* MakeSquareCanvas(const char* name = "c", const char* title = "c", int size = 800) {
    TCanvas* c = new TCanvas(name, title, size, size); // crea canvas quadrata
    c->SetFixedAspectRatio();
    c->SetLeftMargin(0.15);
    c->SetRightMargin(0.15);
    c->SetTopMargin(0.15);
    c->SetBottomMargin(0.15);
    c->SetGrid();
    c->SetTicks(1,1);
    return c;
}


void SetStyle(TH1F*h){
    h->SetLineWidth(LW);
    h->GetXaxis()->CenterTitle(true); h->GetXaxis()->SetLabelFont(62); h->GetXaxis()->SetTitleFont(62); 
    h->GetYaxis()->CenterTitle(true); h->GetYaxis()->SetLabelFont(62); h->GetYaxis()->SetTitleFont(62);
    return;
}

void Evaluate(vector<TTree*> &ISS, vector<TTree*>MC,
              TDirectory *MyDir,   TFile *fO,
              TFile *f_I,          TFile *f_M,
              TString MySel,       double &MyP,
              array<TCanvas*,Ncanvas> &c,  array<TCanvas*,Ncanvas> &c_r,
              array<TLegend*,Ncanvas> &l,  array<TLegend*,Ncanvas> &l_r,
              array<TString, Ncanvas> &myDir,
              const int ibD,         const int ibR,
              const int ibC){
    
    //Cut on the scores
    double pCL2[1]{1-MyP};//10 percentile -> 90% efficiency on RigLabel=1
    double pAE[1]{MyP};//95 percentile -> 90% efficiency on R>0 anomaly score
    double CL2cut[1]{0}, AEcut[1]{0}; //scores>=CL2cut && anomaly<=AEcut
    ostringstream CL2cutISS, CL2cutMC, AEcutISS, AEcutMC;
    ostringstream effCut;
    TString ISSasSel, ISScl2Sel, MCasSel, MCcl2Sel;
    
    //Drawings lines
    TString ISSas_toDraw, ISScl2_toDraw, MCas_toDraw, MCcl2_toDraw;
    TString DrawAS("anomaly_score>>+"), DrawCL2("scores>>+");

    //CL2 and AE scores
    TH1F* hCL2SigISS, *hCL2SigMC, *hAESigISS, *hAESigMC;
    array<TString, NDet> dets{"TOF", "NaF", "AGL"};
    TString CL2SigNameISS, CL2SigNameMC, AESigNameISS, AESigNameMC;
    //Definitions depending from detector
    CL2SigNameISS = "ISS_RposRig_"+dets.at(ibD)+"Label1"; CL2SigNameMC  = "He4_MC_RposRig_"+dets.at(ibD)+"Label1";
    AESigNameISS  = "ISS_Rpos_"+dets.at(ibD);             AESigNameMC   = "He4_MC_Rpos_"+dets.at(ibD);
    //ISS pos CL2
    TString dir = dets.at(ibD)+"/CL2/ISS_pos/"+CL2SigNameISS+"_bin"+to_string(ibR);;
    hCL2SigISS  = (TH1F*)f_I->Get(dir.Data())->Clone(); hCL2SigISS->GetQuantiles(1, CL2cut, pCL2);
    CL2cutISS << fixed << setprecision(5) << CL2cut[0];
    //MC pos CL2
    dir = dets.at(ibD)+"/CL2/MC_pos/"+CL2SigNameMC+"_bin"+to_string(ibR);
    hCL2SigMC = (TH1F*)f_M->Get(dir.Data())->Clone(); hCL2SigMC->GetQuantiles(1, CL2cut, pCL2);      
    CL2cutMC  << fixed << setprecision(5) << CL2cut[0];
    //ISS pos AE
    dir = dets.at(ibD)+"/AE/ISS_pos/"+AESigNameISS+"_bin"+to_string(ibR);
    hAESigISS = (TH1F*)f_I->Get(dir.Data())->Clone(); hAESigISS->GetQuantiles(1, AEcut, pAE);
    AEcutISS  << fixed << setprecision(5) << AEcut[0];
    //MC pos AE
    dir = dets.at(ibD)+"/AE/MC_pos/"+AESigNameMC+"_bin"+to_string(ibR);
    hAESigMC = (TH1F*)f_M->Get(dir.Data())->Clone(); hAESigMC->GetQuantiles(1, AEcut, pAE);
    AEcutMC   << fixed << setprecision(5) << AEcut[0];
    if(!hCL2SigISS || !hCL2SigMC || !hAESigISS || !hAESigMC) printf("Some problems in the score TH1...\n");
    //printf("N. bins: %d, %d, %d, %d\n", hCL2SigISS->GetNbinsX(), hCL2SigMC->GetNbinsX(),
    //        hAESigISS->GetNbinsX(),hAESigMC->GetNbinsX());
    if(hCL2SigISS->GetSumw2N() == 0) hCL2SigISS->Sumw2();
    if(hCL2SigMC->GetSumw2N() == 0)  hCL2SigMC->Sumw2();
    if(hAESigISS->GetSumw2N() == 0)  hAESigISS->Sumw2();
    if(hAESigMC->GetSumw2N() == 0)   hAESigMC->Sumw2();
    hCL2SigISS->Rebin(2); hCL2SigMC->Rebin(2);
    hAESigISS->Rebin(2);  hAESigMC->Rebin(2);


    //Definition of new TH1
    TH1F* hISSas,  *hISScl2,  *hMCas, *hMCcl2;
    TH1F* hISSasR, *hISScl2R, *hMCasR, *hMCcl2R;
    array<TString, NDet> ISSasBase  {"TOF_ISSas",  "NaF_ISSas",  "AGL_ISSas"};
    array<TString, NDet> ISScl2Base{"TOF_ISScl2", "NaF_ISScl2", "AGL_ISScl2"};
    array<TString, NDet> MCasBase  {"TOF_MCas",  "NaF_MCas",  "AGL_MCas"};
    array<TString, NDet> MCcl2Base {"TOF_MCcl2", "NaF_MCcl2", "AGL_MCcl2"};
    TString ISSasName, ISScl2Name, MCasName, MCcl2Name;
    TString ISSasNameR, ISScl2NameR, MCasNameR, MCcl2NameR;
    TString ISSasTitle (";  ISS anomaly score; Arbitrary units");
    TString ISScl2Title(";  ISS classifier score; Arbitrary units");
    TString MCasTitle  ("; MC anomaly score; Arbitrary units");
    TString MCcl2Title ("; MC classifier score; Arbitrary units");

    TString ISSasTitleR (";  ISS anomaly score; Arbitrary units");
    TString ISScl2TitleR(";  ISS classifier score; Arbitrary units");
    TString MCasTitleR  ("; MC anomaly score; Arbitrary units");
    TString MCcl2TitleR ("; MC classifier score; Arbitrary units");

    //MyDir->cd();
    effCut.str("");  effCut << fixed << setprecision(2) << MyP; TString myTitle("eff. "+effCut.str());
    ISSasName = ISSasBase.at(ibD)+"_binR"+to_string(ibR)+"_"+effCut.str(); ISScl2Name = ISScl2Base.at(ibD)+"_binR"+to_string(ibR)+"_"+effCut.str();
    MCasName  = MCasBase.at(ibD)+"_binR"+to_string(ibR)+"_"+effCut.str();  MCcl2Name  = MCcl2Base.at(ibD)+"_binR"+to_string(ibR)+"_"+effCut.str();
    hISSas  = new TH1F(ISSasName.Data(),  ISSasTitle.Data(),  ASbins, ASmin, ASmax);    hISSas->SetTitle(myTitle);
    hISScl2 = new TH1F(ISScl2Name.Data(), ISScl2Title.Data(), CL2bins, CL2min, CL2max); hISScl2->SetTitle(myTitle);
    hMCas   = new TH1F(MCasName.Data(),   MCasTitle.Data(),   ASbins, ASmin, ASmax);    hMCas->SetTitle(myTitle);
    hMCcl2  = new TH1F(MCcl2Name.Data(),  MCcl2Title.Data(),  CL2bins, CL2min, CL2max); hMCcl2->SetTitle(myTitle);
    //Name for ratios
    ISSasNameR = ISSasBase.at(ibD)+"_Ratio_"+"_binR"+to_string(ibR)+"_"+effCut.str(); ISScl2NameR = ISScl2Base.at(ibD)+"_Ratio_"+"_binR"+to_string(ibR)+"_"+effCut.str();
    MCasNameR  = MCasBase.at(ibD)+"_Ratio_"+"_binR"+to_string(ibR)+"_"+effCut.str();  MCcl2NameR  = MCcl2Base.at(ibD)+"_Ratio_"+"_binR"+to_string(ibR)+"_"+effCut.str();

    //Definition of Event selections
    ostringstream Rlow, Rhigh;
    Rlow   << fixed << setprecision(3) << Rbins.at(ibR);
    TString RigSel;
    if(ibR<nRigbins-1) {
        Rhigh  << fixed << setprecision(3) << Rbins.at(ibR+1);
        RigSel = "&& abs(Rinner)>="+TString(Rlow.str())+" && abs(Rinner)<"+TString(Rhigh.str());
    }
    else{
        Rhigh  << fixed << setprecision(3) << Rbins.at(ibR);
        RigSel = "&& abs(Rinner)>="+TString(Rhigh.str());
    }

    ISSasSel  = MySel+RigSel+"&& scores>="+TString(CL2cutISS.str());
    ISScl2Sel = MySel+RigSel+"&& anomaly_score<="+TString(AEcutISS.str()) + "&& RigLabel==1";
    MCasSel   = MySel+RigSel+"&& scores>="+TString(CL2cutMC.str());
    MCcl2Sel  = MySel+RigSel+"&& anomaly_score<="+TString(AEcutMC.str())  + "&& RigLabel==1";
    MCasSel.Prepend("("); MCasSel.Append(")*weight");
    MCcl2Sel.Prepend("("); MCcl2Sel.Append(")*weight");
    /*
    printf("\tscores cuts: \t eff RigLabel==1 -> %.2f eff R>0 -> %.2f \n\t%s %s\n\t%s %s\n", 1-MyP, MyP,
            TString("ISS_CL2scores>="+TString(CL2cutISS.str())).Data(), TString("ISS_AEscore<="+TString(AEcutISS.str())).Data(), 
            TString("MC_CL2scores>="+TString(CL2cutMC.str())).Data(), TString("MC_AEscore<="+TString(AEcutMC.str())).Data());
    */

    //Definition of drawing lines
    ISSas_toDraw  = DrawAS+hISSas->GetName();
    ISScl2_toDraw = DrawCL2+hISScl2->GetName();
    MCas_toDraw   = DrawAS+hMCas->GetName();
    MCcl2_toDraw  = DrawCL2+hMCcl2->GetName();

    //Drawings
    ISS.at(2)->Draw(ISSas_toDraw.Data(),ISSasSel.Data(),"goff");   hISSas = (TH1F*)gDirectory->Get(hISSas->GetName()); SetStyle(hISSas);
    ISS.at(2)->Draw(ISScl2_toDraw.Data(),ISScl2Sel.Data(), "goff"); hISScl2 = (TH1F*)gDirectory->Get(hISScl2->GetName()); SetStyle(hISScl2); 
    MC.at(0)->Draw(MCas_toDraw.Data(),MCasSel.Data(), "goff");     hMCas = (TH1F*)gDirectory->Get(hMCas->GetName()); SetStyle(hMCas);                     
    MC.at(0)->Draw(MCcl2_toDraw.Data(), MCcl2Sel.Data(), "goff");  hMCcl2 = (TH1F*)gDirectory->Get(hMCcl2->GetName()); SetStyle(hMCcl2); 
    if(!hISSas || !hISScl2) printf("Unable to get the TH1\n");
    //printf("Number of entries ISS-as:%.2f Sel: %s \n", hISSas->GetEntries(), ISSasSel.Data());

    //Get Ratios
    hISScl2R = DivideHistograms(hCL2SigISS,hISScl2, ISScl2NameR, myTitle);
    hISSasR  = DivideHistograms(hAESigISS, hISSas,  ISSasNameR,  myTitle);
    hMCcl2R = DivideHistograms(hCL2SigMC,hMCcl2, MCcl2NameR, myTitle);
    hMCasR  = DivideHistograms(hAESigMC, hMCas,  MCasNameR,  myTitle);
    if(!hISScl2R || !hISSasR || !hMCcl2R || !hMCasR) printf("Unable to get the ratio\n");

    //Loop on Canvases 4-> ISScl2, ISSas, MCcl2, MCas
    for(int i=0; i<Ncanvas; ++i) { 

        TString path = dets.at(ibD)+"/"+myDir.at(i)+"/RigBin_"+to_string(ibR);
        fO->cd(path.Data());       
        c.at(i)->cd();
        //Plotting
        switch(i){
            case 0:
                if(ibC==0){
                    TH1* ht = hAESigISS->DrawNormalized("hist PLC");
                    ht->GetYaxis()->SetRangeUser(minY,maxY);
                }
                hISSas->DrawNormalized("hist PLC same");
                hISSas->Write();
                c_r.at(i)->cd();
                if(ibC==0) {
                    hISSasR->GetYaxis()->SetRangeUser(minYR,maxYR);
                    hISSasR->Draw("e PLC");
                }
                else hISSasR->Draw("e PLC same");
                hISSasR->Write();
                break;
            case 1:
                if(ibC==0) {
                    TH1* ht = hAESigMC->DrawNormalized("hist PLC");
                    ht->GetYaxis()->SetRangeUser(minY,maxY);
                }
                hMCas->DrawNormalized("hist PLC same");
                hMCas->Write();
                c_r.at(i)->cd();
                if(ibC==0){
                    hMCasR->GetYaxis()->SetRangeUser(minYR,maxYR);
                    hMCasR->Draw("e PLC");
                }
                else hMCasR->Draw("e PLC same");
                hMCasR->Write();
                break;
            case 2:
                if(ibC==0){
                    TH1 *ht = hCL2SigISS->DrawNormalized("hist PLC");
                    ht->GetYaxis()->SetRangeUser(minY,maxY);
                }
                hISScl2->DrawNormalized("hist PLC same");
                hISScl2->Write();
                c_r.at(i)->cd();
                if(ibC==0){
                    hISScl2R->GetYaxis()->SetRangeUser(minYR,maxYR);
                    hISScl2R->Draw("e PLC");
                }
                else hISScl2R->Draw("e PLC same");
                hISScl2R->Write();
                break;
            case 3:
                if(ibC==0){
                    TH1 *ht = hCL2SigMC->DrawNormalized("hist PLC");
                    ht->GetYaxis()->SetRangeUser(minY,maxY);
                }
                hMCcl2->DrawNormalized("hist PLC same");
                hMCcl2->Write();
                c_r.at(i)->cd();
                if(ibC==0){
                    hMCcl2R->GetYaxis()->SetRangeUser(minYR,maxYR);
                    hMCcl2R->Draw("e PLC");
                }
                else hMCcl2R->Draw("e PLC same");
                hMCcl2R->Write();
                break;
        }
    } //end loop on canvases
    delete hCL2SigISS; delete hCL2SigMC;
    delete hAESigISS;  delete hAESigMC;
    delete hISSas; delete hISScl2;
    delete hMCas;  delete hMCcl2;
} //Evaluate

vector<TTree*> fromTerminal(bool isMC,
                            TString CLtrainOnMCFile,
                            TString CLvalOnISSFile,
                            TString AEtrainOnMCFile,
                            TString AEvalOnISSFile,
                            vector<TTree*> myTrees) {
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
    for(int i=0; i<int(CLtrees.size()); ++i) myTrees.push_back(CLtrees.at(i));


    (isMC) ? printf(" ===> \n Models are trained on MC data with UseLossWeights : %d \n ===> \n", UseLossWeights) : printf(" ===> \n Models are trained on ISS data \n ===> \n");
    
    printf("Returning the trees vector, N. of trees: %d\n", TotTree);
    printf("Sequence of the trees: \n");
    printf("\t 0: MC_pos \t 1: MC_neg \t 2: ISS_pos \t 3: ISS_neg\n\n");
    
    return myTrees;
}

