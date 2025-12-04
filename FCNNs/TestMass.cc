// Author: F.Rossi
// Created on: 24.01.2025
// Last modified on: 24.01.2025
// Revision 1.0 -> moving from TH1D to TH2D
/*
 Plot inverse mass vs scores
 -------------------------------------------------------
    How to use it:

    give the folders date as input parameters and
    just run this command:
        root -l 'TestMass.cc("DataFile")'
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

void TestMass(TString DATAfile="000"){
    const float uma    = 0.9315; //GeV/c^2
    const float LabelSize = 0.045, TitleSize = 0.05;

    const int TotFiles = 1; //number of files
    const int TotTree  = 2; //n. of trees
    const int TotRig   = 2; //n. of rigidities
    const int TotDet   = 2; //n. of detectors

    array<TString, TotFiles> PathToFile{"/home/franz/AMS/output/organized_output/scores/"+DATAfile+"/"+DATAfile+"_2CL_ISS_scores.root"};
    array<TFile*, TotFiles> files;
    TFile *f_out = new TFile("2CL_ISS_mass.root", "RECREATE");
    array<TTree*, TotFiles*TotTree> trees;
    //TH1 vector
    vector<TH1*> H1_R, H1_CSinvMassW;
    //TH2 vectors
    vector<TH2D*> H2_CSinvMassNaF, H2_RinvMassNaF;
    vector<TH2D*> H2_CSinvMassAgl, H2_RinvMassAgl;
    vector<TH2D*> H2_RCS;
    array<double, 50> cutValues;
    for(int i=0; i<cutValues.size(); i++) cutValues.at(i) = 1./cutValues.size()+(i*1./cutValues.size());
    array<TString, TotRig> NameR{"Rinner", "RinnerL1"};
    array<TString, 2> DirName{"ISS_pos", "ISS_neg"};

    //Canvas and legend
    vector<TCanvas*> cc;
    vector<TLegend*> legends;

    //Selections
    TString NaFSel = "hasRich==1 && beta_rich>0.75 && isNaF==1 && "
                    "isBorder_rich==1 && kprob_rich>0.01 && 1<charge_rich && "
                    "charge_rich<3 && ringPMTs2_rich>2 && (measPE_Uncorr_rich/totPE_Uncorr_rich)>0.4";

    TString AglSel = "hasRich==1 && beta_rich>0.953 && isNaF==0 && "
                    "isBorder_rich==1 && kprob_rich>0.01 && 1<charge_rich && "
                    "charge_rich<3 && ringPMTs2_rich>2 && (measPE_Uncorr_rich/totPE_Uncorr_rich)>0.4 && abs(Rinner)>1";
    // TString AglSel = "hasRich==1 && beta_rich>0.953 && isNaF==0 && "
    //                 "isBorder_rich==1 && kprob_rich>0.01 && 1<charge_rich && "
    //                 "charge_rich<3 && ringPMTs2_rich>2 && (measPE_Uncorr_rich/totPE_Uncorr_rich)>0.4 && isAbIGRFpos==1";

    TString RichSel = "hasRich==1 && beta_rich>0.75 && isBorder_rich==1 && "
                            "kprob_rich>0.01 && 1<charge_rich && charge_rich<3 && "
                            "ringPMTs2_rich>2 && (measPE_Uncorr_rich/totPE_Uncorr_rich)>0.4";
    TString AglRinnerInvMass   = AglSel+" && 0.2 <= (1./((2.*abs(Rinner)*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))) && "
                                       "(1./((2.*abs(Rinner)*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))) <= 0.5 && abs(Rinner)>1";
    // TString AglRinnerL1InvMass = AglSel+"0.2 <= (1./((2.*abs(RinnerL1)*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))) && 
    //                                    (1./((2.*abs(RinnerL1)*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))) <= 0.4";
    array<TString, TotTree> InL1Sel{" && RinnerL1>0", " && RinnerL1<0"};
    vector<TString> RSel, RCSSel, invMassNaFSel, invMassAglSel;
    printf("NaF selection: %s\n", NaFSel.Data());
    printf("Agl selection: %s\n", AglSel.Data());
    printf("Rich selection: %s\n", RichSel.Data());
    TString CSSelection, TotSelection, TemName;

    //Binning
    const float mass_MAX = 1., mass_MIN = 1./6.;
    const int nbinsMass = 100;
    const float minRin   = -1., maxRin = 3.5;
    const int nbinsRin = 100;
    const float minCS = 0., maxCS = 1.;
    const int nbinsScore = 100;

    //Drawing
    TString to_draw;
    vector<TString> drawR, drawRCS, drawCSinvMass, drawRinvMass;

    //Organizing the output file
    for(int i=0; i<TotTree; i++){
        f_out->cd();
        f_out->mkdir(DirName.at(i).Data());
        TString subDir;
        for(int j=0; j<TotRig; j++){
            subDir = DirName.at(i)+"/"+NameR.at(j);
            f_out->mkdir(subDir.Data());
        }
    }

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
    for(int i_tree=0; i_tree< TotTree; i_tree++){ //loop on trees
        TString n, nRCS;
        TString t, tRCS;
        //NaF
        TString nCSinvMassNaF, nRinvMassNaF;
        TString tCSinvMassNaF, tRinvMassNaF;
        //AGL
        TString nCSinvMassAgl, nRinvMassAgl;
        TString tCSinvMassAgl, tRinvMassAgl;

        for(int j_rig=0; j_rig<TotRig; j_rig++){ //loop on rig
            if(i_tree==0) {
                n = NameR.at(j_rig)+"Pos";
                RSel.push_back(NameR.at(j_rig)+">0");
                nRCS = NameR.at(j_rig)+"CSPos";
                drawRCS.push_back("scores_pos:log10(abs("+NameR.at(j_rig)+"))>>+"+nRCS);

                nCSinvMassNaF = "CSInvMass_NaF_"+NameR.at(j_rig)+"Pos";
                nRinvMassNaF  = "RinvMass_NaF_"+NameR.at(j_rig)+"Pos";

                nCSinvMassAgl = "CSInvMass_Agl_"+NameR.at(j_rig)+"Pos";
                nRinvMassAgl  = "RinvMass_Agl_"+NameR.at(j_rig)+"Pos";
                drawCSinvMass.push_back("scores_pos:1./((2.*abs("+NameR.at(j_rig)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))");

                if(j_rig==0) H1_CSinvMassW.push_back( new TH1D("CSinvMassWPos", "CSinvMassWPos;Classifier score cut value; Fraction of events with m^{Agl}#in[2, 5] [a.m.u.]", nbinsScore, minCS, maxCS));
            }
            else {
                n = NameR.at(j_rig)+"Neg";
                RSel.push_back(NameR.at(j_rig)+"<0");
                nRCS = NameR.at(j_rig)+"CSNeg";
                drawRCS.push_back("scores_neg:log10(abs("+NameR.at(j_rig)+"))>>+"+nRCS);

                nCSinvMassNaF = "CSInvMass_NaF_"+NameR.at(j_rig)+"Neg";
                nRinvMassNaF  = "RinvMass_NaF_"+NameR.at(j_rig)+"Neg";

                nCSinvMassAgl = "CSInvMass_Agl_"+NameR.at(j_rig)+"Neg";
                nRinvMassAgl  = "RinvMass_Agl_"+NameR.at(j_rig)+"Neg";
                drawCSinvMass.push_back("scores_neg:1./((2.*abs("+NameR.at(j_rig)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315))");

                if(j_rig==0) H1_CSinvMassW.push_back( new TH1D("CSinvMassWNeg", "CSinvMassWNeg;Classifier score cut value; Fraction of events with m^{Agl}#in[2, 5] [a.m.u.]", nbinsScore, minCS, maxCS));
            }
            
            //Rigidities
            drawR.push_back("log10(abs("+NameR.at(j_rig)+"))>>+"+n);
            t = n+"; [log_{10}(GV)]; Events";
            H1_R.push_back(new TH1D(n.Data(),t.Data(), nbinsRin, minRin, maxRin));
            H1_R.back()->GetXaxis()->SetTitleOffset(1.2);
                //R vs CS
            if(j_rig==1) RCSSel.push_back(RichSel+InL1Sel.at(i_tree));
            else RCSSel.push_back(RichSel);
            tRCS = nRCS+"; [log_{10}(GV)]; Classifier scores";
            H2_RCS.push_back(new TH2D(nRCS.Data(), tRCS.Data(), nbinsRin, minRin, maxRin, nbinsScore, minCS, maxCS));
            H2_RCS.back()->GetXaxis()->SetTitleOffset(1.2);

            //NaF
                //CS vs Inv. Mass (NaF)
            if(j_rig==1) invMassNaFSel.push_back(NaFSel+InL1Sel.at(i_tree));
            else invMassNaFSel.push_back(NaFSel);
            tCSinvMassNaF = nCSinvMassNaF+"; RICH NaF #frac{1}{m} [1/a.m.u.]; Classifier scores";
            H2_CSinvMassNaF.push_back(new TH2D(nCSinvMassNaF.Data(), tCSinvMassNaF.Data(), nbinsMass, mass_MIN, mass_MAX, nbinsScore, minCS, maxCS));
            H2_CSinvMassNaF.back()->GetXaxis()->SetTitleOffset(1.2);
                //R vs Inv. Mass (NaF)
            drawRinvMass.push_back("1./((2.*abs("+NameR.at(j_rig)+")*sqrt(1-pow(beta_rich,2)))/(beta_rich*0.9315)):log10(abs("+NameR.at(j_rig)+"))");
            tRinvMassNaF = nRinvMassNaF+" ; log_{10}(|R|) [log_{10}(GV)] ; RICH NaF #frac{1}{m} [1/a.m.u.] ";
            H2_RinvMassNaF.push_back(new TH2D(nRinvMassNaF.Data(), tRinvMassNaF.Data(), nbinsRin, minRin, maxRin, nbinsMass, mass_MIN, mass_MAX));
            H2_RinvMassNaF.back()->GetXaxis()->SetTitleOffset(1.2);

            //AGL
                //CS vs Inv. Mass (Agl)
            if(j_rig==1) invMassAglSel.push_back(AglSel+InL1Sel.at(i_tree));
            else invMassAglSel.push_back(AglSel);
            tCSinvMassAgl = nCSinvMassAgl+"; RICH Agl #frac{1}{m} [1/a.m.u.]; Classifier scores";
            H2_CSinvMassAgl.push_back(new TH2D(nCSinvMassAgl.Data(), tCSinvMassAgl.Data(), nbinsMass, mass_MIN, mass_MAX, nbinsScore, minCS, maxCS));
            H2_CSinvMassAgl.back()->GetXaxis()->SetTitleOffset(1.2);
                //R vs Inv. Mass (Agl)
            tRinvMassAgl = nRinvMassAgl+" ; log_{10}(|R|) [log_{10}(GV)] ; RICH Agl #frac{1}{m} [1/a.m.u.] ";
            H2_RinvMassAgl.push_back(new TH2D(nRinvMassAgl.Data(), tRinvMassAgl.Data(), nbinsRin, minRin, maxRin, nbinsMass, mass_MIN, mass_MAX));
            H2_RinvMassAgl.back()->GetXaxis()->SetTitleOffset(1.2);

        } //end loop on rig
    } // end loop on trees


    //Filling the histos
    for(int i=0; i<H1_R.size(); i++){  // loop on histos
        f_out->cd();
        if(i<TotRig){ // POSITIVES +
            //Rigidities
            trees.at(0)->Draw(drawR.at(i).Data(),RSel.at(i).Data(),"goff");
            H1_R.at(i) = (LogToLin1D((TH1D*)gDirectory->Get(H1_R.at(i)->GetName())));
            f_out->cd(DirName.at(0).Data());
            H1_R.at(i)->Write();
            f_out->cd();
                // R vs CS
            trees.at(0)->Draw(drawRCS.at(i).Data(), RCSSel.at(i).Data(), "goff");

            //NaF
                //CS vs Inv. Mass
            to_draw = drawCSinvMass.at(i)+">>+"+H2_CSinvMassNaF.at(i)->GetName();
            trees.at(0)->Draw(to_draw.Data(), invMassNaFSel.at(i).Data(), "goff");
                //R vs Inv. Mass
            to_draw = drawRinvMass.at(i)+">>+"+H2_RinvMassNaF.at(i)->GetName();
            trees.at(0)->Draw(to_draw.Data(), invMassNaFSel.at(i).Data(), "goff");

            //Agl
                //CS vs Inv. Mass
            to_draw = drawCSinvMass.at(i)+">>+"+H2_CSinvMassAgl.at(i)->GetName();
            trees.at(0)->Draw(to_draw.Data(), invMassAglSel.at(i).Data(), "goff");
                //R vs Inv. Mass
            to_draw = drawRinvMass.at(i)+">>+"+H2_RinvMassAgl.at(i)->GetName();
            trees.at(0)->Draw(to_draw.Data(), invMassAglSel.at(i).Data(), "goff");
        }
        else{ // NEGATIVES -
            //Rigidities
            trees.at(1)->Draw(drawR.at(i).Data(),RSel.at(i).Data(),"goff");
            H1_R.at(i) = (LogToLin1D((TH1D*)gDirectory->Get(H1_R.at(i)->GetName())));
            f_out->cd(DirName.at(1).Data());
            H1_R.at(i)->Write();
            f_out->cd();
                // R vs CS
            trees.at(1)->Draw(drawRCS.at(i).Data(), RCSSel.at(i).Data(), "goff");

            //NaF
                //CS vs Inv. Mass
            to_draw = drawCSinvMass.at(i)+">>+"+H2_CSinvMassNaF.at(i)->GetName();
            trees.at(1)->Draw(to_draw.Data(), invMassNaFSel.at(i).Data(), "goff");
                //R vs Inv. Mass
            to_draw = drawRinvMass.at(i)+">>+"+H2_RinvMassNaF.at(i)->GetName();
            trees.at(1)->Draw(to_draw.Data(), invMassNaFSel.at(i).Data(), "goff");
            
            //Agl
                //CS vs Inv. Mass
            to_draw = drawCSinvMass.at(i)+">>+"+H2_CSinvMassAgl.at(i)->GetName();
            trees.at(1)->Draw(to_draw.Data(), invMassAglSel.at(i).Data(), "goff");
                //R vs Inv. Mass
            to_draw = drawRinvMass.at(i)+">>+"+H2_RinvMassAgl.at(i)->GetName();
            trees.at(1)->Draw(to_draw.Data(), invMassAglSel.at(i).Data(), "goff");
        }
        H2_RCS.at(i)         = (LogToLin2D((TH2D*)gDirectory->Get(H2_RCS.at(i)->GetName())));
        H2_RinvMassNaF.at(i) = (LogToLin2D((TH2D*)gDirectory->Get(H2_RinvMassNaF.at(i)->GetName())));

        TH2D *h2Temp = (TH2D*)gDirectory->Get(H2_CSinvMassNaF.at(i)->GetName());
        H2_CSinvMassNaF.at(i) = (TH2D*)gDirectory->Get(H2_CSinvMassNaF.at(i)->GetName());
        
        H2_RinvMassAgl.at(i) = (LogToLin2D((TH2D*)gDirectory->Get(H2_RinvMassAgl.at(i)->GetName())));
        H2_CSinvMassAgl.at(i) = (TH2D*)gDirectory->Get(H2_CSinvMassAgl.at(i)->GetName());

        if(i<TotRig) {
            TemName = DirName.at(0)+"/"+NameR.at(i%2);
            f_out->cd(TemName.Data());
        }
        else{
            TemName = DirName.at(1)+"/"+NameR.at(i%2);
            f_out->cd(TemName.Data());
        }
        H2_RCS.at(i)->Write();
        H2_CSinvMassNaF.at(i)->Write();
        H2_RinvMassNaF.at(i)->Write();
        H2_CSinvMassAgl.at(i)->Write();
        H2_RinvMassAgl.at(i)->Write();
    } // end loop on histos

    f_out->cd();
    for(int i=0; i<cutValues.size(); i++){
        TString NumeratorSel = "&& scores_pos>="+to_string(cutValues.at(i));
        NumeratorSel = AglRinnerInvMass+NumeratorSel;
        TString DenominatorSel = "&& scores_pos>="+to_string(cutValues.at(i));
        DenominatorSel = AglSel+DenominatorSel;
        double Numerator = trees.at(0)->GetEntries(NumeratorSel.Data())*1.;
        double Denominator = trees.at(0)->GetEntries(DenominatorSel.Data());
        double Fraction, Error;
        if(Denominator!=0) {
            Fraction = Numerator/Denominator;
            if(Numerator!=0) Error = Fraction*sqrt(1/Numerator + 1/Denominator);
            else Error = 0;
        }
        else {
            Fraction = 0;
            Error    = 0;
        }
        int ibin = H1_CSinvMassW.at(0)->Fill(cutValues.at(i), Fraction);
        H1_CSinvMassW.at(0)->SetBinError(ibin, Error);
        if(Fraction>1) {
            printf("Fraction Pos %f Error %f\n", Fraction, Error);
            printf("%s \n%s \n", NumeratorSel.Data(), DenominatorSel.Data());
        }

        NumeratorSel = "&& scores_neg>="+to_string(cutValues.at(i));
        NumeratorSel = AglRinnerInvMass+NumeratorSel;
        DenominatorSel = "&& scores_neg>="+to_string(cutValues.at(i));
        DenominatorSel = AglSel+DenominatorSel;
        Numerator = trees.at(1)->GetEntries(NumeratorSel.Data())*1.;
        Denominator = trees.at(1)->GetEntries(DenominatorSel.Data());
        if(Denominator!=0){
            Fraction = Numerator/Denominator;
            if(Numerator!=0) Error = Fraction*sqrt(1/Numerator + 1/Denominator);
            else Error = 0;
        }
        else{
            Fraction = 0;
            Error    = 0;
        }
        // printf("Fraction Neg %f Error %f\n", Fraction, Error);
        
        ibin = H1_CSinvMassW.at(1)->Fill(cutValues.at(i), Fraction);
        H1_CSinvMassW.at(1)->SetBinError(ibin, Error);
    }
    H1_CSinvMassW.at(0)->Write();
    H1_CSinvMassW.at(1)->Write();
    
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
