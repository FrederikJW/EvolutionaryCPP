#ifndef CPPGREEDY_H
#define CPPGREEDY_H

#include "CPPCandidate.h"
#include "CPPInstance.h"
#include "CPPSolution.h"
#include "RCL.h"

#include <string>
#include <vector>
#include <random>
#include <chrono>

class CPPGreedy {
public:
    enum class GreedyHeuristicType { MaxIncrease, Random };
    enum class CPPMetaheuristic { GRASP, FSS };
    enum class CPPCooling { Geometric, LinearMultiplicative };

    CPPGreedy(CPPInstance* nInstance, std::default_random_engine* generator);
    CPPGreedy(const std::string& fileName, std::default_random_engine* generator);
    ~CPPGreedy();

    void InitAvailable(const std::vector<std::vector<int>>& FixedSet);
    void SolveGreedy(const std::vector<std::vector<int>>& FixedSet = {});
    CPPSolution& getSolution();

private:
    CPPInstance* mInstance;
    CPPSolution* mSolution;
    RCL<CPPCandidate>* mRCL;
    int mRCLSize;
    std::default_random_engine* mGenerator;
    std::vector<std::vector<int>> mAvailableNodes;
    GreedyHeuristicType mGreedyHeuristic;

    CPPCandidate* GetHeuristicMaxIncrease();
    CPPCandidate* GetHeuristic();
    bool AddToSolution(const CPPCandidate& N);
    void RemoveFromAvailable(const CPPCandidate& N);
    void InitGreedy();
};

#endif // CPPGREEDY_H
