#ifndef BUFFERELEMENT_H
#define BUFFERELEMENT_H

#include <vector>
#include <random>

class BufferElement {
public:
    int* mNewLocations;
    int* mOldLocations;
    std::vector<int*> mRelocations;
    int mChange;

    BufferElement() {};
    BufferElement(int size);
    BufferElement(const BufferElement& other);
    ~BufferElement();

    bool HasRelocation(int* r);
    void RemoveRelocation(int iNode);
    bool CanAdd(int nNode, int nClique) const;
    bool Contains(int nNode);
    bool Add(int nNode, int nClique, int oClique);
    bool IsSameDest(int N1, int N2);
    bool Related(const BufferElement& A);
    void Merge(const BufferElement& A);
    std::vector<BufferElement> SplitIndependent();

private:
    void InitializeArrays(int size);
    void CopyArrays(const BufferElement& other);
};

#endif // BUFFERELEMENT_H
