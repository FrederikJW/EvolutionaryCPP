#ifndef RCL_H
#define RCL_H

#include <vector>
#include <list>
#include <limits>
#include <algorithm>

template <typename T>
class RCL {
private:
    std::vector<int> mValues;
    std::vector<T> mCandidates;
    int mSize;
    int mCurrentSize;

    double mMinValue;
    int mMinIndex;

    double mMaxValue;
    int mMaxIndex;

public:
    RCL(int iSize);

    int getCurrentSize() const;

    T getCandidate(int index) const;

    double getValue(int index) const;

    double getMinValue() const;

    double getMaxValue() const;

    std::list<T> sort();

    void reset();

    void add(T iCandidate, int iValue);
};

#include "RCL.hpp"

#endif // RCL_H
