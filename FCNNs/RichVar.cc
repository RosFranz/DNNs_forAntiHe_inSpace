// Author: F.Rossi
// Created on: 30.01.2025
// Last modified on: 30.01.2025
// Revision 1.0 -> moving from TH1D to TH2D
/*
 Plot rich variables
  -------------------------------------------------------
    How to use it:

    give the folders date as input parameters and
    just run this command:
        root -l -b -q 'RichVar.cc("CL2_scoresDate")'
*/

//Works with NAIA v.1.2.
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
#include <TError.h>
#include <TString.h>

using namespace std;

TH1D *LogToLin1D(TH1D *h1);
TH2D *LogToLin2D(TH2D *h2);

void RichVar(TString DATAfile="000"){
    const float uma    = 0.9315; //GeV/c^2
    const float LabelSize = 0.045, TitleSize = 0.05;

    const int TotFiles = 1; //number of files
    const int TotTree  = 2; //n. of trees
    const int TotDet   = 2; //n. of detectors
    const int NVar     = 13;

    array<TString, TotFiles> PathToFile{"/home/franz/AMS/output/organized_output/scores/"+DATAfile+"/"+DATAfile+"_2CL_ISS_scores.root"};
    array<TFile*, TotFiles> files;
    array<TString, 2> DirName{"ISS_pos", "ISS_neg"};
    TFile *f_out = new TFile("RICHvar.root", "RECREATE");
    array<TTree*, TotFiles*TotTree> trees;
    //TH1 vector
    vector<TH1*> H1pos, H1PosSig, H1PosBkg, H1neg;
    //Canvas and legend
    vector<TCanvas*> cc;
    vector<TLegend*> legends;

    array<TString, NVar> VarNames{"kprob_rich",        "charge_rich",        "charge2_rich",
                                  "totPE_Uncorr_rich", "measPE_Uncorr_rich", "measPE_Corr_rich",
                                  "measPE_Uncorr_rich/totPE_Uncorr_rich", "measPE_Corr_rich/totPE_Uncorr_rich",
                                  "chargedPMTs_rich",  "ringPMTs_rich",      "ringPMTs2_rich",
                                  "totHits_rich",       "ringHits_rich"};
    array<TString, NVar> FTitles{"; Kolmogorov probability; Arbitrary units", 
                                  "; Z_{RICH}; Arbitrary units",
                                  "; Z^{2}_{RICH}; Arbitrary units",

                                  "; Total number of PE (no correction); Arbitrary units",
                                  "; Measured number of PE (no correction); Arbitray units",
                                  "; Measured number of PE (with correction); Arbitray units",

                                  "; Uncorrected measured PE(ring)/PE(Total); Arbitray units",
                                  "; Corrected measured PE(ring)/PE(Total); Arbitray units",

                                  "; PMTs crossed by track; Arbtirary units",
                                  "; Fired PMTs in the ring (ringPtr->NpColPMT.size()); Arbitrary units",
                                  "; Fired PMTs in the ring (ringPtr->getPMTs()); Arbitrary units",

                                  "; Total RICH hits (NRichHit()); Arbitrary units",
                                  "; N. ring hits; Arbitrary units"};
    array<TString, NVar> Titles{"kprob_rich",        "charge_rich",        "charge2_rich",
                                "totPE_Uncorr_rich", "measPE_Uncorr_rich", "measPE_Corr_rich",
                                "UncorrPEratio",     "CorrPEratio",
                                "chargedPMTs_rich",  "ringPMTs_rich",      "ringPMTs2_rich",
                                "totHits_rich",       "ringHits_rich"};

    //Selections 
    TString NaFSel = "hasRich==1 && beta_rich>0.75 && isNaF==1 && isBorder_rich==1";
    TString AglSel = "hasRich==1 && beta_rich>0.953 && isNaF==0 && isBorder_rich==1";
    TString AglSigSel = "hasRich==1 && beta_rich>0.953 && isNaF==0 && isBorder_rich==1 && RigLabel==1";
    TString AglBkgSel = "hasRich==1 && beta_rich>0.953 && isNaF==0 && isBorder_rich==1 && RigLabel==0";
    //&& kprob_rich>0.01 && 1<charge2_rich && charge2_rich<16 && ringPMTs2_rich>2 && (measPE_Uncorr_rich/totPE_Uncorr_rich)>0.4
    printf("NaF selection: %s\n", NaFSel.Data());
    printf("Agl selection: %s\n", AglSel.Data());

    //Binning
    const float min = -0.5, min0 = 0.;
    const float max1 = 1., max2 = 2., max50 = 49.5, max100=99.5, max150=149.5;
    const int nbins50 = 50, nbins100 = 100, nbins150=150, mbins200 = 200;

    //Drawings
    array<TString, NVar> DrawPos, DrawPosSig, DrawPosBkg, DrawNeg;
    for(int i=0; i<NVar; i++){
        DrawPos.at(i) = VarNames.at(i)+">>"+Titles.at(i)+"Pos";
        DrawPosSig.at(i) = VarNames.at(i)+">>"+Titles.at(i)+"PosSig";
        DrawPosBkg.at(i) = VarNames.at(i)+">>"+Titles.at(i)+"PosBkg";
        DrawNeg.at(i) = VarNames.at(i)+">>"+Titles.at(i)+"Neg";
    }

    // //Organizing the output file
    // for(int i=0; i<TotTree; i++){
    //     f_out->cd();
    //     f_out->mkdir(DirName.at(i).Data());
    //     TString subDir;
    // }

    //Getting trees
    for(int i_file=0; i_file<TotFiles; i_file++){  // loop on files
            if (gSystem->AccessPathName(PathToFile.at(i_file), kFileExists)) {
            printf("The file %s does not exist.\n", PathToFile.at(i_file).Data());
            return;
        }
        printf("The file %s exists.\n", PathToFile.at(i_file).Data());
        files.at(i_file) = new TFile(PathToFile.at(i_file), "READ");
        if(!files.at(i_file)){
            printf("ERROR!! \n File %s not found\n", PathToFile.at(i_file).Data());
            return;
        }
        files.at(i_file)->cd();
        printf("Reading file ==> %s\n", PathToFile.at(i_file).Data());
        for(int i_tree=0; i_tree<TotTree; i_tree++){
            trees.at(i_tree) = (TTree*)files.at(i_file)->Get(DirName.at(i_tree).Data());
            if(!trees.at(i_file)) {
                printf("%s tree not found in the file %s\n", DirName.at(i_tree).Data(), PathToFile.at(i_file).Data());
                return;
            }
        } //end loop on trees
    } // end loop on files

    f_out->cd();
    //Initialize the histograms
    TString name, FullTitle;
    for(int i_tree=0; i_tree< TotTree; i_tree++){ //loop on trees
        for(int jVar=0; jVar<NVar; jVar++){ //loop on rig
            double MIN = trees.at(0)->GetMinimum(VarNames.at(jVar));
            double MAX = trees.at(0)->GetMaximum(VarNames.at(jVar));
            if(i_tree==0) {
                name = Titles.at(jVar)+"Pos";
                FullTitle = "Pos_"+Titles.at(jVar)+FTitles.at(jVar);
                H1pos.push_back(new TH1D(name.Data(), FullTitle.Data(), nbins50, MIN, MAX));
                H1pos.back()->SetLineWidth(3);

                name = Titles.at(jVar)+"PosSig";
                FullTitle = "PosSig_"+Titles.at(jVar)+FTitles.at(jVar);
                H1PosSig.push_back(new TH1D(name.Data(), FullTitle.Data(), nbins50, MIN, MAX));
                H1PosSig.back()->SetLineColor(kGreen+2);
                H1PosSig.back()->SetLineWidth(3);
                H1PosSig.back()->SetFillStyle(3325);
                H1PosSig.back()->SetFillColor(kGreen+1);

                name = Titles.at(jVar)+"PosBkg";
                FullTitle = "PosBkg_"+Titles.at(jVar)+FTitles.at(jVar);
                H1PosBkg.push_back(new TH1D(name.Data(), FullTitle.Data(), nbins50, MIN, MAX));
                H1PosBkg.back()->SetLineColor(kRed+2);
                H1PosBkg.back()->SetLineWidth(3);
                H1PosBkg.back()->SetFillStyle(3356);
                H1PosBkg.back()->SetFillColor(kRed+1);
                
                H1pos.back()->Sumw2();
                H1PosSig.back()->Sumw2();
                H1PosBkg.back()->Sumw2();
            }
            else {
                name = Titles.at(jVar)+"Neg";
                FullTitle = "Neg_"+Titles.at(jVar)+FTitles.at(jVar);
                //Keep using the positives tree in order to have the same binning
                H1neg.push_back(new TH1D(name.Data(), FullTitle.Data(), nbins50, MIN, MAX));
                H1neg.back()->SetLineColor(kViolet);
                H1neg.back()->SetLineWidth(3);

                H1neg.back()->Sumw2();
            }
            // printf("MIN: %f MAX: %f\n", MIN, MAX);
        } //end loop on rig
    } // end loop on trees


    //Filling the histos
    for(int i=0; i<NVar; i++){  // loop on variables
        f_out->cd();
        //POSITIVES +
        trees.at(0)->Draw(DrawPos.at(i).Data(), AglSel.Data(),"goff");
        H1pos.at(i)=(TH1D*)gDirectory->Get(H1pos.at(i)->GetName());
        trees.at(0)->Draw(DrawPosSig.at(i).Data(), AglSigSel.Data(),"goff");
        H1PosSig.at(i)=(TH1D*)gDirectory->Get(H1PosSig.at(i)->GetName());
        trees.at(0)->Draw(DrawPosBkg.at(i).Data(), AglBkgSel.Data(),"goff");
        H1PosBkg.at(i)=(TH1D*)gDirectory->Get(H1PosBkg.at(i)->GetName());
            //Scaling
        H1PosSig.at(i)->Scale(1./H1pos.at(i)->Integral());
        H1PosBkg.at(i)->Scale(1./H1pos.at(i)->Integral());

        // NEGATIVES -
        trees.at(1)->Draw(DrawNeg.at(i).Data(), AglSel.Data(),"goff");
        H1neg.at(i)=(TH1D*)gDirectory->Get(H1neg.at(i)->GetName());


        TCanvas *c1 = new TCanvas(VarNames.at(i).Data(), VarNames.at(i).Data(), 800, 600);
        c1->SetLogy();
        TH1*h = H1pos.at(i)->DrawNormalized("hist");
        h->GetYaxis()->SetRangeUser(5.e-5, 1.);
        H1PosSig.at(i)->Draw("hist same");
        H1PosBkg.at(i)->Draw("hist same");
        H1neg.at(i)->DrawNormalized("hist same");

        c1->BuildLegend();
        gPad->Write();  

        // TCanvas *c1 = new TCanvas("c1","c1", 800, 600);
        // TPad* pad = new TPad(VarNames.at(i).Data(), VarNames.at(i).Data(), 0, 0, 1, 1);
        // pad->Draw();  // Draw the TPad to initialize it
        // c1->SetLogy();

        // Now draw the histograms on the TPad
        // pad->cd();  // Set the current pad to the one we created
        // TH1*h = H1pos.at(i)->DrawNormalized("hist");
        // h->GetYaxis()->SetRangeUser(5.e-5, 1.);
        // H1PosSig.at(i)->Draw("hist same");
        // H1PosBkg.at(i)->Draw("hist same");
        // H1neg.at(i)->DrawNormalized("hist same");
        // pad->BuildLegend();
        // pad->Write();
              
    } // end loop on histos

    f_out->cd();
   
    return;
}


//Log(R) -> to R in log scale
TH2D * LogToLin2D(TH2D* h2){

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
    float minY = h1Y->GetBinLowEdge(1);
    float maxY = h1Y->GetBinLowEdge(nBinsY+1);
    TString yTitle = h2->GetYaxis()->GetTitle();

    //bin edges for the new histogram
    double xbins[nBinsX+1], ybins[nBinsY+1];
    for(int ii=1; ii<=nBinsX+1; ii++) xbins[ii-1] = pow(10, h1X->GetBinLowEdge(ii));
    for(int jj=1; jj<=nBinsY+1; jj++) ybins[jj-1] = h1Y->GetBinLowEdge(jj);
    
    TH2D *h2_new = new TH2D("h2_new", "h2_new", nBinsX, xbins, nBinsY, ybins);
    for(int xx=1; xx<=nBinsX; xx++){
        for(int yy=1; yy<=nBinsY; yy++){
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


TH1D *LogToLin1D(TH1D *h1){
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
    
    TH1D *h1_new = new TH1D("h1_new", "h1_new", nBinsX, xbins);
    for(int xx=1; xx<=nBinsX; xx++){
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
