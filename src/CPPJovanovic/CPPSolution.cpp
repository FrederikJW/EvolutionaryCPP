// a rewritten version of CPPSolution.cs into C++ https://github.com/rakabog/CPPConsole/blob/master/CPPConsole/CPPSolution.cs
#include "CPPSolution.h"

#include <algorithm>
#include <vector>
#include <list>
#include <numeric>

std::vector<std::vector<int>> CPPSolution::EmptyChange;

CPPSolution::CPPSolution(CPPInstance* nInstance) {
    mInstance = nInstance;
    mChange.resize(mInstance->getNumberOfNodes(), std::vector<int>(mInstance->getNumberOfNodes(), 0));
    Allocate();
}

CPPSolution::CPPSolution(int* pvertex, int numVertices, int objective, CPPInstance* nInstance) : CPPSolutionBase(pvertex, numVertices, objective, nInstance) {
    mChange.resize(mInstance->getNumberOfNodes(), std::vector<int>(mInstance->getNumberOfNodes(), 0));
}

void CPPSolution::Init(int size) {
    EmptyChange.clear();
    EmptyChange.resize(size, std::vector<int>(size, 0));
}

int CPPSolution::getObjective() {
    return CalculateObjective();
}

const std::vector<std::vector<int>>& CPPSolution::getCliques() const {
    return mCliques;
}

const std::vector<int>& CPPSolution::getNodes() const {
    return mNodeClique;
}

void CPPSolution::InitChange() {
    for (int c = 0; c < mInstance->getNumberOfNodes(); ++c) {
        std::copy(EmptyChange[c].begin(), EmptyChange[c].end(), mChange[c].begin());
    }
}

void CPPSolution::AddCandidate(const CPPCandidate& A) {
    AddNodeToClique(A);
}

int CPPSolution::GetChange(int iNode, int iClique) {
    if (iClique >= mCliques.size())
        return 0;
    return mChange[iClique][iNode];
}

int CPPSolution::GetChange(const std::vector<int>& nodes, int iClique) {
    if (nodes.empty())
        return 0;
    return GetChange(nodes[0], iClique);
}

void CPPSolution::UpdateChange(int Clique, int Node) {
    const int numNodes = mInstance->getNumberOfNodes();

    std::vector<int>& changeClique = mChange[Clique];
    const std::vector<int>& weightsNode = mInstance->getWeights()[Node];

    int* changePtr = changeClique.data();
    const int* weightsPtr = weightsNode.data();

    for (int n = 0; n < numNodes; ++n) {
        changePtr[n] += weightsPtr[n];
    }
}

void CPPSolution::Allocate() {
    mCliques.clear();
    mNodeClique.resize(mInstance->getNumberOfNodes(), -1);
}

void CPPSolution::Clear() {
    mCliques.clear();
    std::fill(mNodeClique.begin(), mNodeClique.end(), -1);
    InitChange();
}

int CPPSolution::AddNodeToClique(const CPPCandidate& N) {
    int result = 0;
    for (int n : N.getNodes())
        result += AddNodeToClique(n, N.getClique());
    return result;
}

int CPPSolution::AddNodeToClique(int iNode, int iClique) {
    if (mNodeClique[iNode] != -1)
        return -1;

    if (iClique >= mCliques.size()) {
        mCliques.emplace_back(std::vector<int>{iNode});
        mNodeClique[iNode] = iClique;
        UpdateChange(iClique, iNode);
        return mCliques.size() - 1;
    }

    mNodeClique[iNode] = iClique;
    mCliques[iClique].push_back(iNode);
    UpdateChange(iClique, iNode);
    return -1;
}

int CPPSolution::NumberOfCliques() {
    return mCliques.size();
}

/*
int CPPSolution::CalculateObjective() {
    int result = 0;
    for (const auto& Clique : mCliques) {
        result += CalculateObjectiveForClique(Clique);
    }
    return result;
}*/

int CPPSolution::CalculateObjectiveForClique(std::vector<int> Clique) {
    int result = 0;
    for (size_t i = 0; i < Clique.size(); i++) {
        for (size_t j = 0; j < Clique.size(); j++) {
            if (i > j)
                result += mInstance->getWeights()[Clique[i]][Clique[j]];
        }
    }
    return result;
}
