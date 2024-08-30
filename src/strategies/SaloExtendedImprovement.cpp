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

SaloExtendedImprovement::SaloExtendedImprovement(int knownbest, double minpercent, double tempfactor, int sizefactor) : knownbest(knownbest), minpercent(minpercent), tempfactor(tempfactor), sizefactor(sizefactor), temp(0), problem(nullptr), instance(nullptr) {
    
}

SaloExtendedImprovement::~SaloExtendedImprovement() {
    disposeEnvironment();
}

void SaloExtendedImprovement::improveSolution(Partition& solution, clock_t startTime, int maxSeconds, BestSolutionInfo* frt, int generation_cnt) {
    setStart(solution);
    search(startTime, maxSeconds);
    selectBetter(frt, startTime, generation_cnt);
    printf("Child has been raised to by SA %d\n", frt->best_val);
}

void SaloExtendedImprovement::setEnvironment(Graph& graph) {
    instance = new CPPInstance(graph.getNodeCount(), graph.getMatrix());
    problem = new CPPProblem("file1", "rand500-100", instance);
    problem->SetID(2);
    problem->SetSASelect(SASelectType::Dual);
}

void SaloExtendedImprovement::setStart(Partition& startSol) {
    problem->AllocateSolution(startSol.getPvertex(), startSol.getNnode(), startSol.getValue());
}

void SaloExtendedImprovement::disposeEnvironment() {
    delete instance;
    delete problem;
}

void SaloExtendedImprovement::calibrateTemp() {
    problem->Calibrate(10000);
}

void SaloExtendedImprovement::search(clock_t startTime, int maxSeconds) {
    problem->SASearch();
}

void SaloExtendedImprovement::selectBetter(BestSolutionInfo* frt, clock_t start_time, int generation_cnt) {
    if (getBestObjective() > frt->best_val) {
        Partition* bestPartition = &getBestPartition();
        frt->best_partition->copyPartition(*bestPartition);
        delete bestPartition;
        frt->best_val = getBestObjective();
        frt->best_foundtime = (double)(clock() - start_time) / CLOCKS_PER_SEC;
        frt->best_generation = generation_cnt;
        if (frt->best_val >= knownbest) {
            return;
        }
    }
}

int SaloExtendedImprovement::getBestObjective() {
    return problem->GetSolution().getObjective();
}

Partition& SaloExtendedImprovement::getBestPartition() {
    int nnode = problem->GetInstance()->getNumberOfNodes();
    Partition *partition = new Partition(nnode);

    int* vpart = new int[nnode];

    for (int i = 0; i < nnode; i++) {
        // TODO: does this return the actual best solution?
        vpart[i] = problem->GetSolution().getNodeClique()[i] + 1;
    }

    partition->buildPartition(vpart);
    partition->setValue(getBestObjective());

    delete[] vpart;
    return *partition;
}
