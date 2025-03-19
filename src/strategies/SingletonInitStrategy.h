// initialization strategy of a partition with singelton cliques
#ifndef SINGLETONINITSTRATEGY_H
#define SINGLETONINITSTRATEGY_H

#include "InitialPoolStrategy.h"
#include "../partition/Partition.h"
#include "../population/Population.h"
#include "../graph/Graph.h"

class SingletonInitStrategy : public InitialPoolStrategy {
public:
    using InitialPoolStrategy::InitialPoolStrategy;

    void buildInitialPool(BestSolutionInfo *frt, Population& population, Graph& graph, ImprovementStrategy* improvementStrategy, int maxSeconds, int *generation_cnt) override;
    void generateInitialSolution(Partition& partition, Graph& graph) override;
};

#endif // SINGLETONINITSTRATEGY_H
