#include "CPPProblem.h"
#include "CPPSolution.h"

#include <thread>

CPPProblem::CPPProblem(CPPInstance* nInstance, RandomGenerator* generator)
    : mInstance(nInstance), mRCLSize(2), mGenerator(generator), mMetaHeuristic(FSS), mGreedyHeuristic(MaxIncrease), mRCL(nullptr) {
    mSolution = new CPPSolution(mInstance);
    mSolutionHolder = CPPSolutionHolder();
    InitFSS();
    CPPSolution::Init(mInstance->getNumberOfNodes());
    mSAParams.InitGeometric();
    mSAParams.neighborhoodFactor = 1;
}

CPPProblem::~CPPProblem() {
    delete mRCL;
    delete mSolution;
}

/*
CPPProblem::CPPProblem(const std::string& FileName, const std::string& InstanceName)
    : mRCLSize(2), mFileName(FileName), mInstanceName(InstanceName), mGenerator(2), mMetaHeuristic(FSS), mGreedyHeuristic(MaxIncrease) {
    mInstance = nullptr;
    mSolution = nullptr;
    mRCL = RCL<CPPCandidate>(mRCLSize);
    mLogFileName = "./Log" + mInstanceName;
    std::ofstream S(mLogFileName);
    S.close();
    mSolutionHolder = CPPSolutionHolder();
    InitFSS();
    // CPPSolution::Init(mInstance->getNumberOfNodes());
    mSAParams.InitGeometric();
}
*/
/*
CPPProblem::CPPProblem(const std::string& FileName, const std::string& InstanceName, CPPSolutionBase* solution)
    : mRCLSize(2), mFileName(FileName), mInstanceName(InstanceName), mGenerator(2), mMetaHeuristic(FSS), mGreedyHeuristic(MaxIncrease) {
    mInstance = solution->getInstance();
    mSolution = solution;
    mRCL = RCL<CPPCandidate>(mRCLSize);
    mLogFileName = "./Log" + mInstanceName;
    std::ofstream S(mLogFileName);
    S.close();
    mSolutionHolder = CPPSolutionHolder();
    InitFSS();
    CPPSolution::Init(mInstance->getNumberOfNodes());
    mSAParams.InitGeometric();
}
*/

std::string CPPProblem::GetMethodFileName() {
    std::string Result;

    switch (mMetaHeuristic) {
    case GRASP:
        Result = "GRASP_";
        break;
    case FSS:
        Result = "FSS_";
        break;
    }

    switch (mSASType) {
    case SASelectType::Single:
        Result += "Single";
        break;
    case SASelectType::Dual:
        Result += "Dual";
        break;
    }

    return Result;
}

long CPPProblem::GetBestTime() {
    return mIntermediateSolutionsTimes.back();
}

void CPPProblem::Solve(int iMaxIterations, double iTimeLimit) {
    switch (mMetaHeuristic) {
    case FSS:
        SolveFixSetSearch(iMaxIterations, iTimeLimit);
        break;
    case GRASP:
        SolveGRASP(iMaxIterations, iTimeLimit);
        break;
    }
}

void CPPProblem::InitAvailable(const std::vector<std::vector<int>>& FixedSet) {
    std::vector<int> temp;
    std::vector<bool> usedNodes(mInstance->getNumberOfNodes(), false);

    mAvailableNodes.clear();
    if (FixedSet.empty()) {
        for (int i = 0; i < mInstance->getNumberOfNodes(); ++i) {
            temp.clear();
            temp.push_back(i);
            mAvailableNodes.push_back(temp);
        }
        return;
    }

    for (const auto& l : FixedSet) {
        mAvailableNodes.push_back(l);
        for (int n : l) {
            usedNodes[n] = true;
        }
    }

    for (int i = 0; i < mInstance->getNumberOfNodes(); ++i) {
        if (!usedNodes[i]) {
            temp.clear();
            temp.push_back(i);
            mAvailableNodes.push_back(temp);
        }
    }

    for (size_t i = 0; i < FixedSet.size(); ++i) {
        AddToSolution(CPPCandidate(FixedSet[i], i, 0));
    }
}

void CPPProblem::InitTracking() {
    mIntermediateSolutions.clear();
    mIntermediateSolutionsTimes.clear();
    mIntermediateSolutionsIterations.clear();
}

void CPPProblem::InitFSS() {
    mFixStagnation = 20;
    mFixK = 10;
    mFixInitPopulation = 10;
    mFixN = 50;
}

void CPPProblem::AllocateSolution() {
    delete mSolution;
    switch (mGreedyHeuristic) {
    case MaxIncrease:
        mSolution = new CPPSolution(mInstance);
        break;
    }
    mSolution->setGenerator(mGenerator);
    mSolution->setSASType(mSASType);
}

void CPPProblem::AllocateSolution(CPPSolutionBase* solution) {
    delete mSolution;
    switch (mGreedyHeuristic) {
    case MaxIncrease:
        mSolution = solution;
        break;
    }
    mSolution->setGenerator(mGenerator);
    mSolution->setSASType(mSASType);
}

void CPPProblem::AllocateSolution(int* pvertex, int numVertices, int objective) {
    delete mSolution;
    switch (mGreedyHeuristic) {
    case MaxIncrease:
        mSolution = new CPPSolution(pvertex, numVertices, objective, GetInstance());
        break;
    }
    mSolution->setGenerator(mGenerator);
    mSolution->setSASType(mSASType);
}

std::vector<double> CPPProblem::GetFrequency(int BaseSolutionIndex, const std::vector<int>& SelectedSolutionIndexes) {
    std::vector<double> frequency(mInstance->getNumberOfNodes(), 0);
    CPPSolutionBase* Base = &mSolutionHolder.Solutions()[BaseSolutionIndex];

    for (int sel : SelectedSolutionIndexes) {
        UpdateFrequency(Base, &mSolutionHolder.Solutions()[sel], frequency);
    }

    for (size_t i = 0; i < frequency.size(); ++i) {
        frequency[i] /= (Base->CliqueForNode(i).size() * SelectedSolutionIndexes.size());
    }

    return frequency;
}

void CPPProblem::UpdateEdgeFrequency(std::vector<std::vector<int>>& Occurence, CPPSolutionBase* Base, CPPSolutionBase* Update) {
    for (const auto& l : Base->getCliques()) {
        for (size_t i = 0; i < l.size(); ++i) {
            for (size_t j = i + 1; j < l.size(); ++j) {
                if (Update->InSameClique(l[i], l[j])) {
                    Occurence[l[i]][l[j]]++;
                }
            }
        }
    }
}

std::vector<std::vector<int>> CPPProblem::GetFrequencyEdge(int BaseSolutionIndex, const std::vector<int>& SelectedSolutionIndexes) {
    CPPSolutionBase* Base = &mSolutionHolder.Solutions()[BaseSolutionIndex];
    std::vector<std::vector<int>> Occurance(mInstance->getNumberOfNodes(), std::vector<int>(mInstance->getNumberOfNodes(), 0));

    for (int sel : SelectedSolutionIndexes) {
        UpdateEdgeFrequency(Occurance, Base, &mSolutionHolder.Solutions()[sel]);
    }

    return Occurance;
}

void CPPProblem::UpdateFrequency(CPPSolutionBase* Base, CPPSolutionBase* Test, std::vector<double>& frequency) {
    std::vector<int> CurrentCliqueBase;
    int CurrentNodeClique;

    for (int i = 0; i < mInstance->getNumberOfNodes(); ++i) {
        CurrentCliqueBase = Base->CliqueForNode(i);
        CurrentNodeClique = Test->getNodeClique()[i];

        for (int n : CurrentCliqueBase) {
            if (CurrentNodeClique == Test->getNodeClique()[n]) {
                frequency[i]++;
            }
        }
    }
}

std::vector<std::vector<int>> CPPProblem::GetFixEdge(int N, int K, double FixSize, std::vector<std::vector<int>>& SuperNodes) {
    std::vector<std::vector<int>> resultCliques;
    auto Freq = GetFrequencyEdge((*mGenerator)() % std::min(N, static_cast<int>(mSolutionHolder.Solutions().size())), WeightedRandomSampling::GetWeightedRandomSampling(std::min(N, static_cast<int>(mSolutionHolder.Solutions().size())), std::min(K, static_cast<int>(mSolutionHolder.Solutions().size())), std::vector<double>(N, 1.0 / N), *mGenerator));
    int numberOfNodes = mInstance->getNumberOfNodes();
    int* tNodeClique = new int[numberOfNodes];
    std::iota(tNodeClique, tNodeClique + mInstance->getNumberOfNodes(), 0);

    int counter = static_cast<int>(FixSize * mInstance->getNumberOfNodes() - 5);
    int size = 10;
    int ElemIndex = 0;
    bool Finish = false;

    while (counter > 0) {
        while (ElemIndex < Freq.size()) {
            auto& tPair = Freq[ElemIndex];
            if (tNodeClique[tPair[0]] != tNodeClique[tPair[1]]) {
                int low = std::min(tNodeClique[tPair[0]], tNodeClique[tPair[1]]);
                counter--;
                std::replace_if(tNodeClique, tNodeClique + mInstance->getNumberOfNodes(), [&](int val) { return val == tNodeClique[tPair[1]] || val == tNodeClique[tPair[0]]; }, low);

                if (counter == 0) {
                    Finish = true;
                    break;
                }
            }
            ElemIndex++;
        }

        if (Finish) break;
        size--;
        ElemIndex = 0;
    }

    std::vector<std::vector<int>> CollectCliques(mInstance->getNumberOfNodes()), CollectSuperCliques(mInstance->getNumberOfNodes());
    for (int i = 0; i < mInstance->getNumberOfNodes(); ++i) {
        CollectCliques[tNodeClique[i]].push_back(i);
        CollectSuperCliques[tNodeClique[i]].push_back(i);
    }

    std::vector<std::vector<int>> Res;
    for (const auto& clique : CollectCliques) {
        if (clique.size() > 1) Res.push_back(clique);
    }

    for (const auto& superClique : CollectSuperCliques) {
        if (superClique.size() > 1 && !ContainsList(SuperNodes, superClique)) {
            SuperNodes.push_back(superClique);
        }
    }
    delete[] tNodeClique;

    return Res;
}

bool CPPProblem::ContainsList(const std::vector<std::vector<int>>& Container, const std::vector<int>& Test) {
    for (const auto& l : Container) {
        if (l.size() == Test.size() && std::equal(l.begin(), l.end(), Test.begin())) {
            return true;
        }
    }
    return false;
}

std::vector<std::vector<int>> CPPProblem::GetFix(int N, int K, double FixSize) {
    std::vector<double> Freq = GetFrequency((*mGenerator)() % std::min(N, static_cast<int>(mSolutionHolder.Solutions().size())), WeightedRandomSampling::GetWeightedRandomSampling(std::min(N, static_cast<int>(mSolutionHolder.Solutions().size())), std::min(K, static_cast<int>(mSolutionHolder.Solutions().size())), std::vector<double>(N, 1.0 / N), *mGenerator));
    std::vector<double*> Elem;
    for (int i = 0; i < mInstance->getNumberOfNodes(); ++i) {
        double* TElem = new double[2] { static_cast<double>(i), Freq[i] };
        Elem.push_back(TElem);
    }

    std::sort(Elem.begin(), Elem.end(), [](double* a, double* b) { return a[1] > b[1]; });

    // rewrote resultCliques creation to account for solutions with different clique counts
    size_t maxSize = 0;
    for (const auto& solution : mSolutionHolder.Solutions()) {
        maxSize = std::max(maxSize, solution.getCliques().size());
    }
    std::vector<std::vector<int>> resultCliques(maxSize);

    for (int i = 0; i < FixSize * mInstance->getNumberOfNodes(); ++i) {
        int cNode = static_cast<int>(Elem[i][0]);
        resultCliques[mSolutionHolder.Solutions()[(*mGenerator)() % std::min(N, static_cast<int>(mSolutionHolder.Solutions().size()))].getNodeClique()[cNode]].push_back(cNode);
    }

    std::vector<std::vector<int>> cResultCliques;
    for (const auto& l : resultCliques) {
        if (!l.empty()) {
            cResultCliques.push_back(l);
        }
    }

    for (auto* elem : Elem) delete[] elem;

    return cResultCliques;
}

void CPPProblem::InitGreedy() {
    mSolution->Clear();
    InitAvailable({});
}

bool CPPProblem::CheckBest(double Size) {
    int nValue = mSolution->CalculateObjective();
    if (!mSolution->CheckSolutionValid()) nValue = nValue;

    if (nValue > mBestSolutionValue) {
        mBestSolutionValue = nValue;
        mIntermediateSolutions.push_back(mBestSolutionValue);
        mIntermediateSolutionsIterations.push_back(mNumberOfSolutionsGenerated);
        mIntermediateSolutionsTimes.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - mStartTime).count());
        /*
        LogResult();
        if (Size == -1)
            std::cout << "Value :" << mBestSolutionValue << "  Thread :" << std::this_thread::get_id() << "\n";
        else
            std::cout << "Value :" << mBestSolutionValue << "  Thread :" << std::this_thread::get_id() << "  Fixed : " << Size << "\n";*/
        return true;
    }
    return false;
}

void CPPProblem::SolveFixSetSearch(int MaxGenerated, double iTimeLimit) {
    AllocateSolution();

    std::vector<double> w(mFixN, 1.0 / mFixN);
    double FixSize = 0.50;
    int cSolutionValue = 0;
    double Accept;

    std::vector<std::vector<int>> FixSet;

    mStartTime = std::chrono::steady_clock::now();
    SolveGRASP(mFixInitPopulation, iTimeLimit);
    int counter = 1;

    while (mInstance->getNumberOfNodes() / std::pow(2, counter) > 10) {
        counter++;
    }

    int MaxDiv = counter;
    int FixSetSizeIndex = 0;
    int StagCounter = 0;
    std::vector<int> tt;
    std::vector<std::vector<int>> SuperNodes;
    int cObj;

    for (int i = 0; i < MaxGenerated - mFixInitPopulation; ++i) {
        if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - mStartTime).count() > iTimeLimit) break;

        FixSize = 1 - std::pow(2, -1 * (FixSetSizeIndex + 1));
        FixSet = GetFix(mFixN, mFixK, FixSize);

        SolveGreedy(FixSet);
        cObj = mSolution->CalculateObjective();
        mSolution->LocalSearch();
        cObj = mSolution->CalculateObjective();
        mSolution->SimulatedAnnealing(mSAParams, Accept);
        cObj = mSolution->CalculateObjective();

        mSolutionHolder.Add(*mSolution);
        mNumberOfSolutionsGenerated++;

        if (!CheckBest(FixSize)) {
            StagCounter++;
            if (StagCounter >= mFixStagnation) {
                FixSetSizeIndex++;
                FixSetSizeIndex %= MaxDiv;
                StagCounter = 0;
            }
        }
        else {
            StagCounter = 0;
        }
    }
}

void CPPProblem::CalibrateDoubleMoves(double iTimeLimit) {
    double Accept;
    double LT = 1;
    double UT = 2000;
    double Tolerate = 0.05;
    double DesiredAcceptance = 0.5;
    int cSolutionValue;

    // AllocateSolution();
    mBestSolutionValue = std::numeric_limits<int>::min();
    mStartTime = std::chrono::steady_clock::now();
    InitTracking();

    mSAParams.mInitTemperature = 1000;
    mNumberOfSolutionsGenerated = 0;

    while (true) {
        // TODO: this erases the solution partially
        SolveGreedy();
        mSolution->CalibrateSADoubleMoves(mSAParams, Accept);
        cSolutionValue = mSolution->CalculateObjective();
        mSolution->setObjective(cSolutionValue);
        mSolutionHolder.Add(*mSolution);
        CheckBest();

        if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - mStartTime).count() > iTimeLimit) break;

        if (Accept > DesiredAcceptance + Tolerate) {
            UT = mSAParams.mInitTemperature;
            mSAParams.mInitTemperature = (LT + UT) / 2;
            continue;
        }

        if (Accept < DesiredAcceptance - Tolerate) {
            LT = mSAParams.mInitTemperature;
            mSAParams.mInitTemperature = (LT + UT) / 2;
            continue;
        }
        break;
    }

    printf("Accept probability %.3f, Calibrate temp %.2f\n\n", Accept, mSAParams.mInitTemperature);
}

void CPPProblem::Calibrate(double iTimeLimit) {
    double Accept;
    double LT = 1;
    double UT = 2000;
    double Tolerate = 0.05;
    double DesiredAcceptance = 0.5;
    int cSolutionValue;

    // AllocateSolution();
    mBestSolutionValue = std::numeric_limits<int>::min();
    mStartTime = std::chrono::steady_clock::now();
    InitTracking();

    mSAParams.mInitTemperature = 1000;
    mNumberOfSolutionsGenerated = 0;

    while (true) {
        // TODO: this erases the solution partially
        SolveGreedy();
        mSolution->CalibrateSA(mSAParams, Accept);
        cSolutionValue = mSolution->CalculateObjective();
        mSolution->setObjective(cSolutionValue);
        mSolutionHolder.Add(*mSolution);
        CheckBest();

        if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - mStartTime).count() > iTimeLimit) break;

        if (Accept > DesiredAcceptance + Tolerate) {
            UT = mSAParams.mInitTemperature;
            mSAParams.mInitTemperature = (LT + UT) / 2;
            continue;
        }

        if (Accept < DesiredAcceptance - Tolerate) {
            LT = mSAParams.mInitTemperature;
            mSAParams.mInitTemperature = (LT + UT) / 2;
            continue;
        }
        break;
    }
    
    printf("Accept probability %.3f, Calibrate temp %.2f\n\n", Accept, mSAParams.mInitTemperature);
}

void CPPProblem::CalibrateCool(double iTimeLimit) {
    double Accept;
    double LT = 1;
    double UT = 2000;
    double Tolerate = 0.05;
    double DesiredAcceptance = 0.5;
    int cSolutionValue;

    // AllocateSolution();
    mBestSolutionValue = std::numeric_limits<int>::min();
    mStartTime = std::chrono::steady_clock::now();
    InitTracking();

    mSAParams.mInitTemperature = 1000;
    mNumberOfSolutionsGenerated = 0;

    while (true) {
        // TODO: this erases the solution partially
        SolveGreedy();
        mSolution->CalibrateSACool(mSAParams, Accept);
        cSolutionValue = mSolution->CalculateObjective();
        mSolution->setObjective(cSolutionValue);
        mSolutionHolder.Add(*mSolution);
        CheckBest();

        if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - mStartTime).count() > iTimeLimit) break;

        if (Accept > DesiredAcceptance + Tolerate) {
            UT = mSAParams.mInitTemperature;
            mSAParams.mInitTemperature = (LT + UT) / 2;
            continue;
        }

        if (Accept < DesiredAcceptance - Tolerate) {
            LT = mSAParams.mInitTemperature;
            mSAParams.mInitTemperature = (LT + UT) / 2;
            continue;
        }
        break;
    }

    printf("Accept probability %.3f, Calibrate temp %.2f\n\n", Accept, mSAParams.mInitTemperature);
}

void CPPProblem::SALOSearch() {
    double Accept;
    mSolution->LocalSearch();
    mSolution->SimulatedAnnealing(mSAParams, Accept);
    mSolution->CalculateObjective();
}

void CPPProblem::SALOeCoolSearch() {
    double Accept;
    mSolution->LocalSearch();
    mSolution->SimulatedAnnealingCool(mSAParams, Accept);
    mSolution->CalculateObjective();
}

void CPPProblem::SALODoubleMovesSearch() {
    double Accept;
    // mSolution->LocalSearch();
    mSolution->SimulatedAnnealingWithDoubleMoves(mSAParams, Accept);
    mSolution->CalculateObjective();
}

void CPPProblem::LocalSearch() {
    mSolution->LocalSearch();
}

void CPPProblem::SASearch() {
    double Accept;
    mSolution->SimulatedAnnealing(mSAParams, Accept);
    mSolution->CalculateObjective();
}

void CPPProblem::SolveGRASP(int MaxIterations, double iTimeLimit) {
    int cSolutionValue;
    double Accept;

    for (int i = 0; i < MaxIterations; ++i) {
        if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - mStartTime).count() > iTimeLimit) break;

        SolveGreedy();
        cSolutionValue = mSolution->CalculateObjective();
        mSolution->LocalSearch();
        cSolutionValue = mSolution->CalculateObjective();
        mSolution->SimulatedAnnealing(mSAParams, Accept);
        cSolutionValue = mSolution->CalculateObjective();
        mNumberOfSolutionsGenerated++;

        mSolutionHolder.Add(*mSolution);
        CheckBest();
    }
}

void CPPProblem::SolveGreedy(const std::vector<std::vector<int>>& FixedSet) {
    CPPCandidate* Select = nullptr;
    InitGreedy();

    mSolution->InitChange();
    InitAvailable(FixedSet);

    while (true) {
        Select = GetHeuristic();
        if (Select == nullptr) break;
        AddToSolution(CPPCandidate(*Select));
        delete Select;
    }

    mSolution->FixCliques();
    mSolution->CalculateObjective();
}

CPPCandidate* CPPProblem::GetHeuristicMaxIncrease() {
    int cValue;
    int Select;

    if (mAvailableNodes.empty()) return nullptr;

    delete mRCL;
    mRCL = new RCL<CPPCandidate>(mRCLSize);

    if (mSolution->getCliques().empty()) {
        Select = (*mGenerator)() % mAvailableNodes.size();
        AddToSolution(CPPCandidate(mAvailableNodes[Select], 0, Select));
    }

    int counter = 0;
    for (const auto& n : mAvailableNodes) {
        for (int c = 0; c < mSolution->getNumberOfCliques(); ++c) {
            cValue = mSolution->GetChange(n, c);
            mRCL->add(CPPCandidate(n, c, counter), cValue);
        }
        counter++;
    }

    if (mRCL->getMaxValue() <= 0 || mRCL->getCurrentSize() == 0) {
        int index = (*mGenerator)() % mAvailableNodes.size();
        return new CPPCandidate(mAvailableNodes[index], mSolution->getNumberOfCliques(), index);
    }

    return new CPPCandidate(mRCL->getCandidate((*mGenerator)() % mRCL->getCurrentSize()));
}

CPPCandidate* CPPProblem::GetHeuristic() {
    switch (mGreedyHeuristic) {
    case MaxIncrease:
        return GetHeuristicMaxIncrease();
    }
    return nullptr;
}

bool CPPProblem::AddToSolution(const CPPCandidate& N) {
    if (mGreedyHeuristic == MaxIncrease) RemoveFromAvailable(N);
    mSolution->AddCandidate(N);
    return true;
}

bool CPPProblem::AddToSolutionHolder(CPPSolutionBase& iSolution) {
    mSolutionHolder.Add(iSolution);
    return true;
}

void CPPProblem::RemoveFromAvailable(const CPPCandidate& N) {
    mAvailableNodes.erase(mAvailableNodes.begin() + N.getCandidateIndex());
}
