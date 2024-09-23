#include "Defines.h"
#include "MemeticRun.h"
#include "Recorder.h"
#include "Statistic.h"
#include "Test.h"
#include "strategies/InitialPoolBuilder.h"
#include "strategies/MergeDivideCrossover.h"
#include "strategies/SimulatedAnnealingImprovement.h"
#include "strategies/RCLInitStrategy.h"
#include "strategies/SaloExtendedImprovement.h"
#include "strategies/EvolutionStrategy.h"
#include "strategies/FixedSetEvolution.h"
#include "strategies/SolutionEvolution.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <climits>
#include <string>
// #include <unistd.h>

int nnode;
int nedge;
int** matrix;

// char param_filename[1000] = "instance/rand500-100.txt";
char param_filename[1000] = "instance/Medium Set (25 instances)/p1000-1.txt";
int param_knownbest = 10000000;
int param_time = 5000;
int param_seed = 123456;
int param_max_generations = 100000;
int param_sizefactor = 8;
double param_tempfactor = 0.96;
double param_minpercent = 1.0;
double param_shrink = 0.6;
int param_pool_size = 10;

double totaltime;
int totalgen;
BestSolutionInfo finalBest;
FILE* fout = NULL;

// Enum for Evolution Strategy
enum class EvolutionStrategyCode {
    Sol, // Sol
    FSS  // FSS
};

// Enum for Improvement Strategy
enum class ImprovementStrategyCode {
    SA,  // SA
    SAe  // SAe
};

// Enum for Initial Pool Strategy
enum class InitialPoolStrategyCode {
    RCL, // RCL
    Std  // Std
};

void showUsage() {
    std::cerr << "usage: [-f <file path>] [-t <run time>] [-g <seed>] [-v <best solution>] "
        << "[-x <max generation>] [-b <theta size>] [-c <theta cool>] [-d <theta minper>] "
        << "[-s <shrink ratio>] [-p <pool size>]\n" << std::endl;
}

std::string getAlgorithmConfigCode(EvolutionStrategyCode evol, ImprovementStrategyCode impr, InitialPoolStrategyCode ini) {
    std::string configCode = "";
    switch (impr) {
    case ImprovementStrategyCode::SA:
        configCode += "SA-";
        break;
    case ImprovementStrategyCode::SAe:
        configCode += "SAe-";
        break;
    }

    CrossoverStrategy* mergeDevideCrossover = new MergeDivideCrossover(param_shrink);

    InitialPoolStrategy* initialPoolstrategy = nullptr;
    switch (ini) {
    case InitialPoolStrategyCode::RCL:
        configCode += "RCL-";
        break;
    case InitialPoolStrategyCode::Std:
        configCode += "Std-";
        break;
    }

    EvolutionStrategy* evolutionStrategy = nullptr;
    switch (evol) {
    case EvolutionStrategyCode::Sol:
        configCode += "Sol";
        break;
    case EvolutionStrategyCode::FSS:
        configCode += "FSS";
        break;
    }
    return configCode;
}

void readParameters(int argc, char** argv) {
    for (int i = 1; i < argc; i += 2) {
        if (argv[i][0] != '-' || argv[i][2] != 0) {
            showUsage();
            exit(0);
        }
        else if (argv[i][1] == 'f') {
            strncpy(param_filename, argv[i + 1], 1000);
        }
        else if (argv[i][1] == 't') {
            param_time = atoi(argv[i + 1]);
        }
        else if (argv[i][1] == 'g') {
            param_seed = atoi(argv[i + 1]);
        }
        else if (argv[i][1] == 'v') {
            param_knownbest = atoi(argv[i + 1]);
        }
        else if (argv[i][1] == 'x') {
            param_max_generations = atoi(argv[i + 1]);
        }
        else if (argv[i][1] == 'b') {
            param_sizefactor = atoi(argv[i + 1]);
        }
        else if (argv[i][1] == 'c') {
            param_tempfactor = atof(argv[i + 1]);
        }
        else if (argv[i][1] == 'd') {
            param_minpercent = atof(argv[i + 1]);
        }
        else if (argv[i][1] == 's') {
            param_shrink = atof(argv[i + 1]);
        }
        else if (argv[i][1] == 'p') {
            param_pool_size = atoi(argv[i + 1]);
        }
    }

    if (strlen(param_filename) == 0) {
        std::cerr << "No input data" << std::endl;
        exit(1);
    }
}

FILE* setupRecordFile() {
    char path_cwd[PATH_MAX];
    char file_name[PATH_MAX];

#ifdef _WIN32
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];
    _splitpath(param_filename, drive, dir, fname, ext);
    sprintf(file_name, "%s/rec/%s_%5d.rec", _getcwd(path_cwd, PATH_MAX), fname, param_seed);
#else
    char* graph_name = basename(param_filename);
    getcwd(path_cwd, PATH_MAX);
    sprintf(file_name, "%s/rec/%s_%5d.rec", path_cwd, graph_name, param_seed);
#endif

    std::cout << file_name << std::endl;

    FILE* f = fopen(file_name, "w+");
    if (f == NULL) {
        return 0;
    }
    return f;
}

void verifySolution(Partition* partition, Graph* graph) {
    int score = 0;
    for (int i = 0; i < graph->getNodeCount(); i++) {
        for (int j = i; j < graph->getNodeCount(); j++) {
            if (partition->getPvertex()[i] == partition->getPvertex()[j]) {
                score += graph->getMatrix()[i][j];
            }
        }
    }

    if (partition->getValue() == score) {
        printf("The score of %d has been successfully verified", score);
    }
    else {
        printf("The given score of %d does not equal the verification score of %d", partition->getValue(), score);
    }
}

void reportResult() {
    printf("\n");
    printf("$seed=%d\n", param_seed);
    printf("@solver=CPP\n");
    printf("@para_file=%s\n", param_filename);
    printf("@para_kbv=%d\n", param_knownbest);
    printf("@para_seconds=%d\n", param_time);
    printf("#vnum=%d\n", nnode);
    printf("#objv=%d\n", finalBest.best_val);
    printf("#bestiter=%d\n", finalBest.best_generation);
    printf("#besttime=%.3f\n", finalBest.best_foundtime);
    printf("#totoaltime=%.2f\n", totaltime);
    printf("#totoaliter=%d\n", totalgen);
    printf("#bestpartition=%d\n", finalBest.best_partition->getBucketSize() - 1);
    printf("Solution:");
    for (int i = 0; i < nnode; i++) {
        printf("%d ", finalBest.best_partition->getPvertex()[i]);
    }
    printf("\n");
}

int test_run(int argc, char** argv) {
    StrategyTests tests;
    tests.fullTest();

    return 0;
}

int rcl_test(int argc, char** argv) {
    // loading graph
    Graph graph;
    graph.load(param_filename);

    nnode = graph.getNodeCount();

    Recorder* recorder = new Recorder(param_filename, "RCLTest", true);

    finalBest.best_partition = new Partition(nnode);
    clearResult(&finalBest);

    Population population(param_pool_size);

    int generationCnt = 0;

    ImprovementStrategy* improvementStrategy = new SimulatedAnnealingImprovement(param_knownbest, param_minpercent, param_tempfactor, param_sizefactor, recorder);
    InitialPoolStrategy* RCLStrategy = new RCLInitStrategy(recorder);
    RCLStrategy->buildInitialPool(&finalBest, population, graph, improvementStrategy, param_time, &generationCnt);

    return 0;
}

int bulk_run(int argc, char** argv) {
    Graph graph;
    readParameters(argc, argv);
    srand(param_seed);

    // rand100-5 1407
    // rand100-100 24296
    // rand200-5 4079
    // rand200-100 74924
    // rand300-5 7732
    // rand300-100 152709
    // rand400-5 12133
    // rand400-100 222757

    // file name, known best, improvement strategy, initial pool strategy, evolution strategy
    /*
    std::vector<std::tuple<std::string, int, ImprovementStrategyCode, InitialPoolStrategyCode, EvolutionStrategyCode>> list_of_run_settings = {
        std::make_tuple("instance/Small Set (38 instances)/rand100-5.txt", 1407, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS),
        std::make_tuple("instance/Small Set (38 instances)/rand100-100.txt", 24296, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS),
        std::make_tuple("instance/Small Set (38 instances)/rand200-5.txt", 4079, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS),
        std::make_tuple("instance/Small Set (38 instances)/rand200-100.txt", 74924, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS),
        std::make_tuple("instance/Small Set (38 instances)/rand300-5.txt", 7732, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS),
        std::make_tuple("instance/Small Set (38 instances)/rand300-100.txt", 152709, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS),
        std::make_tuple("instance/Small Set (38 instances)/rand400-5.txt", 12133, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS),
        std::make_tuple("instance/Small Set (38 instances)/rand400-100.txt", 222757, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS),
    };*/
    std::vector<std::tuple<std::string, int, ImprovementStrategyCode, InitialPoolStrategyCode, EvolutionStrategyCode>> list_of_run_settings = {
        std::make_tuple("instance/Small Set (38 instances)/p500-5-1.txt", 17691, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS),
        std::make_tuple("instance/Small Set (38 instances)/p500-5-2.txt", 17169, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS),
        std::make_tuple("instance/Small Set (38 instances)/p500-5-3.txt", 16816, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS),
        std::make_tuple("instance/Small Set (38 instances)/p500-5-4.txt", 16808, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS),
        std::make_tuple("instance/Small Set (38 instances)/p500-5-5.txt", 16957, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS),
        std::make_tuple("instance/Small Set (38 instances)/p500-5-6.txt", 16615, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS),
        std::make_tuple("instance/Small Set (38 instances)/p500-5-7.txt", 16649, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS),
        std::make_tuple("instance/Small Set (38 instances)/p500-5-8.txt", 16756, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS),
        std::make_tuple("instance/Small Set (38 instances)/p500-5-9.txt", 16629, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS),
        std::make_tuple("instance/Small Set (38 instances)/p500-5-10.txt", 17360, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS),
        std::make_tuple("instance/Small Set (38 instances)/p500-100-1.txt", 308896, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS),
        std::make_tuple("instance/Small Set (38 instances)/p500-100-2.txt", 310241, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS),
        std::make_tuple("instance/Small Set (38 instances)/p500-100-3.txt", 310477, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS),
        std::make_tuple("instance/Small Set (38 instances)/p500-100-4.txt", 309567, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS),
        std::make_tuple("instance/Small Set (38 instances)/p500-100-5.txt", 309135, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS),
        std::make_tuple("instance/Small Set (38 instances)/p500-100-6.txt", 310280, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS),
        std::make_tuple("instance/Small Set (38 instances)/p500-100-7.txt", 310063, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS),
        std::make_tuple("instance/Small Set (38 instances)/p500-100-8.txt", 303148, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS),
        std::make_tuple("instance/Small Set (38 instances)/p500-100-9.txt", 305305, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS),
        std::make_tuple("instance/Small Set (38 instances)/p500-100-10.txt", 314864, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS),
    };

    Recorder* bulkRecorder = new Recorder("p500.txt", "BulkRun", true);

    for (const auto& run_settings : list_of_run_settings) {
        std::string filename;
        int knownbest;
        ImprovementStrategyCode improvementStrategyCode;
        InitialPoolStrategyCode initialPoolStrategyCode;
        EvolutionStrategyCode evolutionStrategyCode;

        std::tie(filename, knownbest, improvementStrategyCode, initialPoolStrategyCode, evolutionStrategyCode) = run_settings;
        char filename_charlist[1000];
        std::strcpy(filename_charlist, filename.c_str());

        Recorder* recorder = new Recorder(filename_charlist, getAlgorithmConfigCode(evolutionStrategyCode, improvementStrategyCode, initialPoolStrategyCode), false);

        graph.load(filename_charlist);
        graph.setKnownbest(knownbest);
        nnode = graph.getNodeCount();
        delete finalBest.best_partition;
        finalBest.best_partition = new Partition(nnode);

        std::string line = "";
        for (int i = 0; i < argc; i++) {
            line += argv[i];
        }
        recorder->writeLine(line);

        int run_cnt = 0;
        float sumtime = 0.0;
        int sumres = 0;
        int sumiter = 0;
        int bestInAll = -MAX_VAL;
        int* bestInAlllPartition = new int[nnode];

        while (run_cnt < 1) {
            clearResult(&finalBest);
            clock_t starttime = clock();
            recorder->setStartTime(starttime);

            ImprovementStrategy* improvementStrategy = nullptr;
            switch (improvementStrategyCode) {
            case ImprovementStrategyCode::SA:
                improvementStrategy = new SimulatedAnnealingImprovement(knownbest, param_minpercent, param_tempfactor, param_sizefactor, recorder);
                break;
            case ImprovementStrategyCode::SAe:
                improvementStrategy = new SaloExtendedImprovement(knownbest, param_minpercent, param_tempfactor, param_sizefactor, recorder);
                break;
            }

            CrossoverStrategy* mergeDevideCrossover = new MergeDivideCrossover(param_shrink);

            InitialPoolStrategy* initialPoolstrategy = nullptr;
            switch (initialPoolStrategyCode) {
            case InitialPoolStrategyCode::RCL:
                initialPoolstrategy = new RCLInitStrategy(recorder);
                break;
            case InitialPoolStrategyCode::Std:
                initialPoolstrategy = new InitialPoolBuilder(recorder);
                break;
            }

            EvolutionStrategy* evolutionStrategy = nullptr;
            switch (evolutionStrategyCode) {
            case EvolutionStrategyCode::Sol:
                evolutionStrategy = new SolutionEvolution(mergeDevideCrossover, initialPoolstrategy, improvementStrategy, &graph, recorder, param_max_generations, param_time);
                break;
            case EvolutionStrategyCode::FSS:
                evolutionStrategy = new FixedSetEvolution(mergeDevideCrossover, initialPoolstrategy, improvementStrategy, &graph, recorder, param_max_generations, param_time);
                break;
            }

            evolutionStrategy->run(&finalBest, &totalgen, param_pool_size);

            totaltime = (double)(clock() - starttime) / CLOCKS_PER_SEC;

            recorder->writeLine("idx\t best_v\t npat\t find_t\t find_i\t ttl_t\t  ttl_i");
            recorder->writeLine(std::to_string(run_cnt + 1) + "\t" + std::to_string(finalBest.best_val) + "\t" + std::to_string(finalBest.best_partition->getBucketSize() - 1) + "\t"
                + std::to_string(finalBest.best_foundtime) + "\t" + std::to_string(finalBest.best_generation) + "\t" + std::to_string(totaltime) + "\t" + std::to_string(totalgen));

            if (finalBest.best_val > bestInAll) {
                bestInAll = finalBest.best_val;
            }
            sumtime += finalBest.best_foundtime;
            sumres += finalBest.best_val;
            sumiter += finalBest.best_generation;

            reportResult();

            run_cnt++;

            delete improvementStrategy;
            delete mergeDevideCrossover;
            delete initialPoolstrategy;
            delete evolutionStrategy;
        }

        recorder->writeLine("best result: " + std::to_string(bestInAll));
        recorder->writeLine("average time: " + std::to_string(sumtime / run_cnt));
        recorder->writeLine("average result: " + std::to_string(sumres / run_cnt));
        recorder->writeLine("average best iteration: " + std::to_string(sumiter / run_cnt));
        recorder->writeTimeResults();
        recorder->createTimeResultsFiles();

        verifySolution(finalBest.best_partition, &graph);

        delete recorder;
        delete[] bestInAlllPartition;
    }

    delete bulkRecorder;

    return 0;
}

int normal_run(int argc, char** argv) {
    // fout = NULL;

    Graph graph;

    readParameters(argc, argv);

    srand(param_seed);

    EvolutionStrategyCode evolutionStrategyCode = EvolutionStrategyCode::FSS; // Sol, FSS
    ImprovementStrategyCode improvementStrategyCode = ImprovementStrategyCode::SAe; // SA, SAe
    InitialPoolStrategyCode initialPoolStrategyCode = InitialPoolStrategyCode::RCL; // RCL, Std

    // fout = setupRecordFile();

    Recorder* recorder = new Recorder(param_filename, getAlgorithmConfigCode(evolutionStrategyCode, improvementStrategyCode, initialPoolStrategyCode), true);

    // use graph.load
    graph.load(param_filename);
    graph.setKnownbest(param_knownbest);
    nnode = graph.getNodeCount();
    finalBest.best_partition = new Partition(nnode);

    std::string line = "";
    for (int i = 0; i < argc; i++) {
        line += argv[i];
    }
    recorder->writeLine(line);

    int run_cnt = 0;
    float sumtime = 0.0;
    int sumres = 0;
    int sumiter = 0;
    int bestInAll = -MAX_VAL;
    int* bestInAlllPartition = new int[nnode];

    while (run_cnt < 1) {
        clearResult(&finalBest);
        clock_t starttime = clock();
        recorder->setStartTime(starttime);

        ImprovementStrategy* improvementStrategy = nullptr;
        switch (improvementStrategyCode) {
            case ImprovementStrategyCode::SA:
                improvementStrategy = new SimulatedAnnealingImprovement(param_knownbest, param_minpercent, param_tempfactor, param_sizefactor, recorder);
                break;
            case ImprovementStrategyCode::SAe:
                improvementStrategy = new SaloExtendedImprovement(param_knownbest, param_minpercent, param_tempfactor, param_sizefactor, recorder);
                break;
        }

        CrossoverStrategy* mergeDevideCrossover = new MergeDivideCrossover(param_shrink);

        InitialPoolStrategy* initialPoolstrategy = nullptr;
        switch (initialPoolStrategyCode) {
        case InitialPoolStrategyCode::RCL:
            initialPoolstrategy = new RCLInitStrategy(recorder);
            break;
        case InitialPoolStrategyCode::Std:
            initialPoolstrategy = new InitialPoolBuilder(recorder);
            break;
        }

        EvolutionStrategy* evolutionStrategy = nullptr;
        switch (evolutionStrategyCode) {
        case EvolutionStrategyCode::Sol:
            evolutionStrategy = new SolutionEvolution(mergeDevideCrossover, initialPoolstrategy, improvementStrategy, &graph, recorder, param_max_generations, param_time);
            break;
        case EvolutionStrategyCode::FSS:
            evolutionStrategy = new FixedSetEvolution(mergeDevideCrossover, initialPoolstrategy, improvementStrategy, &graph, recorder, param_max_generations, param_time);
            break;
        }

        evolutionStrategy->run(&finalBest, &totalgen, param_pool_size);

        // MemeticRun memeticRun(mergeDevideCrossover, initialPoolstrategy, improvementStrategy, param_max_generations, param_time);
        // memeticRun.run(&finalBest, graph, &totalgen, param_pool_size);

        totaltime = (double)(clock() - starttime) / CLOCKS_PER_SEC;

        recorder->writeLine("idx\t best_v\t npat\t find_t\t find_i\t ttl_t\t  ttl_i");
        recorder->writeLine(std::to_string(run_cnt + 1) + "\t" + std::to_string(finalBest.best_val) + "\t" + std::to_string(finalBest.best_partition->getBucketSize() - 1) + "\t"
            + std::to_string(finalBest.best_foundtime) + "\t" + std::to_string(finalBest.best_generation) + "\t" + std::to_string(totaltime) + "\t" + std::to_string(totalgen));
        /*fprintf(fout, "%-d\t %-d\t %-d\t %-.2f\t %-d\t %-.2f\t %-d\n", run_cnt + 1, finalBest.best_val, finalBest.best_partition->getBucketSize() - 1,
            finalBest.best_foundtime, finalBest.best_generation, totaltime, totalgen);*/

        if (finalBest.best_val > bestInAll) {
            bestInAll = finalBest.best_val;
        }
        sumtime += finalBest.best_foundtime;
        sumres += finalBest.best_val;
        sumiter += finalBest.best_generation;

        reportResult();

        run_cnt++;

        delete improvementStrategy;
        delete mergeDevideCrossover;
        delete initialPoolstrategy;
        delete evolutionStrategy;
    }

    recorder->writeLine("best result: " + std::to_string(bestInAll));
    recorder->writeLine("average time: " + std::to_string(sumtime / run_cnt));
    recorder->writeLine("average result: " + std::to_string(sumres / run_cnt));
    recorder->writeLine("average best iteration: " + std::to_string(sumiter / run_cnt));
    recorder->writeTimeResults();
    recorder->createTimeResultsFiles();
    /*
    fprintf(fout, "best result: %d\n", bestInAll);
    fprintf(fout, "average time: %.2f\n", sumtime / run_cnt);
    fprintf(fout, "average result: %.2f\n", (float)sumres / run_cnt);
    fprintf(fout, "average best iteration: %d\n", sumiter / run_cnt);
    fclose(fout);*/

    verifySolution(finalBest.best_partition, &graph);

    delete[] bestInAlllPartition;
    delete recorder;

    return 0;
}

int main(int argc, char** argv) {
    // return normal_run(argc, argv);
    return bulk_run(argc, argv);
}
