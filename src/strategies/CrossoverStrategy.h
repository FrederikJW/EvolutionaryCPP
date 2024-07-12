#ifndef CROSSOVERSTRATEGY_H
#define CROSSOVERSTRATEGY_H

#include "../partition/Partition.h"
#include "../graph/Graph.h"

class CrossoverStrategy {
public:
    virtual void crossover(Graph& graph, const Partition& parent1, const Partition& parent2, Partition& child) = 0;
    virtual ~CrossoverStrategy() = default;
};

#endif // CROSSOVERSTRATEGY_H
