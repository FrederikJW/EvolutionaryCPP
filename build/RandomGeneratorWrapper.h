// RandomGeneratorWrapper.h
#ifndef RANDOMGENERATORWRAPPER_H
#define RANDOMGENERATORWRAPPER_H
#include <random>

template <typename RNG>
class RandomGeneratorWrapper {
public:
    using result_type = typename RNG::result_type;

    // Constructor to accept any RNG and forward its arguments
    template <typename... Args>
    RandomGeneratorWrapper(Args&&... args) : rng(std::forward<Args>(args)...) {}

    // Overload operator() to generate a random number
    result_type operator()() {
        return rng();
    }

    // Define min() and max() to match the underlying RNG's limits
    static constexpr result_type min() {
        return RNG::min();
    }

    static constexpr result_type max() {
        return RNG::max();
    }

private:
    RNG rng;
};

#endif // RANDOMGENERATORWRAPPER_H
