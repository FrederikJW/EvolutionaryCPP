// SALO with double moves and filtered 0 edges; extension to SALOe
#include "SaloDoubleMovesZero.h"
#include "../Defines.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <vector>
#include <unordered_map>

void SaloDoubleMovesZero::setEnvironment(Graph& graph) {
    delete instance;
    instance = new CPPInstance(graph.getNodeCount(), graph.getMatrix(), true, false);
    delete problem;
    problem = new CPPProblem(instance, mGenerator);
    problem->SetSASelect(selectType);
    problem->setNeighborhoodFactor(2);
}
