// implementation of the a weighted random sampler based on Vose's Alias method [3]
// used for SALODMS 
#ifndef WEIGHTED_RANDOM_SAMPLER_H
#define WEIGHTED_RANDOM_SAMPLER_H

#include <vector>
#include <random>
#include "Util.h"

class WeightedRandomSampler {
public:
    explicit WeightedRandomSampler(const std::vector<int>& weights);

    int sample();

private:
    void buildAliasTable(const std::vector<double>& probabilities);

    std::vector<double> prob;  // Probability table
    std::vector<int> alias;    // Alias table
    int N;                     // Number of weights
    double total_weight;       // Sum of all weights
    std::mt19937_64 generator;    // Random number generator
};

#endif // WEIGHTED_RANDOM_SAMPLER_H
