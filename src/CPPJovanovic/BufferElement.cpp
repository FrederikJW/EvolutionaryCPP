// rewritten version of BufferElement.cs into C++ https://github.com/rakabog/CPPConsole/blob/master/CPPConsole/BufferElement.cs
#include "BufferElement.h"
#include <algorithm>
#include <iterator>
#include <cstring>

BufferElement::BufferElement(int size) {
    InitializeArrays(size);
    std::fill(mNewLocations, mNewLocations + size, -1);
    std::fill(mOldLocations, mOldLocations + size, -1);
}

BufferElement::BufferElement(const BufferElement& other) {
    InitializeArrays(other.mOldLocations[0]); // Assuming the size can be derived from the first element
    CopyArrays(other);
}

BufferElement::~BufferElement() {
    delete[] mNewLocations;
    delete[] mOldLocations;
    for (auto& ptr : mRelocations) {
        delete[] ptr;
    }
    mRelocations.clear();
}

void BufferElement::InitializeArrays(int size) {
    mNewLocations = new int[size];
    mOldLocations = new int[size];
}

void BufferElement::CopyArrays(const BufferElement& other) {
    std::copy(other.mNewLocations, other.mNewLocations + sizeof(other.mNewLocations) / sizeof(int), mNewLocations);
    std::copy(other.mOldLocations, other.mOldLocations + sizeof(other.mOldLocations) / sizeof(int), mOldLocations);

    for (auto& rel : other.mRelocations) {
        int* temp = new int[2];
        temp[0] = rel[0];
        temp[1] = rel[1];
        mRelocations.push_back(temp);
    }
}

bool BufferElement::HasRelocation(int* r) {
    return mNewLocations[r[0]] == r[1];
}

// Takes a random relocation
/*
int* BufferElement::TakeRandomRelocation(RandomGenerator& generator) {
    std::uniform_int_distribution<> dist(0, mRelocations.size() - 1);
    int select = dist(generator);
    int* result = mRelocations[select];

    mNewLocations[mRelocations[select][0]] = -1;
    mRelocations.erase(mRelocations.begin() + select);

    return result;
}*/

void BufferElement::RemoveRelocation(int iNode) {
    mNewLocations[iNode] = -1;
    mOldLocations[iNode] = -1;

    std::vector<int*> tRelocations;
    for (auto& t : mRelocations) {
        if (t[0] == iNode) {
            delete[] t;
        }
        else {
            tRelocations.push_back(t);
        }
    }
    mRelocations = tRelocations;
}

bool BufferElement::CanAdd(int nNode, int nClique) const {
    return mNewLocations[nNode] == -1;
}

bool BufferElement::Contains(int nNode) {
    return mNewLocations[nNode] != -1;
}

bool BufferElement::Add(int nNode, int nClique, int oClique) {
    if (!CanAdd(nNode, nClique)) return false;

    int* temp = new int[2];
    mNewLocations[nNode] = nClique;
    mOldLocations[nNode] = oClique;
    temp[0] = nNode;
    temp[1] = nClique;
    mRelocations.push_back(temp);

    return true;
}

bool BufferElement::IsSameDest(int N1, int N2) {
    return mNewLocations[N1] == mNewLocations[N2];
}

bool BufferElement::Related(const BufferElement& A) {
    bool* contains = new bool[sizeof(mOldLocations) / sizeof(int)];
    std::fill(contains, contains + sizeof(mOldLocations) / sizeof(int), false);

    for (int i = 0; i < sizeof(mOldLocations) / sizeof(int); i++) {
        if (mOldLocations[i] != -1) contains[mOldLocations[i]] = true;
        if (mNewLocations[i] != -1) contains[mNewLocations[i]] = true;
    }

    for (auto& t : A.mRelocations) {
        if (contains[t[1]]) {
            delete[] contains;
            return true;
        }
        if (contains[A.mOldLocations[t[0]]]) {
            delete[] contains;
            return true;
        }
    }

    delete[] contains;
    return false;
}

void BufferElement::Merge(const BufferElement& A) {
    for (auto& t : A.mRelocations) {
        Add(t[0], t[1], A.mOldLocations[t[0]]);
    }
}

std::vector<BufferElement> BufferElement::SplitIndependent() {
    std::vector<BufferElement> result;
    BufferElement dependent(mOldLocations[0]);
    std::vector<BufferElement> independent;

    for (auto& t : mRelocations) {
        independent.clear();
        dependent = BufferElement(sizeof(mOldLocations) / sizeof(int));
        dependent.Add(t[0], t[1], mOldLocations[t[0]]);

        for (auto& cReloc : result) {
            if (dependent.Related(cReloc)) {
                dependent.Merge(cReloc);
            }
            else {
                independent.push_back(cReloc);
            }
        }

        result = independent;
        result.push_back(dependent);
    }

    return result;
}
