#include "CPPGreedy.h"


CPPGreedy::CPPGreedy(CPPInstance* nInstance):
    mInstance(nInstance),
    mGenerator(2), mRCLSize(2) {      

    // mFileName(fileName),
    // mInstanceName(instanceName), 
    // mSolutionHolder(new CPPSolutionHolder()), mMetaHeuristic(CPPMetaheuristic::FSS),
    // mGreedyHeuristic(GreedyHeuristicType::MaxIncrease) {
    // mLogFileName = "./Log" + mInstanceName;
    // std::ofstream logFile(mLogFileName);
    // logFile.close();

    // InitFSS();
    mRCL = new RCL<CPPCandidate>(mRCLSize);
    mSolution = new CPPSolution(mInstance);
    CPPSolution::Init(mInstance->getNumberOfNodes());

    // mSAParams.InitGeometric();
}

CPPGreedy::CPPGreedy(const std::string& fileName):
    mGenerator(2), mRCLSize(2) {
        
    // mFileName(fileName),
    // mInstanceName(instanceName), 
    // mSolutionHolder(new CPPSolutionHolder()), mMetaHeuristic(CPPMetaheuristic::FSS),
    // mGreedyHeuristic(GreedyHeuristicType::MaxIncrease) {

    // InitFSS();
    mInstance = new CPPInstance(fileName);
    mSolution = new CPPSolution(mInstance);
    mRCL = new RCL<CPPCandidate>(mRCLSize);
    CPPSolution::Init(mInstance->getNumberOfNodes());

    // mSAParams.InitGeometric();
}

    /*
    std::string GetMethodFileName() const {
        std::string result;

        switch (mMetaHeuristic) {
        case CPPMetaheuristic::GRASP:
            result = "GRASP_";
            break;
        case CPPMetaheuristic::FSS:
            result = "FSS_";
            break;
        }

        switch (mSASType) {
        case SASelectType::Single:
            result += "Single";
            break;
        case SASelectType::Dual:
            result += "Dual";
            break;
        }

        return result;
    }*/

void CPPGreedy::InitAvailable(const std::vector<std::vector<int>>& FixedSet) {
    std::vector<int> temp;
    std::vector<bool> usedNodes(mInstance->getNumberOfNodes(), false);

    mAvailableNodes.clear();

    if (FixedSet.empty()) {
        for (int i = 0; i < mInstance->getNumberOfNodes(); ++i) {
            temp = { i };
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
            temp = { i };
            mAvailableNodes.push_back(temp);
        }
    }

    for (size_t i = 0; i < FixedSet.size(); ++i) {
        AddToSolution(CPPCandidate(FixedSet[i], i, 0));
    }
}
    
void CPPGreedy::InitGreedy() {
    mSolution->Clear();
    std::vector<std::vector<int>> emptySet = {};
    InitAvailable(emptySet);
}


void CPPGreedy::SolveGreedy(const std::vector<std::vector<int>>& FixedSet) {
    CPPCandidate* Select = nullptr;
    InitGreedy();
    mSolution->InitChange();
    InitAvailable(FixedSet);

    while (true) {
        Select = GetHeuristic();
        if (Select == nullptr)
            break;
        AddToSolution(*Select);
    }
    // mSolution->FixCliques();
    int Test = mSolution->CalculateObjective();
}

CPPCandidate* CPPGreedy::GetHeuristicMaxIncrease() {
    int cValue;
    int Select;
    if (mAvailableNodes.empty())
        return nullptr;
    mRCL->reset();

    if (mSolution->getCliques().empty()) {
        Select = mGenerator() % mAvailableNodes.size();
        AddToSolution(CPPCandidate(mAvailableNodes[Select], 0, Select));
    }

    int counter = 0;
    for (const auto& n : mAvailableNodes) {
        for (int c = 0; c < mSolution->NumberOfCliques(); ++c) {
            cValue = mSolution->GetChange(n, c);
            mRCL->add(CPPCandidate(n, c, counter), cValue);
        }
        ++counter;
    }

    if ((mRCL->getMaxValue() <= 0) || (mRCL->getCurrentSize() == 0)) {
        int index = mGenerator() % mAvailableNodes.size();
        return new CPPCandidate(mAvailableNodes[index], mSolution->NumberOfCliques(), index);
    }

    return new CPPCandidate(*mRCL->getCandidate(mGenerator() % mRCL->getCurrentSize()));
}

CPPCandidate* CPPGreedy::GetHeuristic() {
    // switch (mGreedyHeuristic) {
    // case GreedyHeuristicType::MaxIncrease:
    //     return GetHeuristicMaxIncrease();
    // }
    // return nullptr;
    return GetHeuristicMaxIncrease();
}

CPPSolution& CPPGreedy::getSolution() {
    return *mSolution;
}

// TODO: reference might be incorrect here
bool CPPGreedy::AddToSolution(const CPPCandidate& N) {
    RemoveFromAvailable(N);
    mSolution->AddCandidate(N);
    return true;
}

void CPPGreedy::RemoveFromAvailable(const CPPCandidate& N) {
    mAvailableNodes.erase(mAvailableNodes.begin() + N.getCandidateIndex());
}
