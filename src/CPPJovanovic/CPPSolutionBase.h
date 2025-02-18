#ifndef CPPSOLUTIONBASE_H
#define CPPSOLUTIONBASE_H

#include "CPPInstance.h"
#include "CPPCandidate.h"
#include "SARelocation.h"
#include "SAParameters.h"
#include "BufferElement.h"
#include "CPPTypes.h"
#include "../RandomGenerator.h"
#include "../util/Util.h"

#include <vector>
#include <random>

enum SASelectType { Single, Dual, Triple, DualNeighbor, SingleEdge, SingleEdgeForcedDual };

class CPPSolutionBase {
protected:
    std::vector<std::vector<int>> mCliques; // holds a vector of nodes for every clique
    std::vector<int> mCliqueSizes; // holds the size of every clique

    int mObjective;
    std::vector<int> mNodeClique; // holds the clique id for every node
    CPPInstance* mInstance;
    std::vector<std::vector<int>> mAllConnections;
    SASelectType mSASelectType;
    std::vector<int> mRestricted;
    RandomGenerator* mGenerator;

    // parameters for SALOoE

    SARelocation nextAcceptRelocation;
    SARelocation nextDenyRelocation;
    int removedClique;
    bool nextRelocationCalculated;
    bool prevAccepted;

public:
    CPPSolutionBase();
    CPPSolutionBase(const CPPSolutionBase& iSolution);
    CPPSolutionBase(int iObjective, std::vector<std::vector<int>> iPartitions);
    CPPSolutionBase(int* pvertex, int numVertices, int objective, CPPInstance* nInstance);

    SASelectType getSASType() const { return mSASelectType; }
    CPPInstance* getInstance() { return mInstance; }
    void setSASType(SASelectType value) { mSASelectType = value; }

    void setGenerator(RandomGenerator* value) { mGenerator = value; }

    void InitRestricted(int Size);
    virtual void Clear();
    virtual void InitChange();
    virtual int getNumberOfCliques() const { return mCliques.size(); }
    virtual bool CheckSolutionValid();
    void Load(const std::string& FileName);
    virtual int CalculateObjectiveForClique(const std::vector<int>& Clique);
    virtual void FixCliques();
    bool InSameClique(int nodeA, int nodeB) const;
    virtual int CalculateObjective();
    virtual int GetChange(int iNode, int iClique) { return -1; }
    virtual int GetChange(const std::vector<int>& nodes, int iClique) { return -1; }
    int calculateDistance(const CPPSolutionBase& iSolution);

    int getObjective() const { return mObjective; }
    void setObjective(int value) { mObjective = value; }

    std::vector<std::vector<int>> getCliques() const { return mCliques; }
    std::vector<int> getNodeClique() const { return mNodeClique; }

    virtual void AddCandidate(const CPPCandidate& A);

    bool CheckSolutionValid(CPPInstance& tInstance);
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
    virtual bool LocalSearch(std::vector<std::vector<int>> Nodes = {});
    bool ImproveMove(std::vector<std::vector<int>>& Nodes);
    bool ImproveMove();
    void ExpandedBuffer(std::vector<BufferElement>& AllTest, int iNode, int iClique, int oClique);

    int CalculateSwap(int A, const std::vector<int>& CliqueA, int B, const std::vector<int>& CliqueB);
    int CalculateRemoveChange(int iNode);
    int CalculateAddChange(int iNode, int iClique);
    void CreateFromNodeClique(const std::vector<int>& iNodeClique);

    // Simulated Annealing Methods
    void SimulatedAnnealingSelectTrio(int n1, int n2, int n3, int& BestChange, std::vector<std::array<int, 2>>& BestRelocations);
    void SASelectDual(SARelocation& Relocation);
    void SASelectDualPrev(SARelocation& Relocation);
    void SASelectDualExt(SARelocation& Relocation);
    void SASelectDualFull(SARelocation& Relocation, int weight = 0, bool forceDualMove = false);
    void SASelectDualSplit(SARelocation& Relocation, int weight = 0, bool forceDualMove = false);
    void SASelectDualNeighborOne(SARelocation& Relocation);
    void SASelectSingle(SARelocation& Relocation);
    void SASelectSingleEdge(SARelocation& Relocation, bool forceDualMove);
    void SASelectDualR(SARelocation& Relocation);
    // void SASelectDualNeighborOne(SARelocation& Relocation);
    void SASelectSingleR(SARelocation& Relocation);
    void SASelectR(SARelocation& Relocations);
    void SASelectDoubleR(SARelocationStruct& RelocationBoth, SARelocationStruct& RelocationN0, SARelocationStruct& RelocationN1);
    void SASelectDoubleR2(SARelocation& RelocationBoth, SARelocation& RelocationN0, SARelocation& RelocationN1);
    void ApplyRelocation(SARelocation Relocation);
    bool SimulatedAnnealing(SAParameters& iSAParameters, double& AcceptRelative);
    bool SimulatedAnnealingWithDoubleMoves(SAParameters& iSAParameters, double& AcceptRelative);
    bool CalibrateSA(SAParameters& iSAParameters, double& Accept);
    bool CalibrateSADoubleMoves(SAParameters& iSAParameters, double& Accept);
    // double FastExp(double x);

    int CalculateMoveChange(const std::vector<int>& iNodes, int iClique);
    int CalculateMoveChange(BufferElement Set);
    int CalculateMoveChange(int iNode, int iClique, int iNodeRemoveChange = INT_MIN);
    bool CheckMove(int iNode, int iClique, int iNodeRemoveChange = INT_MIN);
    bool MoveNode(int iNode, int NewClique);
    bool MoveNodeSA(int iNode, int NewClique);
};

#endif // CPPSOLUTIONBASE_H
