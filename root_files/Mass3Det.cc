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


void Mass3Det(){
    gStyle->SetPalette(55);//kRainBow palette
    gStyle->SetOptStat(0);
    TDirectory *DefDir = gDirectory;
    //printf("First check on dir\n");
    //DefDir->pwd();
    const int NDet= 3;
    const int LW  = 3;
    const float he3 = 2.81, he4 = 3.72; // GeV/c2
    const float bNaF_thL = 0.75,  bNaF_thH = 0.993;
    const float bAGL_thL = 0.953, bAGL_thH = 0.998;
    //NaF
    const float r3NaF_thL = he3 * bNaF_thL * 1./sqrt(1-pow(bNaF_thL,2)) / 2;
    //AGL
    const float r3AGL_thL = he3 * bAGL_thL * 1./sqrt(1-pow(bAGL_thL,2)) / 2;

    const int MassBins = 100;
    const float MinMass = 0, MaxMass = 10; // GeV/c^2

    vector<TTree*> ISS, MC;
    ISS = fromTerminal(false, "2025_06_30", "2025_06_30", "2025_06_30", "2025_06_30");
    MC  = fromTerminal(true,  "2025_06_30", "2025_06_30", "2025_06_30", "2025_06_30");
    //printf("Second check on dir\n");
    DefDir->cd();
    //gDirectory->pwd();
    TH1F *hMC_TotPos = new TH1F("hMC_TotPos","hMC_TotPos; log_{10}(R_{INNER}); Weights", 500, -2, 600);
    TH1F *hMC_TotNeg = new TH1F("hMC_TotNeg","hMC_TotNeg; log_{10}(R_{INNER}); Weights", 500, -2, 600);
    MC.at(0)->Draw("log10(abs(Rinner))>>+hMC_TotPos","1*weight","goff");
    hMC_TotPos = (TH1F*)gDirectory->Get("hMC_TotPos");
    MC.at(1)->Draw("log10(abs(Rinner))>>+hMC_TotNeg","1*weight","goff");
    hMC_TotNeg = (TH1F*)gDirectory->Get("hMC_TotNeg");
    const float MCtotEvents  = hMC_TotPos->Integral();
    const float MCtotEventsN = hMC_TotNeg->Integral();
    delete hMC_TotPos; delete hMC_TotNeg;
    printf("Check on MC TOT events: %f, %f\n", MCtotEvents, MCtotEventsN);


    array<TString, NDet> Sel{"chi2Coo_tof < 4 && chi2Time_tof < 10 && beta_tof>=0.8 && beta_tof<=0.96",
                             "hasRich==1 && beta_rich>=0.75 && beta_rich<=0.993 && isNaF==1 && "
                               "isBorder_rich==0 && kprob_rich>0.01 && 1<charge2_rich && "
                               "charge2_rich<4 && ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4",
                             "hasRich==1 && beta_rich>=0.953 && beta_rich<=0.998 && isNaF==0 && "
                              "hasGoodImpact==1 && kprob_rich>0.01 && 1<charge2_rich && "
                              "charge2_rich<4 && ringPMTs2_rich>5 && (measPE_Corr_rich/totPE_Uncorr_rich)>0.4"};

    ostringstream RvalNaF; RvalNaF  << fixed << setprecision(3) << r3NaF_thL;
    ostringstream RvalAGL; RvalAGL  << fixed << setprecision(3) << r3AGL_thL;
    array<TString, NDet> R3abTH{"&& abs(Rinner)-(3*pow(Rinner,2)*invRerr_IN)>=1.92",
        " && abs(Rinner)-(3*pow(Rinner,2)*invRerr_IN)>="+TString(RvalNaF.str()),
        " && abs(Rinner)-(3*pow(Rinner,2)*invRerr_IN)>="+TString(RvalAGL.str())};

    /*
    printf("====>\n");
    printf("Common selections:\n");
    printf("====>\n");

    printf("\t TOF selection:\n ");
    printf("%s\n\n", Sel.at(0).Data());
    printf("\t RICH-NaF selection:\n");
    printf("%s\n\n", Sel.at(1).Data());
    printf("\t RICH-Agl selection:\n");
    printf("%s\n\n", Sel.at(2).Data());
    */

    //Scores TH1
    TString ScoreFileName;
    ScoreFileName = "NNs_TrainOnMC.root";  TFile *f_MCs  = new TFile(ScoreFileName.Data(),"OPEN");
    ScoreFileName = "NNs_TrainOnISS.root"; TFile *f_ISSs = new TFile(ScoreFileName.Data(),"OPEN");
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
    double pCL2[1]{0.33};//10 percentile -> 90% efficiency on RigLabel=1
    double pAE[1]{0.95};//95 percentile -> 90% efficiency on R>0 anomaly score
    double CL2cut[1]{0}, AEcut[1]{0}; //scores>=CL2cut && anomaly<=AEcut
    array<ostringstream, NDet> CL2cutISS, CL2cutMC, AEcutISS, AEcutMC;
    array<float, NDet> epsISS, epsMC;
    array<double, NDet> intISS, intErrISS, intMC, intErrMC;
    array<TString, NDet> ISSposSel, ISSnegSel, MCposSel, MCnegSel;
    

    //Drawings lines
    array<TString, NDet> ISSpos_toDraw, ISSneg_toDraw, MCpos_toDraw, MCneg_toDraw;
    array<TString, NDet> DrawMass {"(2.*abs(Rinner)*sqrt(1-pow(beta_tof,2)))/beta_tof>>+",
        "(2.*abs(Rinner)*sqrt(1-pow(beta_rich,2)))/beta_rich>>+",
        "(2.*abs(Rinner)*sqrt(1-pow(beta_rich,2)))/beta_rich>>+"};

    //Mass histograms
    array<TCanvas*, NDet> canva;
    array<TLegend*, NDet> legend;
    array<TGaxis*, NDet> axis;
    array<TH1F*, NDet> hISSpos, hISSneg, hMCpos, hMCneg;
    array<TString, NDet> hISSposName{"TOF_ISSpos", "NaF_ISSpos", "AGL_ISSpos"};
    array<TString, NDet> hISSnegName{"TOF_ISSneg", "NaF_ISSneg", "AGL_ISSneg"};
    array<TString, NDet> hMCposName{"TOF_MCpos", "NaF_MCpos", "AGL_MCpos"};
    array<TString, NDet> hMCnegName{"TOF_MCneg", "NaF_MCneg", "AGL_MCneg"};
    array<TString, NDet> hISSposTitle{"ISS data (R_{INNER}>0); Mass [GeV/c^{2}]; Events / 0.1 [GeV/c^{2}]",
                                      "ISS data (R_{INNER}>0); Mass [GeV/c^{2}]; Events / 0.1 [GeV/c^{2}]",
                                      "ISS data (R_{INNER}>0); Mass [GeV/c^{2}]; Events / 0.1 [GeV/c^{2}]"};
    array<TString, NDet> hISSnegTitle{"ISS data (R_{INNER}<0); Mass [GeV/c^{2}]; Events / 0.2 [GeV/c^{2}]",
                                      "ISS data (R_{INNER}<0); Mass [GeV/c^{2}]; Events / 0.2 [GeV/c^{2}]",
                                      "ISS data (R_{INNER}<0); Mass [GeV/c^{2}]; Events / 0.2 [GeV/c^{2}]"};
    array<TString, NDet> hMCposTitle{"^{4}He MC (R_{INNER}>0); Mass [GeV/c^{2}]; Weights / 0.1 [GeV/c^{2}]",
                                     "^{4}He MC (R_{INNER}>0); Mass [GeV/c^{2}]; Weights / 0.1 [GeV/c^{2}]",
                                     "^{4}He MC (R_{INNER}>0); Mass [GeV/c^{2}]; Weights / 0.1 [GeV/c^{2}]"};
    array<TString, NDet> hMCnegTitle{"^{4}He MC (R_{INNER}<0); Mass [GeV/c^{2}]; Weights / 0.2 [GeV/c^{2}]",
                                     "^{4}He MC (R_{INNER}<0); Mass [GeV/c^{2}]; Weights / 0.2 [GeV/c^{2}]",
                                     "^{4}He MC (R_{INNER}<0); Mass [GeV/c^{2}]; Weights / 0.2 [GeV/c^{2}]"};
    
    for(int i=0; i<NDet; i++){
        printf("\n\n===> %s\n", dets.at(i).Data());
        printf("Getting TH1 for CL2 (RigLabel==1) and AE (R>0) ...\n");

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

        printf("CL2: %f, %f, %f. %f \n \t %f, %f, \n AE: %f, %f, %f, %f\n\n",
                hCL2SigISS.at(i)->GetEntries(),  hCL2SigMC.at(i)->GetEntries(),
                hCL2SigISSn.at(i)->GetEntries(), hCL2SigMCn.at(i)->GetEntries(),
                hCL2BkgISSn.at(i)->GetEntries(), hCL2BkgMCn.at(i)->GetEntries(),
                hAESigISS.at(i)->GetEntries(),   hAESigMC.at(i)->GetEntries(),
                hAEBkgISSn.at(i)->GetEntries(),  hAEBkgMCn.at(i)->GetEntries());
        
        //Definition of Mass histograms
        //printf("Defining mass TH1...\n");
        DefDir->cd();
        hISSpos.at(i) = new TH1F(hISSposName.at(i).Data(), hISSposTitle.at(i).Data(), MassBins, MinMass, MaxMass);
        hISSneg.at(i) = new TH1F(hISSnegName.at(i).Data(), hISSnegTitle.at(i).Data(), MassBins/2, MinMass, MaxMass);
        hMCpos.at(i)  = new TH1F(hMCposName.at(i).Data(),   hMCposTitle.at(i).Data(),  MassBins, MinMass, MaxMass);
        hMCneg.at(i)  = new TH1F(hMCnegName.at(i).Data(),   hMCnegTitle.at(i).Data(),  MassBins/2, MinMass, MaxMass);

        //Definition of Event selections
        printf("Event selection...\n");
        ISSposSel.at(i) = Sel.at(i) + R3abTH.at(i) + 
            "&& scores>="+TString(CL2cutISS.at(i).str()) +
            "&& anomaly_score<="+TString(AEcutISS.at(i).str());
        ISSnegSel.at(i) = Sel.at(i) + R3abTH.at(i) +
            "&& scores>="+TString(CL2cutISS.at(i).str()) +
            "&& anomaly_score<="+TString(AEcutISS.at(i).str());
        MCposSel.at(i)  = Sel.at(i) + R3abTH.at(i) +
            "&& scores>="+TString(CL2cutMC.at(i).str()) +
            "&& anomaly_score<="+TString(AEcutMC.at(i).str());
        MCnegSel.at(i)  = Sel.at(i) + R3abTH.at(i) +
            "&& scores>="+TString(CL2cutMC.at(i).str()) +
            "&& anomaly_score<="+TString(AEcutMC.at(i).str());
        MCposSel.at(i).Prepend("("); MCposSel.at(i).Append(")*weight");
        MCnegSel.at(i).Prepend("("); MCnegSel.at(i).Append(")*weight");
        printf("Common selection: %s \n", Sel.at(i).Data()); 
        printf("\tscores cuts: \t eff RigLabel==1 -> %.2f eff R>0 -> %.2f \n\t%s %s\n\t%s %s\n",
                1-pCL2[0], pAE[0],
                TString("ISS_CL2scores>="+TString(CL2cutISS.at(i).str())).Data(), TString("ISS_AEscore<="+TString(AEcutISS.at(i).str())).Data(), 
                TString("MC_CL2scores>="+TString(CL2cutMC.at(i).str())).Data(), TString("MC_AEscore<="+TString(AEcutMC.at(i).str())).Data());

        //Definition of drawing lines
        //printf("Defining drawing lines...\n");
        ISSpos_toDraw.at(i) = DrawMass.at(i)+hISSpos.at(i)->GetName();
        ISSneg_toDraw.at(i) = DrawMass.at(i)+hISSneg.at(i)->GetName();
        MCpos_toDraw.at(i)  = DrawMass.at(i)+hMCpos.at(i)->GetName();
        MCneg_toDraw.at(i)  = DrawMass.at(i)+hMCneg.at(i)->GetName();

        //Drawings
        //printf("Getting mass Th1...\n");
        ISS.at(2)->Draw(ISSpos_toDraw.at(i).Data(),ISSposSel.at(i).Data(),"goff");
        hISSpos.at(i) = (TH1F*)gDirectory->Get(hISSpos.at(i)->GetName());
        hISSpos.at(i)->SetMarkerStyle(20); hISSpos.at(i)->SetLineWidth(LW); hISSpos.at(i)->SetMarkerColor(kBlack); hISSpos.at(i)->SetLineColor(kBlack);
        hISSpos.at(i)->GetXaxis()->CenterTitle(true); hISSpos.at(i)->GetXaxis()->SetLabelFont(62); hISSpos.at(i)->GetXaxis()->SetTitleFont(62); 
        hISSpos.at(i)->GetYaxis()->CenterTitle(true); hISSpos.at(i)->GetYaxis()->SetLabelFont(62); hISSpos.at(i)->GetYaxis()->SetTitleFont(62); hISSpos.at(i)->GetYaxis()->SetMaxDigits(2);
        ISS.at(3)->Draw(ISSneg_toDraw.at(i).Data(),ISSnegSel.at(i).Data(), "goff");
        hISSneg.at(i) = (TH1F*)gDirectory->Get(hISSneg.at(i)->GetName());
        hISSneg.at(i)->SetLineColor(kGreen+1); hISSneg.at(i)->SetLineWidth(LW); hISSneg.at(i)->SetFillColor(kGreen);
        MC.at(0)->Draw(MCpos_toDraw.at(i).Data(),MCposSel.at(i).Data(), "goff");
        hMCpos.at(i) = (TH1F*)gDirectory->Get(hMCpos.at(i)->GetName());
        hMCpos.at(i)->SetLineWidth(LW); hMCpos.at(i)->SetFillColor(kCyan); hMCpos.at(i)->SetFillStyle(3004);
        hMCpos.at(i)->GetXaxis()->CenterTitle(true); hISSpos.at(i)->GetXaxis()->SetLabelFont(62); hISSpos.at(i)->GetXaxis()->SetTitleFont(62);
        hMCpos.at(i)->GetYaxis()->CenterTitle(true); hISSpos.at(i)->GetYaxis()->SetLabelFont(62); hISSpos.at(i)->GetYaxis()->SetTitleFont(62); hMCpos.at(i)->GetYaxis()->SetMaxDigits(2);
        MC.at(1)->Draw(MCneg_toDraw.at(i).Data(), MCnegSel.at(i).Data(), "goff");
        hMCneg.at(i) = (TH1F*)gDirectory->Get(hMCneg.at(i)->GetName());
        hMCneg.at(i)->SetLineWidth(LW); hMCneg.at(i)->SetFillColor(kViolet); hMCneg.at(i)->SetLineColor(kMagenta); hMCneg.at(i)->SetFillStyle(3004);
        /*printf("Number of entries: %f, %f, %f, %f\n", 
                hISSpos.at(i)->Integral(), hISSneg.at(i)->Integral(),
                hMCpos.at(i)->Integral(),  hMCneg.at(i)->Integral());
        */
        
        //Scaling factors for negatives
        CL2sFactor.at(i) = (hCL2SigISSn.at(i)->Integral()+hCL2BkgISSn.at(i)->Integral()) /
                           (hCL2SigMCn.at(i)->Integral() +hCL2BkgMCn.at(i)->Integral());
        AEsFactor.at(i)  = hAEBkgISSn.at(i)->Integral() / hAEBkgMCn.at(i)->Integral();

        //Scale factor MC integral = ISS data integral
        printf("Computing scale factors for MC...\n");
        epsISS.at(i) = hISSpos.at(i)->Integral()/ISS.at(2)->GetEntries();
        epsMC.at(i)  = hMCpos.at(i)->Integral() /MCtotEvents;
        printf("Check on efficiencies: %f, %f\n", epsISS.at(i), epsMC.at(i));
        if(i!=0) hMCpos.at(i)->Scale((epsISS.at(i)/epsMC.at(i))*(ISS.at(2)->GetEntries()/MCtotEvents)*0.9); // Only He4 MC;
        else     hMCpos.at(i)->Scale((epsISS.at(i)/epsMC.at(i))*(ISS.at(2)->GetEntries()/MCtotEvents)*0.85); // Only He4 MC;
        
        epsISS.at(i) = hISSneg.at(i)->Integral()/ISS.at(3)->GetEntries();
        epsMC.at(i)  = hMCneg.at(i)->Integral() /MCtotEventsN;
        printf("Check on efficiencies: %f, %f\n", epsISS.at(i), epsMC.at(i));
        //hMCneg.at(i)->Scale((epsISS.at(i)/epsMC.at(i))*(ISS.at(3)->GetEntries()/MCtotEventsN));
        hMCneg.at(i)->Scale(CL2sFactor.at(i)*AEsFactor.at(i));
        printf("Check on scale factors: %f, %f\n",
                hISSpos.at(i)->Integral()/hMCpos.at(i)->Integral(), hISSneg.at(i)->Integral()/hMCneg.at(i)->Integral());
            
        //Getting integrals inside the He mass region [2,5] GeV/c^2
        intISS.at(i) = hISSneg.at(i)->IntegralAndError(hISSneg.at(i)->FindFixBin(2), hISSneg.at(i)->FindFixBin(5), intErrISS.at(i));
        intMC.at(i)  = hMCneg.at(i)->IntegralAndError( hMCneg.at(i)->FindFixBin(2),  hMCneg.at(i)->FindFixBin(5),  intErrMC.at(i));

        //Canvas
        printf("Creating canvas...\n");
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

    printf("\n ========================================\n");
    printf("Number of events:\n");
    printf("   \t ISS+-Err \t MC+-Err\n");
    for(int i=0; i<NDet; ++i) printf("%s:\t%.2f+-%.2f\t%.2f+-%.2f\n", dets.at(i).Data(),
                                intISS.at(i), intErrISS.at(i),
                                intMC.at(i),  intErrMC.at(i));
    printf(" ========================================\n");

    TFile *f_out = new TFile("/eos/home-f/frrossi/AMS/NicePlots_CL67_AE95.root","RECREATE");
    
    for(int i=0; i<NDet; ++i) {
        f_out->cd();
        f_out->mkdir(dets.at(i).Data());
        f_out->cd(dets.at(i).Data());
        hISSpos.at(i)->Write(); hISSneg.at(i)->Write();
        hMCpos.at(i)->Write();  hMCneg.at(i)->Write();
        canva.at(i)->Write();
    }

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
    for(int i=0; i<CLtrees.size(); ++i) myTrees.push_back(CLtrees.at(i));


    (isMC) ? printf(" ===> \n Models are trained on MC data with UseLossWeights : %d \n ===> \n", UseLossWeights) : printf(" ===> \n Models are trained on ISS data \n ===> \n");
    
    printf("Returning the trees vector, N. of trees: %d\n", TotTree);
    printf("Sequence of the trees: \n");
    printf("\t 0: MC_pos \t 1: MC_neg \t 2: ISS_pos \t 3: ISS_neg\n\n");
    
    return myTrees;
}
