#ifndef EVOLUTIONSTRATEGY_H
#define EVOLUTIONSTRATEGY_H

#include "CrossoverStrategy.h"
#include "InitialPoolStrategy.h"
#include "ImprovementStrategy.h"
#include "../Recorder.h"
#include "../RandomGenerator.h"

class EvolutionStrategy {
public:
    EvolutionStrategy(CrossoverStrategy* crossoverStrategy_, InitialPoolStrategy* initialPoolStrategy_, ImprovementStrategy* improvementStrategy_, Graph* graph_, Recorder* recorder_, int maxGenerations_, int maxSeconds_, RandomGenerator* generator) :
        crossoverStrategy(crossoverStrategy_), initialPoolStrategy(initialPoolStrategy_), improvementStrategy(improvementStrategy_), population(nullptr), childPartition(nullptr), graph(graph_), recorder(recorder_), frt(nullptr), maxGenerations(maxGenerations_), maxSeconds(maxSeconds_), mGenerator(generator) {};

    virtual void run(BestSolutionInfo* frt_, int* totalGen, int poolSize) = 0;
    virtual void runGeneration() = 0;
    virtual ~EvolutionStrategy() = default;
protected:
    int maxGenerations;
    int maxSeconds;
    int generationCnt;
    clock_t startTime;
    double bestTime;

    Partition* childPartition;
    Population* population;
    Graph* graph;
    Recorder* recorder;
    BestSolutionInfo* frt;
    RandomGenerator* mGenerator;

    CrossoverStrategy* crossoverStrategy;
    InitialPoolStrategy* initialPoolStrategy;
    ImprovementStrategy* improvementStrategy;
};

#endif // EVOLUTIONSTRATEGY_H
