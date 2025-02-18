#include "SaloJovaImprovement.h"
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

SaloJovaImprovement::~SaloJovaImprovement() {
    disposeEnvironment();
}

void SaloJovaImprovement::improveSolution(Partition& solution, clock_t startTime, int maxSeconds, BestSolutionInfo* frt, int generation_cnt) {
    recorder->enter("improve_solution");
    setStart(solution);
    search(startTime, maxSeconds, generation_cnt);
    selectBetter(frt, startTime, generation_cnt);
    recorder->exit("improve_solution");
    printf("Child has been raised to by SA %d\n", getBestObjective());
}

void SaloJovaImprovement::setEnvironment(Graph& graph) {
    delete instance;
    instance = new CPPInstance(graph.getNodeCount(), graph.getMatrix());
    delete problem;
    problem = new CPPProblem(instance, mGenerator);
    problem->SetSASelect(selectType);
}

void SaloJovaImprovement::setStart(Partition& startSol) {
    problem->AllocateSolution(startSol.getPvertex(), startSol.getNnode(), startSol.getValue());
}

void SaloJovaImprovement::disposeEnvironment() {
    delete instance;
    delete problem;
}

void SaloJovaImprovement::calibrateTemp() {
    problem->Calibrate(10000);
}

void SaloJovaImprovement::search(clock_t startTime, int maxSeconds, int generation_cnt) {
    problem->SALOSearch();
}

void SaloJovaImprovement::selectBetter(BestSolutionInfo* frt, clock_t start_time, int generation_cnt) {
    if (getBestObjective() > frt->best_val) {
        frt->best_partition->copyPartition(getBestPartition());
        frt->best_val = getBestObjective();
        frt->best_foundtime = (double)(clock() - start_time) / CLOCKS_PER_SEC;
        frt->best_generation = generation_cnt;
        if (frt->best_val >= knownbest) {
            return;
        }
    }
}

int SaloJovaImprovement::getBestObjective() {
    return problem->GetSolution().getObjective();
}

Partition SaloJovaImprovement::getBestPartition() {
    int nnode = problem->GetInstance()->getNumberOfNodes();
    Partition partition(nnode);

    int* vpart = new int[nnode];

    for (int i = 0; i < nnode; i++) {
        // TODO: does this return the actual best solution?
        vpart[i] = problem->GetSolution().getNodeClique()[i] + 1;
    }

    partition.buildPartition(vpart);
    partition.setValue(getBestObjective());

    delete[] vpart;
    return partition;
}
