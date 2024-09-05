#include "SolutionEvolution.h"

void SolutionEvolution::run(BestSolutionInfo* frt_, int* totalGen, int poolSize) {
    frt = frt_;
    bestTime = 0.0;
    startTime = clock();
    generationCnt = 0;

    // TODO: could be created inside initial pool builder
    population = new Population(poolSize);
    childPartition = new Partition(graph->getNodeCount());

    improvementStrategy->setEnvironment(*graph);

    // calibrating initial temperature
    printf("Calibrate the initial temperature.\n");
    initialPoolStrategy->generateInitialSolution(*childPartition, *graph);
    improvementStrategy->setStart(*childPartition);
    improvementStrategy->calibrateTemp();

    printf("Build initial pool.\n");
    initialPoolStrategy->buildInitialPool(frt, *population, *graph, improvementStrategy, maxSeconds, &generationCnt);

    printf("Run evolution.\n");
    while (generationCnt < maxGenerations && (double)(clock() - startTime) / CLOCKS_PER_SEC < maxSeconds && frt->best_val < graph->getKnownbest()) {
        runGeneration();

        generationCnt++;
    }
    *totalGen = generationCnt;
    improvementStrategy->disposeEnvironment();
    printf("Best solution found with value %d at generation %d\n", frt->best_val, frt->best_generation);
}

void SolutionEvolution::runGeneration() {
    printf("\n------------The %dth generation-----------\n", generationCnt);

    double param_shrink = 0.6;

    int poolSize = population->getPoolSize();
    int idx1 = rand() % poolSize;
    int idx2 = rand() % poolSize;
    while (idx1 == idx2) {
        idx2 = rand() % poolSize;
    }

    crossoverStrategy->crossover(*graph, population->getPartition(idx1), population->getPartition(idx2), *childPartition);

    improvementStrategy->improveSolution(*childPartition, startTime, maxSeconds, frt, generationCnt);

    recorder->recordSolution(childPartition, clock());

    population->addPopulation(&(improvementStrategy->getBestPartition()), improvementStrategy->getBestObjective());
    printf("Generation %d, best value: %d, avg value: %.2f, min distance: %d\n",
        generationCnt, population->getMaxObjective(), population->getAverageObjective(), population->getMinDistance());
}
