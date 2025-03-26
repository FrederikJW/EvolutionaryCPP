// RandomGenerator.h
#ifndef RANDOMGENERATOR_H
#define RANDOMGENERATOR_H
#include <cstdint>
#include <memory>
#include <iostream>
#include <random>

// this approach was chosen to easily switch between different random generator methods
// because of the required static min and max methods it is not possible to switch between random generators during runtime or use inheritance

// #define USE_MT19937
// #define USE_MINSTD_RAND
#define USE_XOROSHIRO128PLUS


#if defined(USE_MT19937)

using RandomGenerator = std::mt19937;

#elif defined(USE_MINSTD_RAND)

using RandomGenerator = std::minstd_rand;

#elif defined(USE_XOROSHIRO128PLUS)

// an implementation of XOROSHIRO128+ [6] in C++ source: https://xoroshiro.di.unimi.it/xoroshiro128plus.c 
class RandomGenerator {
public:
    RandomGenerator(uint64_t seed1, uint64_t seed2) {
        state[0] = seed1;
        state[1] = seed2;
    }

    // Single-seed constructor
    RandomGenerator(uint64_t seed) {
        state[0] = seed;
        state[1] = splitmix64(seed); // Derive the second state part from the first seed
    }

    uint64_t operator()() {
        uint64_t s0 = state[0];
        uint64_t s1 = state[1];
        uint64_t result = s0 + s1;

        s1 ^= s0;
        state[0] = rotl(s0, 55) ^ s1 ^ (s1 << 14);
        state[1] = rotl(s1, 36);

        return result;
    }

    static uint64_t min() {
        return 0;
    }

    static uint64_t max() {
        return UINT64_MAX;
    }

private:
    uint64_t state[2];

    static uint64_t rotl(const uint64_t x, int k) {
        return (x << k) | (x >> (64 - k));
    }

    static uint64_t splitmix64(uint64_t z) {
        z += 0x9E3779B97f4A7C15;
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EB;
        return z ^ (z >> 31);
    }
};

#endif // USE_XOROSHIRO128PLUS
#endif // RANDOMGENERATOR_H
