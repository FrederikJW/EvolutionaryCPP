#ifndef SAPARAMETERS_H
#define SAPARAMETERS_H

#include "CPPProblem.h"

class SAParameters {
public:
    CPPProblem::CPPCooling mCooling;
    double mInitTemperature;
    int mSizeRepeat;
    double mCoolingParam;
    double mMinAccept;
    int mMaxStagnation;

    void InitGeometric();
    void InitLinearMultiplicative();
};

#endif // SAPARAMETERS_H
