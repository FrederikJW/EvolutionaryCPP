#include "FixedSetEvolution.h"
#include "../CPPJovanovic/CPPInstance.h"
#include "../CPPJovanovic/CPPProblem.h"
#include "../CPPJovanovic/CPPSolutionBase.h"

#include <chrono>
#include <unordered_map>

void convertCPPSolutionToPartition(Partition& partition, CPPSolutionBase& solution) {
    int nnode = solution.getNodeClique().size();
    int* initpart = new int[nnode];

    for (int i = 0; i < nnode; i++) {
        // +1 because 0 is the non existing clique
        initpart[i] = solution.getNodeClique()[i] + 1;
    }

    partition.buildPartition(initpart);
    partition.setValue(solution.getObjective());
    delete[] initpart;
}

void FixedSetEvolution::run(BestSolutionInfo* frt_, int* totalGen, int poolSize) {
    /*
	frt = frt_;

	CPPInstance* instance = new CPPInstance("instance/rand500-100.txt");
	problem = new CPPProblem("file1", "rand500-100", instance);
	problem->SetID(2);

    // TODO: move this param somewehere else
    int param_knownbest = 310000;

    frt = frt_;
    bestTime = 0.0;
    startTime = clock();
    generationCnt = 0;

    // TODO: could be created inside initial pool builder
    population = new Population(poolSize);
    childPartition = new Partition(graph->getNodeCount());

    improvementStrategy->setEnvironment(*graph);

    // calibrating initial temperature
    printf("Calibrate the initial temperature.\n");
    initialPoolStrategy->generateInitialSolution(*childPartition, *graph);
    improvementStrategy->setStart(*childPartition);
    improvementStrategy->calibrateTemp();

    printf("Build initial pool.\n");
    initialPoolStrategy->buildInitialPool(frt, *population, *graph, improvementStrategy, maxSeconds, &generationCnt);

    printf("Run evolution.\n");
    while (generationCnt < maxGenerations && (double)(clock() - startTime) / CLOCKS_PER_SEC < maxSeconds) {
        runGeneration();

        if (frt->best_val >= param_knownbest) {
            break;
        }

        generationCnt++;
    }
    *totalGen = generationCnt;
    improvementStrategy->disposeEnvironment();
    printf("Best solution found with value %d at generation %d\n", frt->best_val, frt->best_generation);
    */
    // #########################################################################################################

    frt = frt_;
    generationCnt = 0;
    startTime = clock();

    double iTimeLimit = 2000;

    // graph?
    CPPInstance* instance = new CPPInstance(graph->getNodeCount(), graph->getMatrix());
    int nnode = instance->getNumberOfNodes();
    problem = new CPPProblem("file1", "rand500-100", instance);
    problem->SetID(2);

    problem->AllocateSolution();

    population = new Population(poolSize);
    Partition* childPartition = new Partition(nnode);

    int mFixN = 50;
    int mFixStagnation = 20;
    int mFixK = 10;
    int mFixInitPopulation = 10;
    int mNumberOfSolutionsGenerated = 0;

    // fill max generated
    int MaxGenerated = 100000;

    std::vector<double> w(mFixN, 1.0 / mFixN);
    double FixSize = 0.50;
    int cSolutionValue = 0;
    double Accept;

    std::vector<std::vector<int>> FixSet;
    std::chrono::steady_clock::time_point mStartTime;

    mStartTime = std::chrono::steady_clock::now();

    improvementStrategy->setEnvironment(*graph);

    // TODO: use initial population strategy
    // problem->SolveGRASP(mFixInitPopulation, iTimeLimit);
    printf("Calibrate the initial temperature.\n");
    initialPoolStrategy->generateInitialSolution(*childPartition, *graph);
    improvementStrategy->setStart(*childPartition);
    improvementStrategy->calibrateTemp();

    printf("Build initial pool.\n");
    initialPoolStrategy->buildInitialPool(frt, *population, *graph, improvementStrategy, maxSeconds, &generationCnt);

    int counter = 1;

    while (instance->getNumberOfNodes() / std::pow(2, counter) > 10) {
        counter++;
    }

    int MaxDiv = counter;
    int FixSetSizeIndex = 0;
    int StagCounter = 0;
    std::vector<int> tt;
    std::vector<std::vector<int>> SuperNodes;
    int cObj;

    // TODO: if partitions have different number of cliques, getFix can fail
    for (auto partition : population->getPartitions()) {
        CPPSolutionBase* mSolution = new CPPSolutionBase(partition->getPvertex(), nnode, partition->getValue(), instance);
        problem->AddToSolutionHolder(*mSolution);
    }
    
    printf("Run evolution.\n");
    // for (int i = 0; i < MaxGenerated - mFixInitPopulation; ++i) {
    while (generationCnt < maxGenerations && (double)(clock() - startTime) / CLOCKS_PER_SEC < maxSeconds && frt->best_val < graph->getKnownbest()) {
        printf("\n------------The %dth generation-----------\n", generationCnt);

        if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - mStartTime).count() > iTimeLimit) break;

        recorder->enter("run_generation");

        FixSize = 1 - std::pow(2, -1 * (FixSetSizeIndex + 1));
        FixSet = problem->GetFix(mFixN, mFixK, FixSize);

        problem->SolveGreedy(FixSet);

        // TODO this could be optimized by no conversion and using the same solution/partition structure
        convertCPPSolutionToPartition(*childPartition, problem->GetSolution());
 
        improvementStrategy->improveSolution(*childPartition, startTime, maxSeconds, frt, generationCnt);

        recorder->recordSolution(childPartition, clock());

        CPPSolutionBase* mSolution = new CPPSolutionBase(childPartition->getPvertex(), nnode, childPartition->getValue(), instance);

        problem->AddToSolutionHolder(*mSolution);
        mNumberOfSolutionsGenerated++;

        if (!problem->CheckBest(FixSize)) {
            StagCounter++;
            if (StagCounter >= mFixStagnation) {
                FixSetSizeIndex++;
                FixSetSizeIndex %= MaxDiv;
                StagCounter = 0;
            }
        }
        else {
            StagCounter = 0;
        }
        
        recorder->exit("run_generation");

        printf("Generation %d, best value: %d",
            generationCnt, frt->best_val);
        generationCnt++;
    }

    printf("Best solution found with value %d at generation %d\n", frt->best_val, frt->best_generation);
}

void FixedSetEvolution::runGeneration() {

}
