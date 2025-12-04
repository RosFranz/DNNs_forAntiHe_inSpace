#include <vector>
#include <array>
#include <TString.h>
#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>


void test(){
    gStyle->SetErrorX(0);

    //root -l -b -q 'DataMC_comparison.cc("2025_02_26","2025_02_25","2025_01_27","2025_01_27")'
    TString inPath("/eos/home-f/frrossi/AMS/output/organized_output/MLDs/");
    
    TFile *fInMC = new TFile(inPath+"MC/Positive_2025_02_26.root", "READ");
    TTree *tMC = (TTree*)fInMC->Get("MC");
    tMC->SetBranchStatus("*",0);
    TFile *fInISS = new TFile(inPath+"ISS/Positive_2025_01_27.root", "READ");
    TTree *tISS = (TTree*)fInISS->Get("ISS");
    tISS->SetBranchStatus("*",0);
 
    //TFile *fISS = new TFile("/eos/home-f/frrossi/AMS/RIG.root", "READ");
    //TH1F* hCutOff   = (TH1F*)fISS->Get("MyCutOff");
    //TH1F* hExposure = (TH1F*)fISS->Get("MyExposure");
    //TH1F* hEX       = (TH1F*)fISS->Get("Exposure");
    
    TFile *fMC = new TFile("/eos/home-f/frrossi/AMS/ciao.root", "READ");
    //TH1F* hW     = (TH1F*)fMC->Get("MCfluxW");
    TH1F* htimeW = (TH1F*)fMC->Get("MCtimeW");
    htimeW->SetLineColor(kMagenta);
    

    TFile *ftrue = new TFile("/eos/home-f/frrossi/AMS/output/organized_output/2025_03_20/RIG.root", "READ");
    TH1 *h1 = (TH1F*)ftrue->Get("RGBLInL1");
    TH1F* hCutOff   = (TH1F*)ftrue->Get("MyCutOff");
    TH1F* hExposure = (TH1F*)ftrue->Get("MyExposure");
    TH1F* hEX       = (TH1F*)ftrue->Get("Exposure");

    float maxN = hCutOff->GetMaximum();
    for(int i=1; i<=hCutOff->GetNbinsX(); ++i) {
        hCutOff->SetBinContent(i, hCutOff->GetBinContent(i)/maxN);
        hCutOff->SetBinError(i, hCutOff->GetBinError(i)/maxN);
        hExposure->SetBinContent(i, hExposure->GetBinContent(i)*hCutOff->GetBinContent(i));
    }
    TCanvas *c0 = new TCanvas("c0","cutoff weights", 600, 600);
    c0->SetLogy();
    hCutOff->Draw("hist");
    
    TCanvas *c1 = new TCanvas("c1","MC weights", 600, 600);
    c1->SetLogy();
    //hW->Draw("hist");
    htimeW->Draw("hist");
 


    float Rtrue=0, RinnerMC=0, RinnerISS=0;
    double weight=0;
    bool isAbIGRFpos=0;
    tMC->SetBranchStatus("Rtrue",1);
    tMC->SetBranchAddress("Rtrue",&Rtrue);
    tMC->SetBranchStatus("weight",1);
    tMC->SetBranchAddress("weight",&weight);
    tMC->SetBranchStatus("RinnerL1",1);
    tMC->SetBranchAddress("RinnerL1",&RinnerMC);
    
    tISS->SetBranchStatus("RinnerL1",1);
    tISS->SetBranchAddress("RinnerL1",&RinnerISS);
    tISS->SetBranchStatus("isAbIGRFpos",1);
    tISS->SetBranchAddress("isAbIGRFpos",&isAbIGRFpos);
 

    const int nRigbins_ams02He = 69;
    double Rigbins_ams02He[nRigbins_ams02He] = {1.92, 2.15, 2.40, 2.67, 2.97, 3.29, 3.64, 4.02, 4.43, 4.88, 5.37, 5.90, 6.47, 7.09, 7.76,
                                                8.48, 9.26, 10.1, 11.0, 12.0, 13.0, 14.1, 15.3, 16.6, 18.0, 19.5, 21.1, 22.8, 24.7, 26.7,
                                                28.8, 31.1, 33.5, 36.1, 38.9, 41.9, 45.1, 48.5, 52.2, 56.1, 60.3, 64.8, 69.7, 74.9, 80.5,
                                                86.5, 93.0, 100,  108,  116,  125,  135,  147,  160,  175,  192,  211,  233,  259,  291,
                                                330,  379,  441,  525,  643,  822,  1130, 1800, 3000};
    float logRigbins[nRigbins_ams02He];
    for(int i=0; i<nRigbins_ams02He; i++) logRigbins[i] = log10(Rigbins_ams02He[i]);
    
    TH1 *hMCw      = new TH1F("hMCw", "Events MC no CO w; log_{10}(R_{INNER}) [log_{10}(GV)]; Weights", nRigbins_ams02He-1, logRigbins);
    TH1 *hMCtimew  = new TH1F("hMCtimew", "Events MC time w and no CO w; log_{10}(R_{INNER}) [log_{10}(GV)]; Weights", nRigbins_ams02He-1, logRigbins);
    TH1 *hMC2w     = new TH1F("hMC2w", "Events MC with CO w; log_{10}(R_{INNER}) [log_{10}(GV)]; Weights", nRigbins_ams02He-1, logRigbins);
    TH1 *hMC2timew = new TH1F("hMC2timew", "Events MC with time and CO w; log_{10}(R_{INNER}) [log_{10}(GV)]; Weights", nRigbins_ams02He-1, logRigbins);
    hMCw->Sumw2(); hMCtimew->Sumw2(); hMC2w->Sumw2(); hMC2timew->Sumw2();
    TH1 *hISS   = new TH1F("hISS", "Events ISS; log_{10}(R_{INNER}) [log_{10}(GV)]; Evetns", nRigbins_ams02He-1, logRigbins);
    TH1 *hISSco = new TH1F("hISSco", "Events ISS > Cutoff; log_{10}(R_{INNER}) [log_{10}(GV)]; Evetns", nRigbins_ams02He-1, logRigbins);
 
    printf("Number of entries MC: %lld\n", tMC->GetEntries());
    printf("Number of entries ISS: %lld\n", tISS->GetEntries());
    for(int i=0; i<tMC->GetEntries(); i+=2){
        tMC->GetEntry(i,0);
        if(i%int(tMC->GetEntries()/10.)==0) {
            printf("%.2f\r", i/tMC->GetEntries()*1.);
            fflush(stdout);
        }
        
        if(Rtrue>pow(10, hExposure->GetBinLowEdge(hExposure->GetNbinsX())) || Rtrue<1.) continue;
        //if(Rtrue>pow(10, hEX->GetBinLowEdge(hEX->GetNbinsX())) || Rtrue<1.) continue;
        
        //float w = hCutOff->GetBinContent(hCutOff->FindFixBin(log10(Rtrue))) / hCutOff->GetBinContent(hCutOff->FindFixBin(log10(RinnerMC)));
        float w = hExposure->GetBinContent(hExposure->FindFixBin(log10(RinnerMC))) / hExposure->GetBinContent(hExposure->FindFixBin(log10(Rtrue)));
        //float w = hCutOff->GetBinContent(hCutOff->FindFixBin(log10(Rtrue)));
        
        //float w = hEX->GetBinContent(hEX->FindFixBin(log10(RinnerMC))) / hEX->GetBinContent(hEX->FindFixBin(log10(Rtrue)));
        
        //if(hCutOff->FindFixBin(log10(RinnerMC)) >= hCutOff->FindFixBin(log10(Rtrue))) w = 1;
        if(RinnerMC >= Rtrue) w = 1;
        
        //float wMC   = hW->GetBinContent(hW->FindFixBin(log10(Rtrue)));
        float wtime = htimeW->GetBinContent(htimeW->FindFixBin(log10(Rtrue)));
        if(hMCtimew->GetEntries()<20) printf("Time w: %.5f Cutoff w: %.5f, product: %.5f\n", wtime, w, w*wtime);
        
        //hMCw->Fill(log10(RinnerMC),wMC);
        hMCtimew->Fill(log10(RinnerMC), wtime);
        //hMC2w->Fill(log10(RinnerMC),w*wMC);
        
        hMC2timew->Fill(log10(RinnerMC), w*wtime);
        //if(w==1) hMC2timew->Fill(log10(RinnerMC), wtime);
        //else hMC2timew->Fill(log10(RinnerMC), w);
        if(TMath::IsNaN(hMC2timew->Integral())){
            printf("\nIntegal is NAN\n");
            printf("Rtrue: %f, RinnerMC: %f, BinRtrue:%d, BinRinner: %d\n", Rtrue, RinnerMC, hExposure->FindFixBin(log10(Rtrue)), hExposure->FindFixBin(log10(RinnerMC)));
            printf("Time w: %.5f Cutoff w: %.5f, product: %.5f\n", wtime, w, w*wtime);
            return;
        }
    }
    printf("\n");

    for(int i=0; i<tISS->GetEntries(); ++i) {
        if(i%int(tISS->GetEntries()/10.)==0) {
            printf("%.2f\r", i*100./tISS->GetEntries());
            fflush(stdout);
        }
        tISS->GetEntry(i,0);
        if(isAbIGRFpos) hISSco->Fill(log10(RinnerISS));
        hISS->Fill(log10(RinnerISS));
    }
    printf("Integrals hMCtimew: %.3f hMC2timew: %.3f\n", hMCtimew->Integral(), hMC2timew->Integral());
    //hMCw->Scale(hISS->Integral()/hMCw->Integral());
    //hMCw->SetLineWidth(3);
    //hMCw->SetLineColor(kRed);
    
    //hMCtimew->Scale(h1->Integral()/hMCtimew->Integral());
    hISS->SetLineWidth(3);
    hISS->SetLineColor(kBlack);
    hISS->Scale(h1->Integral()/hISS->Integral());
    hISSco->SetLineWidth(3);
    hISSco->SetLineColor(kGreen +1);
    hISSco->Scale(h1->Integral()/hISSco->Integral());
    
    hMCtimew->Scale(h1->Integral()/hMCtimew->Integral());
    hMCtimew->SetLineWidth(3);
    hMCtimew->SetLineColor(kOrange);
    
    //hMC2w->Scale(hISSco->Integral()/hMC2w->Integral());
    //hMC2w->SetLineWidth(3);
    //hMC2w->SetLineColor(kBlue);
    
    //hMC2timew->Scale(h1->Integral()/hMC2timew->Integral());
    hMC2timew->Scale(h1->Integral()/hMC2timew->Integral());
    hMC2timew->SetLineWidth(3);
    hMC2timew->SetLineColor(kMagenta);


    printf("Integrals hISS: %.3f, hISSco: %.3f\n", hISS->Integral(), hISSco->Integral());
    printf("Integrals hMCtimew: %.3f hMC2timew: %.3f\n", hMCtimew->Integral(), hMC2timew->Integral());
    TCanvas *c2= new TCanvas("c2","DataMC",600,600);
    c2->SetLogy();
    //hMCw->Draw("hist same");
    hISS->Draw("e");
    h1->SetMarkerStyle(21);
    h1->SetMarkerColor(kBlue);
    //h1->SetMarkerSize(2);
    h1->Draw("e p* same");
    hMCtimew->Draw("hist same");
    //hISS->Draw("e same");
    //hMC2w->Draw("hist same");
    hMC2timew->Draw("hist same");
    hISSco->Draw("e same");
    c2->BuildLegend();
    

    TFile fOut("/eos/home-f/frrossi/AMS/MCweights.root", "RECREATE");
    fOut.cd();
    c1->Write();
    c2->Write();
    hCutOff->Write("CutOffWeights");
    htimeW->Write("MCtimeWeights");
    fOut.Close();
}
