#ifndef SARELOCATION_H
#define SARELOCATION_H

enum SAMoveType { N0, N1, Both, Swap, Slide, None };

struct SARelocationStruct {
    int mN0 = -1, mC0 = -1;
    int mN1 = -1, mC1 = -1;
    int mChange = 0;
    float mProbChange = 0;
    SAMoveType mMoveType;
};


class SARelocation {
public:
    int mN0;
    int mN1;
    int mC0;
    int mC1;
    int mChange;
    float mProbChange;
    bool forceAccept;

    SAMoveType mMoveType;

    // Constructor to initialize all values to 0
    SARelocation()
        : mN0(0), mN1(0), mC0(0), mC1(0), mChange(0), mProbChange(0), mMoveType(SAMoveType::N0), forceAccept(false) {}

    void copy(SARelocation& relocation) {
        mN0 = relocation.mN0;
        mC0 = relocation.mC0;
        mN1 = relocation.mN1;
        mC1 = relocation.mC1;
        mChange = relocation.mChange;
        mProbChange = relocation.mChange;
        mMoveType = relocation.mMoveType;
        forceAccept = relocation.forceAccept;
    }
};

struct SADualSplitRelocationStruct {
    int mN0 = -1, mN1 = -1;
    int mC0 = -1, mC1 = -1;
    int mChange = INT_MIN;
};


class SADualSplitRelocation {
public:
    int mN0;
    int mN1;
    int mC0;
    int mC1;
    int mChange0;
    int mChange1;
    int mChange;
    bool moveN0First;
    SADualSplitRelocation() : mN0(0), mN1(0), mC0(0), mC1(0), mChange0(0), mChange1(0), mChange(0), moveN0First(true) {}
};

struct SASingleRelocationStruct {
    int  node = -1;
    int  clique = -1;
    int change = INT_MIN;
};


class SASingleRelocation {
public:
    int node;
    int clique;
    int change;

    SASingleRelocation() : node(0), clique(0), change(0) {}
};

#endif // SARELOCATION_H
