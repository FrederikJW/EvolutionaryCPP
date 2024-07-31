#ifndef CPPPROBLEM_H
#define CPPPROBLEM_H

#include "CPPCandidate.h"
#include "CPPInstance.h"
#include "CPPSolution.h"
#include "RCL.h"

#include <string>
#include <vector>
#include <random>
#include <chrono>

class CPPProblem {
public:
    enum class GreedyHeuristicType { MaxIncrease, Random };
    enum class CPPMetaheuristic { GRASP, FSS };
    enum class CPPCooling { Geometric, LinearMultiplicative };

    CPPProblem(CPPInstance* nInstance);
    CPPProblem(const std::string& fileName);

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
    bool AddToSolution(const CPPCandidate& N);
    void RemoveFromAvailable(const CPPCandidate& N);
    void InitGreedy();
};

#endif // CPPPROBLEM_H
