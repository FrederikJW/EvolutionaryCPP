// pool initialization strategy using a restricted candidate list
#ifndef RCLINITSTRATEGY_H
#define RCLINITSTRATEGY_H

#include "InitialPoolStrategy.h"
#include "../partition/Partition.h"
#include "../population/Population.h"
#include "../graph/Graph.h"
#include "../CPPJovanovic/CPPSolution.h"

class RCLInitStrategy : public InitialPoolStrategy {
public:
    using InitialPoolStrategy::InitialPoolStrategy;

    void buildInitialPool(BestSolutionInfo* frt, Population& population, Graph& graph, ImprovementStrategy* improvementStrategy, int maxSeconds, int* generation_cnt) override;
    void generateInitialSolution(Partition& partition, Graph& graph) override;
    void convertCPPSolutionToPartition(Partition& partition, CPPSolution& solution);
};

#endif // RCLINITSTRATEGY_H
