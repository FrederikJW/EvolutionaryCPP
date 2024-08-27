#ifndef FIXEDSETEVOLUTION_H
#define FIXEDSETEVOLUTION_H

#include "EvolutionStrategy.h"
#include "../CPPJovanovic/CPPProblem.h"

class FixedSetEvolution : public EvolutionStrategy {
public:
    using EvolutionStrategy::EvolutionStrategy;

    FixedSetEvolution(CrossoverStrategy* crossoverStrategy_, InitialPoolStrategy* initialPoolStrategy_, ImprovementStrategy* improvementStrategy_, Graph* graph_, int maxGenerations_, int maxSeconds_) : 
        EvolutionStrategy(crossoverStrategy_, initialPoolStrategy_, improvementStrategy_, graph_, maxGenerations_, maxSeconds_), problem(nullptr) {}
    void run(BestSolutionInfo* frt_, int* totalGen, int poolSize) override;
    void runGeneration() override;
private:
    CPPProblem* problem;
};

#endif // FIXEDSETEVOLUTION_H
