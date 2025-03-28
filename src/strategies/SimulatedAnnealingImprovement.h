#ifndef SIMULATEDANNEALINGIMPROVEMENT_H
#define SIMULATEDANNEALINGIMPROVEMENT_H

#include "ImprovementStrategy.h"
#include "../partition/Partition.h"
#include "../graph/Graph.h"
#include "../Statistic.h"
#include "../RandomGenerator.h"
#include <ctime>

typedef struct SA_RT_Data {
    const int* const* gmatrix;
    int gnnode;

    Partition* ppt;      /*current solution*/
    int fcurrent; 	         /*The current objective function*/

    Partition* ppt_best; /*best solution*/
    int fbest;	             /*The ever found best solution*/

    int itr;
    int totalIter;

    int** gammatbl;
}SA_RT_Data;

class SimulatedAnnealingImprovement : public ImprovementStrategy {
public:
    SimulatedAnnealingImprovement(int knownbest_, double minpercent_, double tempfactor_, int sizefactor_, Recorder* recorder_, RandomGenerator* generator, bool withPureDescent) : ImprovementStrategy(knownbest_, minpercent_, tempfactor_, sizefactor_, recorder_, generator), lsdata(nullptr), withPureDescent(withPureDescent) {};
    ~SimulatedAnnealingImprovement();

    void improveSolution(Partition& solution, clock_t startTime, int maxSeconds, BestSolutionInfo* frt, int generation_cnt) override;
    void search(clock_t startTime, int maxSeconds, int generation_cnt) override;
    // void search_original(clock_t startTime, int maxSeconds);
    void setEnvironment(Graph& graph) override;
    void setStart(Partition& startSol) override;
    void calibrateTemp() override;
    void disposeEnvironment() override;    
    int getBestObjective() override;
    Partition getBestPartition() override;

    void selectBetter(BestSolutionInfo* frt, clock_t start_time, int generation_cnt);
    double fastExp(double x);

private:
    void pureDescent();
    void buildCurGamma(const int* pvertex);
    void updateCurGamma(int u, int src, int dest);
    int decideTarget(int dest);
    int changeCurSolution(int u, int dest);

    SA_RT_Data* lsdata;
    bool withPureDescent;
};

#endif // SIMULATEDANNEALINGIMPROVEMENT_H
