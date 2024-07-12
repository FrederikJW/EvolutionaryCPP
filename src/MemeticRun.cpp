#include "MemeticRun.h"
#include <ctime>
#include <cassert>
#include <cstdio>

MemeticRun::MemeticRun(CrossoverStrategy* crossoverStrategy, InitialPoolStrategy* initialPoolStrategy, ImprovementStrategy* improvementStrategy, int maxGenerations, int maxSeconds) : crossoverStrategy(crossoverStrategy), initialPoolStrategy(initialPoolStrategy), improvementStrategy(improvementStrategy), maxGenerations(maxGenerations), maxSeconds(maxSeconds) {}

void MemeticRun::run(BestSolutionInfo* frt, Graph& graph, int* totalGen, int poolSize) {
    // TODO: move this param somewehere else
    int param_knownbest = 309125;
    
    double bestTime = 0.0;
    clock_t startTime = clock();
    int generationCnt = 0;

    // TODO: could be created inside initial pool builder
    Population population(poolSize);
    Partition childPartition(graph.getNodeCount());

    improvementStrategy->setEnvironment(graph);

    // calibrating initial temperature
    printf("Calibrate the initial temperature.\n");
    initialPoolStrategy->generateInitialSolution(childPartition, graph);
    improvementStrategy->setStart(childPartition);
    improvementStrategy->calibrateTemp();

    printf("Build initial pool.\n");
    initialPoolStrategy->buildInitialPool(frt, population, graph, improvementStrategy, maxSeconds);

    printf("Run evolution.\n");
    while (generationCnt < maxGenerations && (double)(clock() - startTime) / CLOCKS_PER_SEC < maxSeconds) {
        runGeneration(frt, graph, population, childPartition, startTime, bestTime, generationCnt);

        if (frt->best_val >= param_knownbest) {
            break;
        }

        generationCnt++;
    }
    *totalGen = generationCnt;
    improvementStrategy->disposeEnvironment();
    printf("Best solution found with value %d at generation %d\n", frt->best_val, frt->best_generation);
}

void MemeticRun::runGeneration(BestSolutionInfo* frt, Graph& graph, Population& population, Partition& childPartition, clock_t startTime, double& bestTime, int& generationCnt) {
    printf("\n------------The %dth generation-----------\n", generationCnt);

    double param_shrink = 0.6;
    
    int poolSize = population.getPoolSize();
    int idx1 = rand() % poolSize;
    int idx2 = rand() % poolSize;
    while (idx1 == idx2) {
        idx2 = rand() % poolSize;
    }

    crossoverStrategy->crossover(graph, population.getPartition(idx1), population.getPartition(idx2), childPartition);

    improvementStrategy->improveSolution(childPartition, startTime, maxSeconds, frt, generationCnt);

    population.addPopulation(&(improvementStrategy->getBestPartition()), improvementStrategy->getBestObjective());
    printf("Generation %d, best value: %d, avg value: %.2f, min distance: %d\n",
        generationCnt, population.getMaxObjective(), population.getAverageObjective(), population.getMinDistance());
}
