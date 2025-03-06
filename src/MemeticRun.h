#ifndef MEMETICRUN_H
#define MEMETICRUN_H

#include "population/Population.h"
#include "partition/Partition.h"
#include "strategies/CrossoverStrategy.h"
#include "strategies/InitialPoolStrategy.h"
#include "strategies/ImprovementStrategy.h"
#include "graph/Graph.h"
#include "Statistic.h"
#include <ctime>

class MemeticRun {
public:
    MemeticRun(CrossoverStrategy* crossoverStrategy, InitialPoolStrategy* initialPoolStrategy, ImprovementStrategy* improvementStrategy, int maxGenerations, int maxSeconds);
    void run(BestSolutionInfo* frt, Graph& graph, int* totalGen, int poolSize);

private:
    int maxGenerations;
    int maxSeconds;
    CrossoverStrategy* crossoverStrategy;
    InitialPoolStrategy* initialPoolStrategy;
    ImprovementStrategy* improvementStrategy;

    void runGeneration(BestSolutionInfo* frt, Graph& graph, Population& population, Partition& childPartition, clock_t startTime, double& bestTime, int& generationCnt);
};

#endif // MEMETICRUN_H
