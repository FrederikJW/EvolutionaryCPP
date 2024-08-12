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

    CPPGreedy(CPPInstance* nInstance);
    CPPGreedy(const std::string& fileName);

    void InitAvailable(const std::vector<std::vector<int>>& FixedSet);
    void SolveGreedy(const std::vector<std::vector<int>>& FixedSet = {});
    CPPSolution& getSolution();

private:
    CPPInstance* mInstance;
    CPPSolution* mSolution;
    RCL<CPPCandidate>* mRCL;
    int mRCLSize;
    std::mt19937 mGenerator;
    std::vector<std::vector<int>> mAvailableNodes;
    GreedyHeuristicType mGreedyHeuristic;

    CPPCandidate* GetHeuristicMaxIncrease();
    CPPCandidate* GetHeuristic();
    bool AddToSolution(CPPCandidate& N);
    void RemoveFromAvailable(CPPCandidate& N);
    void InitGreedy();
};

#endif // CPPGREEDY_H
