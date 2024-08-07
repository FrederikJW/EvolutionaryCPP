#ifndef SAPARAMETERS_H
#define SAPARAMETERS_H

#include "CPPTypes.h"

class SAParameters {
public:
    CPPCooling mCooling;
    double mInitTemperature;
    int mSizeRepeat;
    double mCoolingParam;
    double mMinAccept;
    int mMaxStagnation;

    void InitGeometric();
    void InitLinearMultiplicative();
};

#endif // SAPARAMETERS_H
