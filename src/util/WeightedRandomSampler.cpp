#include "WeightedRandomSampler.h"

WeightedRandomSampler::WeightedRandomSampler(const std::vector<int>& weights)
    : generator(std::random_device{}()), N(weights.size()), total_weight(0.0) {

    // Calculate the total weight
    for (int w : weights) {
        total_weight += w;
    }

    // Normalize the weights to create probabilities
    std::vector<double> probabilities(N);
    for (int i = 0; i < N; ++i) {
        probabilities[i] = static_cast<double>(weights[i]) / total_weight;
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
    std::uniform_int_distribution<int> dist_int(0, N - 1);
    int column = dist_int(generator);

    std::uniform_real_distribution<double> dist_double(0.0, 1.0);
    double p = dist_double(generator);

    if (p < prob[column]) {
        return column;
    }
    else {
        return alias[column];
    }
}
