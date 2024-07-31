#include "SAParameters.h"

void SAParameters::InitGeometric() {
    mCooling = CPPProblem::CPPCooling::Geometric;
    mSizeRepeat = 8;
    mCoolingParam = 0.96;
    mMinAccept = 0.01;
    mMaxStagnation = 5;
}

void SAParameters::InitLinearMultiplicative() {
    mCooling = CPPProblem::CPPCooling::LinearMultiplicative;
    mSizeRepeat = 16;
    mCoolingParam = 0.3;
    mMinAccept = 0.01;
    mMaxStagnation = 5;
}
