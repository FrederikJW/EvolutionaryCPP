// merge divide crossover evolution strategy
#ifndef MDXEVOLUTION_H
#define MDXEVOLUTION_H

#include "EvolutionStrategy.h"
#include "CrossoverStrategy.h"
#include "InitialPoolStrategy.h"
#include "ImprovementStrategy.h"

class MdxEvolution : public EvolutionStrategy {
public:
    using EvolutionStrategy::EvolutionStrategy;

    void run(BestSolutionInfo* frt_, int* totalGen, int poolSize) override;
    void runGeneration() override;

};

#endif // MDX_H
