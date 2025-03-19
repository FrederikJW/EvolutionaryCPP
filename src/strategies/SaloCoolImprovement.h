// SALO with node cooling
#ifndef SALOCOOLIMPROVEMENT_H
#define SALOCOOLIMPROVEMENT_H

#include "SaloImprovement.h"
#include "../partition/Partition.h"
#include "../graph/Graph.h"
#include "../Statistic.h"
#include "../RandomGenerator.h"
#include <ctime>

class SaloCoolImprovement : public SaloImprovement {
public:
    SaloCoolImprovement(int knownbest_, double minpercent_, double tempfactor_, int sizefactor_, Recorder* recorder_, RandomGenerator* generator, bool withPureDescent) : SaloImprovement(knownbest_, minpercent_, tempfactor_, sizefactor_, recorder_, generator, withPureDescent), cooldown_period(1) {};
    
    void search(clock_t startTime, int maxSeconds, int generation_cnt) override;
    // void search_original(clock_t startTime, int maxSeconds);
    void setEnvironment(Graph& graph) override;
    void calibrateTemp() override;

protected:
    std::vector<int> cooldown_buffer;
    std::vector<int> eligible_nodes;
    int cooldown_period;
};

#endif // SALOCOOLIMPROVEMENT_H
