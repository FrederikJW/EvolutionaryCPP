#ifndef INITIALPOOLSTRATEGY_H
#define INITIALPOOLSTRATEGY_H

#include "../population/Population.h"
#include "../Statistic.h"
#include "ImprovementStrategy.h"

class InitialPoolStrategy {
public:
    virtual void buildInitialPool(BestSolutionInfo* frt, Population& population, Graph& graph, ImprovementStrategy* improvementStrategy, int maxSeconds, int* generation_cnt) = 0;
    virtual void generateInitialSolution(Partition& partition, Graph& graph) = 0;
    virtual ~InitialPoolStrategy() = default;
};

#endif // INITIALPOOLSTRATEGY_H
