/*
----->
Get the number of events within the mass band
for each rig bin cutting on selected effiency 
values base on positive rigidities.
----->
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
#include <Rtypes.h>

using namespace std;
const int NCut = 7;
const int NDet= 3;
const int LW  = 3;
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
const float bAGL_thL = 0.96, bAGL_thH = 0.999;
//NaF
const float r3NaF_thL = he3 * bNaF_thL * 1./sqrt(1-pow(bNaF_thL,2)) / 2;
//AGL
const float r3AGL_thL = he3 * bAGL_thL * 1./sqrt(1-pow(bAGL_thL,2)) / 2;

//Binning
const int MassBins = 200;
const float MinMass = 0, MaxMass = 10; // GeV/c^2

vector<TTree*> fromTerminal(bool isMC=false,
                                    TString CLtrainOnMCFile="000",
                                    TString CLvalOnISSFile="000",
                                    TString AEtrainOnMCFile="000",
                                    TString AEvalOnISSFile="000",
                                    vector<TTree*> myTrees = {}
                                );

array<double,6> GetCounts(TFile *fI,                 TFile *fM,                  
                          ofstream &txt,             TFile *f_out,
                          TEventList *&ISSlist,      TEventList *&MClist,
                          vector<TTree*> &ISS,       vector<TTree*> &MC,         TString MySel="",
                          const float MCtotEvents=0, const float MCtotEventsn=0, double MyPae=0.98,
                          double MyPcl2=0.98,        const int iDet=0,           const int ibR=0,
                          TString outName="",        bool Update=false,          bool FirstNNs=false,
                          bool verbose=false);
void MyQuantile(double &xp, double &p, TH1F *h1=nullptr);

void PosStyle(TH1F*h,EColor Lcolor);
void NegStyle(TH1F*h,EColor Lcolor, EColor Fcolor, bool AmImc);
void DefineTH2style(TH2F*h2);


//TEventLists handling
void InitializeEventLists(array<array<TEventList *, NCut>,NCut> &Lists,
                          array<double,NCut> &AE, array<double,NCut> &CL2, 
                          int &id,                array<TString, NDet> &MyDets,
                          bool isMC=false);
void SaveEventLists(array<array<TEventList *, NCut>,NCut> &Lists,
                    int &id,                array<TString, NDet> &MyDets,
                    TFile *fOut,            bool isMC=false);


double GetSignificance (double &Non, double &muBkg);
double GetSignificance2(double &Non, double &Noff, double &tau);


void Significance3Det(){

    gErrorIgnoreLevel = kWarning;
    //Style
    gStyle->SetOptTitle(0);
    gStyle->SetPalette(55);//kRainBow palette
    gStyle->SetOptStat(0);

    //Conditions
    bool UpdateFile = false;
    bool doPrint = false;
    bool FirstCutOnNNs = true;
    //Root outfile
    TString MyOutName;
    //(FirstCutOnNNs) ? MyOutName= "/eos/home-f/frrossi/AMS/Significance/Significance_FirstNNs.root" : MyOutName = "/eos/home-f/frrossi/AMS/Significance/Significance.root";
    (FirstCutOnNNs) ? MyOutName= "Significance_FirstNNs_NEW.root" : MyOutName = "Significance.root";
    //Scores files
    TString ScoreFileName;
    (FirstCutOnNNs) ? ScoreFileName = "scores_TrainOnMC_NNs_NEW.root" : ScoreFileName = "scores_TrainOnMC_beta.root";
    TFile *f_MCs  = new TFile(ScoreFileName.Data(),"OPEN");
    (FirstCutOnNNs) ? ScoreFileName = "scores_TrainOnISS_NNs_NEW.root" : ScoreFileName = "scores_TrainOnISS_beta.root";
    TFile *f_ISSs = new TFile(ScoreFileName.Data(),"OPEN");

    //Selections
    ostringstream RvalNaF; RvalNaF  << fixed << setprecision(3) << r3NaF_thL;
    ostringstream RvalAGL; RvalAGL  << fixed << setprecision(3) << r3AGL_thL;
    // array<TString, NDet> R3abTH{"abs(Rinner)-(3*pow(Rinner,2)*invRerr_IN)>=1.92",
    //     " abs(Rinner)-(3*pow(Rinner,2)*invRerr_IN)>="+TString(RvalNaF.str()),
    //     " abs(Rinner)-(3*pow(Rinner,2)*invRerr_IN)>="+TString(RvalAGL.str())};
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
            "(hasRich==1 && isNaF==0 && beta_rich<1 && (ringHits_dir_rich+ringHits_ref_rich)>=4 && beta_consistencyTOF<0.06) &&"
            "(beta_tof<0.96 || hasRich == 1) "};
    if(!FirstCutOnNNs){
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
    array<TString, NDet>MyDets{"TOF","NaF","AGL"};
     
    //Definition of cut values
    // array<double,NCut> effAE{0.95,0.90,0.85,0.80,0.75,0.70,0.65}, effCL2{0.95,0.90,0.85,0.80,0.75, 0.70, 0.65};
    array<double,NCut> effAE{0.95,0.90,0.85,0.82,0.80,0.78,0.75}, effCL2{0.95,0.90,0.85,0.80,0.75,0.65,0.50};
    TH2F* hSig, *hSig2;
    array<TString, NDet> SigName{"TOF_Sign","NaF_Sign","AGL_Sign"};
    array<TString, NDet> Sig2Name{"TOF_Sign2","NaF_Sign2","AGL_Sign2"};
    array<TString, NDet> SigTitle{"TOF significance; Anomaly score efficiency (R>0); Classifier score efficiency (RigLabel==1); Significance",
                                  "NaF significance; Anomaly score efficiency (R>0); Classifier score efficiency (RigLabel==1); Significance",
                                  "AGL significance; Anomaly score efficiency (R>0); Classifier score efficiency (RigLabel==1); Significance"};
    array<TString, NDet> Sig2Title{"TOF significance 2; Anomaly score efficiency (R>0); Classifier score efficiency (RigLabel==1); Significance",
                                   "NaF significance 2; Anomaly score efficiency (R>0); Classifier score efficiency (RigLabel==1); Significance",
                                   "AGL significance 2; Anomaly score efficiency (R>0); Classifier score efficiency (RigLabel==1); Significance"};
    const double minEffAE  = (*min_element(effAE.begin(), effAE.end())) - 0.04;
    const double minEffCL2 = (*min_element(effCL2.begin(), effCL2.end())) - 0.04;
    const double maxEffAE  = (*max_element(effAE.begin(), effAE.end()))  + 0.04;
    const double maxEffCL2 = (*max_element(effCL2.begin(), effCL2.end())) + 0.04;

    vector<TTree*> myISS, myMC;
    myISS = fromTerminal(false, "2025_10_29", "2025_10_29", "2025_10_29", "2025_10_29");
    myMC  = fromTerminal(true,  "2025_10_29", "2025_10_29", "2025_10_29", "2025_10_29");
    /*
    myISS = fromTerminal(false, "2025_09_12", "2025_09_12", "2025_09_12", "2025_09_12");
    myMC  = fromTerminal(true,  "2025_09_12", "2025_09_12", "2025_09_12", "2025_09_12");
    */
    /* INNER no beta rec
    myISS = fromTerminal(false, "2025_08_29", "2025_08_29", "2025_08_29", "2025_08_29");
    myMC  = fromTerminal(true,  "2025_08_29", "2025_08_29", "2025_08_29", "2025_08_29");
     */
    //INL1
    //myISS = fromTerminal(false, "2025_08_17", "2025_08_17", "2025_08_17", "2025_08_17");
    //myMC  = fromTerminal(true,  "2025_08_17", "2025_08_17", "2025_08_17", "2025_08_17");
    //myISS = fromTerminal(false, "2025_06_30", "2025_06_30", "2025_06_30", "2025_06_30");
    //myMC  = fromTerminal(true,  "2025_06_30", "2025_06_30", "2025_06_30", "2025_06_30");

    TH1F *hMC_TotPos = new TH1F("hMC_TotPos","hMC_TotPos; log_{10}(R_{INNER}); Weights", 500, -2, 600);
    TH1F *hMC_TotNeg = new TH1F("hMC_TotNeg","hMC_TotNeg; log_{10}(R_{INNER}); Weights", 500, -2, 600);
    myMC.at(0)->Draw("log10(abs(Rinner))>>+hMC_TotPos","1*weight*weightISO","goff");
    hMC_TotPos = (TH1F*)gDirectory->Get("hMC_TotPos");
    myMC.at(1)->Draw("log10(abs(Rinner))>>+hMC_TotNeg","1*weight*weightISO","goff");
    hMC_TotNeg = (TH1F*)gDirectory->Get("hMC_TotNeg");
    const float MCtotE  = hMC_TotPos->Integral();
    const float MCtotEn = hMC_TotNeg->Integral();
    delete hMC_TotPos; delete hMC_TotNeg;
    printf("Check on MC TOT events: %f, %f\n", MCtotE, MCtotEn);


    //Output file
    printf("Saving to root file: %s \n", MyOutName.Data());
    TFile *fO = new TFile(MyOutName.Data(),"Recreate");
        //Creating TEventLists (1 for each cut combo)
    array<array<TEventList *, NCut>,NCut> MyISSevLists{}, MyMCevLists{};
    
    //Loop on detectors
    for(int id=0; id<NDet; ++id){

        fO->mkdir(MyDets.at(id).Data());
        TString CreateDir(MyDets.at(id)+"/ISSevLists"); fO->mkdir(CreateDir.Data());
        CreateDir = MyDets.at(id)+"/MCevLists";         fO->mkdir(CreateDir.Data());
        printf("Working on detector %s ...\n", MyDets.at(id).Data());

        //Writing to TXT
        TString outTXT;
        ofstream fileTXT;
        if(FirstCutOnNNs) outTXT = "/eos/home-f/frrossi/AMS/Significance/"+MyDets.at(id)+"_FirstNNs.txt";
        else outTXT = "/eos/home-f/frrossi/AMS/Significance/"+MyDets.at(id)+".txt";
        remove(outTXT.Data());
        fileTXT.open(outTXT.Data(), std::ios::out | std::ios::app);
        fileTXT << std::fixed << std::setprecision(5);
        fileTXT << right << setw(15)  << "R bin N."
            << setw(15)  << "eff CL2" << setw(15) << "eff R>0" 
            << setw(15)  << "ISS CL2" << setw(15) << "ISS AE"
            << setw(15)  << "MC CL2"  << setw(15) << "MC AE"
            << setw(15)  << "epsISSp" << setw(15) << "epsMCp"
            << setw(15)  << "ISSp"    << setw(15) << "MCp"
            << setw(15)  << "epsISSn" << setw(15) << "epsMCn"
            << setw(15)  << "scaleP"  << setw(15) << "scaleN"
            << setw(15)  << "ISS"     << setw(15) << "+-Err" 
            << setw(15)  << "MC"      << setw(15) << "+-Err"
            << setw(15)  << "MCcr"    << setw(15) << "+-Err"
            << setw(15)  << "ISScr"   << setw(15) << "+-Err"
            << setw(15)  << "tau"     << setw(15) << "MCcounts"
            << setw(15)  << "Sign1"   << setw(15) << "Sign2"
            << "\n";
        
        InitializeEventLists(MyISSevLists, effAE, effCL2, id, MyDets, false);
        InitializeEventLists(MyMCevLists,  effAE, effCL2, id, MyDets, true);
        
        //Loop on Rig Bins
        for(int ib=0; ib<nRigbins; ++ib){
            if(id==0 && ib>=2) continue;
            if(id==1 && ib>=5) continue;
            if(id==2 && ib<nRigbins-1 && Rbins.at(ib+1)<r3AGL_thL) continue;
            //Deifining TH2 for significance
            TString h2Name; h2Name = SigName.at(id)+"Rbin_"+to_string(ib);
            hSig = new TH2F(h2Name.Data(),   SigTitle.at(id).Data(),  NCut, minEffAE, maxEffAE, NCut, minEffCL2, maxEffCL2);
            h2Name = Sig2Name.at(id)+"Rbin_"+to_string(ib);
            hSig2 = new TH2F(h2Name.Data(), Sig2Title.at(id).Data(), NCut, minEffAE, maxEffAE, NCut, minEffCL2, maxEffCL2);
            DefineTH2style(hSig); DefineTH2style(hSig2);
            

            //Loop on AE cut
            for(int cutAE=0; cutAE<NCut; ++cutAE){
                //Loop on CL2 cut
                for(int cutCL=0; cutCL<NCut; ++cutCL){
                    //[ISS,ISSerr,MC,MCerr,ISScr,tau]
                    array<double,6> IntAndErr = GetCounts(f_ISSs,                           f_MCs,        
                                                          fileTXT,                          fO,
                                                          MyISSevLists.at(cutAE).at(cutCL), MyMCevLists.at(cutAE).at(cutCL),
                                                          myISS,            myMC,       Sel.at(id),
                                                          MCtotE,           MCtotEn,    effAE.at(cutAE),
                                                          effCL2.at(cutCL), id,         ib,
                                                          MyOutName,        UpdateFile, FirstCutOnNNs,
                                                          doPrint);
                    double Significance  = GetSignificance(IntAndErr.at(0), IntAndErr.at(2));
                    double Significance2 = GetSignificance2(IntAndErr.at(0), IntAndErr.at(4), IntAndErr.at(5));
                    //Writing to file                    
                    fileTXT << setw(15)  << Significance << setw(15)  << Significance2 <<"\n"; 
                                    
                    hSig->Fill(effAE.at(cutAE),effCL2.at(cutCL),Significance);
                    hSig2->Fill(effAE.at(cutAE),effCL2.at(cutCL),Significance2);
                    UpdateFile = true;
                } //end loop CL2
            } //end loop AE
            TString myPath = MyDets.at(id)+"/Rbin_"+to_string(ib);
            fO->cd(myPath.Data());
            hSig->Write(); hSig2->Write();
            delete hSig; delete hSig2;
        } // end loop on Rig bins

        //Saving TEventLists
        SaveEventLists(MyISSevLists, id, MyDets, fO, false);
        SaveEventLists(MyMCevLists, id, MyDets, fO, true);
        fileTXT.close();
    }//end loop on detectors    
    return;
}





double GetSignificance(double &Non, double &muBkg){
    double Significance;
    double T1 = Non*log(Non/muBkg);
    double T2 = -Non + muBkg;
    if(Non==0 || muBkg==0) Significance=0;
    else Significance = (Non-muBkg)/abs(Non-muBkg) * sqrt(2 *(T1+T2));
    if(Non!=0 && muBkg==0) Significance=-3;
    return Significance; 
}

double GetSignificance2(double &Non, double &Noff, double &tau){
    double Significance2;
    double T1 = Non*log(Non*(1+tau)/(Non+Noff));
    double T2 = Noff*log(Noff*(1+tau)/(tau*(Non+Noff)));
    if(Non==0 || tau==0) Significance2=0;
    else Significance2 = sqrt(2*(T1+T2));
    if(Non!=0 && tau==0) Significance2=-3;
    if(isnan(Significance2)) printf("\n\n|||NAN|||\nT1:%f T2:%f Non:%f Noff:%f tau:%f\n\n", T1, T2, Non, Noff, tau);
    return Significance2; 
}

void PosStyle(TH1F *h,EColor Lcolor, bool AmImc){
    h->SetLineWidth(LW); 
    h->GetXaxis()->CenterTitle(true); h->GetXaxis()->SetLabelFont(62); h->GetXaxis()->SetTitleFont(62); 
    h->GetYaxis()->CenterTitle(true); h->GetYaxis()->SetLabelFont(62); h->GetYaxis()->SetTitleFont(62); h->GetYaxis()->SetMaxDigits(2);

    if(AmImc) { h->SetFillColor(Lcolor); h->SetFillStyle(3004); }
    else{ h->SetMarkerStyle(20); h->SetLineColor(Lcolor); h->SetMarkerColor(Lcolor); }
    return;
}
void NegStyle(TH1F*h,EColor Lcolor, EColor Fcolor, bool AmImc){
    h->SetLineColor(Lcolor); h->SetLineWidth(LW); h->SetFillColor(Fcolor);
    if(AmImc) h->SetFillStyle(3004);
    return;
}
void DefineTH2style(TH2F*h2){
    h2->GetXaxis()->CenterTitle(true); h2->GetXaxis()->SetLabelFont(62); h2->GetXaxis()->SetTitleFont(62); 
    h2->GetYaxis()->CenterTitle(true); h2->GetYaxis()->SetLabelFont(62); h2->GetYaxis()->SetTitleFont(62);
}

void InitializeEventLists(
                          array<array<TEventList *, NCut>,NCut> &Lists,
                          array<double,NCut> &AE, array<double,NCut> &CL2, 
                          int &id,                   array<TString, NDet> &MyDets,
                          bool isMC=false
                        ){
    for(int cutAE=0; cutAE<NCut; ++cutAE){ //Loop on AE cut
        for(int cutCL=0; cutCL<NCut; ++cutCL){ //Loop on CL2 cut
            ostringstream effAE, effCL2;
            effAE  << fixed << setprecision(0) << AE.at(cutAE)*100;
            effCL2 << fixed << setprecision(0) << CL2.at(cutCL)*100;
            TString name;
            if(isMC) name = "MC_"+MyDets.at(id)+"_AE_"+TString(effAE.str())+"_CL_"+TString(effCL2.str());
            else     name = "ISS_"+MyDets.at(id)+"_AE_"+TString(effAE.str())+"_CL_"+TString(effCL2.str());
            name.ReplaceAll(".","_");
            Lists.at(cutAE).at(cutCL) = new TEventList(name.Data(),name.Data());
        }
    }
    return;
}
void SaveEventLists(array<array<TEventList *, NCut>,NCut> &Lists,
                    int &id,                array<TString, NDet> &MyDets,
                    TFile *fOut,            bool isMC =false){
    TString path;
    if (isMC) path = MyDets.at(id)+"/MCevLists";
    else      path = MyDets.at(id)+"/ISSevLists";
    fOut->cd(path.Data());
    for(int iAE=0; iAE<NCut; ++iAE){
        for(int iCL=0; iCL<NCut; ++iCL){
            Lists.at(iAE).at(iCL)->Write();
        }
    }
    fOut->cd();
    return;
}


array<double,6> GetCounts(TFile *fI,                 TFile *fM,                  
                          ofstream &txt,             TFile *f_out,
                          TEventList *&ISSlist,      TEventList *&MClist,
                          vector<TTree*> &ISS,       vector<TTree*> &MC,       TString MySel="",
                          const float MCtotEvents=0, const float MCtotEventsn=0, double MyPae=0.98,
                          double MyPcl2=0.98,        const int iDet=0,           const int ibR=0,
                          TString outName="",        bool Update=false,          bool FirstNNs=false,
                          bool verbose=false){
    TDirectory *DefDir = gDirectory;
    
    ostringstream effAE, effCL2;
    effAE  << fixed << setprecision(2) << MyPae;
    effCL2 << fixed << setprecision(2) << MyPcl2; 
    DefDir->cd();
    

    //Efficiency cuts on scores
    double pCL2 = 1-MyPcl2;//10 percentile -> 90% efficiency on RigLabel=1
    double pAE  = MyPae;//95 percentile -> 90% efficiency on R>0 anomaly score
    double CL2cut=0, AEcut=0; //scores>=CL2cut && anomaly<=AEcut
    ostringstream CL2cutISS, CL2cutMC, AEcutISS, AEcutMC;
    float epsISS, epsMC;
    //Integrals
    double intISS,    intISScr,    intMC,    intMCcr,   tau;
    double MCcounts;
    double intErrISS, intErrISScr, intErrMC, intErrMCcr;
    //Selections for samples
    TString ISSposSel, MCposSel, ISSnegSelCR, MCnegSelCR;
    TString MassCut;

    //Scores TH1
    TH1F* hCL2SigISS,  *hCL2SigMC;
    TH1F* hCL2SigISSn, *hCL2BkgISSn, *hCL2SigMCn, *hCL2BkgMCn;
    TH1F* hAESigISS,   *hAESigMC;
    TH1F* hAEBkgISSn,  *hAEBkgMCn;

    //TStrings
    float CL2sFactor, AEsFactor;
    array<TString, NDet> dets{"TOF", "NaF", "AGL"};
    TString CL2SigNameISS, CL2SigNameISSn, CL2BkgNameISSn;
    TString CL2SigNameMC,  CL2SigNameMCn,  CL2BkgNameMCn;
    TString AESigNameISS,  AEBkgNameISSn;
    TString AESigNameMC,   AEBkgNameMCn;
    //Mass
    TCanvas* canva;  TLegend* legend; TGaxis* axis;
    TH1F* hISSpos, *hISSneg, *hISSnegCR, *hMCpos, *hMCneg, *hMCnegCR, *hMCcounts;
    //TStrings
    TString hISSposName, hISSnegName, hISSnegNameCR;
    TString hMCposName,  hMCnegName,  hMCnegNameCR;
    TString hISSposTitle("ISS data (R_{INNER}>0); Mass [GeV/c^{2}]; Events / 0.1 [GeV/c^{2}]");
    TString hISSnegTitle("ISS data (R_{INNER}<0); Mass [GeV/c^{2}]; Events / 0.2 [GeV/c^{2}]");
    TString hISSnegTitleCR("ISS data (R_{INNER}<0) CR; Mass [GeV/c^{2}]; Events / 0.2 [GeV/c^{2}]");
    TString hMCposTitle("^{3+4}He MC (R_{INNER}>0); Mass [GeV/c^{2}]; Weights / 0.1 [GeV/c^{2}]");
    TString hMCnegTitle("^{3+4}He MC (R_{INNER}<0); Mass [GeV/c^{2}]; Weights / 0.2 [GeV/c^{2}]");
    TString hMCnegTitleCR("^{3+4}He MC (R_{INNER}<0) CR; Mass [GeV/c^{2}]; Weights / 0.2 [GeV/c^{2}]");
    TString hMassName;
    //Drawings lines
    TString ISSpos_toDraw, ISSneg_toDraw, ISSnegCR_toDraw, MCpos_toDraw, MCneg_toDraw, MCnegCR_toDraw;
    TString DrawMass;
    
    //Definitions depending from detector
    //CL2 - AE
    CL2SigNameISS = "ISS_RposRig_"+dets.at(iDet)+"Label1";    CL2SigNameISSn = "ISS_RnegRig_"+dets.at(iDet)+"Label1";   CL2BkgNameISSn = "ISS_RnegRig_"+dets.at(iDet)+"Label0";
    CL2SigNameMC  = "He4_MC_RposRig_"+dets.at(iDet)+"Label1"; CL2SigNameMCn = "He4_MC_RnegRig_"+dets.at(iDet)+"Label1"; CL2BkgNameMCn = "He4_MC_RnegRig_"+dets.at(iDet)+"Label0";
    AESigNameISS  = "ISS_Rpos_"+dets.at(iDet);             AEBkgNameISSn = "ISS_Rneg_"+dets.at(iDet);
    AESigNameMC   = "He4_MC_Rpos_"+dets.at(iDet);          AEBkgNameMCn  = "He4_MC_Rneg_"+dets.at(iDet);
    //Mass h1
    hISSposName = dets.at(iDet)+"_ISSpos"; hISSnegName = dets.at(iDet)+"_ISSneg"; hISSnegNameCR = dets.at(iDet)+"_ISSnegCR";
    hMCposName  = dets.at(iDet)+"_MCpos";  hMCnegName = dets.at(iDet)+"_MCneg";   hMCnegNameCR = dets.at(iDet)+"_MCnegCR";
    if(iDet==0) DrawMass = "(2.*abs(Rinner)*sqrt(1-pow(beta_tof,2)))/beta_tof>>+";
    else        DrawMass = "(2.*abs(Rinner)*sqrt(1-pow(beta_rich,2)))/beta_rich>>+";
    //Definition of Mass histograms
    if(verbose) printf("Defining mass TH1...\n");
    DefDir->cd();
    hMassName = hISSposName+"_AEeff_"+effAE.str()+"_CL2eff_"+effCL2.str(); hMassName.ReplaceAll(".","");
    hISSpos   = new TH1F(hMassName.Data(), hISSposTitle.Data(),   MassBins, MinMass, MaxMass);
    hMassName = hISSnegName+"_AEeff_"+effAE.str()+"_CL2eff_"+effCL2.str(); hMassName.ReplaceAll(".","");
    hISSneg   = new TH1F(hMassName.Data(), hISSnegTitle.Data(),   MassBins, MinMass, MaxMass);
    hMassName = hISSnegNameCR+"_AEeff_"+effAE.str()+"_CL2eff_"+effCL2.str(); hMassName.ReplaceAll(".","");
    hISSnegCR = new TH1F(hMassName.Data(), hISSnegTitleCR.Data(), MassBins, MinMass, MaxMass);
    hMassName = hMCposName+"_AEeff_"+effAE.str()+"_CL2eff_"+effCL2.str(); hMassName.ReplaceAll(".","");
    hMCpos    = new TH1F(hMassName.Data(),   hMCposTitle.Data(),  MassBins, MinMass, MaxMass);
    hMassName = hMCnegName+"_AEeff_"+effAE.str()+"_CL2eff_"+effCL2.str(); hMassName.ReplaceAll(".","");
    hMCneg    = new TH1F(hMassName.Data(),   hMCnegTitle.Data(),  MassBins, MinMass, MaxMass);
    hMassName = hMCnegNameCR+"_AEeff_"+effAE.str()+"_CL2eff_"+effCL2.str(); hMassName.ReplaceAll(".","");
    hMCnegCR  = new TH1F(hMassName.Data(), hMCnegTitleCR.Data(),  MassBins, MinMass, MaxMass);
    hMCcounts = new TH1F("hMCcounts",";Mass;Counts", MassBins/2,  MinMass, MaxMass);
    //Definition of drawing lines
    if(verbose) printf("Defining drawing lines...\n");
    ISSpos_toDraw  = DrawMass+hISSpos->GetName(); ISSneg_toDraw = DrawMass+hISSneg->GetName(); ISSnegCR_toDraw = DrawMass+hISSnegCR->GetName();
    MCpos_toDraw   = DrawMass+hMCpos->GetName();  MCneg_toDraw = DrawMass+hMCneg->GetName();   MCnegCR_toDraw = DrawMass+hMCnegCR->GetName();

    //Getting scores in rig bins
    if(verbose) {
        printf("/////////////\n");
        printf("scores cuts: \t eff RigLabel==1 -> %.2f eff R>0 -> %.2f\n", 1-pCL2, pAE);
        printf("\n\n===> %s\n", dets.at(iDet).Data());
        printf("Getting TH1 for CL2 (RigLabel==1) and AE (R>0) ...\n");
    }

    //CL2 ISS
    TString dir = dets.at(iDet)+"/CL2/ISS_pos/"+CL2SigNameISS+"_bin"+to_string(ibR);
    hCL2SigISS  = (TH1F*)fI->Get(dir.Data()); MyQuantile(CL2cut, pCL2, hCL2SigISS); CL2cutISS << fixed << setprecision(5) << CL2cut;
    dir = dets.at(iDet)+"/CL2/ISS_neg/"+CL2SigNameISSn+"_bin"+to_string(ibR); hCL2SigISSn = (TH1F*)fI->Get(dir.Data());
    dir = dets.at(iDet)+"/CL2/ISS_neg/"+CL2BkgNameISSn+"_bin"+to_string(ibR); hCL2BkgISSn = (TH1F*)fI->Get(dir.Data());

    //CL2 MC
    dir = dets.at(iDet)+"/CL2/MC_pos/"+CL2SigNameMC+"_bin"+to_string(ibR);
    hCL2SigMC = (TH1F*)fM->Get(dir.Data()); MyQuantile(CL2cut, pCL2, hCL2SigMC); CL2cutMC  << fixed << setprecision(5) << CL2cut;
    dir = dets.at(iDet)+"/CL2/MC_neg/"+CL2SigNameMCn+"_bin"+to_string(ibR); hCL2SigMCn = (TH1F*)fM->Get(dir.Data());
    dir = dets.at(iDet)+"/CL2/MC_neg/"+CL2BkgNameMCn+"_bin"+to_string(ibR); hCL2BkgMCn = (TH1F*)fM->Get(dir.Data());

    //AE ISS
    dir = dets.at(iDet)+"/AE/ISS_pos/"+AESigNameISS+"_bin"+to_string(ibR);
    hAESigISS = (TH1F*)fI->Get(dir.Data()); MyQuantile(AEcut, pAE, hAESigISS); AEcutISS  << fixed << setprecision(5) << AEcut;
    dir = dets.at(iDet)+"/AE/ISS_neg/"+AEBkgNameISSn+"_bin"+to_string(ibR); hAEBkgISSn = (TH1F*)fI->Get(dir.Data());

    //AE MC
    dir = dets.at(iDet)+"/AE/MC_pos/"+AESigNameMC+"_bin"+to_string(ibR);
    hAESigMC = (TH1F*)fM->Get(dir.Data()); MyQuantile(AEcut, pAE, hAESigMC); AEcutMC  << fixed << setprecision(5) << AEcut;
    dir = dets.at(iDet)+"/AE/MC_neg/"+AEBkgNameMCn+"_bin"+to_string(ibR); hAEBkgMCn = (TH1F*)fM->Get(dir.Data());

    if(verbose) printf("CL2: %f, %f, %f. %f \n \t %f, %f, \n AE: %f, %f, %f, %f\n\n",
            hCL2SigISS->GetEntries(),  hCL2SigMC->GetEntries(),   hCL2SigISSn->GetEntries(),
            hCL2SigMCn->GetEntries(),  hCL2BkgISSn->GetEntries(), hCL2BkgMCn->GetEntries(),
            hAESigISS->GetEntries(),   hAESigMC->GetEntries(),    hAEBkgISSn->GetEntries(),  hAEBkgMCn->GetEntries());
    
    

    //Definition of Event selections
    if(verbose && ibR==0) printf("Event selection...\n");
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
    ISSposSel   = MySel+RigSel+
                " && anomaly_score<="+TString(AEcutISS.str())+" && scores>="+TString(CL2cutISS.str());
    ISSnegSelCR = MySel+RigSel+
                " && anomaly_score>"+TString(AEcutISS.str())+" && scores<"+TString(CL2cutISS.str());
    MCposSel   = MySel+RigSel+
                " && anomaly_score<="+TString(AEcutMC.str())+" && scores>="+TString(CL2cutMC.str());
    MCnegSelCR = MySel+RigSel+
                " && anomaly_score>"+TString(AEcutMC.str())+" && scores<"+TString(CL2cutMC.str());

    MCposSel.Prepend("(");   MCposSel.Append(")*weight*weightISO");
    MCnegSelCR.Prepend("("); MCnegSelCR.Append(")*weight*weightISO");
    if(verbose && ibR==0) printf("Common selection: %s \n", MySel.Data()); 

    if(verbose) printf("\t%s %s\n\t%s %s\n", TString("ISS_CL2scores>="+TString(CL2cutISS.str())).Data(), TString("ISS_AEscore<="+TString(AEcutISS.str())).Data(), 
                TString("MC_CL2scores>=" +TString(CL2cutMC.str())).Data(),  TString("MC_AEscore<=" +TString(AEcutMC.str())).Data());

    //Writing to TXT file 
    txt << right  << setw(15)  << ibR
            << setw(15)  << 1-pCL2                          << setw(15)  << pAE
            << setw(15)  << TString(CL2cutISS.str()).Data() << setw(15)  << TString(AEcutISS.str()).Data()
            << setw(15)  << TString(CL2cutMC.str()).Data()  << setw(15)  << TString(AEcutMC.str()).Data();
    
    //Drawings
    if(verbose) printf("Getting mass Th1...\n");
    ISS.at(2)->Draw(ISSpos_toDraw.Data(),ISSposSel.Data(),"goff");
    hISSpos = (TH1F*)gDirectory->Get(hISSpos->GetName());
    PosStyle(hISSpos, kBlack, false);
        
    ISS.at(3)->Draw(ISSneg_toDraw.Data(),ISSposSel.Data(), "goff");
    hISSneg = (TH1F*)gDirectory->Get(hISSneg->GetName());
    NegStyle(hISSneg,kSpring,kGreen, false);
    
    ISS.at(3)->Draw(ISSnegCR_toDraw.Data(),ISSnegSelCR.Data(),"goff");
    hISSnegCR = (TH1F*)gDirectory->Get(hISSnegCR->GetName());

    MC.at(0)->Draw(MCpos_toDraw.Data(),MCposSel.Data(), "goff");
    hMCpos = (TH1F*)gDirectory->Get(hMCpos->GetName());
    PosStyle(hMCpos,  kCyan, true);
            
    MC.at(1)->Draw(MCneg_toDraw.Data(), MCposSel.Data(), "goff");
    hMCneg = (TH1F*)gDirectory->Get(hMCneg->GetName());
    NegStyle(hMCneg, kMagenta,kViolet, true);

    MC.at(1)->Draw(MCnegCR_toDraw.Data(), MCnegSelCR.Data(), "goff");
    hMCnegCR = (TH1F*)gDirectory->Get(hMCnegCR->GetName());

        //pure counts for MC
    MCneg_toDraw = DrawMass+hMCcounts->GetName();
    MCposSel.ReplaceAll(")*weight*weightISO",")*1");
    MC.at(1)->Draw(MCneg_toDraw.Data(), MCposSel.Data(), "goff");
    hMCcounts = (TH1F*)gDirectory->Get(hMCcounts->GetName());

        //TEventList
    /*
    if(iDet==0) MassCut = "(2.*abs(Rinner)*sqrt(1-pow(beta_tof,2)))/beta_tof";
    else        MassCut = "(2.*abs(Rinner)*sqrt(1-pow(beta_rich,2)))/beta_rich";
    */

    //ISSposSel.Append("&& "+MassCut+">=2 && "+MassCut+"<=5");
    ISS.at(3)->Draw(">>hISSlist",ISSposSel.Data(), "goff");
    TEventList *ISStmpList = (TEventList*)gDirectory->Get("hISSlist");
    if(ISStmpList){
        ISSlist->Add(ISStmpList);
        delete ISStmpList;
    }

    MCposSel.ReplaceAll(")*1",")");
    //MCposSel.ReplaceAll(")*1","");
    //MCposSel.Append("&& "+MassCut+">=2 && "+MassCut+"<=5)");
    MC.at(1)->Draw(">>hMClist", MCposSel.Data(), "goff");
    TEventList* MCtmpList = (TEventList*)gDirectory->Get("hMClist");
    if(MCtmpList){
        MClist->Add(MCtmpList);
        delete MCtmpList;
    }


    //Scaling factors for negatives
    CL2sFactor = (hCL2SigISSn->Integral()+hCL2BkgISSn->Integral()) /
                        (hCL2SigMCn->Integral()+hCL2BkgMCn->Integral());
    AEsFactor  = hAEBkgISSn->Integral() / hAEBkgMCn->Integral();

    //Scale factor MC integral = ISS data integral
    epsISS = hISSpos->Integral()/ISS.at(2)->GetEntries();
    epsMC  = hMCpos->Integral() /MCtotEvents;
    if(verbose){
        printf("Computing scale factors for MC...\n");
        printf("Check on efficiencies: %f, %f\n", epsISS, epsMC);
    }
    txt << right << setw(15)  << epsISS << setw(15)  << epsMC; //Efficiencies for positives
    ISSposSel.Prepend("Rinner>IGRFpos &&");
    txt << right << setw(15)  << ISS.at(2)->GetEntries(ISSposSel.Data()) << setw(15)  << hMCpos->Integral(); //Efficiencies for positives
    if(ibR<5) hMCpos->Scale((epsISS/epsMC)*(ISS.at(2)->GetEntries()/MCtotEvents)); 
    else      hMCpos->Scale((epsISS/epsMC)*(ISS.at(2)->GetEntries()/MCtotEvents));
    
    epsISS = hISSneg->Integral()/ISS.at(3)->GetEntries();
    epsMC  = hMCneg->Integral() /MCtotEventsn;
    if(verbose){
        printf("Check on efficiencies: %f, %f\n", epsISS, epsMC);
        printf("Check on scale factors: %f, %f\n",  hISSpos->Integral()/hMCpos->Integral(), hISSneg->Integral()/hMCneg->Integral());
    }
    txt << right << setw(15)  << epsISS << setw(15)  << epsMC; //Efficienties for negatives
    //Cross-check
    txt << right << setw(15)  << hISSpos->Integral()/hMCpos->Integral() << setw(15)  << hISSneg->Integral()/hMCneg->Integral();
        
    //Getting integrals inside the He mass region [2,5] GeV/c^2
    intISS   = hISSneg->IntegralAndError(hISSneg->FindFixBin(2), hISSneg->FindFixBin(5),intErrISS);
    intISScr = hISSnegCR->IntegralAndError(hISSnegCR->FindFixBin(2), hISSnegCR->FindFixBin(5), intErrISScr);
    intMC    = hMCneg->IntegralAndError( hMCneg->FindFixBin(2), hMCneg->FindFixBin(5),  intErrMC);
    intMCcr  = hMCnegCR->IntegralAndError(hMCnegCR->FindFixBin(2), hMCnegCR->FindFixBin(5), intErrMCcr);
    (intMC==0)? tau = 0 : tau = intMCcr/intMC;
    MCcounts = hMCcounts->Integral( hMCcounts->FindFixBin(2), hMCcounts->FindFixBin(5));

    //Canvas
    if(verbose) printf("Creating canvas...\n");
    TString c_name = TString("c_")+dets.at(iDet)+"AEeff_"+effAE.str()+"_CL2eff_"+effCL2.str();
    c_name.ReplaceAll(".","");
    canva = MakeSquareCanvas(c_name.Data(), c_name.Data()); canva->cd(); canva->SetGrid();
    hISSpos->Draw("e"); hMCpos->Draw("hist same"); canva->Update();
    
    Float_t rightmax = 5*max(hISSneg->GetMaximum(), hMCneg->GetMaximum()); Float_t scale = gPad->GetUymax()/rightmax;
    TH1F *hMCnegCopy = (TH1F*)hMCneg->Clone("MCnegCopy");  TH1F* hISSnegCopy = (TH1F*)hISSneg->Clone("ISSnegCopy");
    hMCnegCopy->Scale(scale); hISSnegCopy->Scale(scale);
    hMCnegCopy->Rebin(2); hISSnegCopy->Rebin(2);
    hISSnegCopy->Draw("hist same"); hMCnegCopy->Draw("hist same");
    legend = canva->BuildLegend(0.5,0.35,0.8,0.83); legend->SetBorderSize(0); legend->SetFillStyle(0);
    //legend->SetNColumns(2);
    hISSpos->Draw("e same");
    axis = new TGaxis(gPad->GetUxmax(),gPad->GetUymin(),gPad->GetUxmax(), gPad->GetUymax(),0,rightmax,510,"+L");
    axis->SetTitle("Events / 0.2 [GeV/c^{2}]"); axis->SetTitleColor(kGreen+1); axis->SetLineColor(kGreen+1); axis->SetLabelColor(kGreen+1);
    axis->CenterTitle(true); axis->SetLabelSize(hISSpos->GetYaxis()->GetLabelSize()); axis->SetTitleOffset(1.2);
    axis->SetTitleSize(hISSpos->GetYaxis()->GetTitleSize()); axis->Draw();    

    TString myTag = "R_{INNER} #in ["+TString(Rlow.str())+", "+TString(Rhigh.str()+"[");
    DrawTag(myTag.Data(), 0.59, 0.86, 0.03, 132);

    //Saving to root file    
    f_out->cd();
    TString MyDir = dets.at(iDet)+"/Rbin_"+to_string(ibR)+"/AEeff_"+effAE.str()+"_CL2eff_"+effCL2.str();
    MyDir.ReplaceAll(".","");
    f_out->mkdir(MyDir.Data());
    f_out->cd(MyDir.Data());
    hISSpos->Write(); hISSneg->Write();
    hMCpos->Write();  hMCneg->Write();
    canva->Write();
    TString DefPDFpath("/eos/home-f/frrossi/AMS/Significance/");
    TString PDFpath = DefPDFpath+MyDir+".pdf";
    canva->SaveAs(PDFpath.Data());
    delete hISSpos;     delete hISSneg; delete hISSnegCR;
    delete hMCpos;      delete hMCneg;  delete hMCnegCR;
    delete hMCnegCopy;  delete hISSnegCopy;
    delete hMCcounts;
    delete canva;   delete axis;    delete legend;

    txt << right << setw(15) << intISS   << setw(15) << intErrISS
                 << setw(15) << intMC    << setw(15) << intErrMC
                 << setw(15) << intMCcr  << setw(15) << intErrMCcr
                 << setw(15) << intISScr << setw(15) << intErrISScr
                 << setw(15) << tau      << setw(15) << MCcounts;

    return array<double,6>{intISS, intErrISS, intMC, intErrMC, intISScr, tau};
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


void MyQuantile(double &xp, double &p, TH1F *h1=nullptr){
    if(!h1){
        printf("IMPOSSIBLE to get quantiles without a TH1F, current pointer is null\n");
        return;
    }
    TGraph *cdfGraph = new TGraph();
    double cumulative = 0;
    for(int i=1; i<=h1->GetNbinsX(); ++i){
        cumulative += h1->GetBinContent(i);
        cdfGraph->SetPoint(i-1, cumulative / h1->Integral(), h1->GetBinCenter(i));
    }
    xp = cdfGraph->Eval(p);
    delete cdfGraph;
    return;
}
