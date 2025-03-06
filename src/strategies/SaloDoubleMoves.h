#ifndef SALOODOUBLEMOVES_H
#define SALOODOUBLEMOVES_H

#include "SaloJovaImprovement.h"
#include "../partition/Partition.h"
#include "../graph/Graph.h"
#include "../Statistic.h"
#include "../CPPJovanovic/CPPSolutionBase.h"
#include "../CPPJovanovic/CPPSolution.h"
#include "../CPPJovanovic/CPPProblem.h"
#include "../CPPJovanovic/CPPInstance.h"
#include <ctime>


class SaloDoubleMoves : public SaloJovaImprovement {
public:
    SaloDoubleMoves(int knownbest_, double minpercent_, double tempfactor_, int sizefactor_, Recorder* recorder_, RandomGenerator* generator, bool withSdls) : SaloJovaImprovement(knownbest_, minpercent_, tempfactor_, sizefactor_, recorder_, generator, withSdls) { selectType = SASelectType::SingleEdge; };

    void setEnvironment(Graph& graph) override;
};

#endif // SALOODOUBLEMOVES_H
