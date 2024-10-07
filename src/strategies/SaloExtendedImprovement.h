#ifndef SALOEXTENDEDIMPROVEMENT_H
#define SALOEXTENDEDIMPROVEMENT_H

#include "ImprovementStrategy.h"
#include "../partition/Partition.h"
#include "../graph/Graph.h"
#include "../Statistic.h"
#include "../CPPJovanovic/CPPSolutionBase.h"
#include "../CPPJovanovic/CPPSolution.h"
#include "../CPPJovanovic/CPPProblem.h"
#include "../CPPJovanovic/CPPInstance.h"
#include <ctime>


class SaloExtendedImprovement : public ImprovementStrategy {
public:
    SaloExtendedImprovement(int knownbest_, double minpercent_, double tempfactor_, int sizefactor_, Recorder* recorder_, std::mt19937* generator) : ImprovementStrategy(knownbest_, minpercent_, tempfactor_, sizefactor_, recorder_, generator), problem(nullptr), instance(nullptr) {};
    ~SaloExtendedImprovement();

    void improveSolution(Partition& solution, clock_t startTime, int maxSeconds, BestSolutionInfo* frt, int generation_cnt) override;
    void search(clock_t startTime, int maxSeconds) override;
    void selectBetter(BestSolutionInfo* frt, clock_t start_time, int generation_cnt);
    void setEnvironment(Graph& graph) override;
    void setStart(Partition& startSol) override;
    void calibrateTemp() override;
    void disposeEnvironment() override;
    int getBestObjective() override;
    Partition getBestPartition() override;

private:

    CPPProblem* problem;
    CPPInstance* instance;
};

#endif // SALOEXTENDEDIMPROVEMENT_H
