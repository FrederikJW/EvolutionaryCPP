#ifndef IMPROVEMENTSTRATEGY_H
#define IMPROVEMENTSTRATEGY_H

#include "../partition/Partition.h"
#include "../graph/Graph.h"
#include "../Statistic.h"
#include "../Recorder.h"
#include "../RandomGenerator.h"
#include <random>
#include <ctime>

class ImprovementStrategy {
public:
    ImprovementStrategy(int knownbest_, double minpercent_, double tempfactor_, int sizefactor_, Recorder* recorder_, RandomGenerator* generator) : knownbest(knownbest_), minpercent(minpercent_), tempfactor(tempfactor_), sizefactor(sizefactor_), recorder(recorder_), temp(0), mGenerator(generator) {};

    virtual void improveSolution(Partition& solution, clock_t startTime, int maxSeconds, BestSolutionInfo* frt, int generation_cnt) = 0;
    virtual void search(clock_t startTime, int maxSeconds) = 0;
    virtual void setEnvironment(Graph& graph) = 0;
    virtual void setStart(Partition& startSol) = 0;
    virtual void calibrateTemp() = 0;
    virtual void disposeEnvironment() = 0;
    virtual int getBestObjective() = 0;
    virtual Partition getBestPartition() = 0;
    virtual ~ImprovementStrategy() = default;
protected:
    Recorder* recorder;
    RandomGenerator* mGenerator;
    double temp;
    int knownbest;
    double minpercent;
    double tempfactor;
    int sizefactor;
};

#endif // IMPROVEMENTSTRATEGY_H
