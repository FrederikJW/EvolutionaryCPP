#ifndef WEIGHTEDRANDOMSAMPLING_H
#define WEIGHTEDRANDOMSAMPLING_H

#include <array>
#include <vector>
#include <algorithm>
#include <random>
#include <cmath>


class WeightedRandomSampling {
public:
    static std::vector<int> GetWeightedRandomSampling(int mN, int mK, const std::vector<double>& mWeights, RandomGenerator& mGenerator) {
        std::vector<int> Result;
        std::vector<std::array<double, 2>> mUi;
        std::uniform_real_distribution<> dis(0.0, 1.0);

        for (int i = 0; i < mN; ++i) {
            double rU = dis(mGenerator);
            // mUi.push_back(std::array<double, 2>{static_cast<double>(i), std::pow(rU, mWeights[i])});
            mUi.push_back({ static_cast<double>(i), std::pow(rU, mWeights[i]) });
        }

        std::sort(mUi.begin(), mUi.end(), [](const std::array<double, 2>& a, const std::array<double, 2>& b) {
            return a[1] < b[1];
            });

        for (int i = 0; i < mK; ++i) {
            Result.push_back(static_cast<int>(mUi[i][0]));
        }

        return Result;
    }

    static std::vector<int> GetWeightedRandomSampling(int mN, int mK, const std::vector<int>& mWeights, RandomGenerator& mGenerator) {
        std::vector<double> dWeights(mN);
        double Sum = 0;

        for (int i = 0; i < mN; ++i) {
            Sum += mWeights[i];
            dWeights[i] = static_cast<double>(mWeights[i]);
        }

        for (int i = 0; i < mN; ++i) {
            dWeights[i] /= Sum;
        }

        return GetWeightedRandomSampling(mN, mK, dWeights, mGenerator);
    }
};

#endif // WEIGHTEDRANDOMSAMPLING_H
