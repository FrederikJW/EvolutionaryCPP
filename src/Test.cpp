#include "Test.h"

void StrategyTests::testSAImprovement() {
    printf("-----Test Simulated Annealing-----\n");

    char filename[1000] = "instance/Medium Set (25 instances)/p1000-1.txt";
    int knownbest = 10000000;
    double minpercent = 1.0;
    double tempfactor = 0.96;
    int sizefactor = 8;
    int time = 500;
    int generationCnt = 0;

    BestSolutionInfo finalBest;
    Graph graph;
    graph.load(filename);
    graph.setKnownbest(knownbest);
    int nnode = graph.getNodeCount();
    RandomGenerator* randomGenerator = new RandomGenerator(param_seed);

    finalBest.best_partition = new Partition(nnode);
    clearResult(&finalBest);

    Recorder* recorder = new Recorder(filename, "SATest", false);
    Partition* childPartition = new Partition(graph.getNodeCount());

    ImprovementStrategy* improvementStrategy = new SimulatedAnnealingImprovement(knownbest, minpercent, tempfactor, sizefactor, recorder, randomGenerator, true);
    improvementStrategy->setEnvironment(graph);

    // build Partition
    int* initpart = new int[nnode];
    int sum = 0;
    for (int i = 0; i < nnode; i++) {
        initpart[i] = i + 1;
        sum += graph.getMatrix()[i][i];
    }
    childPartition->buildPartition(initpart);
    childPartition->setValue(sum);
    delete[] initpart;

    improvementStrategy->setStart(*childPartition);
    improvementStrategy->calibrateTemp();

    improvementStrategy->improveSolution(*childPartition, clock(), time, &finalBest, generationCnt);

    delete improvementStrategy;
    delete recorder;
    delete childPartition;
    delete randomGenerator;
}

void StrategyTests::testSAeImprovement() {
    printf("-----Test Simulated Annealing Extended-----\n");

    char filename[1000] = "instance/Medium Set (25 instances)/p1000-1.txt";
    int knownbest = 10000000;
    double minpercent = 1.0;
    double tempfactor = 0.96;
    int sizefactor = 8;
    int time = 500;
    int generationCnt = 0;

    BestSolutionInfo finalBest;
    Graph graph;
    graph.load(filename);
    graph.setKnownbest(knownbest);
    int nnode = graph.getNodeCount();
    RandomGenerator* randomGenerator = new RandomGenerator(param_seed);

    finalBest.best_partition = new Partition(nnode);
    clearResult(&finalBest);

    Recorder* recorder = new Recorder(filename, "SATest", false);
    Partition* childPartition = new Partition(graph.getNodeCount());

    ImprovementStrategy* improvementStrategy = new SaloExtendedImprovement(knownbest, minpercent, tempfactor, sizefactor, recorder, randomGenerator, true);
    improvementStrategy->setEnvironment(graph);

    // build Partition
    int* initpart = new int[nnode];
    int sum = 0;
    for (int i = 0; i < nnode; i++) {
        initpart[i] = i + 1;
        sum += graph.getMatrix()[i][i];
    }
    childPartition->buildPartition(initpart);
    childPartition->setValue(sum);
    delete[] initpart;

    improvementStrategy->setStart(*childPartition);
    improvementStrategy->calibrateTemp();

    improvementStrategy->improveSolution(*childPartition, clock(), time, &finalBest, generationCnt);

    delete improvementStrategy;
    delete recorder;
    delete childPartition;
    delete randomGenerator;
}

void StrategyTests::testStdInitStrategy() {
    printf("-----Test Standard Pool Builder-----\n");

    char filename[1000] = "instance/Medium Set (25 instances)/p1000-1.txt";
    int pool_size = 10;
    int knownbest = 10000000;
    double minpercent = 1.0;
    double tempfactor = 0.96;
    int sizefactor = 8;
    int time = 500;
    int generationCnt = 0;

    BestSolutionInfo finalBest;
    Graph graph;
    graph.load(filename);
    graph.setKnownbest(knownbest);
    int nnode = graph.getNodeCount();
    RandomGenerator* randomGenerator = new RandomGenerator(param_seed);

    Partition* childPartition = new Partition(graph.getNodeCount());
    Recorder* recorder = new Recorder(filename, "StdInitTest", false);

    finalBest.best_partition = new Partition(nnode);
    clearResult(&finalBest);

    Population population(pool_size);

    // assuming SimulatedAnnealingImprovement works correctly; needs to be testet previously in a full test execution
    ImprovementStrategy* improvementStrategy = new SimulatedAnnealingImprovement(knownbest, minpercent, tempfactor, sizefactor, recorder, randomGenerator, true);
    InitialPoolStrategy* poolBuilder = new SingletonInitStrategy(recorder, randomGenerator);

    improvementStrategy->setEnvironment(graph);
    poolBuilder->generateInitialSolution(*childPartition, graph);
    improvementStrategy->setStart(*childPartition);
    improvementStrategy->calibrateTemp();
    poolBuilder->buildInitialPool(&finalBest, population, graph, improvementStrategy, time, &generationCnt);

    delete improvementStrategy;
    delete poolBuilder;
    delete recorder;
    delete childPartition;
    delete randomGenerator;
}

void StrategyTests::testRCLInitStrategy() {
    printf("-----Test RCL Pool Builder-----\n");

    char filename[1000] = "instance/Medium Set (25 instances)/p1000-1.txt";
    int pool_size = 10;
    int knownbest = 10000000;
    double minpercent = 1.0;
    double tempfactor = 0.96;
    int sizefactor = 8;
    int time = 500;
    int generationCnt = 0;

    BestSolutionInfo finalBest;
    Graph graph;
    graph.load(filename);
    graph.setKnownbest(knownbest);
    int nnode = graph.getNodeCount();
    RandomGenerator* randomGenerator = new RandomGenerator(param_seed);

    Partition* childPartition = new Partition(graph.getNodeCount());
    Recorder* recorder = new Recorder(filename, "RCLTest", false);

    finalBest.best_partition = new Partition(nnode);
    clearResult(&finalBest);

    Population population(pool_size);

    // assuming SimulatedAnnealingImprovement works correctly; needs to be testet previously in a full test execution
    ImprovementStrategy* improvementStrategy = new SimulatedAnnealingImprovement(knownbest, minpercent, tempfactor, sizefactor, recorder, randomGenerator, true);
    InitialPoolStrategy* RCLStrategy = new RCLInitStrategy(recorder, randomGenerator);

    improvementStrategy->setEnvironment(graph);
    RCLStrategy->generateInitialSolution(*childPartition, graph);
    improvementStrategy->setStart(*childPartition);
    improvementStrategy->calibrateTemp();
    RCLStrategy->buildInitialPool(&finalBest, population, graph, improvementStrategy, time, &generationCnt);

    delete improvementStrategy;
    delete RCLStrategy;
    delete recorder;
    delete childPartition;
    delete randomGenerator;
}

void StrategyTests::testSolEvolution() {
    printf("-----Test Solution Crossover Evolution-----\n");

    char filename[1000] = "instance/Medium Set (25 instances)/p1000-1.txt";
    int knownbest = 10000000;
    double minpercent = 1.0;
    double tempfactor = 0.96;
    int sizefactor = 8;
    int time = 500;
    int generationCnt = 0;

    BestSolutionInfo finalBest;
    Graph graph;
    graph.load(filename);
    graph.setKnownbest(knownbest);
    int nnode = graph.getNodeCount();

    finalBest.best_partition = new Partition(nnode);
    clearResult(&finalBest);

    Recorder* recorder = new Recorder(filename, "CrossEvoTest", false);

    int run_cnt = 0;
    float sumtime = 0.0;
    int sumres = 0;
    int sumiter = 0;
    int bestInAll = -MAX_VAL;
    int* bestInAlllPartition = new int[nnode];
    int totalgen = 0;

    while (run_cnt < 1) {
        clearResult(&finalBest);
        RandomGenerator* randomGenerator = new RandomGenerator(param_seed + run_cnt);

        ImprovementStrategy* improvementStrategy = new SimulatedAnnealingImprovement(param_knownbest, param_minpercent, param_tempfactor, param_sizefactor, recorder, randomGenerator, true);
        CrossoverStrategy* mergeDevideCrossover = new MergeDivideCrossover(param_shrink);
        InitialPoolStrategy* initialPoolstrategy = new SingletonInitStrategy(recorder, randomGenerator);
        EvolutionStrategy* evolutionStrategy = new MdxEvolution(mergeDevideCrossover, initialPoolstrategy, improvementStrategy, &graph, recorder, param_max_generations, param_time, randomGenerator);

        evolutionStrategy->run(&finalBest, &totalgen, param_pool_size);

        if (finalBest.best_val > bestInAll) {
            bestInAll = finalBest.best_val;
        }
        sumtime += finalBest.best_foundtime;
        sumres += finalBest.best_val;
        sumiter += finalBest.best_generation;

        run_cnt++;

        delete improvementStrategy;
        delete mergeDevideCrossover;
        delete initialPoolstrategy;
        delete evolutionStrategy;
        delete randomGenerator;
    }

    // verifySolution(finalBest.best_partition, &graph);

    delete[] bestInAlllPartition;
    delete recorder;
}

void StrategyTests::testFSSEvolution() {
    printf("-----Test Fix Set Search Evolution-----\n");

    char filename[1000] = "instance/Medium Set (25 instances)/p1000-1.txt";
    int knownbest = 10000000;
    double minpercent = 1.0;
    double tempfactor = 0.96;
    int sizefactor = 8;
    int time = 500;
    int generationCnt = 0;

    BestSolutionInfo finalBest;
    Graph graph;
    graph.load(filename);
    graph.setKnownbest(knownbest);
    int nnode = graph.getNodeCount();

    finalBest.best_partition = new Partition(nnode);
    clearResult(&finalBest);

    Recorder* recorder = new Recorder(filename, "CrossEvoTest", false);

    int run_cnt = 0;
    float sumtime = 0.0;
    int sumres = 0;
    int sumiter = 0;
    int bestInAll = -MAX_VAL;
    int* bestInAlllPartition = new int[nnode];
    int totalgen = 0;

    while (run_cnt < 1) {
        clearResult(&finalBest);
        RandomGenerator* randomGenerator = new RandomGenerator(param_seed + run_cnt);

        ImprovementStrategy* improvementStrategy = new SimulatedAnnealingImprovement(param_knownbest, param_minpercent, param_tempfactor, param_sizefactor, recorder, randomGenerator, true);
        CrossoverStrategy* mergeDevideCrossover = new MergeDivideCrossover(param_shrink);
        InitialPoolStrategy* initialPoolstrategy = new SingletonInitStrategy(recorder, randomGenerator);
        EvolutionStrategy* evolutionStrategy = new FixedSetEvolution(mergeDevideCrossover, initialPoolstrategy, improvementStrategy, &graph, recorder, param_max_generations, param_time, randomGenerator);

        evolutionStrategy->run(&finalBest, &totalgen, param_pool_size);

        if (finalBest.best_val > bestInAll) {
            bestInAll = finalBest.best_val;
        }
        sumtime += finalBest.best_foundtime;
        sumres += finalBest.best_val;
        sumiter += finalBest.best_generation;

        run_cnt++;

        delete improvementStrategy;
        delete mergeDevideCrossover;
        delete initialPoolstrategy;
        delete evolutionStrategy;
        delete randomGenerator;
    }

    // verifySolution(finalBest.best_partition, &graph);

    delete[] bestInAlllPartition;
    delete recorder;
}

void StrategyTests::fullTest() {
    testFSSEvolution();
}
