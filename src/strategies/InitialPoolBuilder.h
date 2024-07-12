#ifndef INITIALPOOLBUILDER_H
#define INITIALPOOLBUILDER_H

#include "InitialPoolStrategy.h"
#include "../partition/Partition.h"
#include "../population/Population.h"
#include "../graph/Graph.h"

class InitialPoolBuilder : public InitialPoolStrategy {
public:
    InitialPoolBuilder();
    void buildInitialPool(BestSolutionInfo *frt, Population& population, Graph& graph, ImprovementStrategy* improvementStrategy, int maxSeconds) override;
    void generateInitialSolution(Partition& partition, Graph& graph) override;
};

#endif // INITIALPOOLBUILDER_H
