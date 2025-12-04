#include "TRandom3.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>

using namespace std;

void GetBkgToy(TRandom3 &gen, TH1F *&hBkg, const int &nBkg, const double &muBkg, const double &stdBkg){
    hBkg = new TH1F("hBkg",";X;Event density",52,-10.5,15.5);
    for(int i=0; i<nBkg; ++i) hBkg->Fill(gen.Gaus(muBkg,stdBkg));
    return; 
}
void GetSigToy(TRandom3 &gen, TH1F *&hSig, int &nSig, const double &muSig, const double &stdSig){
    hSig = new TH1F("hSig",";X;Event densityt",52,-10.5,15.5);
    for(int i=0; i<nSig; ++i) hSig->Fill(gen.Gaus(muSig,stdSig));
    return;
}

double FindUpperLimit(double &nD_obs,      double &nMC_exp,    
                      double w,            TRandom3 &rng,
                      double CL = 0.95,    int maxToys = 1e6,
                      double tol = 2.5e-3, TH1F *h1=nullptr
                      ) {

    double nMC_toy = w*rng.Poisson(nMC_exp);
    double s_low = 0.0;
    double s_high = 200; // enlarge if needed

    // binary search
    while (s_high - s_low > 0.01) {
        double s_mid = 0.5 * (s_low + s_high);

        int nPass = 0;
        int nToys = 0;
        double F = 0.0, sigmaF = 1.0;

        // increase toys until significance criterion met
        for (int batch = 0; batch < 20; ++batch) {
            int batchSize = 1000+(batch*1000);
            for (int i = 0; i < batchSize; i++) {
                // if (rng.Poisson(s_mid+nMC_toy) > nD_obs) nPass++;
                if (rng.Poisson(s_mid+nMC_exp) > nD_obs) nPass++;
            }
            nToys += batchSize;
            F = double(nPass) / nToys;
            sigmaF = sqrt(F*(1.0-F)/nToys);

            // stopping condition
            if (fabs(F - CL) > 5.0*sigmaF){
                break;
            }
            if (fabs(F - CL) < tol && sigmaF < tol){
                break;
            }
            if (nToys >= maxToys){
                break;
            }
        }
        if (F < CL) s_low = s_mid;
        else s_high = s_mid;
    }
    h1->Fill(0.5*(s_low+s_high));
    return 0.5*(s_low+s_high);
}


void FindUpperLimitPZ(double &nD_obs,      double &nMC_exp, 
                      double w,            TRandom3 &rng,
                      double CL = 0.95,    int maxToys = 1e6,
                      double tol = 2.5e-3, TH1F *h1=nullptr) {
    const int Nbkg = 1000;

    for(int ib=0; ib<Nbkg; ib++){ //Loop on bkg uncertainty
        double nMC_toy = w*rng.Poisson(nMC_exp);

        double Denominator =0, Numerator=0; 

        for(int id=0; id<=nD_obs; ++id){ //Loop on observed events
            //Prob on BKG
            double BKGprob = TMath::Poisson(nD_obs-id, nMC_toy); // P(n; λ) = e^(-λ) λ^n / n!
            Denominator += BKGprob;

            double s_low = 0.0;
            double s_high = 200; // enlarge if needed
            while (s_high - s_low > 0.01) { // binary search
                double s_mid = 0.5 * (s_low + s_high);

                int nPass = 0;
                int nToys = 0;
                double F = 0.0, sigmaF = 1.0;

                // increase toys until significance criterion met
                for (int batch = 0; batch < 20; ++batch) {
                    int batchSize = 10000+(batch*1000);
                    for (int i = 0; i < batchSize; i++) {
                        if (rng.Poisson(s_mid) > id) nPass++;
                    }
                    nToys += batchSize;
                    F = double(nPass) / nToys;
                    sigmaF = sqrt(F*(1.0-F)/nToys);
        
                    // stopping condition
                    if (fabs(F - CL) > 5.0*sigmaF){
                    //    printf("F %f, sigma %f, Sig %f\n", F, sigmaF, s_mid);
                       break;
                    }
                    if (fabs(F - CL) < tol && sigmaF < tol){
                    //    printf("Condition 2 %f\n", nMC_toy);
                       break;
                    }
                    if (nToys >= maxToys){
                    //    printf("Condition 3\n");
                       break;
                    }
                }
                // too few toys under nD_obs -> need larger signal
                if (F < CL) s_low = s_mid;
                else s_high = s_mid;
            } //end binary search
            Numerator+=(0.5*(s_low+s_high)*BKGprob);
        } //end loop on observed events    
        h1->Fill(Numerator/Denominator);
    } //end loop on bkg uncertainty
    return;
}


void Coverage(){
    const int Nucl = 100;
    TRandom3 MyGen(1);
    
    const int Nbkg = 100, NsigTest=10;
    const int NstdBKG = 4, NmuSIG = 2;
    // int Nsig[NsigTest]; for(int i=0; i<NsigTest; ++i) Nsig[i]=i;
    int Nsig[NsigTest]{0,2,4,6,8,10,12,14,16,18};
    const double BKGmu = 0, SIGstd=0.5;
    const double BKGstd[NstdBKG]{0.5,1.5,3.0,4.0};
    const double SIGmu[NmuSIG]{2.81,3.73}; //He3 and He4

    array<TString, NmuSIG> sigNames{"He3","He4"};

    double Coverage=0;

    array<TH2F*,3> h2;
    array<TH1F*,3> hCov;
    TFile *fOut = new TFile("Coverage.root","RECREATE");
    for(int imu=0; imu<NmuSIG; ++imu){ //Loop on signal mu
        printf("Testing mu-SIG = %.1f\n", SIGmu[imu]);

        for(int istd=1; istd<NstdBKG; ++istd){ //Loop on bkg std

            printf("  ==> Testing std-BKG = %.1f\n", BKGstd[istd]);
            ostringstream bkg;
            bkg << fixed << setprecision(1) << BKGstd[istd];
            TString Name; Name = "hNsigNcl_"+sigNames.at(imu)+"_BKG_"+TString(bkg.str());
            Name.ReplaceAll(".","");
            auto maxIter = std::max_element(std::begin(Nsig), std::end(Nsig));
            h2.at(imu) = new TH2F(Name.Data(),"; n^{SIG}_{TRUE}; n^{SIG}_{CL}", *maxIter+1, Nsig[0]-0.5, *maxIter+0.5, 30,-0.5,29.5);
            h2.at(imu)->GetXaxis()->CenterTitle(true); h2.at(imu)->GetYaxis()->CenterTitle(true); 
            Name = "hCov_"+sigNames.at(imu)+"_BKG_"+TString(bkg.str());
            Name.ReplaceAll(".","");
            hCov.at(imu) = new TH1F(Name.Data(),"; n^{SIG}_{TRUE}; Coverage", *maxIter+1, Nsig[0]-0.5, *maxIter+0.5);
            for(int iSig=0; iSig<NsigTest; ++iSig){
                printf("\t True Nsig = %d\n", Nsig[iSig]);
                Coverage = 0;
                for(int iCL=0; iCL<Nucl; ++iCL){
                    TH1F *hBKG, *hSIG;
                    GetBkgToy(MyGen, hBKG, Nbkg, BKGmu, BKGstd[istd]);
                    GetSigToy(MyGen, hSIG, Nsig[iSig], SIGmu[imu], SIGstd);
                    TH1F *hCL = new TH1F("hCL",";CL;Counts",200,-0.5,199.5);
                    double Obs   = hSIG->Integral(hSIG->FindFixBin(2),hSIG->FindFixBin(5)); 
                    double exBkg = hBKG->Integral(hSIG->FindFixBin(2),hSIG->FindFixBin(5)); 
                    Obs+=exBkg;
                    

                    FindUpperLimitPZ(Obs, exBkg, 1., MyGen, 0.95, 1e6, 2.5e-3, hCL);
                    double xp[1], p[1]{0.5};
                    hCL->GetQuantiles(1,xp,p);
                    if(xp[0]>Nsig[iSig]) Coverage++;
                    h2.at(imu)->Fill(Nsig[iSig],xp[0]);
                    if(iCL<10) {
                        printf("\t\t N OBS: %.0f, N BKG = %.0f", Obs, exBkg);
                        printf("\t\t CL %.3f\n", xp[0]);
                    }

                    // double CL = FindUpperLimit(Obs, exBkg, 1., MyGen, 0.95, 1e6, 2.5e-3, hCL);
                    // if(iCL==0) printf(" CL %.3f\n", xp[0]);
                    // printf(" CL %.3f\n", CL);
                    // if(CL>Nsig[iSig]) Coverage++;
                    // h2.at(imu)->Fill(Nsig[iSig],CL);
                    
                    delete hCL;
                    delete hBKG; delete hSIG;
                }
                Coverage = Coverage/Nucl;
                printf("\t Coverage: %.3f +/- %.3f\n", Coverage,sqrt(Coverage/Nucl));
                hCov.at(imu)->SetBinContent(hCov.at(imu)->FindFixBin(Nsig[iSig]),Coverage);
                hCov.at(imu)->SetBinError(hCov.at(imu)->FindFixBin(Nsig[iSig]),sqrt(Coverage/Nucl));
            }
            fOut->cd();
            h2.at(imu)->Write();
            hCov.at(imu)->Write();
            return;
        } //end loop on bkg std 
    } //end loop on sig mu
    return;
}
