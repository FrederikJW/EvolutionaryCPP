#include "InitialPoolBuilder.h"
#include "SimulatedAnnealingImprovement.h"
#include <ctime>
#include <cstdlib>
#include <cstdio>

void InitialPoolBuilder::buildInitialPool(BestSolutionInfo *frt, Population& population, Graph& graph, ImprovementStrategy* improvementStrategy, int maxSeconds, int* generation_cnt) {
    clock_t startTime = clock();
    int nnode = graph.getNodeCount();

    Partition childPartition(nnode);

    while (population.partitionCount() < population.getPoolSize()) {
        printf("population size: %d\n", population.partitionCount());
        recorder->enter("build_solution");
        generateInitialSolution(childPartition, graph);
        recorder->exit("build_solution");
        improvementStrategy->improveSolution(childPartition, startTime, maxSeconds, frt, *generation_cnt);

        recorder->recordSolution(frt->best_partition, clock());

        (*generation_cnt)++;
        if ((double)(clock() - startTime) / CLOCKS_PER_SEC >= maxSeconds)
            break;

        population.addPopulation(&(improvementStrategy->getBestPartition()), improvementStrategy->getBestObjective());
    }
    printf("Best/Average value in the init population %d/%.3f\n", population.getMaxObjective(), population.getAverageObjective());
    printf("Average distance in the init population %.3f\n", population.getAvgDistance());
}

void InitialPoolBuilder::generateInitialSolution(Partition& partition, Graph& graph) {
    int nnode = graph.getNodeCount();
    int* initpart = new int[nnode];
    int sum = 0;
    for (int i = 0; i < nnode; i++) {
        initpart[i] = i + 1;
        sum += graph.getMatrix()[i][i];
    }
    partition.buildPartition(initpart);
    partition.setValue(sum);
    delete[] initpart;
}
