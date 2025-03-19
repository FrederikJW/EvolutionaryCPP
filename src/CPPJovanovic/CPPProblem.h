// a rewritten version of CPPProblem.cs into C++ https://github.com/rakabog/CPPConsole/blob/master/CPPConsole/CPPProblem.cs
// with added functionality for SALO with double moves and SALOe with node cooling
#ifndef CPP_PROBLEM_H
#define CPP_PROBLEM_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <chrono>
#include <random>
#include <algorithm>
#include <numeric>
#include "CPPInstance.h"
#include "CPPSolutionBase.h"
#include "CPPSolutionHolder.h"
#include "RCL.h"
#include "CPPCandidate.h"
#include "SAParameters.h"
#include "WeightedRandomSampling.h"
#include "CPPTypes.h"
#include "../RandomGenerator.h"

class CPPProblem {
private:
    CPPInstance* mInstance;
    CPPSolutionBase* mSolution;
    CPPSolutionHolder mSolutionHolder;

    RCL<CPPCandidate>* mRCL;

    int mRCLSize;
    RandomGenerator* mGenerator;
    std::vector<std::vector<int>> mAvailableNodes;
    GreedyHeuristicType mGreedyHeuristic;
    int mBestSolutionValue;

    CPPMetaheuristic mMetaHeuristic;
    std::chrono::steady_clock::time_point mStartTime;
    SASelectType mSASType;

    int mFixStagnation;
    int mFixK;
    int mFixInitPopulation;
    int mFixN;
    SAParameters mSAParams;

    std::vector<int> mIntermediateSolutions;
    std::vector<long> mIntermediateSolutionsTimes;
    std::vector<long> mIntermediateSolutionsIterations;
    int mNumberOfSolutionsGenerated;

public:
    CPPProblem(CPPInstance* nInstance, RandomGenerator* generator);
    ~CPPProblem();

    std::string GetMethodFileName();
    int GetBestSolution() const { return mBestSolutionValue; }
    GreedyHeuristicType GetGreedyHeuristic() const { return mGreedyHeuristic; }
    void SetGreedyHeuristic(GreedyHeuristicType value) { mGreedyHeuristic = value; }

    CPPMetaheuristic GetMetaheuristic() const { return mMetaHeuristic; }
    void SetMetaheuristic(CPPMetaheuristic value) { mMetaHeuristic = value; }

    SASelectType GetSASelect() const { return mSASType; }
    void SetSASelect(SASelectType value) { mSASType = value; }

    CPPInstance* GetInstance() { return mInstance; }
    CPPSolutionBase& GetSolution() { return *mSolution; }

    long GetBestTime();
    void Solve(int iMaxIterations, double iTimeLimit);
    void InitAvailable(const std::vector<std::vector<int>>& FixedSet);
    void InitTracking();
    void InitFSS();

    void setNeighborhoodFactor(long factor) { mSAParams.neighborhoodFactor = factor; }

    void AllocateSolution();
    void AllocateSolution(CPPSolutionBase* solution);
    void AllocateSolution(int* pvertex, int numVertices, int objective);
    std::vector<double> GetFrequency(int BaseSolutionIndex, const std::vector<int>& SelectedSolutionIndexes);
    void UpdateEdgeFrequency(std::vector<std::vector<int>>& Occurence, CPPSolutionBase* Base, CPPSolutionBase* Update);
    std::vector<std::vector<int>> GetFrequencyEdge(int BaseSolutionIndex, const std::vector<int>& SelectedSolutionIndexes);
    void UpdateFrequency(CPPSolutionBase* Base, CPPSolutionBase* Test, std::vector<double>& frequency);
    std::vector<std::vector<int>> GetFixEdge(int N, int K, double FixSize, std::vector<std::vector<int>>& SuperNodes);
    bool ContainsList(const std::vector<std::vector<int>>& Container, const std::vector<int>& Test);
    std::vector<std::vector<int>> GetFix(int N, int K, double FixSize);
    void InitGreedy();
    bool CheckBest(double Size = -1);
    void SolveFixSetSearch(int MaxGenerated, double iTimeLimit);
    void Calibrate(double iTimeLimit);
    void CalibrateCool(double iTimeLimit);
    void CalibrateDoubleMoves(double iTimeLimit);
    void SALOSearch();
    void SALOeCoolSearch();
    void SALODoubleMovesSearch();
    void LocalSearch();
    void SASearch();
    void SolveGRASP(int MaxIterations, double iTimeLimit);
    void SolveGreedy(const std::vector<std::vector<int>>& FixedSet = {});
    CPPCandidate* GetHeuristicMaxIncrease();
    CPPCandidate* GetHeuristic();
    bool AddToSolution(const CPPCandidate& N);
    bool AddToSolutionHolder(CPPSolutionBase& iSolution);
    void RemoveFromAvailable(const CPPCandidate& N);
};

#endif // CPP_PROBLEM_H
