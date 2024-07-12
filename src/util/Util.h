#ifndef UTIL_H
#define UTIL_H

#include <string>

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

#endif // UTIL_H
