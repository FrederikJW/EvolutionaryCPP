#include "InitialPoolBuilder.h"
#include "ImprovementStrategy.h"
#include "RCLInitStrategy.h"
#include "../CPPJovanovic/CPPGreedy.h"
#include "../CPPJovanovic/CPPInstance.h"
#include <ctime>
#include <cstdlib>
#include <cstdio>

RCLInitStrategy::RCLInitStrategy() {}

void RCLInitStrategy::buildInitialPool(BestSolutionInfo* frt, Population& population, Graph& graph, ImprovementStrategy* improvementStrategy, int maxSeconds, int* generation_cnt) {
    clock_t startTime = clock();
    int nnode = graph.getNodeCount();

    Partition childPartition(nnode);

    CPPInstance* instance = new CPPInstance(nnode, graph.getMatrix());;
    CPPGreedy* problem = new CPPGreedy(instance);

    while (population.partitionCount() < population.getPoolSize()) {
        printf("population size: %d\n", population.partitionCount());
        problem->SolveGreedy();

        // convert to partition
        convertCPPSolutionToPartition(childPartition, problem->getSolution());
        
        improvementStrategy->improveSolution(childPartition, startTime, maxSeconds, frt, *generation_cnt);
        (*generation_cnt)++;
        if ((double)(clock() - startTime) / CLOCKS_PER_SEC >= maxSeconds)
            break;
        population.addPopulation(&(improvementStrategy->getBestPartition()), improvementStrategy->getBestObjective());
    }
    
    printf("Best/Average value in the init population %d/%.3f\n", population.getMaxObjective(), population.getAverageObjective());

    /*
    clock_t startTime = clock();
    int nnode = graph.getNodeCount();

    Partition childPartition(nnode);

    while (population.partitionCount() < population.getPoolSize()) {
        printf("population size: %d\n", population.partitionCount());
        generateInitialSolution(childPartition, graph);
        improvementStrategy->improveSolution(childPartition, startTime, maxSeconds, frt, *generation_cnt);
        (*generation_cnt)++;
        if ((double)(clock() - startTime) / CLOCKS_PER_SEC >= maxSeconds)
            break;

        population.addPopulation(&(improvementStrategy->getBestPartition()), improvementStrategy->getBestObjective());
    }
    */
}

void RCLInitStrategy::convertCPPSolutionToPartition(Partition& partition, CPPSolution& solution) {
    int nnode = solution.getNodes().size();
    int* initpart = new int[nnode];

    for (int i = 0; i < nnode; i++) {
        // +1 because 0 is the non existing clique
        initpart[i] = solution.getNodes()[i] + 1;
    }

    partition.buildPartition(initpart);
    partition.setValue(solution.getObjective());
    delete[] initpart;
}

void RCLInitStrategy::generateInitialSolution(Partition& partition, Graph& graph) {
    CPPInstance* instance = new CPPInstance(graph.getNodeCount(), graph.getMatrix());
    CPPGreedy* problem = new CPPGreedy(instance);

    problem->SolveGreedy();

    convertCPPSolutionToPartition(partition, problem->getSolution());
}
