#ifndef CPPSOLUTIONHOLDER_H
#define CPPSOLUTIONHOLDER_H

#include <vector>
#include "CPPSolutionBase.h"


class CPPSolutionHolder {
private:
    std::vector<CPPSolutionBase> mSolutions;
    int mMaxSize;
    int mMinObjective;

public:
    CPPSolutionHolder() {
        mSolutions = std::vector<CPPSolutionBase>();
        mMaxSize = 100;
    }

    std::vector<CPPSolutionBase>& Solutions() {
        return mSolutions;
    }

    void Clear() {
        mSolutions.clear();
    }

    bool Add(CPPSolutionBase& iSolution) {
        CPPSolutionBase nSol;

        if (!iSolution.CheckSolutionValid()) {
            return false;
        }

        if (mSolutions.empty()) {
            nSol = CPPSolutionBase(iSolution);
            nSol.setObjective(iSolution.CalculateObjective());
            mSolutions.push_back(nSol);
            mMinObjective = nSol.getObjective();
            return true;
        }

        // check distance
        /*
        int distanceSum = 0;
        int minDistance = INT_MAX;
        for (CPPSolutionBase& l : mSolutions) {
            int distance = iSolution.calculateDistance(l);
            distanceSum += distance;
            if (distance < minDistance) {
                minDistance = distance;
            }
        }
        float distanceAvg = distanceSum / mSolutions.size();
        printf("Avg distance: %.3f\nMin distance: %d\n", distanceAvg, minDistance);*/

        if (!((mSolutions.size() < mMaxSize) || (iSolution.getObjective() > mMinObjective))) {
            return false;
        }

        int iObjective = iSolution.getObjective();
        for (CPPSolutionBase& l : mSolutions) {
            if (l.IsSame(iObjective, iSolution.getCliques())) {
                return false;
            }
        }

        for (size_t i = 0; i < mSolutions.size(); ++i) {
            if (mSolutions[i].getObjective() <= iObjective) {
                nSol = CPPSolutionBase(iSolution);
                nSol.setObjective(iSolution.CalculateObjective());
                mSolutions.insert(mSolutions.begin() + i, nSol);
                break;
            }
        }

        if (mSolutions.size() > mMaxSize) {
            mSolutions.pop_back();
            mMinObjective = mSolutions[mMaxSize - 1].getObjective();
        }

        return true;
    }
};

#endif // CPPSOLUTIONHOLDER_H
