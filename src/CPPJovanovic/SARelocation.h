#ifndef SARELOCATION_H
#define SARELOCATION_H

enum SAMoveType { N0, N1, Both, Swap, Slide, None };

class SARelocation {
public:
    int mN0;
    int mN1;
    int mC0;
    int mC1;
    int mChange;

    SAMoveType mMoveType;

    // Constructor to initialize all values to 0
    SARelocation()
        : mN0(0), mN1(0), mC0(0), mC1(0), mChange(0), mMoveType(SAMoveType::N0) {}
};

#endif // SARELOCATION_H
