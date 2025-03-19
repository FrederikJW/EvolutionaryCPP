// experimental pre version of SALODM
#ifndef SALOOVEREDGESFORCEDDUALIMPROVEMENT_H
#define SALOOVEREDGESFORCEDDUALIMPROVEMENT_H

#include "ImprovementStrategy.h"
#include "../partition/Partition.h"
#include "../graph/Graph.h"
#include "../Statistic.h"
#include "../CPPJovanovic/CPPSolutionBase.h"
#include "../CPPJovanovic/CPPSolution.h"
#include "../CPPJovanovic/CPPProblem.h"
#include "../CPPJovanovic/CPPInstance.h"
#include <ctime>


class SaloOverEdgesForcedDualImprovement : public ImprovementStrategy {
public:
    SaloOverEdgesForcedDualImprovement(int knownbest_, double minpercent_, double tempfactor_, int sizefactor_, Recorder* recorder_, RandomGenerator* generator) : ImprovementStrategy(knownbest_, minpercent_, tempfactor_, sizefactor_, recorder_, generator), problem(nullptr), instance(nullptr), selectType(SASelectType::SingleEdgeForcedDual) {};
    ~SaloOverEdgesForcedDualImprovement();

    void improveSolution(Partition& solution, clock_t startTime, int maxSeconds, BestSolutionInfo* frt, int generation_cnt) override;
    void search(clock_t startTime, int maxSeconds, int generation_cnt) override;
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
    SASelectType selectType;
};

#endif // SALOOVEREDGESFORCEDDUALIMPROVEMENT_H
