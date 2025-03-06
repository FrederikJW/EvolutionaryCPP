#ifndef STRATEGYTEST_H
#define STRATEGYTEST_H

#include "Statistic.h"
#include "strategies/SingletonInitStrategy.h"
#include "strategies/RCLInitStrategy.h"
#include "strategies/SimulatedAnnealingImprovement.h"
#include "strategies/SaloExtendedImprovement.h"
#include "strategies/MergeDivideCrossover.h"
#include "strategies/MdxEvolution.h"
#include "strategies/FixedSetEvolution.h"

class StrategyTests {
public:
    void testSAImprovement();
    void testSAeImprovement();
    void testStdInitStrategy();
    void testRCLInitStrategy();
    void testSolEvolution();
    void testFSSEvolution();
    void fullTest();
};

#endif // STRATEGYTEST_H