#ifndef CPPINSTANCE_H
#define CPPINSTANCE_H

#include <vector>
#include <string>
#include <random>
#include <array>
#include "../util/WeightedRandomSampler.h"

class CPPInstance {
private:
    int mNumberOfNodes;
    int numberOfEdges;
    std::vector<std::vector<int>> mWeights;
    std::vector<std::vector<int>> mNegativeWeights;
    std::vector<int> neighborSize;
    std::vector<std::vector<int>> neighbors;
    std::vector<std::array<int, 3>> edges;
    WeightedRandomSampler* edgeSampler;

public:
    CPPInstance(const std::string& FileName);
    CPPInstance(int nnode, int** matrix);
    ~CPPInstance();

    int getNumberOfNodes() const;
    int getNumberOfEdges() const;
    const std::array<int, 3>& CPPInstance::getSampledRandEdge();
    const std::vector<std::vector<int>>& getWeights() const;
    const std::vector<std::vector<int>>& getNegativeWeights() const;
    int getWeight(int n1, int n2) const;
    const std::vector<int>& getNeighborSize() const;
    const std::vector<std::vector<int>>& getNeighbors() const;
    const std::vector<std::array<int, 3>>& getEdges() const;

    void InitNegativeWeights();
    void InitNeighbors();
    void InitEdges();
    void InitEdgeSampler();
    void Allocate();
    void Load(const std::string& FileName);
    void LoadFromMatrix(int nnode, int** matrix);
    void LoadMIP1(const std::string& FileName);
    void LoadMIP_GT(const std::string& FileName);
    double GetCForModularityMaximization(const std::string& FileName);
    double LoadMIP_MM(const std::string& FileName);
    void LoadMIP_Convert(const std::string& FileName);
};

#endif // CPPINSTANCE_H
