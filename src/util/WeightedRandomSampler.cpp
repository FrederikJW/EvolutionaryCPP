// implementation of the a weighted random sampler based on Vose's Alias method [3]
// used for SALODMS 
#include <numeric>
#include "WeightedRandomSampler.h"

WeightedRandomSampler::WeightedRandomSampler(const std::vector<int>& weights)
    : generator(std::random_device{}()), N(weights.size()), total_weight(0.0) {

    double T = 20;

    std::vector<double> exp_weights;
    for (double w : weights) {
        exp_weights.push_back(FastExp(w / T));
    }

    double sum_exp_weights = std::accumulate(exp_weights.begin(), exp_weights.end(), 0.0);

    // Compute probabilities
    std::vector<double> probabilities;
    for (double exp_weight : exp_weights) {
        probabilities.push_back(exp_weight / sum_exp_weights);
    }
    
    // Build the alias table
    buildAliasTable(probabilities);
}

void WeightedRandomSampler::buildAliasTable(const std::vector<double>& probabilities) {
    prob.resize(N);
    alias.resize(N);
    std::vector<double> scaled_probabilities(N);
    std::vector<int> small, large;

    // Scale probabilities by N and categorize them
    for (int i = 0; i < N; ++i) {
        scaled_probabilities[i] = probabilities[i] * N;
        if (scaled_probabilities[i] < 1.0) {
            small.push_back(i);
        }
        else {
            large.push_back(i);
        }
    }

    // Construct the alias and probability tables
    while (!small.empty() && !large.empty()) {
        int l = small.back();
        small.pop_back();
        int g = large.back();
        large.pop_back();

        prob[l] = scaled_probabilities[l];
        alias[l] = g;

        scaled_probabilities[g] = (scaled_probabilities[g] + scaled_probabilities[l]) - 1.0;
        if (scaled_probabilities[g] < 1.0) {
            small.push_back(g);
        }
        else {
            large.push_back(g);
        }
    }

    // Assign remaining probabilities
    while (!large.empty()) {
        int g = large.back();
        large.pop_back();
        prob[g] = 1.0;
    }

    while (!small.empty()) {
        int l = small.back();
        small.pop_back();
        prob[l] = 1.0;
    }
}

int WeightedRandomSampler::sample() {
    uint64_t r = generator();
    uint32_t column = (uint32_t)(r & 0xffffffff) % N;
    uint32_t pint = (uint32_t)(r >> 32);
    double p = (double)pint * (1.0 / 4294967296.0);

    if (p < prob[column]) {
        return column;
    }
    else {
        return alias[column];
    }
}
