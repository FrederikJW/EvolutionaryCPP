#ifndef SALOODOUBLEMOVESZERO_H
#define SALOODOUBLEMOVESZERO_H

#include "SaloDoubleMoves.h"
#include "../partition/Partition.h"
#include "../graph/Graph.h"
#include "../Statistic.h"
#include "../CPPJovanovic/CPPSolutionBase.h"
#include "../CPPJovanovic/CPPSolution.h"
#include "../CPPJovanovic/CPPProblem.h"
#include "../CPPJovanovic/CPPInstance.h"
#include <ctime>


class SaloDoubleMovesZero : public SaloDoubleMoves {
public:
    using SaloDoubleMoves::SaloDoubleMoves;

    void setEnvironment(Graph& graph) override;
};

#endif // SALOODOUBLEMOVESZERO_H
