#include <iostream>
#include <fstream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <array>
#include <vector>

#include <TSystem.h>
#include <TString.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>
#include <TFitResultPtr.h>

using namespace std;
void CL2_labelEff(){
    const float LabelLimit = 0.20;
    const int RecRigBins = 100, LabBins = 202;
    const float minRecRig = 0, maxRecRig = 3.5; //log values
    const float minLab = -1.01, maxLab = 1.01;
    gStyle->SetOptStat(0); gStyle->SetErrorX(0); gStyle->SetPalette(55);


    TFile *fMCpos  = new TFile("/eos/home-f/frrossi/AMS/output/organized_output/MLDs/MC/He4_Positives_2025_09_21.root","OPEN");
    TFile *fMCneg  = new TFile("/eos/home-f/frrossi/AMS/output/organized_output/MLDs/MC/He4_2025_09_21.root","OPEN");
    TFile *fISSpos = new TFile("/eos/home-f/frrossi/AMS/output/organized_output/MLDs/ISS/Positives_2025_09_21.root","OPEN");
    TFile *fISSneg = new TFile("/eos/home-f/frrossi/AMS/output/organized_output/MLDs/ISS/2025_09_21.root","OPEN");

    TTree *MCpos = (TTree*)fMCpos->Get("MC"); TTree *MCneg = (TTree*)fMCneg->Get("MC");
    TTree *ISSpos = (TTree*)fISSpos->Get("ISS"); TTree *ISSneg = (TTree*)fISSneg->Get("ISS");

    TH2F *hMCposL = new TH2F("MCpos_CL_label","MC R_{INNER}>0; log10(|R_{INNER}|); #frac{R_{UH}-R_{LH}}{R_{UH}+R_{LH}}", RecRigBins, minRecRig, maxRecRig, LabBins, minLab, maxLab);
    TH2F *hMCnegL = new TH2F("MCneg_CL_label","MC R_{INNER}<0; log10(|R_{INNER}|); #frac{R_{UH}-R_{LH}}{R_{UH}+R_{LH}}", RecRigBins, minRecRig, maxRecRig, LabBins, minLab, maxLab);
    TH2F *hISSposL = new TH2F("ISSpos_CL_label","ISS R_{INNER}>0; log10(|R_{INNER}|); #frac{R_{UH}-R_{LH}}{R_{UH}+R_{LH}}", RecRigBins, minRecRig, maxRecRig, LabBins, minLab, maxLab);
    TH2F *hISSnegL = new TH2F("ISSneg_CL_label","ISS R_{INNER}<0; log10(|R_{INNER}|); #frac{R_{UH}-R_{LH}}{R_{UH}+R_{LH}}", RecRigBins, minRecRig, maxRecRig, LabBins, minLab, maxLab);

    MCpos->Draw("(RinnerUH-RinnerLH)/(RinnerUH+RinnerLH):log10(Rinner)  >>+MCpos_CL_label","1*weight*weightISO","goff");
    hMCposL = (TH2F*)gDirectory->Get(hMCposL->GetName());
    MCneg->Draw("(RinnerUH-RinnerLH)/(RinnerUH+RinnerLH):log10(-Rinner) >>+MCneg_CL_label","1*weight*weightISO","goff");
    hMCnegL = (TH2F*)gDirectory->Get(hMCnegL->GetName());
    ISSpos->Draw("(RinnerUH-RinnerLH)/(RinnerUH+RinnerLH):log10(Rinner) >>+ISSpos_CL_label","","goff");
    hISSposL = (TH2F*)gDirectory->Get(hISSposL->GetName());
    ISSneg->Draw("(RinnerUH-RinnerLH)/(RinnerUH+RinnerLH):log10(-Rinner)>>+ISSneg_CL_label","","goff");
    hISSnegL = (TH2F*)gDirectory->Get(hISSnegL->GetName());
    
    TH1D *hMCposLX = hMCposL->ProjectionX();
    TH1D *hMCposEff = new TH1D("hMCposEff","MC ^{4}He R(>0);log_{10}(R_{GEN});Efficiency of |#frac{R_{UH}-R_{LH}}{R_{UH}+R_{LH}}| < 0.2",
         hMCposLX->GetNbinsX(), hMCposLX->GetBinLowEdge(1), hMCposLX->GetBinLowEdge(hMCposLX->GetNbinsX()+1));
    hMCposEff->SetMarkerColor(kOrange-3); hMCposEff->SetLineColor(kOrange-3); hMCposEff->SetLineWidth(3);
    TH1D *hMCnegEff = new TH1D("hMCnegEff","MC ^{4}He R(<0);log_{10}(R_{GEN});Efficiency of |#frac{R_{UH}-R_{LH}}{R_{UH}+R_{LH}}| < 0.2",
         hMCposLX->GetNbinsX(), hMCposLX->GetBinLowEdge(1), hMCposLX->GetBinLowEdge(hMCposLX->GetNbinsX()+1));
    hMCnegEff->SetMarkerColor(kAzure+7); hMCnegEff->SetLineColor(kAzure+7); hMCnegEff->SetLineWidth(3);

    TH1D *hISSposEff = new TH1D("hISSposEff","ISS R(>0);log_{10}(|R_{INNER}|);Efficiency of |#frac{R_{UH}-R_{LH}}{R_{UH}+R_{LH}}| < 0.2",
         hMCposLX->GetNbinsX(), hMCposLX->GetBinLowEdge(1), hMCposLX->GetBinLowEdge(hMCposLX->GetNbinsX()+1));
    hISSposEff->SetMarkerColor(kRed); hISSposEff->SetLineColor(kRed); 
    TH1D *hISSnegEff = new TH1D("hISSnegEff","ISS R(<0);log_{10}(|R_{INNER}|);Efficiency of |#frac{R_{UH}-R_{LH}}{R_{UH}+R_{LH}}| < 0.2",
         hMCposLX->GetNbinsX(), hMCposLX->GetBinLowEdge(1), hMCposLX->GetBinLowEdge(hMCposLX->GetNbinsX()+1));
    hISSnegEff->SetMarkerColor(kBlue); hISSnegEff->SetLineColor(kBlue); 

    double GoodInt, TotInt;
    double GoodIntErr, TotIntErr, Err;
    for(int ibin=1; ibin<=hMCposLX->GetNbinsX(); ++ibin){
        TH1D *hMCposLY  = hMCposL->ProjectionY("hMCposLY", ibin, ibin, "e");
        TH1D *hISSposLY = hISSposL->ProjectionY("hISSposLY", ibin, ibin, "e");

        if(hMCposLY->Integral(1,hMCposLY->GetNbinsX())>0){
            GoodInt = hMCposLY->IntegralAndError(hMCposLY->FindFixBin(-LabelLimit), hMCposLY->FindFixBin(LabelLimit), GoodIntErr);
            TotInt  = hMCposLY->IntegralAndError(1,hMCposLY->GetNbinsX(), TotIntErr);
            int thisBin = hMCposEff->Fill(hMCposLX->GetBinCenter(ibin), GoodInt/TotInt);
            Err = GoodInt/TotInt*sqrt(pow(GoodIntErr/GoodInt,2)+pow(TotIntErr/TotInt,2));
            hMCposEff->SetBinError(thisBin, Err);
        }

        if(hISSposLY->Integral(1,hISSposLY->GetNbinsX())>0){
            GoodInt = hISSposLY->IntegralAndError(hISSposLY->FindFixBin(-LabelLimit), hISSposLY->FindFixBin(LabelLimit), GoodIntErr);
            TotInt  = hISSposLY->IntegralAndError(1,hISSposLY->GetNbinsX(), TotIntErr);
            int thisBin = hISSposEff->Fill(hMCposLX->GetBinCenter(ibin), GoodInt/TotInt);
            if (GoodInt==0) Err =0;
            else Err = GoodInt/TotInt*sqrt(pow(GoodIntErr/GoodInt,2)+pow(TotIntErr/TotInt,2));
            hISSposEff->SetBinError(thisBin, Err);
        }
        
    }
    for(int ibin=1; ibin<=hMCposLX->GetNbinsX(); ibin=ibin+5){
        TH1D *hMCnegLY  = hMCnegL->ProjectionY("hMCnegLY", ibin, ibin+4, "e");
        TH1D *hISSnegLY = hISSnegL->ProjectionY("hISSnegLY", ibin, ibin+4, "e");

        if(hMCnegLY->Integral(1,hMCnegLY->GetNbinsX())>0 && ibin+4 <= hMCposLX->GetNbinsX()){
            GoodInt = hMCnegLY->IntegralAndError(hMCnegLY->FindFixBin(-LabelLimit), hMCnegLY->FindFixBin(LabelLimit), GoodIntErr);
            TotInt  = hMCnegLY->IntegralAndError(1,hMCnegLY->GetNbinsX(), TotIntErr);
            int thisBin = hMCnegEff->Fill((hMCposLX->GetBinLowEdge(ibin)+hMCposLX->GetBinLowEdge(ibin+5))/2., GoodInt/TotInt);
            Err = GoodInt/TotInt*sqrt(pow(GoodIntErr/GoodInt,2)+pow(TotIntErr/TotInt,2));
            hMCnegEff->SetBinError(thisBin, Err);
        }


        if(hISSnegLY->Integral(1,hISSnegLY->GetNbinsX())>0 && ibin+4 <= hMCposLX->GetNbinsX()){
            GoodInt = hISSnegLY->IntegralAndError(hISSnegLY->FindFixBin(-LabelLimit), hISSnegLY->FindFixBin(LabelLimit), GoodIntErr);
            TotInt  = hISSnegLY->IntegralAndError(1,hISSnegLY->GetNbinsX(), TotIntErr);
            int thisBin = hISSnegEff->Fill((hMCposLX->GetBinLowEdge(ibin)+hMCposLX->GetBinLowEdge(ibin+5))/2., GoodInt/TotInt);
            if(GoodInt==0) Err = 0;
            else Err = GoodInt/TotInt*sqrt(pow(GoodIntErr/GoodInt,2)+pow(TotIntErr/TotInt,2));
            hISSnegEff->SetBinError(thisBin, Err);
        }
    }

    TCanvas *c1 = MakeSquareCanvas("c1","c1");
    printf("Ciao\n");
    hMCposEff->GetYaxis()->SetNdivisions(520); hMCposEff->GetXaxis()->CenterTitle(true); hMCposEff->GetYaxis()->CenterTitle(true);
    hMCposEff->GetYaxis()->SetTitleOffset(1.8); hMCposEff->GetXaxis()->SetTitleOffset(1.2);
    // hMCposEff->SetMarkerStyle(22); hMCposEff->SetMarkerSize(1.2); 
    // hMCnegEff->SetMarkerStyle(22); hMCnegEff->SetMarkerSize(1.2);
    hISSposEff->GetYaxis()->SetNdivisions(520); hISSposEff->GetXaxis()->CenterTitle(true); hISSposEff->GetYaxis()->CenterTitle(true);
    hISSposEff->SetMarkerStyle(20); hISSposEff->SetMarkerSize(1.2); 
    hISSnegEff->SetMarkerStyle(20); hISSnegEff->SetMarkerSize(1.2);
    
    hMCposEff->Draw("hist e");
    hMCnegEff->Draw("hist e same");
    hISSposEff->Draw("E same");
    hISSnegEff->Draw("E same");
    
    TLegend *l = c1->BuildLegend();
    l->SetBorderSize(0); l->SetFillColorAlpha(kWhite, 0.0); l->SetNColumns(2);
    hMCposEff->SetTitle("");
    c1->SaveAs("CL_labelEff.pdf");
    
    TCanvas *c2 = MakeSquareCanvas("c2","c2");
    hISSposEff->Draw("E");
    hISSnegEff->Draw("E same");
    TLegend *l2 = c2->BuildLegend();
    l2->SetBorderSize(0);
    
    TFile *fOut = new TFile("CL_labelEfficiency.root","RECREATE");
    c1->Write(); c2->Write();
    hMCposEff->Write(); hMCnegEff->Write();
    hISSposEff->Write(); hISSnegEff->Write();
    hMCposL->Write(); hMCnegL->Write();
    hISSposL->Write(); hISSnegL->Write();
    return; 
}
