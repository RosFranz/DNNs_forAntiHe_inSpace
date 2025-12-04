#include <vector>
#include <array>
#include <TString.h>
#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>
#include <TMath.h>

using namespace std;

void FeatPlotterCL2(TString MCposDate="000", TString MCnegDate="000", TString ISSposDate="000", TString ISSnegDate="000") {
    gErrorIgnoreLevel = kWarning;
    const float minY = 5.e-5, maxY =1;
    const int SigFillStyle=3004, BkgFillStyle=3005;
    string outPath = "/eos/home-f/frrossi/AMS/ams_network/root_files/FeaturesPlot/CL2";
    TString inPath  = "/eos/home-f/frrossi/AMS/output/organized_output/MLDs/";
    const int TotTree  = 2; // Number of trees divided by MC and ISS
    const int TotFiles = 4; // Number of files
    const bool UseWeight = true;
    const bool UseAllBranches = false;
    bool HasWeight = true;
    const int merge = 8; //number of bins to be merged on the y projection 
    const int NBranches = 22;
    TString preSel = "(beta_tof<0.96 || (beta_tof>0.96 && hasRich==1) ) ";
    TString SelMCsig, SelMCbkg;
    TString SelISSsig, SelISSbkg;
    TString to_draw, c_name, saveTo;
    array<TString,TotTree> ISSRigSig{"ISSpos","ISSneg"};
    array<TString,TotTree> MCRigSig{"MCpos","MCneg"};
    array<TString,TotTree> PosOrNeg{"R_{INNER}>0","R_{INNER}<0"};
    array<TString, TotTree> Label{"|#frac{R_{UH}-R_{LH}}{R_{UH}+R_{LH}}|<0.2","|#frac{R_{UH}-R_{LH}}{R_{UH}+R_{LH}}|>0.2"};
    printf("------->>>\n");
    printf("Currently used pre-selection: %s\n", preSel.Data());
    printf("------->>>\n\n\n");

    array<TString, NBranches> Bnames = { "TRK_TrackResidualL2_Y",    "TRK_TrackResidualL3_Y",               "TRK_TrackResidualL4_Y",
                                         "TRK_TrackResidualL5_Y",    "TRK_TrackResidualL6_Y",               "TRK_TrackResidualL7_Y", 
                                         "TRK_TrackResidualL3_X",    "TRK_TrackResidualL5_X",               "TRK_TrackResidualL6_X",
                                         "InnerHit",                 "(Rinner-RinnerL1)/(Rinner+RinnerL1)",
                                         "totPE_Uncorr_rich",        "charge2_rich",                        "ringPMTs2_rich",
                                         "TRK_MAXchXYonTrackPlane1", "TRK_MAXchXYonTrackPlane2", "TRK_MAXchXYonTrackPlane3", "TRK_MAXchXYonTrackPlane4",
                                         "TRK_MINchXYonTrackPlane1", "TRK_MINchXYonTrackPlane2", "TRK_MINchXYonTrackPlane3", "TRK_MINchXYonTrackPlane4"};
                   
    array<EColor, TotTree> MCcolors{kSpring, kRed};
    array<EColor, TotTree> ISScolors{kAzure, kMagenta};
    const int LineW = 3;
    vector<TString> goodBranches;

    array<TString, TotFiles> PathToFile{
        inPath+"MC/He4_Positive_" + MCposDate + ".root",
        inPath+"MC/He4_" + MCnegDate + ".root",
        inPath+"ISS/Positive_" + ISSposDate + ".root",
        inPath+"ISS/" + ISSnegDate + ".root"
    };

    array<TFile*, TotFiles> files;
    array<TTree*, TotTree> MCtrees, ISStrees;
    array<TEventList*, TotTree> MCeventLists, ISSeventLists;

    TString MCtreeName = "MC"; TString ISStreeName = "ISS";

    // Getting trees
    // MCtrees->[MC_pos, MC_neg]
    // ISStree->[ISS_pos, ISS_neg]
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
        if(i_file<2){ // MC trees
            MCtrees.at(i_file) = (TTree*)files.at(i_file)->Get(MCtreeName.Data());
            if (!MCtrees.at(i_file)) {
                printf("%s tree not found in the file %s\n", MCtreeName.Data(), PathToFile.at(i_file).Data());
                return ;
            }
            printf("Total N.entries: %lld", MCtrees.at(i_file)->GetEntries());
            TString lisName = ">>MCEventList"+to_string(i_file);
            int MyEntries = MCtrees.at(i_file)->Draw(lisName.Data(),preSel.Data());
            lisName.ReplaceAll(">>","");
            MCeventLists.at(i_file) = (TEventList*)gDirectory->Get(lisName.Data());
            MCtrees.at(i_file)->SetEventList(MCeventLists.at(i_file));
            printf(",  selected entries: %d\n\n", MyEntries);
        }
        else{ // ISS trees
            ISStrees.at(i_file-TotTree) = (TTree*)files.at(i_file)->Get(ISStreeName.Data());
            if (!ISStrees.at(i_file-TotTree)) {
                printf("%s tree not found in the file %s\n", MCtreeName.Data(), PathToFile.at(i_file).Data());
                return;
            }
            printf("Total N.entries: %lld", ISStrees.at(i_file-TotTree)->GetEntries());
            TString lisName = ">>ISSEventList"+to_string(i_file-TotTree);
            int MyEntries = ISStrees.at(i_file-TotTree)->Draw(lisName.Data(),preSel.Data());
            lisName.ReplaceAll(">>","");
            ISSeventLists.at(i_file-TotTree) = (TEventList*)gDirectory->Get(lisName.Data());
            ISStrees.at(i_file-TotTree)->SetEventList(ISSeventLists.at(i_file-TotTree));
            printf(",  selected entries: %d\n\n", MyEntries);
        }
    } // End loop on files

    //Check if MC trees have weights
    for(int i=0; i<MCtrees.size(); ++i){
        TBranch *check = MCtrees.at(i)->GetBranch("weight");
        if(check == nullptr) {
            printf("MC tree at file %s has no weights\n", PathToFile.at(i).Data());
            HasWeight = false;
            break;
        }
        else HasWeight = true;
    }

    if(HasWeight) printf("\n\n--->>> The WEIGHT will be USED <<<-----\n\n\n");

    //Getting list of branches from MC and ISS
    TObjArray* MClist = MCtrees.at(0)->GetListOfBranches();
    int MCnbr = MCtrees.at(0)->GetNbranches();
    TObjArray* ISSlist = ISStrees.at(0)->GetListOfBranches();
    int ISSnbr = ISStrees.at(0)->GetNbranches();
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

    for(int i=0; i<TotTree; ++i){
        MCtrees.at(i)->SetBranchStatus("*",0);
        MCtrees.at(i)->SetBranchStatus("weight",1);
        MCtrees.at(i)->SetBranchStatus("beta_tof",1);
        MCtrees.at(i)->SetBranchStatus("hasRich",1);
        MCtrees.at(i)->SetBranchStatus("RinnerUH",1);
        MCtrees.at(i)->SetBranchStatus("RinnerLH",1);
        
        ISStrees.at(i)->SetBranchStatus("*",0);
        ISStrees.at(i)->SetBranchStatus("beta_tof",1);
        ISStrees.at(i)->SetBranchStatus("hasRich",1);
        ISStrees.at(i)->SetBranchStatus("RinnerUH",1);
        ISStrees.at(i)->SetBranchStatus("RinnerLH",1);

        for(int ib=0; ib<goodBranches.size(); ++ib){
            if(goodBranches.at(ib).CompareTo("(Rinner-RinnerL1)/(Rinner+RinnerL1)")==0){
                MCtrees.at(i)->SetBranchStatus("Rinner",1);
                MCtrees.at(i)->SetBranchStatus("RinnerL1",1);
                ISStrees.at(i)->SetBranchStatus("Rinner",1);
                ISStrees.at(i)->SetBranchStatus("RinnerL1",1);
            }
            else{
                if(i==0) printf("Branch: %s\n",goodBranches.at(ib).Data());
                MCtrees.at(i)->SetBranchStatus(goodBranches.at(ib).Data(),1);
                ISStrees.at(i)->SetBranchStatus(goodBranches.at(ib).Data(),1);
            }
        }
    }

    //Outfile
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_c);
    std::stringstream ss;
    ss << std::put_time(now_tm, "%Y_%m_%d");
    string date_string = ss.str();

    //Loop on branches
    printf("Number of branches: %zu\n", goodBranches.size());
    printf("Total number of operations: %.2zu\n", TotTree*goodBranches.size());
    printf("Progress: \n");
    for(int ib=0; ib<goodBranches.size(); ++ib){
        //Loop on trees (positives and negatives)
        for(int jt=0; jt<TotTree; ++jt){
            double minBin, maxBin;
            int nbins;
             
            minBin = std::min(MCtrees.at(jt)->GetMinimum(goodBranches.at(ib).Data()), ISStrees.at(jt)->GetMinimum(goodBranches.at(ib).Data()));
            maxBin = std::max(MCtrees.at(jt)->GetMaximum(goodBranches.at(ib).Data()), ISStrees.at(jt)->GetMaximum(goodBranches.at(ib).Data()));
            nbins = int((maxBin-minBin)*25./(maxBin-minBin));

            
            if(goodBranches.at(ib).CompareTo("charge2_rich")==0) nbins = int(maxBin-minBin);
            //check if minBin and maxBin are NAN
            if(TMath::IsNaN(minBin) || !TMath::Finite(minBin)) minBin = int(0);
            if(TMath::IsNaN(maxBin) || !TMath::Finite(maxBin)) maxBin = int(100);
            //check if minBin and maxBin are integers:
            double dummy1, dummy2;
            if(modf(minBin, &dummy1) == 0.0 && modf(maxBin, &dummy2) == 0.0){
                if(minBin==0 && maxBin!=1) {
                    nbins = int(maxBin-minBin);
                    if(nbins<=0) {
                        printf("Name: %s, Max: %f, Min: %f\n", goodBranches.at(ib).Data(), maxBin, minBin);
                        fflush(stdout);
                    }
                    minBin = minBin-0.5;
                    maxBin = maxBin-0.5;
                }
            }
            else{
                to_draw = goodBranches.at(ib)+">>hmc";
                MCtrees.at(jt)->Draw(to_draw.Data(),"","goff");
                TH1 *hmc  = (TH1*)gDirectory->Get("hmc");
                to_draw = goodBranches.at(ib)+">>hiss";
                ISStrees.at(jt)->Draw(to_draw.Data(),"","goff");
                TH1 *hiss = (TH1*)gDirectory->Get("hiss");
                double MyMax = std::max((hmc->GetMean()+(5*hmc->GetStdDev())), (hiss->GetMean()+(5*hiss->GetStdDev())));
                double MyMin = std::min((hmc->GetMean()-(5*hmc->GetStdDev())), (hiss->GetMean()-(5*hiss->GetStdDev())));
                if(maxBin > MyMax) maxBin = MyMax;           
                if(minBin<0){
                    if(minBin>MyMin) minBin = MyMin;
                }
                else{
                    if(minBin<MyMin) minBin = MyMin;
                }
                nbins = int((maxBin-minBin)*25./(maxBin-minBin));
                delete hmc; delete hiss;
            }
            if(goodBranches.at(ib).Contains("Residual")){
                maxBin=+0.05; minBin=-0.05;
                nbins = int((maxBin-minBin)*25./(maxBin-minBin));
            }
            else if(goodBranches.at(ib).Contains("RinnerL1")){
                maxBin=1.01; minBin=-1.01;
                nbins = int((maxBin-minBin)*25./(maxBin-minBin));
            }

            // printf("Name: %s, Max: %f, Min: %f\n", goodBranches.at(ib).Data(), maxBin, minBin);
            if(nbins<0) continue;

            
            
            //MC sig
            TString title = "; "+goodBranches.at(ib)+";Normalized weights";
            TH1F *hMCsig = new TH1F("hMCsig", title.Data(), nbins, minBin, maxBin);
            SelMCsig = "("+preSel+"&& abs((RinnerUH-RinnerLH)/(RinnerUH+RinnerLH))<=0.2)*weight";
            to_draw = goodBranches.at(ib)+">>+hMCsig";
            if(UseWeight && HasWeight) MCtrees.at(jt)->Draw(to_draw.Data(), SelMCsig.Data(),"goff");
            else  MCtrees.at(jt)->Draw(to_draw.Data(), preSel.Data(),"goff");
            hMCsig = (TH1F*)gDirectory->Get("hMCsig");
            //MC bkg
            TH1F *hMCbkg = new TH1F("hMCbkg", title.Data(), nbins, minBin, maxBin);
            SelMCbkg = "("+preSel+"&& abs((RinnerUH-RinnerLH)/(RinnerUH+RinnerLH))>0.2)*weight";
            to_draw = goodBranches.at(ib)+">>+hMCbkg";
            if(UseWeight && HasWeight) MCtrees.at(jt)->Draw(to_draw.Data(), SelMCbkg.Data(),"goff");
            else  MCtrees.at(jt)->Draw(to_draw.Data(), preSel.Data(),"goff");
            hMCbkg = (TH1F*)gDirectory->Get("hMCbkg");

            c_name = "c_"+MCRigSig.at(jt)+"_"+goodBranches.at(ib);
            if(goodBranches.at(ib).Contains("RinnerL1")) c_name = "c_"+MCRigSig.at(jt)+"_SigmaInL1";
            TCanvas *cMC = MakeSquareCanvas(c_name.Data(), c_name.Data()); cMC->SetLogy();
            hMCsig->SetLineColor(MCcolors.at(0)); hMCsig->SetFillColor(MCcolors.at(0)-9);
            hMCsig->SetFillStyle(SigFillStyle);   hMCsig->SetTitle(Label.at(0).Data());
            hMCbkg->SetLineColor(MCcolors.at(1)); hMCbkg->SetFillColor(MCcolors.at(1)-9);
            hMCbkg->SetFillStyle(BkgFillStyle);   hMCbkg->SetTitle(Label.at(1).Data());
            TH1* hTemp = hMCsig->DrawNormalized("hist"); hTemp->GetYaxis()->SetRangeUser(minY, maxY);
            hMCbkg->DrawNormalized("hist same");
            TLegend *l = cMC->BuildLegend(); l->SetBorderSize(0); l->SetFillStyle(0);
            hTemp->SetTitle(""); hTemp->GetYaxis()->CenterTitle(true); hTemp->GetXaxis()->CenterTitle(true);
            DrawTag("^{4}He MC simulation",0.15,0.87,0.05);
            DrawTag(PosOrNeg.at(jt).Data(),0.7,0.87,0.04,52);
            saveTo = outPath+"/"+c_name+".pdf";
            cMC->SaveAs(saveTo.Data());
            
            //ISS sig
            title = "; "+goodBranches.at(ib)+";Normalized events";
            TH1F *hISSsig = new TH1F("hISSsig", title.Data(), nbins, minBin, maxBin);
            to_draw = goodBranches.at(ib)+">>+hISSsig";
            SelISSsig = "("+preSel+"&& abs((RinnerUH-RinnerLH)/(RinnerUH+RinnerLH))<=0.2)";
            ISStrees.at(jt)->Draw(to_draw.Data(), SelISSsig.Data(),"goff");
            hISSsig = (TH1F*)gDirectory->Get("hISSsig");
            //ISS bkg
            TH1F *hISSbkg = new TH1F("hISSbkg", title.Data(), nbins, minBin, maxBin);
            SelISSbkg = "("+preSel+"&& abs((RinnerUH-RinnerLH)/(RinnerUH+RinnerLH))>0.2)";
            to_draw = goodBranches.at(ib)+">>+hISSbkg";
            ISStrees.at(jt)->Draw(to_draw.Data(), SelISSbkg.Data(),"goff");
            hISSbkg = (TH1F*)gDirectory->Get("hISSbkg");
            
            c_name = "c_"+ISSRigSig.at(jt)+"_"+goodBranches.at(ib);
            if(goodBranches.at(ib).Contains("RinnerL1")) c_name = "c_"+ISSRigSig.at(jt)+"_SigmaInL1";
            TCanvas *cISS = MakeSquareCanvas(c_name.Data(), c_name.Data()); cISS->SetLogy();
            hISSsig->SetLineColor(ISScolors.at(0)); hISSsig->SetFillColor(ISScolors.at(0)-9);
            hISSsig->SetFillStyle(SigFillStyle);    hISSsig->SetTitle(Label.at(0).Data());
            hISSbkg->SetLineColor(ISScolors.at(1)); hISSbkg->SetFillColor(ISScolors.at(1)-9);
            hISSbkg->SetFillStyle(BkgFillStyle);    hISSbkg->SetTitle(Label.at(1).Data());
            hTemp = hISSsig->DrawNormalized("hist"); hTemp->GetYaxis()->SetRangeUser(minY, maxY);
            hISSbkg->DrawNormalized("hist same");
            l = cISS->BuildLegend(); l->SetBorderSize(0); l->SetFillStyle(0);
            hTemp->SetTitle(""); hTemp->GetYaxis()->CenterTitle(true); hTemp->GetXaxis()->CenterTitle(true);
            DrawTag("ISS-data",0.15,0.87,0.05);
            DrawTag(PosOrNeg.at(jt).Data(),0.7,0.87,0.04,52);
            saveTo = outPath+"/"+c_name+".pdf";
            cISS->SaveAs(saveTo.Data());

            delete hTemp;  delete hISSsig; delete hISSbkg; delete hMCsig; delete hMCbkg; delete l;
            if(ib%int(goodBranches.size()/10.)==0) {
                printf("%.2f\r", ib*100./goodBranches.size());
                fflush(stdout);
            }
        }
    }
    printf("\n");
    return;
}
