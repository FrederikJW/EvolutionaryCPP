#ifndef SOLUTIONEVOLUTION_H
#define SOLUTIONEVOLUTION_H

#include "EvolutionStrategy.h"
#include "CrossoverStrategy.h"
#include "InitialPoolStrategy.h"
#include "ImprovementStrategy.h"

class SolutionEvolution : public EvolutionStrategy {
public:
    using EvolutionStrategy::EvolutionStrategy;

    void run(BestSolutionInfo* frt_, int* totalGen, int poolSize) override;
    void runGeneration() override;

};

#endif // SOLUTIONEVOLUTION_H
