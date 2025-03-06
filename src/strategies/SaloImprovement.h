#ifndef SALOIMPROVEMENT_H
#define SALOIMPROVEMENT_H

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

class SaloImprovement : public ImprovementStrategy {
public:
    SaloImprovement(int knownbest_, double minpercent_, double tempfactor_, int sizefactor_, Recorder* recorder_, RandomGenerator* generator, bool withPureDescent) : ImprovementStrategy(knownbest_, minpercent_, tempfactor_, sizefactor_, recorder_, generator), lsdata(nullptr), withPureDescent(withPureDescent) {};
    virtual ~SaloImprovement();

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
    double fastExpSafe(double x);

protected:
    void pureDescent();
    void buildCurGamma(const int* pvertex);
    void updateCurGamma(int u, int src, int dest);
    int decideTarget(int dest);
    int changeCurSolution(int u, int dest);

    SA_RT_Data* lsdata;
    bool withPureDescent;
    std::uniform_real_distribution<double> uni_dist;
    std::uniform_int_distribution<int> node_dist;
};

#endif // SALOIMPROVEMENT_H
