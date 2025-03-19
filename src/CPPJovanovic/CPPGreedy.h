// a minimalistic version of CPPProblem.cs in C++ https://github.com/rakabog/CPPConsole/blob/master/CPPConsole/CPPProblem.cs

#ifndef CPPGREEDY_H
#define CPPGREEDY_H

#include "CPPCandidate.h"
#include "CPPInstance.h"
#include "CPPSolution.h"
#include "RCL.h"
#include "../RandomGenerator.h"

#include <string>
#include <vector>
#include <random>
#include <chrono>

class CPPGreedy {
public:
    enum class GreedyHeuristicType { MaxIncrease, Random };
    enum class CPPMetaheuristic { GRASP, FSS };
    enum class CPPCooling { Geometric, LinearMultiplicative };

    CPPGreedy(CPPInstance* nInstance, RandomGenerator* generator);
    CPPGreedy(const std::string& fileName, RandomGenerator* generator);
    ~CPPGreedy();

    void InitAvailable(const std::vector<std::vector<int>>& FixedSet);
    void SolveGreedy(const std::vector<std::vector<int>>& FixedSet = {});
    CPPSolution& getSolution();

private:
    CPPInstance* mInstance;
    CPPSolution* mSolution;
    RCL<CPPCandidate>* mRCL;
    int mRCLSize;
    RandomGenerator* mGenerator;
    std::vector<std::vector<int>> mAvailableNodes;
    GreedyHeuristicType mGreedyHeuristic;

    CPPCandidate* GetHeuristicMaxIncrease();
    CPPCandidate* GetHeuristic();
    bool AddToSolution(const CPPCandidate& N);
    void RemoveFromAvailable(const CPPCandidate& N);
    void InitGreedy();
};

#endif // CPPGREEDY_H
