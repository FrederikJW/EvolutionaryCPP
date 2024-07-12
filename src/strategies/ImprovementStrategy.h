#ifndef IMPROVEMENTSTRATEGY_H
#define IMPROVEMENTSTRATEGY_H

#include "../partition/Partition.h"
#include "../graph/Graph.h"
#include "../Statistic.h"
#include <ctime>

class ImprovementStrategy {
public:
    virtual void improveSolution(Partition& solution, clock_t startTime, int maxSeconds, BestSolutionInfo* frt, int generation_cnt) = 0;
    virtual void search(clock_t startTime, int maxSeconds) = 0;
    virtual void setEnvironment(Graph& graph) = 0;
    virtual void setStart(Partition& startSol) = 0;
    virtual void calibrateTemp() = 0;
    virtual void disposeEnvironment() = 0;
    virtual int getBestObjective() = 0;
    virtual Partition& getBestPartition() = 0;
    virtual ~ImprovementStrategy() = default;
};

#endif // IMPROVEMENTSTRATEGY_H
