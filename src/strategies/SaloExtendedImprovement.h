// SALOe by [2]
#ifndef SALOEXTENDEDIMPROVEMENT_H
#define SALOEXTENDEDIMPROVEMENT_H

#include "SaloJovaImprovement.h"
#include "../partition/Partition.h"
#include "../graph/Graph.h"
#include "../Statistic.h"
#include "../CPPJovanovic/CPPSolutionBase.h"
#include "../CPPJovanovic/CPPSolution.h"
#include "../CPPJovanovic/CPPProblem.h"
#include "../CPPJovanovic/CPPInstance.h"
#include <ctime>


class SaloExtendedImprovement : public SaloJovaImprovement {
public:
    SaloExtendedImprovement(int knownbest_, double minpercent_, double tempfactor_, int sizefactor_, Recorder* recorder_, RandomGenerator* generator, bool withSdls) : SaloJovaImprovement(knownbest_, minpercent_, tempfactor_, sizefactor_, recorder_, generator, withSdls) { selectType = SASelectType::Dual; };
};

#endif // SALOEXTENDEDIMPROVEMENT_H
