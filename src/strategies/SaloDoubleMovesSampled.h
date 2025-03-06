#ifndef SALOODOUBLEMOVESSAMPLED_H
#define SALOODOUBLEMOVESSAMPLED_H

#include "SaloDoubleMoves.h"
#include "../partition/Partition.h"
#include "../graph/Graph.h"
#include "../Statistic.h"
#include "../CPPJovanovic/CPPSolutionBase.h"
#include "../CPPJovanovic/CPPSolution.h"
#include "../CPPJovanovic/CPPProblem.h"
#include "../CPPJovanovic/CPPInstance.h"
#include <ctime>


class SaloDoubleMovesSampled : public SaloDoubleMoves {
public:
    using SaloDoubleMoves::SaloDoubleMoves;

    void setEnvironment(Graph& graph) override;
};

#endif // SALOODOUBLEMOVESSAMPLED_H
