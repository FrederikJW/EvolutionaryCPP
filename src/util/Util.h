#ifndef UTIL_H
#define UTIL_H

#include <string>
#include "../partition/Partition.h"
#include "../graph/Graph.h"

struct Parameters {
    std::string filename;
    int knownBest;
    int time;
    int seed;
    int maxGenerations;
    int sizeFactor;
    double tempFactor;
    double minPercent;
    double shrink;
    int poolSize;
};

void verifySolution(Partition* partition, Graph* graph);

double FastExp(double x);

#endif // UTIL_H
