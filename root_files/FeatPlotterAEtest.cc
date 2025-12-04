#include <vector>
#include <array>
#include <TString.h>
#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>
#include <TMath.h>
#include <TEventList.h>
#include <TRandom3.h>
#include <algorithm>

/*
Plots normalised features like the one scaled by the custom scaler
*/

using namespace std;


const int NBranches = 30;
std::pair<TEventList*, TEventList*> SplitEventList(TEventList *elist, double fracTrain = 0.7, int seed = 1234);
void ScaleSignalBranches(TTree* tree, array<TString,NBranches> &branches, array<TString,NBranches> &MySel,
                         array<double,NBranches> &mean, array<double,NBranches> &std,
                         array<double,NBranches> &minB, array<double,NBranches> &maxB,
                         array<int,NBranches> &myBins,
                         TString fileName="", bool isMC=false, bool isPosRig=false,
                         TEventList* evList=nullptr);
void RetrieveTh1(TString &fileName, array<TH1F*,NBranches> &myTH);

void FeatPlotterAEtest(TString MCposDate="000", TString MCnegDate="000", TString ISSposDate="000", TString ISSnegDate="000") {
    gErrorIgnoreLevel = kWarning;
    const float minY = 5.e-7, maxY =1;
    const int SigFillStyle=3004, BkgFillStyle=3005;
    TString outPath = "/eos/home-f/frrossi/AMS/ams_network/root_files/FeaturesPlot/AE";
    TString inPath  = "/eos/home-f/frrossi/AMS/output/organized_output/MLDs/";
    const int TotTree  = 2; // Number of trees divided by MC and ISS
    const int TotFiles = 4; // Number of files
    TString preSel = "(beta_tof<0.96 || (beta_tof>0.96 && hasRich==1) ) && abs(Rinner)<200";
    TString CombSel, SelNeg;
    TString c_name, saveTo;
    array<TString,TotFiles> MCorISS{"MC","MC","ISS","ISS"};
    array<TString, TotTree> Label{"R_{INNER}>0","R_{INNER}<0"};
    array<TString, TotFiles> Tags{"^{4}He MC simulation","^{4}He MC simulation","ISS-data","ISS-data"};

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
                                         "InnerHitX",                "InnerHitY",           
                                         "ACC_AntiCounter", 
                                         "TOF_OnTimeClusterL3",      "TOF_OnTimeClusterL4",
                                         "TRK_MAXchXYonTrackPlane1", "TRK_MAXchXYonTrackPlane2", "TRK_MAXchXYonTrackPlane3", "TRK_MAXchXYonTrackPlane4",
                                         "TRK_MINchXYonTrackPlane1", "TRK_MINchXYonTrackPlane2", "TRK_MINchXYonTrackPlane3", "TRK_MINchXYonTrackPlane4"};
    array<TString, NBranches> CustomSel = { "RinnerL1!=0 && Rinner/RinnerL1>0", "",
                                            "TRK_TrackResidualL2_Y>-2",    "TRK_TrackResidualL3_Y>-2",   "TRK_TrackResidualL4_Y>-2",
                                            "TRK_TrackResidualL5_Y>-2",    "TRK_TrackResidualL6_Y>-2",   "TRK_TrackResidualL7_Y>-2", 
                                            "TRK_TrackResidualL8_Y>-2",
                                            "TRK_TrackResidualL3_X>-2",    "TRK_TrackResidualL5_X>-2",   "TRK_TrackResidualL7_X>-2",
                                            "TRK_TrackResidualL8_X>-2",
                                            "TRK_NormEdep2L5_XY>-2",       "TRK_NormEdep2L6_XY>-2",      "TRK_NormEdep2L7_XY>-2",
                                            "TRK_NormEdep2L8_XY>-2",
                                            "", "",           
                                            "", 
                                            "", "",
                                            "TRK_MAXchXYonTrackPlane1>0",   "TRK_MAXchXYonTrackPlane2>0",   "TRK_MAXchXYonTrackPlane3>0",   "TRK_MAXchXYonTrackPlane4>0",
                                            "TRK_MINchXYonTrackPlane1<100", "TRK_MINchXYonTrackPlane2<100", "TRK_MINchXYonTrackPlane3<100", "TRK_MINchXYonTrackPlane4<100"};
                   
    array<double, NBranches> MYmean,   MYstd;
    array<double, NBranches> MYmaxBin, MYminBin;
    array<int, NBranches>    MYbins;
    array<TString, TotFiles> RootNames{"MCpos.root", "MCneg.root","ISSpos.root","ISSneg.root"};
    
    array<EColor, TotTree> MCcolors{kSpring, kRed};
    array<EColor, TotTree> ISScolors{kAzure, kMagenta};

    array<TString, TotFiles> PathToFile{
        inPath+"MC/He4_Positive_" + MCposDate + ".root",
        inPath+"MC/He4_" + MCnegDate + ".root",
        inPath+"ISS/Positive_" + ISSposDate + ".root",
        inPath+"ISS/" + ISSnegDate + ".root"
    };
    array<TString, TotFiles> TreeName{"MC","MC", "ISS", "ISS"};


    // Getting trees
    bool AmImc=false;
    for (int i_file = 0; i_file < TotFiles; i_file++) {  // Loop on files
        if(i_file<2) AmImc=true;
        else AmImc = false;
        TFile* myFile;
        TTree* myTree;
        TEventList *myList;

        if (gSystem->AccessPathName(PathToFile.at(i_file), kFileExists)) {
            printf("The file %s does not exist.\n", PathToFile.at(i_file).Data());
            return;
        }
        printf("The file %s exists.\n", PathToFile.at(i_file).Data());
        myFile = new TFile(PathToFile.at(i_file), "READ");
        if (!myFile) {
            printf("ERROR!! \n File %s not found\n", PathToFile.at(i_file).Data());
            return;
        }
        myFile->cd();
        printf("Reading file ==> %s\n", PathToFile.at(i_file).Data());

        myTree = (TTree*)myFile->Get(TreeName.at(i_file).Data());
        printf("%s tree found in the file %s\n", TreeName.at(i_file).Data(), PathToFile.at(i_file).Data());
        if (!myTree) {
            printf("%s tree not found in the file %s\n", TreeName.at(i_file).Data(), PathToFile.at(i_file).Data());
            return ;
        }
        printf("Total N.entries: %lld", myTree->GetEntries());

        TString lisName = ">>MyEventList"+to_string(i_file);
        int MyEntries = myTree->Draw(lisName.Data(),preSel.Data(),"",10000000);
        lisName.ReplaceAll(">>","");
        myList = (TEventList*)gDirectory->Get(lisName.Data());
        myTree->SetEventList(myList);
        printf(",  selected entries: %d\n\n", MyEntries);

        if(i_file%2==0) ScaleSignalBranches(myTree, Bnames, CustomSel,
                            MYmean, MYstd,
                            MYminBin, MYmaxBin,
                            MYbins,
                            RootNames.at(i_file), AmImc, true,
                            myList);
        else            ScaleSignalBranches(myTree, Bnames, CustomSel,
                            MYmean, MYstd,
                            MYminBin, MYmaxBin,
                            MYbins,
                            RootNames.at(i_file), AmImc, false);
        delete myList;
        delete myTree;
        delete myFile;

    } // End loop on files



    array<TH1F*, NBranches> MyhPos, MyhNeg;
    for(int i=0; i<TotFiles; i+=2){
        printf("Working on files %s %s ...\n", RootNames.at(i).Data(), RootNames.at(i+1).Data());
        RetrieveTh1(RootNames.at(i), MyhPos);
        RetrieveTh1(RootNames.at(i+1), MyhNeg);

        for(int ib=0; ib<NBranches; ++ib){
            TString c_name = "c_"+MCorISS.at(i)+"_"+Bnames.at(ib);

            if(Bnames.at(ib).Contains("RinnerL1")) c_name = "c_"+MCorISS.at(i)+"_SigmaInL1";
            if(Bnames.at(ib).Contains("RinnerUH")) c_name = "c_"+MCorISS.at(i)+"_SigmaIn";

            TCanvas *canvas = MakeSquareCanvas(c_name.Data(), c_name.Data()); canvas->SetLogy();
            if(i>2) {
                MyhPos.at(ib)->SetLineColor(ISScolors.at(0)); MyhPos.at(ib)->SetFillColor(ISScolors.at(0)-9);
                MyhNeg.at(ib)->SetLineColor(ISScolors.at(1)); MyhNeg.at(ib)->SetFillColor(ISScolors.at(1)-9);
            }
            else{
                MyhPos.at(ib)->SetLineColor(MCcolors.at(0));  MyhPos.at(ib)->SetFillColor(MCcolors.at(0)-9);
                MyhNeg.at(ib)->SetLineColor(MCcolors.at(1));  MyhNeg.at(ib)->SetFillColor(MCcolors.at(1)-9);
            }
            MyhPos.at(ib)->SetFillStyle(SigFillStyle);    MyhPos.at(ib)->SetTitle(Label.at(0).Data());
            MyhNeg.at(ib)->SetFillStyle(BkgFillStyle);    MyhNeg.at(ib)->SetTitle(Label.at(1).Data());
            TH1* hTemp = MyhPos.at(ib)->DrawNormalized("hist"); hTemp->GetYaxis()->SetRangeUser(minY, maxY);
            MyhNeg.at(ib)->DrawNormalized("hist same");
            TLegend *l = canvas->BuildLegend(); l->SetBorderSize(0); l->SetFillStyle(0);
            hTemp->SetTitle(""); hTemp->GetYaxis()->CenterTitle(true); hTemp->GetXaxis()->CenterTitle(true);
            DrawTag(Tags.at(i).Data(),0.15,0.87,0.05);
            saveTo = outPath+"/"+c_name+".pdf";
            canvas->SaveAs(saveTo.Data());
            delete hTemp;
            delete l;
        }
    }
    return;
    printf("\n");
    return;
}








// Function to split a TEventList into 70% train, 30% test
std::pair<TEventList*, TEventList*> SplitEventList(TEventList *elist, double fracTrain, int seed) {
    if (!elist) {
        Error("SplitEventList", "Input TEventList is null!");
        return {nullptr, nullptr};
    }

    Long64_t nentries = elist->GetN();
    Long64_t ntrain   = static_cast<Long64_t>(fracTrain * nentries);

    // Create output lists
    TEventList *elist_train = new TEventList("elist_train", "Training subset");
    TEventList *elist_test  = new TEventList("elist_test",  "Test subset");

    // Fill vector with indices
    std::vector<Long64_t> indices;
    indices.reserve(nentries);
    for (Long64_t i = 0; i < nentries; ++i)
        indices.push_back(elist->GetEntry(i));

    // Shuffle indices (to randomize the split)
    std::mt19937 gen(seed);
    std::shuffle(indices.begin(), indices.end(),gen);

    // Split into two lists
    for (Long64_t i = 0; i < nentries; ++i) {
        if (i < ntrain) elist_train->Enter(indices[i]);
        else            elist_test->Enter(indices[i]);
    }

    return {elist_train, elist_test};
}







void ScaleSignalBranches(TTree* tree, array<TString,NBranches> &branches, array<TString,NBranches> &MySel,
                         array<double,NBranches> &mean, array<double,NBranches> &std,
                         array<double,NBranches> &minB, array<double,NBranches> &maxB,
                         array<int,NBranches> &myBins,
                         TString fileName, bool isMC, bool isPosRig,
                         TEventList* evList){

    //Selection
    TString CombSel;

    if (isMC) printf("\n\n--->>> The WEIGHT will be USED <<<-----\n\n\n");

    //Enabling the right branches
    tree->SetBranchStatus("*",0);
    if(isMC) tree->SetBranchStatus("weight",1);
    tree->SetBranchStatus("beta_tof",1);
    tree->SetBranchStatus("hasRich",1);
    tree->SetBranchStatus("RinnerUH",1);
    tree->SetBranchStatus("RinnerLH",1);
    
    for(int ib=0; ib<branches.size(); ++ib){
        if(branches.at(ib).CompareTo("(Rinner-RinnerL1)/(Rinner+RinnerL1)")==0){
            tree->SetBranchStatus("Rinner",1);
            tree->SetBranchStatus("RinnerL1",1);
        }
        else if(branches.at(ib).CompareTo("(RinnerUH-RinnerLH)/(RinnerUH+RinnerLH)")==0){
            tree->SetBranchStatus("RinnerUH",1);
            tree->SetBranchStatus("RinnerLH",1);
        }
        else{
            tree->SetBranchStatus(branches.at(ib).Data(),1);
        }
    }

    printf("Opening File with name %s \n", fileName.Data());
    TFile *fTemp = new TFile(fileName.Data(),"RECREATE");
    

    //Splitting training and validation TEventList 
    pair<TEventList*,TEventList*> TrainTestList;
    if(isPosRig) TrainTestList = SplitEventList(evList);  

    //Loop on branches
    for(int ib=0; ib<branches.size(); ++ib){

        TString title, ThName, tempB;
        TString to_draw;

        //Selection defintion
        if(MySel.at(ib).Length()!=0){
            if (isMC) CombSel = "("+MySel.at(ib)+")*weight";
            else      CombSel = "("+MySel.at(ib)+")";
        }
        else{
            if(isMC) CombSel = "(1)*weight";
            else     CombSel = "";
        }
            
        //Title
        if(isMC) title = "; "+branches.at(ib)+";Normalized weights";
        else     title = "; "+branches.at(ib)+";Normalized events";

        if(isPosRig){
            //Initiali binning
            minB.at(ib) = tree->GetMinimum(branches.at(ib).Data());
            maxB.at(ib) = tree->GetMaximum(branches.at(ib).Data());
            myBins.at(ib) = int(25);

            //Training list
            tree->SetEventList(TrainTestList.first);

            TH1F *hs = new TH1F("hs", title.Data(), myBins.at(ib), minB.at(ib), maxB.at(ib));
            to_draw = branches.at(ib)+">>+hs";
            tree->Draw(to_draw.Data(), CombSel.Data(),"goff");
            hs  = (TH1F*)gDirectory->Get("hs");
            mean.at(ib) = hs->GetMean();
            std.at(ib)  = hs->GetStdDev();
            delete hs;

            //Evaluate range of scaled branch on positive from 70% to 100%
            tree->SetEventList(TrainTestList.second);
            to_draw = "("+branches.at(ib)+"-"+TString(to_string(mean.at(ib)))+")/"+TString(to_string(std.at(ib)))+">>hst";
            tree->Draw(to_draw.Data(),CombSel.Data(),"goff");
            TH1* hst  = (TH1*)gDirectory->Get("hst");
            minB.at(ib) = hst->GetBinLowEdge(1); maxB.at(ib) = hst->GetBinLowEdge(hst->GetNbinsX()+1); 
            delete hst;


            if(branches.at(ib).CompareTo("charge2_rich")==0) myBins.at(ib) = int(maxB.at(ib)-minB.at(ib));
            //check if minB.at(ib) and maxB.at(ib) are NAN
            if(TMath::IsNaN(minB.at(ib)) || !TMath::Finite(minB.at(ib))) minB.at(ib) = int(0);
            if(TMath::IsNaN(maxB.at(ib)) || !TMath::Finite(maxB.at(ib))) maxB.at(ib) = int(100);
            //check if minB.at(ib) and maxB.at(ib) are integers:
            double dummy1, dummy2;
            if(modf(minB.at(ib), &dummy1) == 0.0 && modf(maxB.at(ib), &dummy2) == 0.0){
                if(minB.at(ib)==0 && maxB.at(ib)!=1) {
                    myBins.at(ib) = int(maxB.at(ib)-minB.at(ib));
                    if(myBins.at(ib)<=0) {
                        printf("Name: %s, Max: %f, Min: %f\n", branches.at(ib).Data(), maxB.at(ib), minB.at(ib));
                        fflush(stdout);
                    }
                    minB.at(ib) = minB.at(ib)-0.5;
                    maxB.at(ib) = maxB.at(ib)-0.5;
                }
            }
        }
        
        printf("Current binning: %d, %f, %f\n", myBins.at(ib), minB.at(ib), maxB.at(ib));  
        if(myBins.at(ib)<0) continue;
        
        //Retrieve the scaled histo Signal and Background
        //sig
        TH1F *myh = new TH1F("myh", title.Data(), myBins.at(ib), minB.at(ib), maxB.at(ib));
        to_draw = "("+branches.at(ib)+"-"+TString(to_string(mean.at(ib)))+")/"+TString(to_string(std.at(ib)))+">>+myh";
        tree->Draw(to_draw.Data(), CombSel.Data(),"goff");
        myh = (TH1F*)gDirectory->Get("myh"); 
        if(branches.at(ib).Contains("RinnerL1"))    tempB = "SigmaInL1";
        else if(branches.at(ib).Contains("RinnerUH")) tempB = "SigmaIn";
        else tempB = branches.at(ib);
        if(isMC){
            if(isPosRig) ThName = tempB+"_MCpos";
            else         ThName = tempB+"_MCneg";
        }
        else{
            if(isPosRig) ThName = tempB+"_ISSpos";
            else         ThName = tempB+"_ISSneg";
        }
        myh->SetName(ThName.Data());
        myh->Write(ThName.Data());
        delete myh; 
    }
    fTemp->Close();
    return;
}

void RetrieveTh1(TString &fileName, array<TH1F*,NBranches> &myTH){
    TFile *f = new TFile(fileName.Data(),"OPEN");
    TList *list = f->GetListOfKeys();
    int isig=0;
    for (auto k: *list) {
        TKey *key = static_cast<TKey*>(k);
        TClass *cl = gROOT->GetClass(key->GetClassName());

        if (cl->InheritsFrom(TH1F::Class())) {
            myTH.at(isig) = key->ReadObject<TH1F>();
            isig++;
        }
    }
    //f->Close();
    return;
}


