#ifndef CPPSOLUTIONBASE_H
#define CPPSOLUTIONBASE_H

#include "CPPInstance.h"
#include "CPPCandidate.h"
#include "SARelocation.h"
#include "SAParameters.h"
#include "BufferElement.h"

#include <vector>
#include <random>

enum SASelectType { Single, Dual, Triple };

class CPPSolutionBase {
protected:
    std::vector<std::vector<int>> mCliques;
    std::vector<int> mCliqueSizes;

    int mObjective;
    std::vector<int> mNodeClique;
    CPPInstance* mInstance;
    int mMaxBuffer;
    std::vector<std::vector<int>> mAllConnections;
    SASelectType mSASelectType;
    std::vector<int> mRestricted;
    std::default_random_engine mGenerator;

public:
    CPPSolutionBase();
    CPPSolutionBase(const CPPSolutionBase& iSolution);
    CPPSolutionBase(int iObjective, std::vector<std::vector<int>> iPartitions);

    SASelectType getSASType() const { return mSASelectType; }
    void setSASType(SASelectType value) { mSASelectType = value; }

    std::default_random_engine getGenerator() const { return mGenerator; }
    void setGenerator(const std::default_random_engine& value) { mGenerator = value; }

    void InitRestricted(int Size);
    virtual void Clear();
    virtual void InitChange();
    virtual int getNumberOfCliques() const { return mCliques.size(); }
    virtual bool CheckSolutionValid();
    void Load(const std::string& FileName);
    virtual int CalculateObjectiveForClique(const std::vector<int>& Clique);
    virtual void FixCliques();
    bool InSameClique(int nodeA, int nodeB);
    virtual int CalculateObjective();
    virtual int GetChange(int A, int B) { return -1; }
    virtual int GetChange(const std::vector<int>& A, int B) { return -1; }

    int getObjective() const { return mObjective; }
    void setObjective(int value) { mObjective = value; }

    std::vector<std::vector<int>> getCliques() const { return mCliques; }
    std::vector<int> getNodeClique() const { return mNodeClique; }

    virtual void AddCandidate(CPPCandidate A);

    bool CheckSolutionValid(CPPInstance tInstance);
    std::vector<int> CliqueForNode(int iNode);

    bool IsSame(int iObjective, const std::vector<std::vector<int>>& iPartitions);

    int CalculateAllConnections(int node, int clique);
    void UpdateAllConnections(int nNode, int nClique);
    void UpdateAllConnectionsRestricted(int nNodeIndex, int nClique);
    void InitAllConnections();
    void CreateRelocations(int n1, int n2, int n3, int c, std::vector<std::array<int, 2>>& BestRelocations);
    void CreateRelocations(int n1, int n2, int c, std::vector<std::array<int, 2>>& BestRelocations);


    bool RemoveEmptyClique(bool bUpdateAllConections = false, bool bUseCliqueSize = false);
    bool RemoveEmptyCliqueSA(bool bUpdateAllConections = false, bool bUseCliqueSize = false);
    virtual bool LocalSearch(std::default_random_engine& iGenerator, std::vector<std::vector<int>> Nodes = {});
    bool ImproveMove(std::vector<std::vector<int>>& Nodes);
    bool ImproveMove();
    void ExpandedBuffer(std::vector<BufferElement>& AllTest, int iNode, int iClique, int oClique);

    int CalculateSwap(int A, const std::vector<int>& CliqueA, int B, const std::vector<int>& CliqueB);
    int CalculateRemoveChange(int iNode);
    int CalculateAddChange(int iNode, int iClique);
    void CreateFromNodeClique(const std::vector<int>& iNodeClique);

    // Simulated Annealing Methods
    void SimulatedAnnealingSelectTrio(int n1, int n2, int n3, int& BestChange, std::vector<std::array<int, 2>>& BestRelocations, std::default_random_engine& iGenerator);
    void SASelectDual(SARelocation& Relocation);
    void SASelectDualPrev(SARelocation& Relocation);
    void SASelectDualExt(SARelocation& Relocation);
    void SASelectSingle(SARelocation& Relocation);
    void SASelectDual(SARelocation& Relocation, std::default_random_engine& iGenerator);
    void SASelectSingle(SARelocation& Relocation, std::default_random_engine& iGenerator);
    void SASelect(SARelocation& Relocations, std::default_random_engine& iGenerator);
    void ApplyRelocation(SARelocation Relocation);
    bool SimulatedAnealing(std::default_random_engine& iGenerator, SAParameters& iSAParameters, double& AcceptRelative);
    bool CalibrateSA(std::default_random_engine& iGenerator, SAParameters& iSAParameters, double& Accept);
    double FastExp(double x);

    int CalculateMoveChange(const std::vector<int>& iNodes, int iClique);
    int CalculateMoveChange(BufferElement Set);
    int CalculateMoveChange(int iNode, int iClique, int iNodeRemoveChange = INT_MIN);
    bool CheckMove(int iNode, int iClique, int iNodeRemoveChange = INT_MIN);
    bool MoveNode(int iNode, int NewClique);
    bool MoveNodeSA(int iNode, int NewClique);
};

#endif // CPPSOLUTIONBASE_H
