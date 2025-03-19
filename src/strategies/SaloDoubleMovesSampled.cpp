// SALO with double moves and sampled edges; extension to SALOe
#include "SaloDoubleMovesSampled.h"
#include "../Defines.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <vector>
#include <unordered_map>

void SaloDoubleMovesSampled::setEnvironment(Graph& graph) {
    delete instance;
    instance = new CPPInstance(graph.getNodeCount(), graph.getMatrix(), false, true);
    delete problem;
    problem = new CPPProblem(instance, mGenerator);
    problem->SetSASelect(selectType);
    problem->setNeighborhoodFactor(2);
}
