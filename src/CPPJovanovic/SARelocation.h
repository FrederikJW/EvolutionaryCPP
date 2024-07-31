#ifndef SARELOCATION_H
#define SARELOCATION_H

enum SAMoveType { N0, N1, Both, Swap, Slide };

class SARelocation {
public:
    int mN0;
    int mN1;
    int mC0;
    int mC1;
    int mChange;

    SAMoveType mMoveType;
};

#endif // SARELOCATION_H
