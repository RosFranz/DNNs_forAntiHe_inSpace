#include <vector>
#include <cmath>
#include <numeric>
#include <iostream>

#include <TDirectory.h>
#include <TString.h>
#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>
#include <TH1.h>

using namespace std;
vector<TTree*> fromTerminal(bool isMC=false,
                                    TString CLtrainOnMCFile="000",
                                    TString CLvalOnISSFile="000",
                                    TString AEtrainOnMCFile="000",
                                    TString AEvalOnISSFile="000",
                                    vector<TTree*> myTrees = {}
                                );

double distanceCorrelation(const std::vector<double>& x, const std::vector<double>& y);


void dCor(){
    vector<TTree*> myISS, myMC;
    vector<double> massRICH, betaRICH, CL, AE;
    float massR=0, betaR=0; //for ISS double, MC float
    float score=0, as=0;
    TString Sel("(hasRich==1 && beta_rich>=0.953 && beta_rich<=0.998 && isNaF==0 && hasGoodImpact==1 &&"
                "kprob_rich>0.01 && 1<charge2_rich && charge2_rich<4 && ringPMTs2_rich>5 &&"
                "(measPE_Corr_rich/totPE_Uncorr_rich)>0.4 &&abs(Rinner)-(3*pow(Rinner,2)*invRerr_IN)>=4.419)");
    myISS = fromTerminal(false, "2025_06_30", "2025_06_30", "2025_06_30", "2025_06_30");
    /*
    myMC  = fromTerminal(true,  "2025_06_30", "2025_06_30", "2025_06_30", "2025_06_30");

    myMC.at(1)->SetBranchAddress("mass_rich",&massR);
    myMC.at(1)->SetBranchAddress("beta_rich",&betaR);
    myMC.at(1)->SetBranchAddress("scores_neg",&score);
    myMC.at(1)->SetBranchAddress("NEGanomaly_score",&as);


    Sel.Append("*weight");
    myMC.at(1)->Draw(">>myList",Sel.Data(),"goff");
    TEventList *MClist = (TEventList*)gDirectory->Get("myList");
    MClist->Print();
    myMC.at(1)->SetEventList(MClist);

    for(int i=0; i<MClist->GetN(); ++i){
        myMC.at(1)->GetEntry(MClist->GetEntry(i));
        massRICH.push_back(massR); betaRICH.push_back(betaR);
        CL.push_back(score); AE.push_back(as);
    }
    */
    myISS.at(3)->SetBranchAddress("mass_rich",&massR);
    myISS.at(3)->SetBranchAddress("beta_rich",&betaR);
    myISS.at(3)->SetBranchAddress("scores_neg",&score);
    myISS.at(3)->SetBranchAddress("NEGanomaly_score",&as);


    myISS.at(3)->Draw(">>myList",Sel.Data(),"goff");
    TEventList *MClist = (TEventList*)gDirectory->Get("myList");
    MClist->Print();
    myISS.at(3)->SetEventList(MClist);

    for(int i=0; i<MClist->GetN(); ++i){
        myISS.at(3)->GetEntry(MClist->GetEntry(i));
        massRICH.push_back(massR); betaRICH.push_back(betaR);
        CL.push_back(score); AE.push_back(as);
        if (i>10000) break;
    }
    std::ofstream out("xy.csv");
    for (int i = 0; i < AE.size(); ++i) out << massRICH.at(i) << "," << betaRICH.at(i) << "," << CL.at(i) << "," << AE.at(i) <<"\n";
    out.close();
    
    double dCor1 = distanceCorrelation(CL,AE); 
    double dCor2 = distanceCorrelation(massRICH,betaRICH);
    printf("Mass-beta: %f \t CL-AE: %f\n", dCor2,dCor1);
    TH1F *hMB = new TH1F("dCor between Mass and Beta AGL",";dCor;Events",100,0,.2);
    hMB->SetLineWidth(3);
    TH1F *hCA = new TH1F("dCor between CL2 and AE",";dCor;Events",100,0,.2);
    hCA->SetLineWidth(3); hCA->SetLineColor(kRed);
    int ab_dCor = 0;
    for(int i=0; i<100; i++){
        std::random_device rd;
        mt19937 g(rd());
        shuffle(massRICH.begin(), massRICH.end(), g);
        shuffle(AE.begin(), AE.end(), g);
        hMB->Fill(distanceCorrelation(massRICH,betaRICH));
        double t_dCor = distanceCorrelation(CL,AE);
        if(t_dCor>dCor1) ab_dCor++;
        hCA->Fill(t_dCor);
    }
    hMB->Draw("hist"); hCA->Draw("hist same");
    printf("Fraction of permutation: %f\n", ab_dCor/100.);
    return;
}



double distanceCorrelation(const std::vector<double>& x, const std::vector<double>& y) {
    int n = x.size();
    if (n != y.size() || n < 2) return 0.0;

    // Step 1: Compute pairwise distance matrices
    std::vector<std::vector<double>> A(n, std::vector<double>(n));
    std::vector<std::vector<double>> B(n, std::vector<double>(n));

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j) {
            A[i][j] = std::abs(x[i] - x[j]);
            B[i][j] = std::abs(y[i] - y[j]);
        }

    // Step 2: Double center distance matrices
    auto doubleCenter = [&](std::vector<std::vector<double>>& M) {
        std::vector<double> row_mean(n, 0), col_mean(n, 0);
        double total_mean = 0;

        // row and column means
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j) {
                row_mean[i] += M[i][j];
                col_mean[j] += M[i][j];
                total_mean += M[i][j];
            }

        for (int i = 0; i < n; ++i) {
            row_mean[i] /= n;
            col_mean[i] /= n;
        }
        total_mean /= (n * n);

        // Centering
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                M[i][j] = M[i][j] - row_mean[i] - col_mean[j] + total_mean;
    };

    doubleCenter(A);
    doubleCenter(B);

    // Step 3: Compute dot products
    double dCovXY = 0, dVarX = 0, dVarY = 0;
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j) {
            dCovXY += A[i][j] * B[i][j];
            dVarX  += A[i][j] * A[i][j];
            dVarY  += B[i][j] * B[i][j];
        }

    dCovXY /= (n * n);
    dVarX  /= (n * n);
    dVarY  /= (n * n);

    if (dVarX == 0 || dVarY == 0) return 0.0; // Avoid division by 0
    return std::sqrt(dCovXY / std::sqrt(dVarX * dVarY));
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
            } // End CL MC
            if (i_file == 1 && i_tree >= 2) { // CL ISS data
                CLtrees.at(i_tree) = (TTree*)files.at(i_file)->Get(DirName.at(i_tree).Data());
                if (!CLtrees.at(i_tree)) {
                    printf("%s tree not found in the file %s\n", DirName.at(i_tree).Data(), PathToFile.at(i_file).Data());
                    return {};
                }
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
