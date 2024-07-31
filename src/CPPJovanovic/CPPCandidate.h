#ifndef CPPCANDIDATE_H
#define CPPCANDIDATE_H

#include <vector>

class CPPCandidate {
private:
    int mClique;
    std::vector<int> mNodes;
    int mCandidateIndex;

public:
    CPPCandidate();
    CPPCandidate(const std::vector<int>& nNodes, int nClique, int nCandidateIndex);

    int getClique() const;
    void setClique(int value);

    int getCandidateIndex() const;
    void setCandidateIndex(int value);

    const std::vector<int>& getNodes() const;
};

#endif // CPPCANDIDATE_H
