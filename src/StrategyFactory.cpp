// unused; delete File later

#include <iostream>
#include <unordered_map>
#include <functional>
#include <memory>

#include "strategies/EvolutionStrategy.h"
#include "strategies/ImprovementStrategy.h"
#include "strategies/InitialPoolStrategy.h"

// Factory class
class StrategyFactory {
public:
    using CreateEvolutionStrategyFn = std::function<std::unique_ptr<EvolutionStrategy>()>;
    using CreateImprovementStrategyFn = std::function<std::unique_ptr<ImprovementStrategy>()>;
    using CreateInitialPoolStrategyFn = std::function<std::unique_ptr<InitialPoolStrategy>()>;

    void registerEvolutionStrategy(const std::string& className, CreateEvolutionStrategyFn createFn) {
        createEvolutionFnMap[className] = createFn;
    }

    void registerImprovementStrategy(const std::string& className, CreateImprovementStrategyFn createFn) {
        createImprovementFnMap[className] = createFn;
    }

    void registerInitialPoolStrategy(const std::string& className, CreateInitialPoolStrategyFn createFn) {
        createInitialPoolFnMap[className] = createFn;
    }

    std::unique_ptr<EvolutionStrategy> createEvolutionStrategy(const std::string& className) {
        if (createEvolutionFnMap.find(className) != createEvolutionFnMap.end()) {
            return createEvolutionFnMap[className]();
        }
        return nullptr;  // Return null if className not found
    }

    std::unique_ptr<ImprovementStrategy> createImprovementStrategy(const std::string& className) {
        if (createImprovementFnMap.find(className) != createImprovementFnMap.end()) {
            return createImprovementFnMap[className]();
        }
        return nullptr;  // Return null if className not found
    }


    std::unique_ptr<InitialPoolStrategy> createInitialPoolStrategy(const std::string& className) {
        if (createInitialPoolFnMap.find(className) != createInitialPoolFnMap.end()) {
            return createInitialPoolFnMap[className]();
        }
        return nullptr;  // Return null if className not found
    }

private:
    std::unordered_map<std::string, CreateEvolutionStrategyFn> createEvolutionFnMap;
    std::unordered_map<std::string, CreateImprovementStrategyFn> createImprovementFnMap;
    std::unordered_map<std::string, CreateInitialPoolStrategyFn> createInitialPoolFnMap;
};