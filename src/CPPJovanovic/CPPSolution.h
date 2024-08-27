#ifndef CPPSOLUTION_H
#define CPPSOLUTION_H

#include "CPPInstance.h"
#include "CPPCandidate.h"
#include "CPPSolutionBase.h"

#include <vector>
#include <string>

class CPPSolution : public CPPSolutionBase {
private:
    std::vector<std::vector<int>> mChange;
    static std::vector<std::vector<int>> EmptyChange;

public:
    using CPPSolutionBase::CPPSolutionBase;

    CPPSolution() {};
    CPPSolution(CPPInstance* nInstance);
    CPPSolution(int* pvertex, int numVertices, int objective, CPPInstance* nInstance);

    static void Init(int size);

    int getObjective();
    const std::vector<std::vector<int>>& getCliques() const;
    const std::vector<int>& getNodes() const;
    void InitChange();
    void AddCandidate(CPPCandidate& A) override;
    int GetChange(int iNode, int iClique) override;
    int GetChange(const std::vector<int>& nodes, int iClique) override;
    void UpdateChange(int Clique, int Node);
    void Allocate();
    void Clear();
    int AddNodeToClique(CPPCandidate& N);
    int AddNodeToClique(int iNode, int iClique);
    int NumberOfCliques();
    int CalculateObjectiveForClique(std::vector<int> Clique);
};

#endif // CPPSOLUTION_H
