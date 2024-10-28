#ifndef WEIGHTED_RANDOM_SAMPLER_H
#define WEIGHTED_RANDOM_SAMPLER_H

#include <vector>
#include <random>

class WeightedRandomSampler {
public:
    // Constructor that takes a vector of integer weights
    explicit WeightedRandomSampler(const std::vector<int>& weights);

    // Function to retrieve a random index
    int sample();

private:
    // Helper function to build the alias table
    void buildAliasTable(const std::vector<double>& probabilities);

    // Member variables
    std::vector<double> prob;  // Probability table
    std::vector<int> alias;    // Alias table
    int N;                     // Number of weights
    double total_weight;       // Sum of all weights
    std::mt19937 generator;    // Random number generator
};

#endif // WEIGHTED_RANDOM_SAMPLER_H
