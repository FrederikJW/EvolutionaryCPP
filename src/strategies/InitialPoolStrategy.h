#ifndef INITIALPOOLSTRATEGY_H
#define INITIALPOOLSTRATEGY_H

#include "../population/Population.h"
#include "../Statistic.h"
#include "../Recorder.h"
#include "ImprovementStrategy.h"

class InitialPoolStrategy {
public:
    InitialPoolStrategy(Recorder* recorder_, std::mt19937* generator) : recorder(recorder_), mGenerator(generator) {};
    virtual void buildInitialPool(BestSolutionInfo* frt, Population& population, Graph& graph, ImprovementStrategy* improvementStrategy, int maxSeconds, int* generation_cnt) = 0;
    virtual void generateInitialSolution(Partition& partition, Graph& graph) = 0;
    virtual ~InitialPoolStrategy() = default;

protected:
    Recorder* recorder;
    std::mt19937* mGenerator;
};

#endif // INITIALPOOLSTRATEGY_H
