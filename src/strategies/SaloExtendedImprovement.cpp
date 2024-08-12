#include "SaloExtendedImprovement.h"
#include "../Defines.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <vector>
#include <unordered_map>

// TODO: move these parameter to somewhere else
#define EMPTY_IDX 0

std::vector<std::vector<int>> convertPvertexToMcliques(int* pvertex, int numVertices) {
    std::unordered_map<int, std::vector<int>> groupMap;

    for (int i = 0; i < numVertices; ++i) {
        groupMap[pvertex[i]].push_back(i);
    }

    std::vector<std::vector<int>> groups(groupMap.size());

    int groupIndex = 0;
    for (auto& pair : groupMap) {
        groups[groupIndex++] = std::move(pair.second);
    }

    return groups;
}

SaloExtendedImprovement::SaloExtendedImprovement(int knownbest, double minpercent, double tempfactor, int sizefactor) : knownbest(knownbest), minpercent(minpercent), tempfactor(tempfactor), sizefactor(sizefactor), temp(0), problem(nullptr) {
    CPPInstance* instance = new CPPInstance("instance/rand500-100.txt");
    problem = new CPPProblem("file1", "rand500-100", instance);
    problem->SetID(2);
    problem->SetSASelect(SASelectType::Dual);
}

SaloExtendedImprovement::~SaloExtendedImprovement() {
    disposeEnvironment();
    delete problem;
}

void SaloExtendedImprovement::improveSolution(Partition& solution, clock_t startTime, int maxSeconds, BestSolutionInfo* frt, int generation_cnt) {
    setStart(solution);
    search(startTime, maxSeconds);
}

void SaloExtendedImprovement::setEnvironment(Graph& graph) {
    // not needed?
}

void SaloExtendedImprovement::setStart(Partition& startSol) {
    problem->AllocateSolution(startSol.getPvertex(), startSol.getNnode(), startSol.getValue());
}

void SaloExtendedImprovement::disposeEnvironment() {
    // not needed?
}

void SaloExtendedImprovement::calibrateTemp() {
    problem->Calibrate(10000);
}

void SaloExtendedImprovement::search(clock_t startTime, int maxSeconds) {
    problem->SASearch();
}

int SaloExtendedImprovement::getBestObjective() {
    return problem->GetBestSolution();
}

Partition& SaloExtendedImprovement::getBestPartition() {
    int nnode = problem->GetInstance()->getNumberOfNodes();
    Partition *partition = new Partition(nnode);

    int* vpart = new int[nnode];

    for (int i = 0; i < nnode; i++) {
        vpart[i] = problem->GetSolution()->getNodeClique()[i] + 1;
    }

    partition->buildPartition(vpart);

    delete[] vpart;
    return *partition;
}
