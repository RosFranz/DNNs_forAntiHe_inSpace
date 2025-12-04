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
#include <TH1.h>

using namespace std;
const int NCut = 7;
const int NDet= 3;
const int LW  = 3;
vector<TTree*> fromTerminal(bool isMC=false,
                                    TString CLtrainOnMCFile="000",
                                    TString CLvalOnISSFile="000",
                                    TString AEtrainOnMCFile="000",
                                    TString AEvalOnISSFile="000",
                                    vector<TTree*> myTrees = {}
                                );

array<array<double,NDet>,6> GetCounts(vector<TTree*> ISS = {},   vector<TTree*> MC={},
                                      const float MCtotEvents=0, const float MCtotEventsn=0,
                                      double MyPae=0.98,         double MyPcl2=0.98,
                                      TString outName="",        bool Update=false,
                                      bool FirstNNs=false,       bool verbose=false);

array<double,NDet> GetSignificance(array<double,NDet> Non, array<double,NDet> muBkg);
array<double,NDet> GetSignificance2(array<double,NDet> Non, array<double,NDet> Noff, array<double,NDet> tau);


void Significance3Det(){
    gStyle->SetPalette(55);//kRainBow palette
    gStyle->SetOptStat(0);
    bool UpdateFile = false;
    bool doPrint = false;
    bool FirstCutOnNNs = false;
    TString MyOutName;
    (FirstCutOnNNs) ? MyOutName= "/eos/home-f/frrossi/AMS/Significance/Significance_FirstNNs.root" : MyOutName = "/eos/home-f/frrossi/AMS/Significance/Significance.root";

    array<double,NCut> effAE{0.95,0.90,0.85,0.80,0.75,0.70,0.65}, effCL2{0.95,0.90,0.85,0.80,0.75, 0.70, 0.65};
    array<TH2F*, NDet> hSig, hSig2;
    array<TString, NDet> SigName{"TOF_Sign","NaF_Sign","AGL_Sign"};
    array<TString, NDet> Sig2Name{"TOF_Sign2","NaF_Sign2","AGL_Sign2"};
    array<TString, NDet> SigTitle{"TOF significance; Anomaly score efficiency (R>0); Classifier score efficiency (RigLabel==1); Significance",
                                  "NaF significance; Anomaly score efficiency (R>0); Classifier score efficiency (RigLabel==1); Significance",
                                  "AGL significance; Anomaly score efficiency (R>0); Classifier score efficiency (RigLabel==1); Significance"};
    array<TString, NDet> Sig2Title{"TOF significance 2; Anomaly score efficiency (R>0); Classifier score efficiency (RigLabel==1); Significance",
                                   "NaF significance 2; Anomaly score efficiency (R>0); Classifier score efficiency (RigLabel==1); Significance",
                                   "AGL significance 2; Anomaly score efficiency (R>0); Classifier score efficiency (RigLabel==1); Significance"};
    array<EColor, NDet> MyColors{kBlue, kMagenta, kRed};
    const int Nbins = 25;
    const double minEffAE  = (*min_element(effAE.begin(), effAE.end())) - 0.04;
    const double minEffCL2 = (*min_element(effCL2.begin(), effCL2.end())) - 0.04;
    const double maxEffAE  = (*max_element(effAE.begin(), effAE.end()))  + 0.04;
    const double maxEffCL2 = (*max_element(effCL2.begin(), effCL2.end())) + 0.04;

    for(int i=0; i<NDet; ++i) {
        hSig.at(i) = new TH2F(SigName.at(i).Data(), SigTitle.at(i).Data(), Nbins, minEffAE, maxEffAE, Nbins, minEffCL2, maxEffCL2);
        // hSig.at(i)->SetLineWidth(LW); hSig.at(i)->SetMarkerStyle(20+(1*i)); hSig.at(i)->SetMarkerColor(MyColors.at(i));
        // hSig.at(i)->SetLineColor(MyColors.at(i));
        hSig.at(i)->GetXaxis()->CenterTitle(true); hSig.at(i)->GetXaxis()->SetLabelFont(62); hSig.at(i)->GetXaxis()->SetTitleFont(62); 
        hSig.at(i)->GetYaxis()->CenterTitle(true); hSig.at(i)->GetYaxis()->SetLabelFont(62); hSig.at(i)->GetYaxis()->SetTitleFont(62);

        hSig2.at(i) = new TH2F(Sig2Name.at(i).Data(), Sig2Title.at(i).Data(), Nbins, minEffAE, maxEffAE, Nbins, minEffCL2, maxEffCL2);
        hSig2.at(i)->GetXaxis()->CenterTitle(true); hSig2.at(i)->GetXaxis()->SetLabelFont(62); hSig2.at(i)->GetXaxis()->SetTitleFont(62); 
        hSig2.at(i)->GetYaxis()->CenterTitle(true); hSig2.at(i)->GetYaxis()->SetLabelFont(62); hSig2.at(i)->GetYaxis()->SetTitleFont(62);
    }
    
    vector<TTree*> myISS, myMC;
    myISS = fromTerminal(false, "2025_06_30", "2025_06_30", "2025_06_30", "2025_06_30");
    myMC  = fromTerminal(true,  "2025_06_30", "2025_06_30", "2025_06_30", "2025_06_30");

    TH1F *hMC_TotPos = new TH1F("hMC_TotPos","hMC_TotPos; log_{10}(R_{INNER}); Weights", 500, -2, 600);
    TH1F *hMC_TotNeg = new TH1F("hMC_TotNeg","hMC_TotNeg; log_{10}(R_{INNER}); Weights", 500, -2, 600);
    myMC.at(0)->Draw("log10(abs(Rinner))>>+hMC_TotPos","1*weight","goff");
    hMC_TotPos = (TH1F*)gDirectory->Get("hMC_TotPos");
    myMC.at(1)->Draw("log10(abs(Rinner))>>+hMC_TotNeg","1*weight","goff");
    hMC_TotNeg = (TH1F*)gDirectory->Get("hMC_TotNeg");
    const float MCtotE  = hMC_TotPos->Integral();
    const float MCtotEn = hMC_TotNeg->Integral();
    delete hMC_TotPos; delete hMC_TotNeg;
    printf("Check on MC TOT events: %f, %f\n", MCtotE, MCtotEn);

    for(int cutCL=0; cutCL<NCut; ++cutCL){
        for(int cutAE=0; cutAE<NCut; ++cutAE){
            //0->Counts on ISS, 1->errors on ISS, 2->Counts on MC, 3->errors on MC
            array<array<double, NDet>,6> IntAndErr = GetCounts(myISS,           myMC,
                                                               MCtotE,          MCtotEn,
                                                               effAE.at(cutAE), effCL2.at(cutCL),
                                                               MyOutName,       UpdateFile,
                                                               FirstCutOnNNs,   doPrint);
            array<double, NDet> Significance  = GetSignificance(IntAndErr.at(0), IntAndErr.at(2));
            array<double, NDet> Significance2 = GetSignificance2(IntAndErr.at(0), IntAndErr.at(4), IntAndErr.at(5));
            for(int idet=0; idet<NDet; idet++) {
                hSig.at(idet)->Fill(effAE.at(cutAE),effCL2.at(cutCL),Significance.at(idet));
                hSig2.at(idet)->Fill(effAE.at(cutAE),effCL2.at(cutCL),Significance2.at(idet));
            }
            UpdateFile = true;
        }
    }
    TFile *fO = new TFile(MyOutName.Data(),"UPDATE");
    for(int i=0; i<NDet; ++i){
        hSig.at(i)->Write();
        hSig2.at(i)->Write();
    } 
    return;
}

array<double,NDet> GetSignificance(array<double,NDet> Non, array<double,NDet> muBkg){
    array<double, NDet> Significance;
    for(int i=0; i<NDet; ++i){
        double T1 = Non.at(i)*log(Non.at(i)/muBkg.at(i));
        double T2 = -Non.at(i) + muBkg.at(i);
        if(Non.at(i)==0 || muBkg.at(i)==0) Significance.at(i)=0;
        else Significance.at(i) = (Non.at(i)-muBkg.at(i))/abs(Non.at(i)-muBkg.at(i)) * sqrt(2 *(T1+T2));
        if(Non.at(i)!=0 && muBkg.at(i)==0) Significance.at(i)=-3;
    }
    return Significance; 
}

array<double,NDet> GetSignificance2(array<double,NDet> Non, array<double,NDet> Noff, array<double,NDet> tau){
    array<double, NDet> Significance2;
    for(int i=0; i<NDet; ++i){
        double T1 = Non.at(i)*log(Non.at(i)*(1+tau.at(i))/(Non.at(i)+Noff.at(i)));
        double T2 = Noff.at(i)*log(Noff.at(i)*(1+tau.at(i))/(tau.at(i)*(Non.at(i)+Noff.at(i))));
        if(Non.at(i)==0 || tau.at(i)==0) Significance2.at(i)=0;
        else Significance2.at(i) = sqrt(2*(T1+T2));
        if(Non.at(i)!=0 && tau.at(i)==0) Significance2.at(i)=-3;
        if(isnan(Significance2.at(i))) printf("\n\n|||NAN|||\nT1:%f T2:%f Non:%f Noff:%f tau:%f\n\n", T1, T2, Non.at(i), Noff.at(i), tau.at(i));
    }
    return Significance2; 
}

array<array<double,NDet>,6> GetCounts(vector<TTree*> ISS,      vector<TTree*> MC,
                                      const float MCtotEvents, const float MCtotEventsN,
                                      double MyPae=0.98,       double MyPcl2=0.98,
                                      TString outName,         bool Update=false,
                                      bool FirstNNs=false,     bool verbose=false){
    TDirectory *DefDir = gDirectory;
    const float he3 = 2.81, he4 = 3.72; // GeV/c2
    const float bNaF_thL = 0.75,  bNaF_thH = 0.993;
    const float bAGL_thL = 0.96, bAGL_thH = 0.999;
    //NaF
    const float r3NaF_thL = he3 * bNaF_thL * 1./sqrt(1-pow(bNaF_thL,2)) / 2;
    //AGL
    const float r3AGL_thL = he3 * bAGL_thL * 1./sqrt(1-pow(bAGL_thL,2)) / 2;

    //Binning
    const int MassBins = 100;
    const float MinMass = 0, MaxMass = 10; // GeV/c^2

    ostringstream effAE, effCL2;
    effAE  << fixed << setprecision(2) << MyPae;
    effCL2 << fixed << setprecision(2) << MyPcl2; 
    DefDir->cd();
    

     ostringstream RvalNaF; RvalNaF  << fixed << setprecision(3) << r3NaF_thL;
    ostringstream RvalAGL; RvalAGL  << fixed << setprecision(3) << r3AGL_thL;
    array<TString, NDet> R3abTH{"abs(Rinner)-(3*pow(Rinner,2)*invRerr_IN)>=1.92",
        " abs(Rinner)-(3*pow(Rinner,2)*invRerr_IN)>="+TString(RvalNaF.str()),
        " abs(Rinner)-(3*pow(Rinner,2)*invRerr_IN)>="+TString(RvalAGL.str())};

    array<TString, NDet> MinSel{
        "abs(Rinner)>=1.92",
        "abs(Rinner)>=1.92 && hasRich==1 && isNaF==1",
        "abs(Rinner)>="+TString(RvalAGL.str())+" && hasRich==1 && isNaF==0"
        };
    array<TString, NDet> BetaQual{
        "&& chi2Coo_tof < 4   && chi2Time_tof < 10 && "
            "beta_tof>0.72     && beta_tof<=0.96",
        "&& isBorder_rich==0  && kprob_rich>0.01 && "
            "1<charge2_rich    && charge2_rich<4 && "
            "ringPMTs2_rich>5  && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4 && "
            "beta_rich>=0.80   && beta_rich<=0.994",
        "&& hasGoodImpact==1 && kprob_rich>0.01 && "
            "1<charge2_rich   && charge2_rich<4 && "
            "ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4 && "
            "beta_rich>=0.96  && beta_rich<=0.999"
        };
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
    

   

    //Scores TH1
    TString ScoreFileName;
    (FirstNNs) ? ScoreFileName = "scores_TrainOnMC_NNs.root" : ScoreFileName = "scores_TrainOnMC_beta.root";
    TFile *f_MCs  = new TFile(ScoreFileName.Data(),"OPEN");
    (FirstNNs) ? ScoreFileName = "scores_TrainOnISS_NNs.root" : ScoreFileName = "scores_TrainOnISS_beta.root";
    TFile *f_ISSs = new TFile(ScoreFileName.Data(),"OPEN");
    array<TH1F*, NDet> hCL2SigISS, hCL2SigMC;
    array<TH1F*, NDet> hCL2SigISSn,hCL2BkgISSn, hCL2SigMCn, hCL2BkgMCn;
    array<TH1F*, NDet> hAESigISS,  hAESigMC;
    array<TH1F*, NDet> hAEBkgISSn, hAEBkgMCn;
    array<float, NDet> CL2sFactor, AEsFactor;
    array<TString, NDet> dets{"TOF", "NaF", "AGL"};
    array<TString, NDet> CL2SigNameISS {"ISS_RposRig_TOFLabel1",   "ISS_RposRig_NaFLabel1",    "ISS_RposRig_AGLLabel1"};
    array<TString, NDet> CL2SigNameISSn{"ISS_RnegRig_TOFLabel1",   "ISS_RnegRig_NaFLabel1",    "ISS_RnegRig_AGLLabel1"};
    array<TString, NDet> CL2BkgNameISSn{"ISS_RnegRig_TOFLabel0",   "ISS_RnegRig_NaFLabel0",    "ISS_RnegRig_AGLLabel0"};

    array<TString, NDet> CL2SigNameMC {"He4_MC_RposRig_TOFLabel1", "He4_MC_RposRig_NaFLabel1", "He4_MC_RposRig_AGLLabel1"};
    array<TString, NDet> CL2SigNameMCn{"He4_MC_RnegRig_TOFLabel1", "He4_MC_RnegRig_NaFLabel1", "He4_MC_RnegRig_AGLLabel1"};
    array<TString, NDet> CL2BkgNameMCn{"He4_MC_RnegRig_TOFLabel0", "He4_MC_RnegRig_NaFLabel0", "He4_MC_RnegRig_AGLLabel0"};

    array<TString, NDet> AESigNameISS {"ISS_Rpos_TOF",   "ISS_Rpos_NaF",    "ISS_Rpos_AGL"};
    array<TString, NDet> AEBkgNameISSn{"ISS_Rneg_TOF",   "ISS_Rneg_NaF",    "ISS_Rneg_AGL"};

    array<TString, NDet> AESigNameMC {"He4_MC_Rpos_TOF", "He4_MC_Rpos_NaF", "He4_MC_Rpos_AGL"};
    array<TString, NDet> AEBkgNameMCn{"He4_MC_Rneg_TOF", "He4_MC_Rneg_NaF", "He4_MC_Rneg_AGL"};

    //Efficiency cuts on scores
    double pCL2[1]{1-MyPcl2};//10 percentile -> 90% efficiency on RigLabel=1
    double pAE[1]{MyPae};//95 percentile -> 90% efficiency on R>0 anomaly score
    double CL2cut[1]{0}, AEcut[1]{0}; //scores>=CL2cut && anomaly<=AEcut
    array<ostringstream, NDet> CL2cutISS, CL2cutMC, AEcutISS, AEcutMC;
    array<float, NDet> epsISS, epsMC;
    array<double, NDet> intISS, intErrISS, intISScr, intErrISScr, intMC, intErrMC, intMCcr, intErrMCcr, tau;
    array<TString, NDet> ISSposSel, ISSnegSel, MCposSel, MCnegSel, ISSnegSelCR, MCnegSelCR;
    

    //Drawings lines
    array<TString, NDet> ISSpos_toDraw, ISSneg_toDraw, ISSnegCR_toDraw, MCpos_toDraw, MCneg_toDraw, MCnegCR_toDraw;
    array<TString, NDet> DrawMass {"(2.*abs(Rinner)*sqrt(1-pow(beta_tof,2)))/beta_tof>>+",
        "(2.*abs(Rinner)*sqrt(1-pow(beta_rich,2)))/beta_rich>>+",
        "(2.*abs(Rinner)*sqrt(1-pow(beta_rich,2)))/beta_rich>>+"};

    //Mass histograms
    array<TCanvas*, NDet> canva;
    array<TLegend*, NDet> legend;
    array<TGaxis*, NDet> axis;
    array<TH1F*, NDet> hISSpos, hISSneg, hISSnegCR, hMCpos, hMCneg, hMCnegCR;
    array<TString, NDet> hISSposName {"TOF_ISSpos", "NaF_ISSpos",   "AGL_ISSpos"};
    array<TString, NDet> hISSnegName {"TOF_ISSneg", "NaF_ISSneg",   "AGL_ISSneg"};
    array<TString, NDet> hISSnegNameCR{"TOF_ISSnegCR", "NaF_ISSnegCR",   "AGL_ISSnegCR"};
    array<TString, NDet> hMCposName  {"TOF_MCpos",   "NaF_MCpos",   "AGL_MCpos"};
    array<TString, NDet> hMCnegName  {"TOF_MCneg",   "NaF_MCneg",   "AGL_MCneg"};
    array<TString, NDet> hMCnegNameCR{"TOF_MCnegCR", "NaF_MCnegCR", "AGL_MCnegCR"};
    array<TString, NDet> hISSposTitle{"ISS data (R_{INNER}>0); Mass [GeV/c^{2}]; Events / 0.1 [GeV/c^{2}]",
                                      "ISS data (R_{INNER}>0); Mass [GeV/c^{2}]; Events / 0.1 [GeV/c^{2}]",
                                      "ISS data (R_{INNER}>0); Mass [GeV/c^{2}]; Events / 0.1 [GeV/c^{2}]"};
    array<TString, NDet> hISSnegTitle{"ISS data (R_{INNER}<0); Mass [GeV/c^{2}]; Events / 0.2 [GeV/c^{2}]",
                                      "ISS data (R_{INNER}<0); Mass [GeV/c^{2}]; Events / 0.2 [GeV/c^{2}]",
                                      "ISS data (R_{INNER}<0); Mass [GeV/c^{2}]; Events / 0.2 [GeV/c^{2}]"};
    array<TString, NDet> hISSnegTitleCR{"ISS data (R_{INNER}<0) CR; Mass [GeV/c^{2}]; Events / 0.2 [GeV/c^{2}]",
                                      "ISS data (R_{INNER}<0) CR; Mass [GeV/c^{2}]; Events / 0.2 [GeV/c^{2}]",
                                      "ISS data (R_{INNER}<0) CR; Mass [GeV/c^{2}]; Events / 0.2 [GeV/c^{2}]"};
    array<TString, NDet> hMCposTitle{"^{4}He MC (R_{INNER}>0); Mass [GeV/c^{2}]; Weights / 0.1 [GeV/c^{2}]",
                                     "^{4}He MC (R_{INNER}>0); Mass [GeV/c^{2}]; Weights / 0.1 [GeV/c^{2}]",
                                     "^{4}He MC (R_{INNER}>0); Mass [GeV/c^{2}]; Weights / 0.1 [GeV/c^{2}]"};
    array<TString, NDet> hMCnegTitle{"^{4}He MC (R_{INNER}<0); Mass [GeV/c^{2}]; Weights / 0.2 [GeV/c^{2}]",
                                     "^{4}He MC (R_{INNER}<0); Mass [GeV/c^{2}]; Weights / 0.2 [GeV/c^{2}]",
                                     "^{4}He MC (R_{INNER}<0); Mass [GeV/c^{2}]; Weights / 0.2 [GeV/c^{2}]"};
    array<TString, NDet> hMCnegTitleCR{"^{4}He MC (R_{INNER}<0) CR; Mass [GeV/c^{2}]; Weights / 0.2 [GeV/c^{2}]",
                                       "^{4}He MC (R_{INNER}<0) CR; Mass [GeV/c^{2}]; Weights / 0.2 [GeV/c^{2}]",
                                       "^{4}He MC (R_{INNER}<0) CR; Mass [GeV/c^{2}]; Weights / 0.2 [GeV/c^{2}]"};



    array<TString, NDet> outTXT;
    array<ofstream, NDet> fileTXT;
    for(int i=0; i<NDet; ++i) {
        if(FirstNNs) outTXT.at(i) = "/eos/home-f/frrossi/AMS/Significance/"+dets.at(i)+"_FirstNNs.txt";
        else outTXT.at(i) = "/eos/home-f/frrossi/AMS/Significance/"+dets.at(i)+".txt";
        if(!Update) remove(outTXT.at(i).Data());
        fileTXT.at(i).open(outTXT.at(i).Data(), std::ios::out | std::ios::app);
        fileTXT.at(i) << std::fixed << std::setprecision(5);
        if(!Update){
            fileTXT.at(i) << right << setw(20)  << "eff CL2" 
                << setw(20)  << "eff R>0" 
                << setw(20)  << "ISS CL2" 
                << setw(20)  << "setISS AE"
                << setw(20)  << "MC CL2"
                << setw(20)  << "MC AE"
                << setw(20)  << "epsISSpos"
                << setw(20)  << "epsMCpos"
                << setw(20)  << "epsISSneg" 
                << setw(20)  << "epsMCneg"
                << setw(20)  << "scalePos"
                << setw(20)  << "scaleNeg"
                << "\n";
        }
    }
    
    printf("/////////////\n");
    printf("scores cuts: \t eff RigLabel==1 -> %.2f eff R>0 -> %.2f\n", 1-pCL2[0], pAE[0]);
    for(int i=0; i<NDet; i++){
        printf("\n\n===> %s\n", dets.at(i).Data());
        if(verbose) printf("Getting TH1 for CL2 (RigLabel==1) and AE (R>0) ...\n");

        TString dir = dets.at(i)+"/CL2/"+CL2SigNameISS.at(i);
        hCL2SigISS.at(i)  = (TH1F*)f_ISSs->Get(dir.Data());
        hCL2SigISS.at(i)->GetQuantiles(1, CL2cut, pCL2);
        CL2cutISS.at(i)  << fixed << setprecision(5) << CL2cut[0];
        dir = dets.at(i)+"/CL2/"+CL2SigNameISSn.at(i); hCL2SigISSn.at(i) = (TH1F*)f_ISSs->Get(dir.Data());
        dir = dets.at(i)+"/CL2/"+CL2BkgNameISSn.at(i); hCL2BkgISSn.at(i) = (TH1F*)f_ISSs->Get(dir.Data());

        dir = dets.at(i)+"/CL2/"+CL2SigNameMC.at(i);
        hCL2SigMC.at(i) = (TH1F*)f_MCs->Get(dir.Data());
        hCL2SigMC.at(i)->GetQuantiles(1, CL2cut, pCL2);
        CL2cutMC.at(i)  << fixed << setprecision(5) << CL2cut[0];
        dir = dets.at(i)+"/CL2/"+CL2SigNameMCn.at(i); hCL2SigMCn.at(i) = (TH1F*)f_ISSs->Get(dir.Data());
        dir = dets.at(i)+"/CL2/"+CL2BkgNameMCn.at(i); hCL2BkgMCn.at(i) = (TH1F*)f_ISSs->Get(dir.Data());

        dir = dets.at(i)+"/AE/"+AESigNameISS.at(i);
        hAESigISS.at(i) = (TH1F*)f_ISSs->Get(dir.Data());
        hAESigISS.at(i)->GetQuantiles(1, AEcut, pAE);
        AEcutISS.at(i)  << fixed << setprecision(5) << AEcut[0];
        dir = dets.at(i)+"/AE/"+AEBkgNameISSn.at(i); hAEBkgISSn.at(i) = (TH1F*)f_ISSs->Get(dir.Data());


        dir = dets.at(i)+"/AE/"+AESigNameMC.at(i);
        hAESigMC.at(i) = (TH1F*)f_MCs->Get(dir.Data());
        hAESigMC.at(i)->GetQuantiles(1, AEcut, pAE);
        AEcutMC.at(i)  << fixed << setprecision(5) << AEcut[0];
        dir = dets.at(i)+"/AE/"+AEBkgNameMCn.at(i); hAEBkgMCn.at(i) = (TH1F*)f_ISSs->Get(dir.Data());

        if(verbose) printf("CL2: %f, %f, %f. %f \n \t %f, %f, \n AE: %f, %f, %f, %f\n\n",
                    hCL2SigISS.at(i)->GetEntries(),  hCL2SigMC.at(i)->GetEntries(),
                    hCL2SigISSn.at(i)->GetEntries(), hCL2SigMCn.at(i)->GetEntries(),
                    hCL2BkgISSn.at(i)->GetEntries(), hCL2BkgMCn.at(i)->GetEntries(),
                    hAESigISS.at(i)->GetEntries(),   hAESigMC.at(i)->GetEntries(),
                    hAEBkgISSn.at(i)->GetEntries(),  hAEBkgMCn.at(i)->GetEntries());
        
        //Definition of Mass histograms
        if(verbose) printf("Defining mass TH1...\n");
        DefDir->cd();
        hISSpos.at(i)   = new TH1F(hISSposName.at(i).Data(), hISSposTitle.at(i).Data(),   MassBins,   MinMass, MaxMass);
        hISSneg.at(i)   = new TH1F(hISSnegName.at(i).Data(), hISSnegTitle.at(i).Data(),   MassBins/2, MinMass, MaxMass);
        hISSnegCR.at(i) = new TH1F(hISSnegNameCR.at(i).Data(), hISSnegTitleCR.at(i).Data(), MassBins/2, MinMass, MaxMass);
        hMCpos.at(i)    = new TH1F(hMCposName.at(i).Data(),   hMCposTitle.at(i).Data(),   MassBins,   MinMass, MaxMass);
        hMCneg.at(i)    = new TH1F(hMCnegName.at(i).Data(),   hMCnegTitle.at(i).Data(),   MassBins/2, MinMass, MaxMass);
        hMCnegCR.at(i)  = new TH1F(hMCnegNameCR.at(i).Data(), hMCnegTitleCR.at(i).Data(), MassBins/2, MinMass, MaxMass);

        //Definition of Event selections
        if(verbose) printf("Event selection...\n");
        if(FirstNNs){
            ISSposSel.at(i) = MinSel.at(i)+BetaQual.at(i)+
                            " && scores_pos>="+TString(CL2cutISS.at(i).str()) +
                            " && anomaly_score<="+TString(AEcutISS.at(i).str())+
                            BetaQual.at(i);
            ISSnegSel.at(i) = MinSel.at(i)+
                            " && scores_neg>="+TString(CL2cutISS.at(i).str()) +
                            " && NEGanomaly_score<="+TString(AEcutISS.at(i).str()) +
                            BetaQual.at(i);
            ISSnegSelCR.at(i) = MinSel.at(i)+
                            " && scores_neg<"+TString(CL2cutISS.at(i).str()) +
                            " && NEGanomaly_score>"+TString(AEcutISS.at(i).str()) +
                            BetaQual.at(i);
            MCposSel.at(i)  = MinSel.at(i)+
                            " && scores_pos>="+TString(CL2cutMC.at(i).str()) +
                            " && anomaly_score<="+TString(AEcutMC.at(i).str())+
                            BetaQual.at(i);
            MCnegSel.at(i)  = MinSel.at(i)+
                            " && scores_neg>="+TString(CL2cutMC.at(i).str()) +
                            " && NEGanomaly_score<="+TString(AEcutMC.at(i).str()) +
                            BetaQual.at(i);
            MCnegSelCR.at(i) = MinSel.at(i)+
                            " && scores_neg<"+TString(CL2cutMC.at(i).str()) +
                            "&& NEGanomaly_score>"+TString(AEcutMC.at(i).str())+
                            BetaQual.at(i);
        }
        else{
            ISSposSel.at(i) = Sel.at(i) + 
                "&& scores_pos>="+TString(CL2cutISS.at(i).str()) +
                "&& anomaly_score<="+TString(AEcutISS.at(i).str());
            ISSnegSel.at(i) = Sel.at(i) +
                "&& scores_neg>="+TString(CL2cutISS.at(i).str()) +
                "&& NEGanomaly_score<="+TString(AEcutISS.at(i).str());
            ISSnegSelCR.at(i) = Sel.at(i) +
                "&& scores_neg<"+TString(CL2cutISS.at(i).str()) +
                "&& NEGanomaly_score>"+TString(AEcutISS.at(i).str());
            MCposSel.at(i)  = Sel.at(i) +
                "&& scores_pos>="+TString(CL2cutMC.at(i).str()) +
                "&& anomaly_score<="+TString(AEcutMC.at(i).str());
            MCnegSel.at(i)  = Sel.at(i) +
                "&& scores_neg>="+TString(CL2cutMC.at(i).str()) +
                "&& NEGanomaly_score<="+TString(AEcutMC.at(i).str());
            MCnegSelCR.at(i)  = Sel.at(i) +
                "&& scores_neg<"+TString(CL2cutMC.at(i).str()) +
                "&& NEGanomaly_score>"+TString(AEcutMC.at(i).str());
        }
        
        MCposSel.at(i).Prepend("(");   MCposSel.at(i).Append(")*weight");
        MCnegSel.at(i).Prepend("(");   MCnegSel.at(i).Append(")*weight");
        MCnegSelCR.at(i).Prepend("("); MCnegSelCR.at(i).Append(")*weight");
        if(verbose) printf("Common selection: %s \n", Sel.at(i).Data()); 
        printf("\t%s %s\n\t%s %s\n",
                TString("ISS_CL2scores>="+TString(CL2cutISS.at(i).str())).Data(), TString("ISS_AEscore<="+TString(AEcutISS.at(i).str())).Data(), 
                TString("MC_CL2scores>="+TString(CL2cutMC.at(i).str())).Data(), TString("MC_AEscore<="+TString(AEcutMC.at(i).str())).Data());
        fileTXT.at(i) << right << setw(20)  << 1-pCL2[0]
                << setw(20)  << pAE[0]
                << setw(20)  << TString(CL2cutISS.at(i).str()).Data()
                << setw(20)  << TString(AEcutISS.at(i).str()).Data()
                << setw(20)  << TString(CL2cutMC.at(i).str()).Data()
                << setw(20)  << TString(AEcutMC.at(i).str()).Data();

        //Definition of drawing lines
        if(verbose) printf("Defining drawing lines...\n");
        ISSpos_toDraw.at(i)   = DrawMass.at(i)+hISSpos.at(i)->GetName();
        ISSneg_toDraw.at(i)   = DrawMass.at(i)+hISSneg.at(i)->GetName();
        ISSnegCR_toDraw.at(i) = DrawMass.at(i)+hISSnegCR.at(i)->GetName();

        MCpos_toDraw.at(i)    = DrawMass.at(i)+hMCpos.at(i)->GetName();
        MCneg_toDraw.at(i)    = DrawMass.at(i)+hMCneg.at(i)->GetName();
        MCnegCR_toDraw.at(i)  = DrawMass.at(i)+hMCnegCR.at(i)->GetName();

        //Drawings
        if(verbose) printf("Getting mass Th1...\n");
        ISS.at(2)->Draw(ISSpos_toDraw.at(i).Data(),ISSposSel.at(i).Data(),"goff");
        hISSpos.at(i) = (TH1F*)gDirectory->Get(hISSpos.at(i)->GetName());
        hISSpos.at(i)->SetMarkerStyle(20); hISSpos.at(i)->SetLineWidth(LW); hISSpos.at(i)->SetMarkerColor(kBlack); hISSpos.at(i)->SetLineColor(kBlack);
        hISSpos.at(i)->GetXaxis()->CenterTitle(true); hISSpos.at(i)->GetXaxis()->SetLabelFont(62); hISSpos.at(i)->GetXaxis()->SetTitleFont(62); 
        hISSpos.at(i)->GetYaxis()->CenterTitle(true); hISSpos.at(i)->GetYaxis()->SetLabelFont(62); hISSpos.at(i)->GetYaxis()->SetTitleFont(62); hISSpos.at(i)->GetYaxis()->SetMaxDigits(2);
        ISS.at(3)->Draw(ISSneg_toDraw.at(i).Data(),ISSnegSel.at(i).Data(), "goff");
        hISSneg.at(i) = (TH1F*)gDirectory->Get(hISSneg.at(i)->GetName());
        hISSneg.at(i)->SetLineColor(kGreen+1); hISSneg.at(i)->SetLineWidth(LW); hISSneg.at(i)->SetFillColor(kGreen);
        ISS.at(3)->Draw(ISSnegCR_toDraw.at(i).Data(),ISSnegSelCR.at(i).Data(),"goff");
        hISSnegCR.at(i) = (TH1F*)gDirectory->Get(hISSnegCR.at(i)->GetName());

        MC.at(0)->Draw(MCpos_toDraw.at(i).Data(),MCposSel.at(i).Data(), "goff");
        hMCpos.at(i) = (TH1F*)gDirectory->Get(hMCpos.at(i)->GetName());
        hMCpos.at(i)->SetLineWidth(LW); hMCpos.at(i)->SetFillColor(kCyan); hMCpos.at(i)->SetFillStyle(3004);
        hMCpos.at(i)->GetXaxis()->CenterTitle(true); hISSpos.at(i)->GetXaxis()->SetLabelFont(62); hISSpos.at(i)->GetXaxis()->SetTitleFont(62);
        hMCpos.at(i)->GetYaxis()->CenterTitle(true); hISSpos.at(i)->GetYaxis()->SetLabelFont(62); hISSpos.at(i)->GetYaxis()->SetTitleFont(62); hMCpos.at(i)->GetYaxis()->SetMaxDigits(2);
        MC.at(1)->Draw(MCneg_toDraw.at(i).Data(), MCnegSel.at(i).Data(), "goff");
        hMCneg.at(i) = (TH1F*)gDirectory->Get(hMCneg.at(i)->GetName());
        hMCneg.at(i)->SetLineWidth(LW); hMCneg.at(i)->SetFillColor(kViolet); hMCneg.at(i)->SetLineColor(kMagenta); hMCneg.at(i)->SetFillStyle(3004);
        MC.at(1)->Draw(MCnegCR_toDraw.at(i).Data(), MCnegSelCR.at(i).Data(), "goff");
        hMCnegCR.at(i) = (TH1F*)gDirectory->Get(hMCnegCR.at(i)->GetName());
        /*printf("Number of entries: %f, %f, %f, %f\n", 
                hISSpos.at(i)->Integral(), hISSneg.at(i)->Integral(),
                hMCpos.at(i)->Integral(),  hMCneg.at(i)->Integral());
        */
        
        //Scaling factors for negatives
        CL2sFactor.at(i) = (hCL2SigISSn.at(i)->Integral()+hCL2BkgISSn.at(i)->Integral()) /
                           (hCL2SigMCn.at(i)->Integral() +hCL2BkgMCn.at(i)->Integral());
        AEsFactor.at(i)  = hAEBkgISSn.at(i)->Integral() / hAEBkgMCn.at(i)->Integral();

        //Scale factor MC integral = ISS data integral
        if(verbose) printf("Computing scale factors for MC...\n");
        epsISS.at(i) = hISSpos.at(i)->Integral()/ISS.at(2)->GetEntries();
        epsMC.at(i)  = hMCpos.at(i)->Integral() /MCtotEvents;
        printf("Check on efficiencies: %f, %f\n", epsISS.at(i), epsMC.at(i));
        fileTXT.at(i) << right << setw(20)  << epsISS.at(i)
                << setw(20)  << epsMC.at(i);
        if(i!=0) hMCpos.at(i)->Scale((epsISS.at(i)/epsMC.at(i))*(ISS.at(2)->GetEntries()/MCtotEvents)*0.9); // Only He4 MC;
        else     hMCpos.at(i)->Scale((epsISS.at(i)/epsMC.at(i))*(ISS.at(2)->GetEntries()/MCtotEvents)*0.85); // Only He4 MC;
        
        epsISS.at(i) = hISSneg.at(i)->Integral()/ISS.at(3)->GetEntries();
        epsMC.at(i)  = hMCneg.at(i)->Integral() /MCtotEventsN;
        printf("Check on efficiencies: %f, %f\n", epsISS.at(i), epsMC.at(i));
        fileTXT.at(i) << right << setw(20)  << epsISS.at(i)
                << setw(20)  << epsMC.at(i);
        //hMCneg.at(i)->Scale((epsISS.at(i)/epsMC.at(i))*(ISS.at(3)->GetEntries()/MCtotEventsN));
        // hMCneg.at(i)->Scale(CL2sFactor.at(i)*AEsFactor.at(i));
        // hMCnegCR.at(i)->Scale(CL2sFactor.at(i)*AEsFactor.at(i));
        printf("Check on scale factors: %f, %f\n",
                hISSpos.at(i)->Integral()/hMCpos.at(i)->Integral(), hISSneg.at(i)->Integral()/hMCneg.at(i)->Integral());
        fileTXT.at(i) << right << setw(20)  << hISSpos.at(i)->Integral()/hMCpos.at(i)->Integral()
                << setw(20)  << hISSneg.at(i)->Integral()/hMCneg.at(i)->Integral()
                << "\n";
            
        //Getting integrals inside the He mass region [2,5] GeV/c^2
        intISS.at(i)   = hISSneg.at(i)->IntegralAndError(hISSneg.at(i)->FindFixBin(2),     hISSneg.at(i)->FindFixBin(5),   intErrISS.at(i));
        intISScr.at(i) = hISSnegCR.at(i)->IntegralAndError(hISSnegCR.at(i)->FindFixBin(2), hISSnegCR.at(i)->FindFixBin(5), intErrISScr.at(i));
        intMC.at(i)  = hMCneg.at(i)->IntegralAndError( hMCneg.at(i)->FindFixBin(2),    hMCneg.at(i)->FindFixBin(5),   intErrMC.at(i));
        intMCcr.at(i)= hMCnegCR.at(i)->IntegralAndError(hMCnegCR.at(i)->FindFixBin(2), hMCnegCR.at(i)->FindFixBin(5), intErrMCcr.at(i));
        
        (intMC.at(i)==0)? tau.at(i) = 0 :tau.at(i) = intMCcr.at(i)/intMC.at(i);


        //Canvas
        if(verbose) printf("Creating canvas...\n");
        TString c_name = TString("c_")+dets.at(i);
        canva.at(i) = new TCanvas(c_name.Data(), c_name.Data(), 600, 600); canva.at(i)->cd(); canva.at(i)->SetGrid();
        //hMCpos.at(i)->GetYaxis()->SetRange(0,max(hMCpos.at(i)->GetMaximum()*1.1,hISSpos.at(i)->GetMaximum()*1.1));
        hISSpos.at(i)->Draw("e");
        hMCpos.at(i)->Draw("hist same");
        canva.at(i)->Update();
        
        Float_t rightmax = 3*max(hISSneg.at(i)->GetMaximum(), hMCneg.at(i)->GetMaximum()); Float_t scale = gPad->GetUymax()/rightmax;
        hMCneg.at(i)->Scale(scale); hISSneg.at(i)->Scale(scale); hISSneg.at(i)->Draw("hist same"); hMCneg.at(i)->Draw("hist same");
        //printf("rightmax: %f, Uymax: %f, Uxmin:%f, Uxmax:%f \n", rightmax, gPad->GetUymax(), gPad->GetUxmin(), gPad->GetUxmax());
        legend.at(i) = canva.at(i)->BuildLegend(); legend.at(i)->SetBorderSize(0); legend.at(i)->SetNColumns(2);
        hISSpos.at(i)->Draw("e same");
        axis.at(i) = new TGaxis(gPad->GetUxmax(),gPad->GetUymin(),gPad->GetUxmax(), gPad->GetUymax(),0,rightmax,510,"+L");
        axis.at(i)->SetTitle("Events / 0.2 [GeV/c^{2}]"); axis.at(i)->SetTitleColor(kGreen+1); axis.at(i)->SetLineColor(kGreen+1); axis.at(i)->SetLabelColor(kGreen+1);
        axis.at(i)->CenterTitle(true); axis.at(i)->SetLabelSize(hISSpos.at(i)->GetYaxis()->GetLabelSize());
        axis.at(i)->SetTitleSize(hISSpos.at(i)->GetYaxis()->GetTitleSize()); axis.at(i)->Draw();    
    }

    array<double, NDet> myZ1 = GetSignificance(intISS, intMC);
    array<double, NDet> myZ2 = GetSignificance2(intISS, intISScr, tau);

    TString sigTXT = outName; sigTXT.ReplaceAll(".root",".txt");
    if(!Update) remove(sigTXT.Data());
    ofstream MysigTXT(sigTXT.Data(), std::ios::out | std::ios::app);
    MysigTXT << std::fixed << std::setprecision(5);
    if(!Update){
        MysigTXT << right  << setw(20)  << "DETECTOR"
            << setw(20) << "eff CL2"
            << setw(20) << "eff AE"
            << setw(20) << "ISS" 
            << setw(20) << "+-Err" 
            << setw(20) << "MC"
            << setw(20) << "+-Err"
            << setw(20) << "MCcr"
            << setw(20) << "+-Err"
            << setw(20) << "ISScr"
            << setw(20) << "+-Err"
            << setw(20) << "Sign1"
            << setw(20) << "Sign2"
            << "\n";
    }
    for(int i=0; i<NDet; ++i) MysigTXT << right << setw(20)  << dets.at(i).Data()
                                      << setw(20)  << 1-pCL2[0]
                                      << setw(20)  << pAE[0]
                                      << setw(20)  << intISS.at(i) 
                                      << setw(20)  << intErrISS.at(i)
                                      << setw(20)  << intMC.at(i)
                                      << setw(20)  << intErrMC.at(i)
                                      << setw(20)  << intMCcr.at(i) 
                                      << setw(20)  << intErrMCcr.at(i)
                                      << setw(20)  << intISScr.at(i)
                                      << setw(20)  << intErrISScr.at(i)
                                      << setw(20)  << myZ1.at(i) 
                                      << setw(20)  << myZ2.at(i) 
                                      <<"\n"; 
    MysigTXT.close();
    for(int i=0; i<NDet; ++i) fileTXT.at(i).close();

    TFile *f_out;
    (Update) ? printf("Saving to root file: %s with option UPDATE\n", outName.Data()) : printf("Saving to root file: %s with option RECREATE\n", outName.Data());
    if(Update) f_out = new TFile(outName.Data(),"UPDATE");
    else f_out = new TFile(outName.Data(),"RECREATE");
    
    for(int i=0; i<NDet; ++i) {
        f_out->cd();
        if(!Update) f_out->mkdir(dets.at(i).Data());
        TString MyDir = dets.at(i)+"/"+"AEeff_"+effAE.str()+"_CL2eff_"+effCL2.str();
        f_out->mkdir(MyDir.Data());
        f_out->cd(MyDir.Data());
        hISSpos.at(i)->Write(); hISSneg.at(i)->Write();
        hMCpos.at(i)->Write();  hMCneg.at(i)->Write();
        canva.at(i)->Write();
        delete hISSpos.at(i);  delete hISSneg.at(i);
        delete hISSnegCR.at(i);
        delete hMCpos.at(i);   delete hMCneg.at(i);
        delete hMCnegCR.at(i);
        delete canva.at(i);
    }
    f_out->Close(); delete f_out;

    return array<array<double, NDet>,6>{intISS, intErrISS, intMC, intErrMC, intISScr, tau};
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
