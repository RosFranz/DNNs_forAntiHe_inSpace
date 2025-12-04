#include <vector>
#include <array>
#include <TString.h>
#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>
#include <TMath.h>

/*
Plots normalised features like the one scaled by the custom scaler
*/

using namespace std;
//Common constants
const int NBranches = 23;

std::pair<TEventList*, TEventList*> SplitEventList(TEventList *elist, double fracTrain = 0.7, int seed = 1234);
void ScaleSignalBranches(TTree* tree, array<TString,NBranches> &branches, array<TString,NBranches> &MySel,
                         array<double,NBranches> &mean, array<double,NBranches> &std,
                         array<double,NBranches> &minB, array<double,NBranches> &maxB,
                         array<int,NBranches> &myBins,
                         TString fileName="", bool isMC=false, bool isPosRig=false,
                         TEventList* evList=nullptr);
void RetrieveTh1(TString &fileName, array<TH1F*,NBranches> &myTHsig, array<TH1F*,NBranches> &myTHbkg);


void FeatPlotterCL2test(TString MCposDate="000", TString MCnegDate="000", TString ISSposDate="000", TString ISSnegDate="000") {
    gErrorIgnoreLevel = kWarning;
    const float minY = 5.e-7, maxY =1;
    const int SigFillStyle=3004, BkgFillStyle=3005;
    TString outPath = "/eos/home-f/frrossi/AMS/ams_network/root_files/FeaturesPlot/CL2";
    TString inPath  = "/eos/home-f/frrossi/AMS/output/organized_output/MLDs/";
    const int TotTree  = 2; // Number of trees divided by MC and ISS
    const int TotFiles = 4; // Number of files
    TString c_name, saveTo;
    array<TString,TotFiles> RigSig{"MCpos","MCneg","ISSpos","ISSneg"};
    array<TString,TotFiles> PosOrNeg{"R_{INNER}>0","R_{INNER}<0", "R_{INNER}>0","R_{INNER}<0"};
    array<TString, TotTree> Label{"|#frac{R_{UH}-R_{LH}}{R_{UH}+R_{LH}}|<0.2","|#frac{R_{UH}-R_{LH}}{R_{UH}+R_{LH}}|>0.2"};
    array<TString, TotFiles> Tags{"^{4}He MC simulation","^{4}He MC simulation","ISS-data","ISS-data"};

    TString preSel = "(beta_tof<0.96 || (beta_tof>0.96 && hasRich==1) ) ";
    printf("------->>>\n");
    printf("Currently used pre-selection: %s\n", preSel.Data());
    printf("------->>>\n\n\n");

    array<TString, NBranches> Bnames = { "TRK_TrackResidualL2_Y",    "TRK_TrackResidualL3_Y",               "TRK_TrackResidualL4_Y",
                                         "TRK_TrackResidualL5_Y",    "TRK_TrackResidualL6_Y",               "TRK_TrackResidualL7_Y", 
                                         "TRK_TrackResidualL3_X",    "TRK_TrackResidualL5_X",               "TRK_TrackResidualL6_X",
                                         "InnerHitX",                "InnerHitY",                 
                                         "(Rinner-RinnerL1)/(Rinner+RinnerL1)",
                                         "totPE_Uncorr_rich",        "charge2_rich",                        "ringPMTs2_rich",
                                         "TRK_MAXchXYonTrackPlane1", "TRK_MAXchXYonTrackPlane2", "TRK_MAXchXYonTrackPlane3", "TRK_MAXchXYonTrackPlane4",
                                         "TRK_MINchXYonTrackPlane1", "TRK_MINchXYonTrackPlane2", "TRK_MINchXYonTrackPlane3", "TRK_MINchXYonTrackPlane4"};
    array<TString, NBranches> CustomSel = { "TRK_TrackResidualL2_Y>-2",    "TRK_TrackResidualL3_Y>-2",               "TRK_TrackResidualL4_Y>-2",
                                            "TRK_TrackResidualL5_Y>-2",    "TRK_TrackResidualL6_Y>-2",               "TRK_TrackResidualL7_Y>-2", 
                                            "TRK_TrackResidualL3_X>-2",    "TRK_TrackResidualL5_X>-2",               "TRK_TrackResidualL6_X>-2",
                                            "",                "",                 
                                            "RinnerL1!=0 && Rinner/RinnerL1>0",
                                            "hasRich==1",                   "hasRich==1",                   "hasRich==1",
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
        myFile = new TFile(PathToFile.at(i_file), "READ");
        if (!myFile) {
            printf("ERROR!! \n File %s not found\n", PathToFile.at(i_file).Data());
            return;
        }
        myFile->cd();
        printf("Reading file ==> %s\n", PathToFile.at(i_file).Data());
        
        myTree = (TTree*)myFile->Get(TreeName.at(i_file).Data());
        if (!myTree) {
            printf("%s tree not found in the file %s\n", TreeName.at(i_file).Data(), PathToFile.at(i_file).Data());
            return ;
        }
        printf("Total N.entries: %lld", myTree->GetEntries());

        TString lisName = ">>MyEventList";
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
    printf("I am out of the first file loop\n");

    
    array<TH1F*, NBranches> hSig, hBkg;
    for(int i=0; i<TotFiles; ++i){
        printf("Working on file %s ...\n", RootNames.at(i).Data());
        RetrieveTh1(RootNames.at(i), hSig, hBkg);
        printf("Size of arraya %zu, %zu\n", hSig.size(), hBkg.size());
        for(int ib=0; ib<NBranches; ++ib){
            TString c_name = "c_"+RigSig.at(i)+"_"+Bnames.at(ib);
            if(Bnames.at(ib).Contains("RinnerL1")) c_name = "c_"+RigSig.at(i)+"_SigmaInL1";
            TCanvas *canvas = MakeSquareCanvas(c_name.Data(), c_name.Data()); canvas->SetLogy();
            if(i>2) {
                hSig.at(ib)->SetLineColor(ISScolors.at(0)); hSig.at(ib)->SetFillColor(ISScolors.at(0)-9);
            }
            else{
                hSig.at(ib)->SetLineColor(MCcolors.at(0));  hSig.at(ib)->SetFillColor(MCcolors.at(0)-9);
            }
            //printf("CHECKING \n");
            //printf("%s %s \n", hSig.at(ib)->GetName(), hBkg.at(ib)->GetName());
            hSig.at(ib)->SetFillStyle(SigFillStyle);    hSig.at(ib)->SetTitle(Label.at(0).Data());
            if(i>2){
                hBkg.at(ib)->SetLineColor(ISScolors.at(1)); hBkg.at(ib)->SetFillColor(ISScolors.at(1)-9);
            }
            else{
                hBkg.at(ib)->SetLineColor(MCcolors.at(1));  hBkg.at(ib)->SetFillColor(MCcolors.at(1)-9);
            }
            hBkg.at(ib)->SetFillStyle(BkgFillStyle);    hBkg.at(ib)->SetTitle(Label.at(1).Data());
            TH1* hTemp = hSig.at(ib)->DrawNormalized("hist"); hTemp->GetYaxis()->SetRangeUser(minY, maxY);
            hBkg.at(ib)->DrawNormalized("hist same");
            TLegend *l = canvas->BuildLegend(); l->SetBorderSize(0); l->SetFillStyle(0);
            hTemp->SetTitle(""); hTemp->GetYaxis()->CenterTitle(true); hTemp->GetXaxis()->CenterTitle(true);
            DrawTag(Tags.at(i).Data(),0.15,0.87,0.05);
            DrawTag(PosOrNeg.at(i).Data(),0.7,0.87,0.04,52);
            saveTo = outPath+"/"+c_name+".pdf";
            canvas->SaveAs(saveTo.Data());
            delete hTemp; delete l; delete canvas;
        }
    }
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
    TString SelSig, SelBkg;

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
        else{
            tree->SetBranchStatus(branches.at(ib).Data(),1);
        }
    }

    printf("Opening File with name %s \n", fileName.Data());
    TFile *fTemp = new TFile(fileName.Data(),"RECREATE");
    



    //Splitting training and validation TEventList 
    pair<TEventList*,TEventList*> TrainTestList;
    if(isPosRig) TrainTestList = SplitEventList(evList);  
    printf("Splitting this and size of branches: %zu\n", branches.size());

    //Loop on branches
    for(int ib=0; ib<branches.size(); ++ib){

        TString title, SigName, BkgName;
        TString to_draw, tempB;

        //Selection defintion
        if(MySel.at(ib).Length()!=0){
            if (isMC) {
                SelSig = "("+MySel.at(ib)+" && abs((RinnerUH-RinnerLH)/(RinnerUH+RinnerLH))<=0.2)*weight";
                SelBkg = "("+MySel.at(ib)+" && abs((RinnerUH-RinnerLH)/(RinnerUH+RinnerLH))>0.2)*weight";
            }
            else{
                SelSig = "("+MySel.at(ib)+" && abs((RinnerUH-RinnerLH)/(RinnerUH+RinnerLH))<=0.2)";
                SelBkg = "("+MySel.at(ib)+" && abs((RinnerUH-RinnerLH)/(RinnerUH+RinnerLH))>0.2)";
            }
        }
        else{
            if(isMC){
                SelSig = "( abs((RinnerUH-RinnerLH)/(RinnerUH+RinnerLH))<=0.2)*weight";
                SelBkg = "( abs((RinnerUH-RinnerLH)/(RinnerUH+RinnerLH))>0.2)*weight";
            }
            else{
                SelSig = "( abs((RinnerUH-RinnerLH)/(RinnerUH+RinnerLH))<=0.2)";
                SelBkg = "( abs((RinnerUH-RinnerLH)/(RinnerUH+RinnerLH))>0.2)";
            }
        }
            
        //Title
        if(isMC) title = "; "+branches.at(ib)+";Normalized weights";
        else     title = "; "+branches.at(ib)+";Normalized events";

        if(isPosRig){
            //Initiali binning
            minB.at(ib) = tree->GetMinimum(branches.at(ib).Data());
            maxB.at(ib) = tree->GetMaximum(branches.at(ib).Data());
            myBins.at(ib) = int(100);

            //Training list
            tree->SetEventList(TrainTestList.first);

            TH1F *hs = new TH1F("hs", title.Data(), myBins.at(ib), minB.at(ib), maxB.at(ib));
            to_draw = branches.at(ib)+">>+hs";
            tree->Draw(to_draw.Data(), SelSig.Data(),"goff");
            hs  = (TH1F*)gDirectory->Get("hs");
            mean.at(ib) = hs->GetMean();
            std.at(ib)  = hs->GetStdDev();
            delete hs;

            //Evaluate range of scaled branch on positive from 70% to 100%
            tree->SetEventList(TrainTestList.second);
            to_draw = "("+branches.at(ib)+"-"+TString(to_string(mean.at(ib)))+")/"+TString(to_string(std.at(ib)))+">>hst";
            tree->Draw(to_draw.Data(),SelSig.Data(),"goff");
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
        TH1F *hsig = new TH1F("hsig", title.Data(), myBins.at(ib), minB.at(ib), maxB.at(ib));
        to_draw = "("+branches.at(ib)+"-"+TString(to_string(mean.at(ib)))+")/"+TString(to_string(std.at(ib)))+">>+hsig";
        tree->Draw(to_draw.Data(), SelSig.Data(),"goff");
        hsig = (TH1F*)gDirectory->Get("hsig"); 
        //bkg
        TH1F *hbkg = new TH1F("hbkg", title.Data(), myBins.at(ib), minB.at(ib), maxB.at(ib));
        to_draw = "("+branches.at(ib)+"-"+TString(to_string(mean.at(ib)))+")/"+TString(to_string(std.at(ib)))+">>+hbkg";
        tree->Draw(to_draw.Data(), SelBkg.Data(),"goff");
        hbkg = (TH1F*)gDirectory->Get("hbkg");


        if(branches.at(ib).Contains("RinnerL1")) tempB = "SigmaInL1";
        else tempB = branches.at(ib);
        if(isMC){
            if(isPosRig){
                SigName = tempB+"_SignalMCpos";
                BkgName = tempB+"_BkgMCpos";
            }
            else{
                SigName = tempB+"_SignalMCneg";
                BkgName = tempB+"_BkgMCneg";
            }
        }
        else{
            if(isPosRig){
                SigName = tempB+"_SignalISSpos";
                BkgName = tempB+"_BkgISSpos";
            }
            else{
                SigName = tempB+"_SignalISSneg";
                BkgName = tempB+"_BkgISSneg";
            }
        }
        hsig->SetName(SigName.Data());
        hbkg->SetName(BkgName.Data());
        hsig->Write(SigName.Data());
        hbkg->Write(BkgName.Data());
        delete hsig; 
        delete hbkg;
    }
    fTemp->Close();
    return;
}

void RetrieveTh1(TString &fileName, array<TH1F*,NBranches> &myTHsig, array<TH1F*,NBranches> &myTHbkg){
    TFile *f = new TFile(fileName.Data(),"OPEN");
    TList *list = f->GetListOfKeys();
    int isig=0, ibkg=0;
    for (auto k: *list) {
        TKey *key = static_cast<TKey*>(k);
        TClass *cl = gROOT->GetClass(key->GetClassName());
        if (cl->InheritsFrom("TH1F")) {
            TH1F *h = key->ReadObject<TH1F>();
            std::string hname = h->GetName();

            if (hname.find("Signal") != std::string::npos) {
                myTHsig.at(isig) = (TH1F*)h->Clone();
                //printf("Checking sig %s %d\n", myTHsig.at(isig)->GetName(), isig);
                isig++;
            }
            else if (hname.find("Bkg") != std::string::npos) {
                myTHbkg.at(ibkg) = (TH1F*)h->Clone();
                //printf("Checking bkg %s %d\n", myTHbkg.at(ibkg)->GetName(), ibkg);
                ibkg++;
            }
        }
    }
    //f->Close();
    return;
}
