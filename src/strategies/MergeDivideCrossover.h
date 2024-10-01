#ifndef MERGEDIVIDECROSSOVER_H
#define MERGEDIVIDECROSSOVER_H

#include "CrossoverStrategy.h"
#include "../partition/Partition.h"

class MergeDivideCrossover : public CrossoverStrategy {
public:
    MergeDivideCrossover(float shrink);
    void crossover(Graph& graph, const Partition& parent1, const Partition& parent2, Partition& child, std::mt19937* generator) override;

private:
    float shrink;
    void cliqueCover(int** weMat, int** conMat, int n, int* regrp, int* weight);
};

#endif // MERGEDIVIDECROSSOVER_H
