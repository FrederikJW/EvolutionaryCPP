// SALOe with node cooling
#ifndef SALOECOOLIMPROVEMENT_H
#define SALOECOOLIMPROVEMENT_H

#include "SaloJovaImprovement.h"
#include "../partition/Partition.h"
#include "../graph/Graph.h"
#include "../Statistic.h"
#include "../CPPJovanovic/CPPSolutionBase.h"
#include "../CPPJovanovic/CPPSolution.h"
#include "../CPPJovanovic/CPPProblem.h"
#include "../CPPJovanovic/CPPInstance.h"
#include <ctime>


class SaloeCoolImprovement : public SaloJovaImprovement {
public:
    SaloeCoolImprovement(int knownbest_, double minpercent_, double tempfactor_, int sizefactor_, Recorder* recorder_, RandomGenerator* generator, bool withSdls) : SaloJovaImprovement(knownbest_, minpercent_, tempfactor_, sizefactor_, recorder_, generator, withSdls) { selectType = SASelectType::SingleEdge; };
    
    void search(clock_t startTime, int maxSeconds, int generation_cnt) override;
    void calibrateTemp() override;
};

#endif // SALOECOOLIMPROVEMENT_H
