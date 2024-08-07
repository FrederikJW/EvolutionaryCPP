#include "SAParameters.h"

void SAParameters::InitGeometric() {
    mCooling = CPPCooling::Geometric;
    mSizeRepeat = 8;
    mCoolingParam = 0.96;
    mMinAccept = 0.01;
    mMaxStagnation = 5;
}

void SAParameters::InitLinearMultiplicative() {
    mCooling = CPPCooling::LinearMultiplicative;
    mSizeRepeat = 16;
    mCoolingParam = 0.3;
    mMinAccept = 0.01;
    mMaxStagnation = 5;
}
