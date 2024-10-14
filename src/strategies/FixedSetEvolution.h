#ifndef FIXEDSETEVOLUTION_H
#define FIXEDSETEVOLUTION_H

#include "EvolutionStrategy.h"
#include "../CPPJovanovic/CPPProblem.h"

class FixedSetEvolution : public EvolutionStrategy {
public:
    using EvolutionStrategy::EvolutionStrategy;

    FixedSetEvolution(CrossoverStrategy* crossoverStrategy_, InitialPoolStrategy* initialPoolStrategy_, ImprovementStrategy* improvementStrategy_, Graph* graph_, Recorder* recorder_, int maxGenerations_, int maxSeconds_, RandomGenerator* generator) :
        EvolutionStrategy(crossoverStrategy_, initialPoolStrategy_, improvementStrategy_, graph_, recorder_, maxGenerations_, maxSeconds_, generator), problem(nullptr) {}
    ~FixedSetEvolution();
    void run(BestSolutionInfo* frt_, int* totalGen, int poolSize) override;
    void runGeneration() override;
private:
    CPPProblem* problem;
};

#endif // FIXEDSETEVOLUTION_H
