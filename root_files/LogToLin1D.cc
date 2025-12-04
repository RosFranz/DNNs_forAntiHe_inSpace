
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
TH1F *LogToLin1F(TH1F *h1){
    TString Name = h1->GetName();
    TString Title = h1->GetTitle();

    //Rigidity axis
    int nBinsX = h1->GetNbinsX();
    float minR = pow(10, h1->GetBinLowEdge(1));
    float maxR = pow(10, h1->GetBinLowEdge(nBinsX+1));
    //Y axis
    TString yTitle = h1->GetYaxis()->GetTitle();

    //bin edges for the new histogram
    double xbins[nBinsX+1];
    for(int ii=1; ii<=nBinsX+1; ii++) xbins[ii-1] = pow(10, h1->GetBinLowEdge(ii));
    
    TH1F *h1_new = new TH1F("h1_new", "h1_new", nBinsX, xbins);
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


TH2F * LogToLin2F(TH2F* h2){

    TString Name = h2->GetName();
    TString Title = h2->GetTitle();

    //Projection
    TH1 *h1X = h2->ProjectionX();
    TH1 *h1Y = h2->ProjectionY();

    //Rigidity axis
    int nBinsX = h1X->GetNbinsX();
    float minR = pow(10, h1X->GetBinLowEdge(1));
    float maxR = pow(10, h1X->GetBinLowEdge(nBinsX+1));
    //Mass - CS axis
    int nBinsY = h1Y->GetNbinsX();
    float minY = pow(10,h1Y->GetBinLowEdge(1));
    float maxY = pow(10,h1Y->GetBinLowEdge(nBinsY+1));
    TString yTitle = h2->GetYaxis()->GetTitle();

    //bin edges for the new histogram
    double xbins[nBinsX+1], ybins[nBinsY+1];
    for(int ii=1; ii<=nBinsX+1; ii++) xbins[ii-1] = pow(10, h1X->GetBinLowEdge(ii));
    for(int jj=1; jj<=nBinsY+1; jj++) ybins[jj-1] = pow(10, h1Y->GetBinLowEdge(jj));
    
    TH2F *h2_new = new TH2F("h2_new", "h2_new", nBinsX, xbins, nBinsY, ybins);
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


TH2F * LogToLin2FonX(TH2F* h2){

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

    TH2F *h2_new = new TH2F("h2_new", "h2_new", nBinsX, xbins, nBinsY, ybins);
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
