// merge divide crossover evolution strategy
#include "MdxEvolution.h"

void MdxEvolution::run(BestSolutionInfo* frt_, int* totalGen, int poolSize) {
    frt = frt_;
    bestTime = 0.0;
    startTime = clock();
    generationCnt = 0;

    delete population;
    population = new Population(poolSize);
    delete childPartition;
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
        recorder->enter("run_generation");
        runGeneration();
        recorder->exit("run_generation");

        generationCnt++;
    }
    *totalGen = generationCnt;
    printf("Best solution found with value %d at generation %d\n", frt->best_val, frt->best_generation);
}

void MdxEvolution::runGeneration() {
    printf("\n------------The %dth generation-----------\n", generationCnt);

    double param_shrink = 0.6;

    int poolSize = population->getPoolSize();
    int idx1 = (*mGenerator)() % poolSize;
    int idx2 = (*mGenerator)() % poolSize;
    while (idx1 == idx2) {
        idx2 = (*mGenerator)() % poolSize;
    }

    crossoverStrategy->crossover(*graph, population->getPartition(idx1), population->getPartition(idx2), *childPartition, mGenerator);

    improvementStrategy->improveSolution(*childPartition, startTime, maxSeconds, frt, generationCnt);

    recorder->recordSolution(frt->best_partition, clock());

    population->addPopulation(improvementStrategy->getBestPartition(), improvementStrategy->getBestObjective());
    printf("Generation %d, best value: %d, avg value: %.2f, min distance: %d\n",
        generationCnt, population->getMaxObjective(), population->getAverageObjective(), population->getMinDistance());
}
