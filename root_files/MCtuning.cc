/*
----->
First attempt to tune the MC
----->
Created on: 22.10.2025
Last modified on: 
Usage:
    root -l DataMC_comparison("MCposData", "ISSposData");
*/

#include <vector>
#include <array>
#include <TString.h>
#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>
#include <TMath.h>

using namespace std;

void MCtuning(TString MCposDate="000", TString ISSposDate="000") {
    string outPath = "/eos/home-f/frrossi/AMS/ams_network/root_files/DataMC/";
    TString inPath  = "/eos/home-f/frrossi/AMS/output/organized_output/MLDs/";
    const int TotTree  = 2; // Number of trees divided by MC and ISS
    const int TotFiles = 2; // Number of files
    const bool UseWeight = true;
    const bool UseAllBranches = false;
    bool HasWeight = true;
    const int merge = 8; //number of bins to be merged on the y projection 
    const int NBranches = 34;
    TString preSel = "(NTrTrack<3 && ACC_AntiCounter<2 && abs(Rinner)>1.2 && chi2Time_tof < 10 && "
        "(hasRich==0 || (hasRich==1 && beta_rich<1 && (ringHits_dir_rich+ringHits_ref_rich)>=4 && beta_consistencyTOF<0.06)) && (beta_tof<0.96 || hasRich == 1)"
        "&& Rinner>18 && Rinner<24.7)";
    printf("------->>>\n");
    printf("Currently used pre-selection: %s\n", preSel.Data());
    printf("------->>>\n\n\n");

    array<TString, NBranches> Bnames = { "TRK_TrackResidualL2_Y", "TRK_TrackResidualL3_Y", "TRK_TrackResidualL4_Y",
                                         "TRK_TrackResidualL5_Y", "TRK_TrackResidualL6_Y", "TRK_TrackResidualL7_Y", 
                                         "TRK_TrackResidualL8_Y",
                                         "TRK_TrackResidualL3_X", "TRK_TrackResidualL5_X", "TRK_TrackResidualL6_X",
                                         "TRK_TrackResidualL7_X", "TRK_TrackResidualL8_X",
                                         "TRK_NormEdep2L5_XY",    "TRK_NormEdep2L6_XY",    "TRK_NormEdep2L7_XY",
                                         "TRK_NormEdep2L8_XY",
                                         "InnerHitY",             "InnerHitX",         
                                         "TRK_MAXchXYonTrackPlane1", "TRK_MAXchXYonTrackPlane2", "TRK_MAXchXYonTrackPlane3", "TRK_MAXchXYonTrackPlane4",
                                         "TRK_MINchXYonTrackPlane1", "TRK_MINchXYonTrackPlane2", "TRK_MINchXYonTrackPlane3", "TRK_MINchXYonTrackPlane4",
                                         "ACC_AntiCounter", 
                                         "TOF_OnTimeClusterL3",   "TOF_OnTimeClusterL4",
                                         "totPE_Uncorr_rich",     "charge2_rich",          "ringPMTs2_rich",
                                         "(Rinner-RinnerL1)/(Rinner+RinnerL1)", "(RinnerUH-RinnerLH)/(RinnerUH+RinnerLH)"};
    const int LineW = 3;
    vector<TString> goodBranches;
    const int nRigbins_ams02He = 69;
    double Rigbins_ams02He[nRigbins_ams02He] = {
        1.92, 2.15, 2.40, 2.67, 2.97, 3.29, 3.64, 4.02, 4.43, 4.88, 5.37, 5.90, 6.47, 7.09, 7.76,
        8.48, 9.26, 10.1, 11.0, 12.0, 13.0, 14.1, 15.3, 16.6, 18.0, 19.5, 21.1, 22.8, 24.7, 26.7,
        28.8, 31.1, 33.5, 36.1, 38.9, 41.9, 45.1, 48.5, 52.2, 56.1, 60.3, 64.8, 69.7, 74.9, 80.5, 
        86.5, 93.0, 100,  108,  116,  125,  135,  147,  160,  175,  192,  211,  233,  259,  291,
        330,  379,  441,  525,  643,  822,  1130, 1800, 3000};

    array<TString, TotFiles> PathToFile{
        inPath+"MC/He4_Positives_" + MCposDate + ".root",
        inPath+"ISS/Positives_" + ISSposDate + ".root",
    };

    array<TFile*, TotFiles> files;
    TTree* MCtree, *ISStree;
    TEventList* MCeventLists, *ISSeventLists;

    TString MCtreeName = "MC"; TString ISStreeName = "ISS";

    // Getting trees
    for (int i_file = 0; i_file < TotFiles; i_file++) {  // Loop on files
        if (gSystem->AccessPathName(PathToFile.at(i_file), kFileExists)) {
            printf("The file %s does not exist.\n", PathToFile.at(i_file).Data());
            return;
        }
        printf("The file %s exists.\n", PathToFile.at(i_file).Data());
        files.at(i_file) = new TFile(PathToFile.at(i_file), "READ");
        if (!files.at(i_file)) {
            printf("ERROR!! \n File %s not found\n", PathToFile.at(i_file).Data());
            return;
        }
        files.at(i_file)->cd();
        printf("Reading file ==> %s\n", PathToFile.at(i_file).Data());
        if(i_file==0){ // MC tree
            MCtree = (TTree*)files.at(i_file)->Get(MCtreeName.Data());
            if (!MCtree) {
                printf("%s tree not found in the file %s\n", MCtreeName.Data(), PathToFile.at(i_file).Data());
                return ;
            }
            printf("Total N.entries: %lld", MCtree->GetEntries());
            TString lisName = ">>MCEventList"+to_string(i_file);
            int MyEntries = MCtree->Draw(lisName.Data(),preSel.Data());
            lisName.ReplaceAll(">>","");
            MCeventLists = (TEventList*)gDirectory->Get(lisName.Data());
            MCtree->SetEventList(MCeventLists);
            printf(",  selected entries: %d\n\n", MyEntries);
        }
        else{ // ISS trees
            ISStree = (TTree*)files.at(i_file)->Get(ISStreeName.Data());
            if (!ISStree) {
                printf("%s tree not found in the file %s\n", MCtreeName.Data(), PathToFile.at(i_file).Data());
                return;
            }
            printf("Total N.entries: %lld", ISStree->GetEntries());
            TString lisName = ">>ISSEventList"+to_string(i_file-TotTree);
            int MyEntries = ISStree->Draw(lisName.Data(),preSel.Data());
            lisName.ReplaceAll(">>","");
            ISSeventLists = (TEventList*)gDirectory->Get(lisName.Data());
            ISStree->SetEventList(ISSeventLists);
            printf(",  selected entries: %d\n\n", MyEntries);
        }
    } // End loop on files

    //Check if MC trees have weights
    TBranch *check = MCtree->GetBranch("weight");
    if(check == nullptr) {
        printf("MC tree at file %s has no weights\n", PathToFile.at(0).Data());
        HasWeight = false;
    }
    else HasWeight = true;
    if(HasWeight) printf("\n\n--->>> The WEIGHT will be USED <<<-----\n\n\n");

    //Getting list of branches from MC and ISS
    TObjArray* MClist = MCtree->GetListOfBranches();
    int MCnbr = MCtree->GetNbranches();
    TObjArray* ISSlist = ISStree->GetListOfBranches();
    int ISSnbr = ISStree->GetNbranches();
    if(UseAllBranches){
        //Check consistency between branches
        for(int i_MC=0; i_MC<MCnbr; ++i_MC){
            TString MCname = MClist->At(i_MC)->GetName();
            for(int i_ISS=0; i_ISS<ISSnbr; ++i_ISS){
                TString ISSname = ISSlist->At(i_ISS)->GetName();
                if(MCname.CompareTo(ISSname)==0) goodBranches.push_back(MCname);
            }
        }
    }
    else for(int i=0; i<Bnames.size(); ++i) goodBranches.push_back(Bnames.at(i));    

    //Outfile
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_c);
    std::stringstream ss;
    ss << std::put_time(now_tm, "%Y_%m_%d");
    string date_string = ss.str();
    outPath = outPath+date_string+".root";
    TFile outfile(outPath.c_str(), "RECREATE");
    outfile.cd();
    outfile.mkdir("positives");
    outfile.mkdir("negatives");



    //ISS histos
    TString to_draw = "TRK_TrackResidualL2_Y>>+hL2yISS";
    TH1F *hL2yISS = new TH1F("hL2yISS",";L2 Y residual;",100,-0.02,0.05);
    ISStree->Draw(to_draw.Data(),"","goff");
    hL2yISS = (TH1F*)gDirectory->Get("hL2yISS");
    printf("ISS L2 %f\n", hL2yISS->GetEntries());

    to_draw = "TRK_TrackResidualL4_Y>>+hL4yISS";
    TH1F *hL4yISS = new TH1F("hL4yISS",";L4 Y residual;",100,-0.02,0.05);
    ISStree->Draw(to_draw.Data(),"","goff");
    hL4yISS = (TH1F*)gDirectory->Get("hL4yISS");
    printf("ISS L4 %f\n", hL4yISS->GetEntries());

    to_draw = "TRK_TrackResidualL2_Y:TRK_TrackResidualL4_Y>>+h2ISS";
    TH2F *h2ISS = new TH2F("h2ISS",";L4 Y residual;L2 Y residual",100,-0.02,0.05,100,-0.02,0.05);
    ISStree->Draw(to_draw.Data(),"","goff");
    h2ISS = (TH2F*)gDirectory->Get("h2ISS");
    printf("ISS L4:L2 %f\n", h2ISS->GetEntries());
    //MC histos
    to_draw = "TRK_TrackResidualL2_Y>>+hL2yMC";
    TH1F *hL2yMC = new TH1F("hL2yMC",";;L2 Y residual",100,-0.02,0.05);
    MCtree->Draw(to_draw.Data(),"1*weight","goff");
    hL2yMC = (TH1F*)gDirectory->Get("hL2yMC");
    printf("MC L2 %f\n", hL2yMC->GetEntries());

    to_draw = "TRK_TrackResidualL4_Y>>+hL4yMC";
    TH1F *hL4yMC = new TH1F("hL4yMC",";;L4 Y residual",100,-0.02,0.05);
    MCtree->Draw(to_draw.Data(),"1*weight","goff");
    hL4yMC = (TH1F*)gDirectory->Get("hL4yMC");
    printf("MC L4 %f\n", hL4yMC->GetEntries());

    TH2F *h2MC = new TH2F("h2MC",";L4 Y residual;L2 Y residual",100,-0.02,0.05,100,-0.02,0.05);

    float MCL2Y=0, MCL4Y=0;
    double w=0;
    MCtree->SetBranchAddress("TRK_TrackResidualL2_Y", &MCL2Y);
    MCtree->SetBranchAddress("TRK_TrackResidualL4_Y", &MCL4Y);
    MCtree->SetBranchAddress("weight", &w);

    for(int i=0; i<MCeventLists->GetN(); ++i){
        MCtree->GetEntry(MCeventLists->GetEntry(i));
        if(MCL2Y!=-2){
            MCL2Y = ((MCL2Y-hL2yMC->GetMean())/hL2yMC->GetStdDev() * hL2yISS->GetStdDev()) + hL2yISS->GetMean();
        }
        if(MCL4Y!=-2){
            MCL4Y = ((MCL4Y-hL4yMC->GetMean())/hL4yMC->GetStdDev() * hL4yISS->GetStdDev()) + hL4yISS->GetMean();
        }
        h2MC->Fill(MCL4Y,MCL2Y,w);
    }
    printf("MC L4:L2 %f\n", h2MC->GetEntries());


    h2ISS->Write();
    h2MC->Write();

    //TCanvas *cISS = MakeSquareCanvas("cISS","cISS");
    //h2ISS->Draw("colz");
    //TCanvas *cMC = MakeSquareCanvas("cMC","cMC");
    //h2MC->Draw("colz");


    //Loop on branches
    printf("Working bin-per-bin on each branches\n");
    printf("Number of branches: %zu\n", goodBranches.size());
    printf("Number of Rigidity bins: %d\n", nRigbins_ams02He);
    printf("Total number of operations: %.2f\n", TotTree*goodBranches.size()*1.*(nRigbins_ams02He/merge));
    printf("Progress: \n");
    /*
    for(int ib=0; ib<goodBranches.size(); ++ib){
        //Loop on trees (positives and negatives)
        for(int jt=0; jt<TotTree; ++jt){
            double minBin, maxBin;
            int nbins;
            if(goodBranches.at(ib).CompareTo("(Rinner-RinnerL1)/(Rinner+RinnerL1)")==0 ||
               goodBranches.at(ib).CompareTo("(RinnerUH-RinnerLH)/(RinnerUH+RinnerLH)")==0){
                minBin=-1.01;
                maxBin=1.01;
            }
            else if(goodBranches.at(ib).Contains("Residual")){
                maxBin=+0.05; minBin=-0.05;
            }
            else{
                minBin = std::min(MCtree.at(jt)->GetMinimum(goodBranches.at(ib).Data()), ISStree.at(jt)->GetMinimum(goodBranches.at(ib).Data()));
                maxBin = std::max(MCtree.at(jt)->GetMaximum(goodBranches.at(ib).Data()), ISStree.at(jt)->GetMaximum(goodBranches.at(ib).Data()));
            }
            nbins = int((maxBin-minBin)*100./(maxBin-minBin));
            
            if(goodBranches.at(ib).CompareTo("charge2_rich")==0) nbins = int(maxBin-minBin);
            //check if minBin and maxBin are NAN
            if(TMath::IsNaN(minBin) || !TMath::Finite(minBin)) minBin = int(0);
            if(TMath::IsNaN(maxBin) || !TMath::Finite(maxBin)) maxBin = int(100);
            //check if minBin and maxBin are integers:
            double dummy1, dummy2;
            if(modf(minBin, &dummy1) == 0.0 && modf(maxBin, &dummy2) == 0.0){
                if(minBin==0 && maxBin!=1) {
                    nbins = int(maxBin);
                    if(nbins<=0) {
                        printf("Name: %s, Max: %f, Min: %f\n", goodBranches.at(ib).Data(), maxBin, minBin);
                        fflush(stdout);
                    }
                    minBin = minBin-0.5;
                    maxBin = maxBin-0.5;
                }
            }
            // printf("Name: %s, Max: %f, Min: %f\n", goodBranches.at(ib).Data(), maxBin, minBin);
            if(nbins<0) continue;
            TString title = goodBranches.at(ib)+"; R (GV); "+goodBranches.at(ib);
            TH2F *h2MC = new TH2F("h2MC", title.Data(), nRigbins_ams02He - 1, Rigbins_ams02He, nbins, minBin, maxBin);
            TH2F *h2ISS = new TH2F("h2ISS", title.Data(), nRigbins_ams02He - 1, Rigbins_ams02He, nbins, minBin, maxBin);
            
            TString to_draw = goodBranches.at(ib)+":abs(Rinner)>>h2MC";
            if(UseWeight && HasWeight) MCtree.at(jt)->Draw(to_draw.Data(), "1*weight","goff");
            else  MCtree.at(jt)->Draw(to_draw.Data(), "","goff");
            h2MC = (TH2F*)gDirectory->Get("h2MC");
            
            to_draw = goodBranches.at(ib)+":abs(Rinner)>>h2ISS";
            ISStree.at(jt)->Draw(to_draw.Data(), "","goff");
            h2ISS = (TH2F*)gDirectory->Get("h2ISS");
            
            // Creating folder for this feature
            TString path;
            
            if(jt%2==0) {
                if(goodBranches.at(ib).CompareTo("(Rinner-RinnerL1)/(Rinner+RinnerL1)")==0) path = "positives/SigmaInnerL1";
                else if (goodBranches.at(ib).CompareTo("(RinnerUH-RinnerLH)/(RinnerUH+RinnerLH)")==0) path = "positives/SigmaUpLow";
                else path = "positives/"+goodBranches.at(ib);
            }
            else {
                if(goodBranches.at(ib).CompareTo("(Rinner-RinnerL1)/(Rinner+RinnerL1)")==0) path = "negatives/SigmaInnerL1";
                else if (goodBranches.at(ib).CompareTo("(RinnerUH-RinnerLH)/(RinnerUH+RinnerLH)")==0) path = "negatives/SigmaUpLow";
                else path = "negatives/"+goodBranches.at(ib);
            }

            outfile.mkdir(path.Data());
            outfile.cd(path.Data());

            //Check the Y axis range
            double maxMean = max(h2MC->GetMean(2),h2ISS->GetMean(2));
            double maxStd = max(h2MC->GetStdDev(2), h2ISS->GetStdDev(2));
            if((maxBin > maxMean*4.*maxStd) && (maxBin>8.e+3)){
                // if(maxStd>1.e2 && maxMean<10) maxBin = maxMean.*maxStd;
                // else 
                maxBin = maxMean+(2.*maxStd);
                h2MC->GetYaxis()->SetRangeUser(minBin,maxBin);
                h2ISS->GetYaxis()->SetRangeUser(minBin,maxBin);
                printf("Max mean: %f, Max StdDev: %f\n", maxMean, maxStd);
            }

            //Looping on the Y projections
            for(int ibin=1; ibin<=h2MC->GetNbinsX(); ibin=ibin+merge){
                TString MCyName = "MCbin_"+to_string(ibin);
                TH1D *MCyProj, *ISSyProj;
                if(ibin+merge-1<=h2MC->GetNbinsX()) MCyProj = h2MC->ProjectionY(MCyName.Data(),ibin,ibin+merge-1,"e");
                else MCyProj = h2MC->ProjectionY(MCyName.Data(),ibin,h2MC->GetNbinsX(),"e");
                MCyProj->SetLineWidth(LineW);
                MCyProj->SetLineColor(MCcolors.at(jt));

                TString ISSyName = "ISSbin_"+to_string(ibin);
                if(ibin+merge-1<=h2MC->GetNbinsX()) ISSyProj = h2ISS->ProjectionY(ISSyName.Data(),ibin,ibin+merge-1,"e");
                else ISSyProj = h2ISS->ProjectionY(ISSyName.Data(),ibin,h2ISS->GetNbinsX(),"e");
                ISSyProj->SetLineWidth(LineW);
                ISSyProj->SetLineColor(ISScolors.at(jt));

                //Scaling and plotting
                MCyProj->Scale(ISSyProj->Integral()/MCyProj->Integral());
                TString c_name = "c_"+TString(to_string(ibin))+"_"+goodBranches.at(ib);
                TCanvas *c = new TCanvas(c_name.Data(),c_name.Data(), 600, 600);
                c->SetLogy();

                TRatioPlot * rp = new TRatioPlot(MCyProj, ISSyProj, "divsym");
                rp->Draw();
                rp->GetUpperRefYaxis()->SetTitle(goodBranches.at(ib).Data());
                rp->GetLowerRefYaxis()->SetTitle("MC/ISS");
                rp->GetLowerRefYaxis()->SetRangeUser(0.5, 2.0);
                rp->GetUpperPad()->cd();
                TLegend *leg = new TLegend(0.3, 0.7, 0.68, 0.9);
                TString header = "R_{INNER} #in ["+to_string(Rigbins_ams02He[ibin-1])+","+to_string(Rigbins_ams02He[ibin+merge-2])+"]";
                TString MCleg, ISSleg;
                if(jt%2==0) {
                    MCleg = "MC R_{INNER}>0";
                    ISSleg = "ISS R_{INNER}>0";
                }
                else{
                    MCleg = "MC R_{INNER}<0";
                    ISSleg = "ISS R_{INNER}<0";
                }
                leg->SetHeader(header.Data());
                leg->AddEntry(MCyProj, MCleg.Data());
                leg->AddEntry(ISSyProj, ISSleg.Data());
                leg->Draw("same");
                
                // ISSyProj->Draw("e");
                // MCyProj->Draw("hist same");

                c->Write(to_string(ibin).c_str());
                delete c; delete rp; delete leg;
                if(ibin+merge-1>h2MC->GetNbinsX()) break;
            }            
            outfile.cd();
            delete h2MC;  delete h2ISS;
            if(ib%int(goodBranches.size()/10.)==0) {
                printf("%.2f\r", ib*100./goodBranches.size());
                fflush(stdout);
            }
        }
    }*/
    printf("\n");
    return;
}
