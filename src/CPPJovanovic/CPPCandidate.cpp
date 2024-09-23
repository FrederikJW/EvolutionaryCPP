#include "CPPCandidate.h"

#include <vector>

CPPCandidate::CPPCandidate() : mClique(0), mNodes({}), mCandidateIndex(0) {}

CPPCandidate::CPPCandidate(const std::vector<int>& nNodes, int nClique, int nCandidateIndex)
    : mClique(nClique), mNodes(nNodes), mCandidateIndex(nCandidateIndex) {}

CPPCandidate::~CPPCandidate() {}

int CPPCandidate::getClique() const {
    return mClique;
}

void CPPCandidate::setClique(int value) {
    mClique = value;
}

int CPPCandidate::getCandidateIndex() const {
    return mCandidateIndex;
}

void CPPCandidate::setCandidateIndex(int value) {
    mCandidateIndex = value;
}

const std::vector<int>& CPPCandidate::getNodes() const {
    return mNodes;
}
