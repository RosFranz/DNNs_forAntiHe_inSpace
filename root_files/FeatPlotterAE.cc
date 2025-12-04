#include <vector>
#include <array>
#include <TString.h>
#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>
#include <TMath.h>

using namespace std;

void FeatPlotterAE(TString MCposDate="000", TString MCnegDate="000", TString ISSposDate="000", TString ISSnegDate="000") {
    gErrorIgnoreLevel = kWarning;
    const float minY = 5.e-5, maxY =1;
    const int SigFillStyle=3004, BkgFillStyle=3005;
    string outPath = "/eos/home-f/frrossi/AMS/ams_network/root_files/FeaturesPlot/AE";
    TString inPath  = "/eos/home-f/frrossi/AMS/output/organized_output/MLDs/";
    const int TotTree  = 2; // Number of trees divided by MC and ISS
    const int TotFiles = 4; // Number of files
    const bool UseWeight = true;
    const bool UseAllBranches = false;
    bool HasWeight = true;
    const int merge = 8; //number of bins to be merged on the y projection 
    const int NBranches = 29;
    TString preSel = "(beta_tof<0.96 || (beta_tof>0.96 && hasRich==1) ) ";
    TString SelMCsig, SelMCbkg;
    TString SelISSsig, SelISSbkg;
    TString to_draw, c_name, saveTo;
    array<TString,TotTree> ISSRigSig{"ISSpos","ISSneg"};
    array<TString,TotTree> MCorISS{"MC","ISS"};
    array<TString,TotTree> PosOrNeg{"R_{INNER}>0","R_{INNER}<0"};
    array<TString, TotTree> Label{"R_{INNER}>0","R_{INNER}<0"};
    printf("------->>>\n");
    printf("Currently used pre-selection: %s\n", preSel.Data());
    printf("------->>>\n\n\n");

    array<TString, NBranches> Bnames = { "(Rinner-RinnerL1)/(Rinner+RinnerL1)", "(RinnerUH-RinnerLH)/(RinnerUH+RinnerLH)",
                                         "TRK_TrackResidualL2_Y",    "TRK_TrackResidualL3_Y",   "TRK_TrackResidualL4_Y",
                                         "TRK_TrackResidualL5_Y",    "TRK_TrackResidualL6_Y",   "TRK_TrackResidualL7_Y", 
                                         "TRK_TrackResidualL8_Y",
                                         "TRK_TrackResidualL3_X",    "TRK_TrackResidualL5_X",   "TRK_TrackResidualL7_X",
                                         "TRK_TrackResidualL8_X",
                                         "TRK_NormEdep2L5_XY",       "TRK_NormEdep2L6_XY",      "TRK_NormEdep2L7_XY",
                                         "TRK_NormEdep2L8_XY",
                                         "InnerHit",            
                                         "ACC_AntiCounter", 
                                         "TOF_OnTimeClusterL3",      "TOF_OnTimeClusterL4",
                                         "TRK_MAXchXYonTrackPlane1", "TRK_MAXchXYonTrackPlane2", "TRK_MAXchXYonTrackPlane3", "TRK_MAXchXYonTrackPlane4",
                                         "TRK_MINchXYonTrackPlane1", "TRK_MINchXYonTrackPlane2", "TRK_MINchXYonTrackPlane3", "TRK_MINchXYonTrackPlane4"};
                   
    array<EColor, TotTree> MCcolors{kSpring, kRed};
    array<EColor, TotTree> ISScolors{kAzure, kMagenta};
    const int LineW = 3;
    vector<TString> goodBranches;

    array<TString, TotFiles> PathToFile{
        inPath+"MC/He4_Positive_" + MCposDate + ".root",
        inPath+"ISS/Positive_" + ISSposDate + ".root",
        inPath+"MC/He4_" + MCnegDate + ".root",
        inPath+"ISS/" + ISSnegDate + ".root"
    };

    array<TFile*, TotFiles> files;
    array<TTree*, TotTree> PosTrees, NegTrees;
    array<TEventList*, TotTree> PosEventLists, NegEventLists;

    array<TString,2> MCISStreeName{"MC","ISS"};

    // Getting trees
    // PosTrees->[MC_pos, ISS_pos]
    // Negtrees->[MC_neg, ISS_neg]
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
        if(i_file<2){ // Pos trees
            PosTrees.at(i_file) = (TTree*)files.at(i_file)->Get(MCISStreeName.at(i_file%2).Data());
            printf("%s tree found in the file %s\n", MCISStreeName.at(i_file%2).Data(), PathToFile.at(i_file).Data());
            if (!PosTrees.at(i_file)) {
                printf("%s tree not found in the file %s\n", MCISStreeName.at(i_file%2).Data(), PathToFile.at(i_file).Data());
                return ;
            }
            printf("Total N.entries: %lld", PosTrees.at(i_file)->GetEntries());
            TString lisName = ">>PosEventList"+to_string(i_file);
            int MyEntries = PosTrees.at(i_file)->Draw(lisName.Data(),preSel.Data());
            lisName.ReplaceAll(">>","");
            PosEventLists.at(i_file) = (TEventList*)gDirectory->Get(lisName.Data());
            PosTrees.at(i_file)->SetEventList(PosEventLists.at(i_file));
            printf(",  selected entries: %d\n\n", MyEntries);
        }
        else{ // Neg trees
            NegTrees.at(i_file-TotTree) = (TTree*)files.at(i_file)->Get(MCISStreeName.at(i_file%2).Data());
            printf("%s tree found in the file %s\n", MCISStreeName.at(i_file%2).Data(), PathToFile.at(i_file).Data());
            if (!NegTrees.at(i_file-TotTree)) {
                printf("%s tree not found in the file %s\n", MCISStreeName.at(i_file%2).Data(), PathToFile.at(i_file).Data());
                return;
            }
            printf("Total N.entries: %lld", NegTrees.at(i_file-TotTree)->GetEntries());
            TString lisName = ">>NegEventList"+to_string(i_file-TotTree);
            int MyEntries = NegTrees.at(i_file-TotTree)->Draw(lisName.Data(),preSel.Data());
            lisName.ReplaceAll(">>","");
            NegEventLists.at(i_file-TotTree) = (TEventList*)gDirectory->Get(lisName.Data());
            NegTrees.at(i_file-TotTree)->SetEventList(NegEventLists.at(i_file-TotTree));
            printf(",  selected entries: %d\n\n", MyEntries);
        }
    } // End loop on files

    /*
     * Be Careful this cause the script to break
    //Check if MC trees have weights 
    TBranch *checkPos = PosTrees.at(0)->GetBranch("weight");
    TBranch *checkNeg = NegTrees.at(0)->GetBranch("weight");
    if(checkPos == nullptr || checkNeg == nullptr) {
        if(checkPos == nullptr) printf("MC tree at file %s has no weights\n", PathToFile.at(0).Data());
        if(checkNeg == nullptr) printf("MC tree at file %s has no weights\n", PathToFile.at(2).Data());
        HasWeight = false;
    }
    else HasWeight = true;
    delete checkPos; delete checkNeg;
    */
    if(HasWeight) printf("\n\n--->>> The WEIGHT will be USED <<<-----\n\n\n");
     
    if(UseAllBranches){
        //Getting list of branches from MC and ISS
        TObjArray* MClist = PosTrees.at(0)->GetListOfBranches();
        int MCnbr = PosTrees.at(0)->GetNbranches();
        TObjArray* ISSlist = NegTrees.at(0)->GetListOfBranches();
        int ISSnbr = NegTrees.at(0)->GetNbranches();
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

    //Enabling only the right branches
    for(int i=0; i<TotTree; ++i){
        PosTrees.at(i)->ResetBranchAddresses();
        PosTrees.at(i)->SetBranchStatus("*",0);
        if(i==0) PosTrees.at(i)->SetBranchStatus("weight",1);
        PosTrees.at(i)->SetBranchStatus("beta_tof",1);
        PosTrees.at(i)->SetBranchStatus("hasRich",1);
        
        NegTrees.at(i)->ResetBranchAddresses();
        NegTrees.at(i)->SetBranchStatus("*",0);
        if(i==0) NegTrees.at(i)->SetBranchStatus("weight",1);
        NegTrees.at(i)->SetBranchStatus("beta_tof",1);
        NegTrees.at(i)->SetBranchStatus("hasRich",1);

        for(int ib=0; ib<goodBranches.size(); ++ib){
            printf("Branch: %s\n", goodBranches.at(ib).Data());
            if(goodBranches.at(ib).CompareTo("(Rinner-RinnerL1)/(Rinner+RinnerL1)")==0){
                PosTrees.at(i)->SetBranchStatus("Rinner",1);
                PosTrees.at(i)->SetBranchStatus("RinnerL1",1);
                NegTrees.at(i)->SetBranchStatus("Rinner",1);
                NegTrees.at(i)->SetBranchStatus("RinnerL1",1);
            }
            if(goodBranches.at(ib).CompareTo("(RinnerUH-RinnerLH)/(RinnerUH+RinnerLH)")==0){
                PosTrees.at(i)->SetBranchStatus("RinnerUH",1);
                PosTrees.at(i)->SetBranchStatus("RinnerLH",1);
                NegTrees.at(i)->SetBranchStatus("RinnerUH",1);
                NegTrees.at(i)->SetBranchStatus("RinnerLH",1);
            }
            else{
                if(i==0) printf("Branch: %s\n",goodBranches.at(ib).Data());
                PosTrees.at(i)->SetBranchStatus(goodBranches.at(ib).Data(),1);
                NegTrees.at(i)->SetBranchStatus(goodBranches.at(ib).Data(),1);
            }
        }
    }

    //Loop on branches
    printf("Number of branches: %zu\n", goodBranches.size());
    printf("Total number of operations: %.2zu\n", TotTree*goodBranches.size());
    printf("Progress: \n");
    for(int ib=0; ib<goodBranches.size(); ++ib){
        //Loop on trees (positives and negatives)
        printf("Working on branch: %s\n", goodBranches.at(ib).Data());
        for(int jt=0; jt<TotTree; ++jt){
            double minBin, maxBin;
            int nbins;
             
            minBin = std::min(PosTrees.at(jt)->GetMinimum(goodBranches.at(ib).Data()), NegTrees.at(jt)->GetMinimum(goodBranches.at(ib).Data()));
            maxBin = std::max(PosTrees.at(jt)->GetMaximum(goodBranches.at(ib).Data()), NegTrees.at(jt)->GetMaximum(goodBranches.at(ib).Data()));
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
                PosTrees.at(jt)->Draw(to_draw.Data(),"","goff");
                TH1 *hmc  = (TH1*)gDirectory->Get("hmc");
                to_draw = goodBranches.at(ib)+">>hiss";
                NegTrees.at(jt)->Draw(to_draw.Data(),"","goff");
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
            else if(goodBranches.at(ib).Contains("Rinner")){
                maxBin=1.01; minBin=-1.01;
                nbins = 25;
            }

            // printf("Name: %s, Max: %f, Min: %f\n", goodBranches.at(ib).Data(), maxBin, minBin);
            if(nbins<0) continue;

            
            printf("Current binning: %d, %f, %f\n", nbins, minBin, maxBin);    
            //Pos Rig
            TString title;
            if(jt==0) title = "; "+goodBranches.at(ib)+";Normalized weights";
            else      title = "; "+goodBranches.at(ib)+";Normalized events";
            TH1F *hPos = new TH1F("hPos", title.Data(), nbins, minBin, maxBin);
            if (jt==0) SelMCsig = "("+preSel+")*weight";
            else       SelMCsig = "("+preSel+")";
            to_draw = goodBranches.at(ib)+">>+hPos";
            if(UseWeight && HasWeight) PosTrees.at(jt)->Draw(to_draw.Data(), SelMCsig.Data(),"goff");
            else  PosTrees.at(jt)->Draw(to_draw.Data(), preSel.Data(),"goff");
            printf("Pos: to_draw-> %s selection-> %s\n", to_draw.Data(), SelMCsig.Data());
            hPos = (TH1F*)gDirectory->Get("hPos");
            //Neg Rig
            TH1F *hNeg = new TH1F("hNeg", title.Data(), nbins, minBin, maxBin);
            if(jt==0) SelMCbkg = "("+preSel+")*weight";
            else      SelMCbkg = "("+preSel+")";
            to_draw = goodBranches.at(ib)+">>+hNeg";
            if(UseWeight && HasWeight) NegTrees.at(jt)->Draw(to_draw.Data(), SelMCbkg.Data(),"goff");
            else  NegTrees.at(jt)->Draw(to_draw.Data(), preSel.Data(),"goff");
            printf("Neg: to_draw-> %s selection-> %s\n", to_draw.Data(), SelMCbkg.Data());
            hNeg = (TH1F*)gDirectory->Get("hNeg");

            c_name = "c_"+MCorISS.at(jt)+"_"+goodBranches.at(ib);
            if(goodBranches.at(ib).Contains("RinnerL1")) c_name = "c_"+MCorISS.at(jt)+"_SigmaInL1";
            if(goodBranches.at(ib).Contains("RinnerUH")) c_name = "c_"+MCorISS.at(jt)+"_SigmaIn";
            TCanvas *cPosNeg = MakeSquareCanvas(c_name.Data(), c_name.Data()); cPosNeg->SetLogy();
            if(jt==0){
                hPos->SetLineColor(MCcolors.at(0)); hPos->SetFillColor(MCcolors.at(0)-9);
                hNeg->SetLineColor(MCcolors.at(1)); hNeg->SetFillColor(MCcolors.at(1)-9);
            }
            else{
                hPos->SetLineColor(ISScolors.at(0)); hPos->SetFillColor(ISScolors.at(0)-9);
                hNeg->SetLineColor(ISScolors.at(1)); hNeg->SetFillColor(ISScolors.at(1)-9);
            }
            hPos->SetFillStyle(SigFillStyle);   hPos->SetTitle(Label.at(0).Data());
            hNeg->SetFillStyle(BkgFillStyle);   hNeg->SetTitle(Label.at(1).Data());
            TH1* hTemp = hPos->DrawNormalized("hist"); hTemp->GetYaxis()->SetRangeUser(minY, maxY);
            hNeg->DrawNormalized("hist same");
            TLegend *l = cPosNeg->BuildLegend(); l->SetBorderSize(0); l->SetFillStyle(0);
            hTemp->SetTitle(""); hTemp->GetYaxis()->CenterTitle(true); hTemp->GetXaxis()->CenterTitle(true);
            if(jt==0) DrawTag("^{4}He MC simulation",0.15,0.87,0.05);
            else      DrawTag("ISS-data",0.15,0.87,0.05);
            saveTo = outPath+"/"+c_name+".pdf";
            cPosNeg->SaveAs(saveTo.Data());
            
            delete hTemp; delete hPos; delete hNeg; delete l;
            /*
            if(ib%int(goodBranches.size()/10.)==0) {
                printf("%.2f\r", ib*100./goodBranches.size());
                fflush(stdout);
            }
            */
        }
    }
    printf("\n");
    return;
}
