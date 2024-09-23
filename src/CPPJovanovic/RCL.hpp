#include "RCL.h"

template <typename T>
RCL<T>::RCL(int iSize)
    : mSize(iSize), mCurrentSize(0),
      mMinValue(std::numeric_limits<double>::max()),
      mMaxValue(std::numeric_limits<double>::lowest()) {
    mValues.resize(mSize);
    mCandidates.resize(mSize);
}

template <typename T>
int RCL<T>::getCurrentSize() const {
    return mCurrentSize;
}

template <typename T>
T RCL<T>::getCandidate(int index) {
    return mCandidates[index];
}

template <typename T>
double RCL<T>::getValue(int index) const {
    return mValues[index];
}

template <typename T>
double RCL<T>::getMinValue() const {
    return mMinValue;
}

template <typename T>
double RCL<T>::getMaxValue() const {
    return mMaxValue;
}

template <typename T>
std::list<T> RCL<T>::sort() {
    std::vector<std::pair<int, int>> tempList;
    for (int i = 0; i < mCurrentSize; ++i) {
        tempList.push_back(std::make_pair(mValues[i], i));
    }

    std::sort(tempList.rbegin(), tempList.rend());

    std::list<T> result;
    for (const auto& el : tempList) {
        result.push_back(mCandidates[el.second]);
    }

    return result;
}

template <typename T>
void RCL<T>::reset() {
    mCurrentSize = 0;
    mMinValue = std::numeric_limits<double>::max();
    mMaxValue = std::numeric_limits<double>::lowest();
}

template <typename T>
void RCL<T>::add(T iCandidate, int iValue) {
    if (mCurrentSize < mSize) {
        mValues[mCurrentSize] = iValue;
        mCandidates[mCurrentSize] = iCandidate;

        if (mMinValue > iValue) {
            mMinValue = iValue;
            mMinIndex = mCurrentSize;
        }

        if (mMaxValue < iValue) {
            mMaxValue = iValue;
            mMaxIndex = mCurrentSize;
        }

        ++mCurrentSize;
        return;
    }

    if (mMinValue >= iValue) {
        return;
    }

    if (mMaxValue < iValue) {
        mMaxValue = iValue;
    }

    mValues[mMinIndex] = iValue;
    mCandidates[mMinIndex] = iCandidate;

    mMinIndex = -1;
    mMinValue = std::numeric_limits<double>::max();

    for (int i = 0; i < mSize; ++i) {
        if (mMinValue > mValues[i]) {
            mMinValue = mValues[i];
            mMinIndex = i;
        }
    }
}
