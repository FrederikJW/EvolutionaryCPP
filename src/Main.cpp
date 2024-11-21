#include "Defines.h"
#include "MemeticRun.h"
#include "Recorder.h"
#include "Statistic.h"
#include "Test.h"
#include "strategies/InitialPoolBuilder.h"
#include "strategies/MergeDivideCrossover.h"
#include "strategies/SimulatedAnnealingImprovement.h"
#include "strategies/RCLInitStrategy.h"
#include "strategies/SaloImprovement.h"
#include "strategies/SaloExtendedImprovement.h"
#include "strategies/SaloOverEdgesImprovement.h"
#include "strategies/SaloOverEdgesForcedDualImprovement.h"
#include "strategies/SaloDualNeighborImprovement.h"
#include "strategies/EvolutionStrategy.h"
#include "strategies/FixedSetEvolution.h"
#include "strategies/SolutionEvolution.h"
#include "util/Util.h"
#include "RandomGenerator.h"
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
std::string param_filename_string = std::string(PROJECT_ROOT_PATH) + "/instance/Medium Set (25 instances)/p1000-1.txt";
char param_filename[1000];
int param_knownbest = 10000000;
int param_time = 500;
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
    SALO,  // SAe
    SALOe,
    SALOoE,
    SOEFD,
    SALODN,
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
    case ImprovementStrategyCode::SALO:
        configCode += "SALO-";
        break;
    case ImprovementStrategyCode::SALOe:
        configCode += "SALOe-";
        break;
    case ImprovementStrategyCode::SALOoE:
        configCode += "SALOoE-";
        break;
    case ImprovementStrategyCode::SOEFD:
        configCode += "SOEFD-";
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
    const char* cstr = param_filename_string.c_str();
    std::strcpy(param_filename, cstr);

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
    RandomGenerator* randomGenerator = new RandomGenerator(param_seed);

    nnode = graph.getNodeCount();

    Recorder* recorder = new Recorder(param_filename, "RCLTest", true);

    finalBest.best_partition = new Partition(nnode);
    clearResult(&finalBest);

    Population population(param_pool_size);

    int generationCnt = 0;

    ImprovementStrategy* improvementStrategy = new SimulatedAnnealingImprovement(param_knownbest, param_minpercent, param_tempfactor, param_sizefactor, recorder, randomGenerator);
    InitialPoolStrategy* RCLStrategy = new RCLInitStrategy(recorder, randomGenerator);
    RCLStrategy->buildInitialPool(&finalBest, population, graph, improvementStrategy, param_time, &generationCnt);

    return 0;
}

int bulk_run(int argc, char** argv) {

    Graph graph;
    readParameters(argc, argv);

    /*  
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand100-5.txt", 1407, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand100-100.txt", 24296, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand200-5.txt", 4079, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand200-100.txt", 74924, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand300-5.txt", 7732, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand300-100.txt", 152709, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand400-5.txt", 12133, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand400-100.txt", 222757, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand500-5.txt", 17127, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand500-100.txt", 309125, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-1.txt", 17691, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 20),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-2.txt", 17169, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 20),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-3.txt", 16816, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 20),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-4.txt", 16808, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 20),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-5.txt", 16957, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 20),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-6.txt", 16615, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 20),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-7.txt", 16649, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 20),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-8.txt", 16756, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 20),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-9.txt", 16629, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 20),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-10.txt", 17360, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 20),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-1.txt", 308896, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 20),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-2.txt", 310241, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 20),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-3.txt", 310477, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 20),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-4.txt", 309567, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 20),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-5.txt", 309135, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 20),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-6.txt", 310280, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 20),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-7.txt", 310063, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 20),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-8.txt", 303148, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 20),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-9.txt", 305305, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 20),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-10.txt", 314864, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 20),*/

    /*
            std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand100-5.txt", 1407, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand100-5.txt", 1407, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand100-5.txt", 1407, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand100-100.txt", 24296, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand100-100.txt", 24296, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand100-100.txt", 24296, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand200-5.txt", 4079, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand200-5.txt", 4079, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand200-5.txt", 4079, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand200-100.txt", 74924, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand200-100.txt", 74924, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand200-100.txt", 74924, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand300-5.txt", 7732, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand300-5.txt", 7732, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand300-5.txt", 7732, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand300-100.txt", 152709, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand300-100.txt", 152709, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand300-100.txt", 152709, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand400-5.txt", 12133, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand400-5.txt", 12133, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand400-5.txt", 12133, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand400-100.txt", 222757, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand400-100.txt", 222757, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand400-100.txt", 222757, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand500-5.txt", 17127, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand500-5.txt", 17127, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand500-5.txt", 17127, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand500-100.txt", 309125, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand500-100.txt", 309125, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand500-100.txt", 309125, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
    */

    // std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand400-5.txt", 12133, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
    // std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/rand400-5.txt", 12133, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
    
    /*
            std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-1.txt", 17691, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-1.txt", 17691, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-1.txt", 17691, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-2.txt", 17169, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-2.txt", 17169, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-2.txt", 17169, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-3.txt", 16816, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-3.txt", 16816, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-3.txt", 16816, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-4.txt", 16808, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-4.txt", 16808, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-4.txt", 16808, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-5.txt", 16957, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-5.txt", 16957, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-5.txt", 16957, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-6.txt", 16615, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-6.txt", 16615, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-6.txt", 16615, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-7.txt", 16649, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-7.txt", 16649, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-7.txt", 16649, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-8.txt", 16756, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-8.txt", 16756, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-8.txt", 16756, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-9.txt", 16629, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-9.txt", 16629, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-9.txt", 16629, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-10.txt", 17360, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-10.txt", 17360, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-10.txt", 17360, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-1.txt", 308896, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-1.txt", 308896, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-1.txt", 308896, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-2.txt", 310241, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-2.txt", 310241, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-2.txt", 310241, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-3.txt", 310477, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-3.txt", 310477, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-3.txt", 310477, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-4.txt", 309567, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-4.txt", 309567, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-4.txt", 309567, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-5.txt", 309135, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-5.txt", 309135, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-5.txt", 309135, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-6.txt", 310280, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-6.txt", 310280, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-6.txt", 310280, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-7.txt", 310063, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-7.txt", 310063, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-7.txt", 310063, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-8.txt", 303148, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-8.txt", 303148, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-8.txt", 303148, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-9.txt", 305305, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-9.txt", 305305, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-9.txt", 305305, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-10.txt", 314864, ImprovementStrategyCode::SALO, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-10.txt", 314864, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-10.txt", 314864, ImprovementStrategyCode::SALOoE, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 10),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/MCF/Wang250.txt", 419, ImprovementStrategyCode::SOEFD, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 3),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/MCF/Wang250.txt", 419, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 3),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/MCF/Wang800.txt", 1788, ImprovementStrategyCode::SOEFD, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 3),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/MCF/Wang800.txt", 1788, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 3),
     */

    /*
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-1.txt", 17691, ImprovementStrategyCode::SOEFD, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-2.txt", 17169, ImprovementStrategyCode::SOEFD, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-3.txt", 16816, ImprovementStrategyCode::SOEFD, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-4.txt", 16808, ImprovementStrategyCode::SOEFD, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-5.txt", 16957, ImprovementStrategyCode::SOEFD, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-6.txt", 16615, ImprovementStrategyCode::SOEFD, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-7.txt", 16649, ImprovementStrategyCode::SOEFD, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-8.txt", 16756, ImprovementStrategyCode::SOEFD, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-9.txt", 16629, ImprovementStrategyCode::SOEFD, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-10.txt", 17360, ImprovementStrategyCode::SOEFD, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-1.txt", 308896, ImprovementStrategyCode::SOEFD, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-2.txt", 310241, ImprovementStrategyCode::SOEFD, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-3.txt", 310477, ImprovementStrategyCode::SOEFD, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-4.txt", 309567, ImprovementStrategyCode::SOEFD, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-5.txt", 309135, ImprovementStrategyCode::SOEFD, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-6.txt", 310280, ImprovementStrategyCode::SOEFD, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-7.txt", 310063, ImprovementStrategyCode::SOEFD, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-8.txt", 303148, ImprovementStrategyCode::SOEFD, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-9.txt", 305305, ImprovementStrategyCode::SOEFD, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-10.txt", 314864, ImprovementStrategyCode::SOEFD, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 5),*/

    // file name, known best, improvement strategy, initial pool strategy, evolution strategy, number of runs
    std::vector<std::tuple<std::string, int, ImprovementStrategyCode, InitialPoolStrategyCode, EvolutionStrategyCode, int>> list_of_run_settings = {
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-1.txt", 17691, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::Std, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-1.txt", 17691, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::Sol, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-2.txt", 17169, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::Std, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-2.txt", 17169, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::Sol, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-3.txt", 16816, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::Std, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-3.txt", 16816, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::Sol, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-4.txt", 16808, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::Std, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-4.txt", 16808, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::Sol, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-5.txt", 16957, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::Std, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-5.txt", 16957, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::Sol, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-6.txt", 16615, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::Std, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-6.txt", 16615, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::Sol, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-7.txt", 16649, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::Std, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-7.txt", 16649, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::Sol, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-8.txt", 16756, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::Std, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-8.txt", 16756, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::Sol, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-9.txt", 16629, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::Std, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-9.txt", 16629, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::Sol, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-10.txt", 17360, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::Std, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-10.txt", 17360, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::Sol, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-1.txt", 308896, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::Std, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-1.txt", 308896, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::Sol, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-2.txt", 310241, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::Std, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-2.txt", 310241, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::Sol, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-3.txt", 310477, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::Std, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-3.txt", 310477, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::Sol, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-4.txt", 309567, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::Std, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-4.txt", 309567, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::Sol, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-5.txt", 309135, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::Std, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-5.txt", 309567, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::Sol, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-6.txt", 310280, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::Std, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-6.txt", 309567, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::Sol, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-7.txt", 310063, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::Std, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-7.txt", 309567, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::Sol, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-8.txt", 303148, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::Std, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-8.txt", 309567, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::Sol, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-9.txt", 305305, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::Std, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-9.txt", 309567, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::Sol, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-10.txt", 314864, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::Std, EvolutionStrategyCode::FSS, 5),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-10.txt", 309567, ImprovementStrategyCode::SALOe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::Sol, 5),
    };
    /*
    std::vector<std::tuple<std::string, int, ImprovementStrategyCode, InitialPoolStrategyCode, EvolutionStrategyCode, int>> list_of_run_settings = {
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Small Set (38 instances)/p500-5-3.txt", 16816, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 20),
    };*/

    Recorder* bulkRecorder = new Recorder("p500.txt", "InvertedComparison", true);

    for (const auto& run_settings : list_of_run_settings) {
        bulkRecorder->clearTimeResults();
        std::string filename;
        int knownbest;
        ImprovementStrategyCode improvementStrategyCode;
        InitialPoolStrategyCode initialPoolStrategyCode;
        EvolutionStrategyCode evolutionStrategyCode;
        int total_runs;

        std::tie(filename, knownbest, improvementStrategyCode, initialPoolStrategyCode, evolutionStrategyCode, total_runs) = run_settings;
        char filename_charlist[1000];
        std::strcpy(filename_charlist, filename.c_str());

        Recorder* recorder = new Recorder(filename_charlist, getAlgorithmConfigCode(evolutionStrategyCode, improvementStrategyCode, initialPoolStrategyCode), false);
        bulkRecorder->writeLine("\n" + filename + ":\n" + getAlgorithmConfigCode(evolutionStrategyCode, improvementStrategyCode, initialPoolStrategyCode));

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
        int found_best = 0;

        // TODO: correct generator and seeds for bulk runs
        while (run_cnt < total_runs) {
            RandomGenerator* randomGenerator = new RandomGenerator(param_seed + run_cnt);
            printf("Seed: %d\n", param_seed + run_cnt);
            clearResult(&finalBest);
            clock_t starttime = clock();
            recorder->writeLine("Seed: " + std::to_string(param_seed + run_cnt));
            recorder->setStartTime(starttime);

            ImprovementStrategy* improvementStrategy = nullptr;
            switch (improvementStrategyCode) {
            case ImprovementStrategyCode::SA:
                improvementStrategy = new SimulatedAnnealingImprovement(knownbest, param_minpercent, param_tempfactor, param_sizefactor, bulkRecorder, randomGenerator);
                break;
            case ImprovementStrategyCode::SALO:
                improvementStrategy = new SaloImprovement(knownbest, param_minpercent, param_tempfactor, param_sizefactor, bulkRecorder, randomGenerator);
                break;
            case ImprovementStrategyCode::SALOe:
                improvementStrategy = new SaloExtendedImprovement(knownbest, param_minpercent, param_tempfactor, param_sizefactor, bulkRecorder, randomGenerator);
                break;
            case ImprovementStrategyCode::SALOoE:
                improvementStrategy = new SaloOverEdgesImprovement(knownbest, param_minpercent, param_tempfactor, param_sizefactor, bulkRecorder, randomGenerator);
                break;
            case ImprovementStrategyCode::SOEFD:
                improvementStrategy = new SaloOverEdgesForcedDualImprovement(knownbest, param_minpercent, param_tempfactor, param_sizefactor, bulkRecorder, randomGenerator);
                break;
            case ImprovementStrategyCode::SALODN:
                improvementStrategy = new SaloDualNeighborImprovement(knownbest, param_minpercent, param_tempfactor, param_sizefactor, bulkRecorder, randomGenerator);
            }

            CrossoverStrategy* mergeDevideCrossover = new MergeDivideCrossover(param_shrink);

            InitialPoolStrategy* initialPoolstrategy = nullptr;
            switch (initialPoolStrategyCode) {
            case InitialPoolStrategyCode::RCL:
                initialPoolstrategy = new RCLInitStrategy(bulkRecorder, randomGenerator);
                break;
            case InitialPoolStrategyCode::Std:
                initialPoolstrategy = new InitialPoolBuilder(bulkRecorder, randomGenerator);
                break;
            }

            EvolutionStrategy* evolutionStrategy = nullptr;
            switch (evolutionStrategyCode) {
            case EvolutionStrategyCode::Sol:
                evolutionStrategy = new SolutionEvolution(mergeDevideCrossover, initialPoolstrategy, improvementStrategy, &graph, bulkRecorder, param_max_generations, param_time, randomGenerator);
                break;
            case EvolutionStrategyCode::FSS:
                evolutionStrategy = new FixedSetEvolution(mergeDevideCrossover, initialPoolstrategy, improvementStrategy, &graph, bulkRecorder, param_max_generations, param_time, randomGenerator);
                break;
            }

            evolutionStrategy->run(&finalBest, &totalgen, param_pool_size);

            totaltime = (double)(clock() - starttime) / CLOCKS_PER_SEC;

            if (finalBest.best_val < knownbest) {
                // bulkRecorder->writeLine("Did not find known best: " + std::to_string(finalBest.best_val) + " < " + std::to_string(knownbest));
            }
            else {
                found_best++;
                // bulkRecorder->writeLine("Found known best: " + std::to_string(finalBest.best_val) + " = " + std::to_string(knownbest));
            }

            /*
            bulkRecorder->writeLine("idx\t best_v\t npat\t find_t\t find_i\t ttl_t\t  ttl_i");
            bulkRecorder->writeLine(std::to_string(run_cnt + 1) + "\t" + std::to_string(finalBest.best_val) + "\t" + std::to_string(finalBest.best_partition->getBucketSize() - 1) + "\t"
                + std::to_string(finalBest.best_foundtime) + "\t" + std::to_string(finalBest.best_generation) + "\t" + std::to_string(totaltime) + "\t" + std::to_string(totalgen));
            */
            /*
            recorder->writeLine("idx\t best_v\t npat\t find_t\t find_i\t ttl_t\t  ttl_i");
            recorder->writeLine(std::to_string(run_cnt + 1) + "\t" + std::to_string(finalBest.best_val) + "\t" + std::to_string(finalBest.best_partition->getBucketSize() - 1) + "\t"
                + std::to_string(finalBest.best_foundtime) + "\t" + std::to_string(finalBest.best_generation) + "\t" + std::to_string(totaltime) + "\t" + std::to_string(totalgen));
            */
            if (finalBest.best_val > bestInAll) {
                bestInAll = finalBest.best_val;
            }
            sumtime += finalBest.best_foundtime;
            sumres += finalBest.best_val;
            sumiter += finalBest.best_generation;

            reportResult();
            verifySolution(finalBest.best_partition, &graph);

            run_cnt++;

            delete improvementStrategy;
            delete mergeDevideCrossover;
            delete initialPoolstrategy;
            delete evolutionStrategy;
            delete randomGenerator;
        }

        bulkRecorder->writeLine("Found best: " + std::to_string(found_best) + "/" + std::to_string(total_runs));

        bulkRecorder->writeLine("best result: " + std::to_string(bestInAll));
        bulkRecorder->writeLine("average time: " + std::to_string(sumtime / run_cnt));
        bulkRecorder->writeLine("average result: " + std::to_string(sumres / run_cnt));
        bulkRecorder->writeLine("average best iteration: " + std::to_string(sumiter / run_cnt));
        bulkRecorder->writeTimeResults();
        bulkRecorder->writeLine("-------------------------------------------------------------");
        // recorder->writeTimeResults();
        // recorder->createTimeResultsFiles();

        verifySolution(finalBest.best_partition, &graph);

        delete recorder;
        delete[] bestInAlllPartition;
    }

    delete bulkRecorder;

    return 0;
}

int normal_run(int argc, char** argv) {

    Graph graph;

    readParameters(argc, argv);

    srand(param_seed);

    EvolutionStrategyCode evolutionStrategyCode = EvolutionStrategyCode::FSS; // Sol, FSS
    ImprovementStrategyCode improvementStrategyCode = ImprovementStrategyCode::SALOe; // SA, SAe
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
        RandomGenerator* randomGenerator = new RandomGenerator(param_seed + run_cnt);
        clearResult(&finalBest);
        clock_t starttime = clock();
        recorder->setStartTime(starttime);

        ImprovementStrategy* improvementStrategy = nullptr;
        switch (improvementStrategyCode) {
            case ImprovementStrategyCode::SA:
                improvementStrategy = new SimulatedAnnealingImprovement(param_knownbest, param_minpercent, param_tempfactor, param_sizefactor, recorder, randomGenerator);
                break;
            case ImprovementStrategyCode::SALO:
                improvementStrategy = new SaloImprovement(param_knownbest, param_minpercent, param_tempfactor, param_sizefactor, recorder, randomGenerator);
                break;
            case ImprovementStrategyCode::SALOe:
                improvementStrategy = new SaloExtendedImprovement(param_knownbest, param_minpercent, param_tempfactor, param_sizefactor, recorder, randomGenerator);
                break;
        }

        CrossoverStrategy* mergeDevideCrossover = new MergeDivideCrossover(param_shrink);

        InitialPoolStrategy* initialPoolstrategy = nullptr;
        switch (initialPoolStrategyCode) {
        case InitialPoolStrategyCode::RCL:
            initialPoolstrategy = new RCLInitStrategy(recorder, randomGenerator);
            break;
        case InitialPoolStrategyCode::Std:
            initialPoolstrategy = new InitialPoolBuilder(recorder, randomGenerator);
            break;
        }

        EvolutionStrategy* evolutionStrategy = nullptr;
        switch (evolutionStrategyCode) {
        case EvolutionStrategyCode::Sol:
            evolutionStrategy = new SolutionEvolution(mergeDevideCrossover, initialPoolstrategy, improvementStrategy, &graph, recorder, param_max_generations, param_time, randomGenerator);
            break;
        case EvolutionStrategyCode::FSS:
            evolutionStrategy = new FixedSetEvolution(mergeDevideCrossover, initialPoolstrategy, improvementStrategy, &graph, recorder, param_max_generations, param_time, randomGenerator);
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
        delete randomGenerator;
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

int benchmark_improvement(int argc, char** argv) {

    Graph graph;
    readParameters(argc, argv);

         // file name, known best, improvement strategy, initial pool strategy, evolution strategy, number of runs
    std::vector<std::tuple<std::string, int, ImprovementStrategyCode, int>> list_of_run_settings = {
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-1.txt", 17691, ImprovementStrategyCode::SOEFD, 50),
        // std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-1.txt", 17691, ImprovementStrategyCode::SALOe, 50),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-2.txt", 17169, ImprovementStrategyCode::SOEFD, 50),
        // std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-2.txt", 17169, ImprovementStrategyCode::SALOe, 50),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-3.txt", 16816, ImprovementStrategyCode::SOEFD, 50),
        // std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-3.txt", 16816, ImprovementStrategyCode::SALOe, 50),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-4.txt", 16808, ImprovementStrategyCode::SOEFD, 50),
        // std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-4.txt", 16808, ImprovementStrategyCode::SALOe, 50),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-5.txt", 16957, ImprovementStrategyCode::SOEFD, 50),
        // std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-5.txt", 16957, ImprovementStrategyCode::SALOe, 50),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-6.txt", 16615, ImprovementStrategyCode::SOEFD, 50),
        // std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-6.txt", 16615, ImprovementStrategyCode::SALOe, 50),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-7.txt", 16649, ImprovementStrategyCode::SOEFD, 50),
        // std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-7.txt", 16649, ImprovementStrategyCode::SALOe, 50),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-8.txt", 16756, ImprovementStrategyCode::SOEFD, 50),
        // std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-8.txt", 16756, ImprovementStrategyCode::SALOe, 50),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-9.txt", 16629, ImprovementStrategyCode::SOEFD, 50),
        // std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-9.txt", 16629, ImprovementStrategyCode::SALOe, 50),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-10.txt", 17360, ImprovementStrategyCode::SOEFD, 50),
        // std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-5-10.txt", 17360, ImprovementStrategyCode::SALOe, 50),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-1.txt", 308896, ImprovementStrategyCode::SOEFD, 50),
        // std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-1.txt", 308896, ImprovementStrategyCode::SALOe, 50),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-2.txt", 310241, ImprovementStrategyCode::SOEFD, 50),
        // std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-2.txt", 310241, ImprovementStrategyCode::SALOe, 50),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-3.txt", 310477, ImprovementStrategyCode::SOEFD, 50),
        // std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-3.txt", 310477, ImprovementStrategyCode::SALOe, 50),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-4.txt", 309567, ImprovementStrategyCode::SOEFD, 50),
        // std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-4.txt", 309567, ImprovementStrategyCode::SALOe, 50),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-5.txt", 309135, ImprovementStrategyCode::SOEFD, 50),
        // std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-5.txt", 309135, ImprovementStrategyCode::SALOe, 50),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-6.txt", 310280, ImprovementStrategyCode::SOEFD, 50),
        // std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-6.txt", 310280, ImprovementStrategyCode::SALOe, 50),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-7.txt", 310063, ImprovementStrategyCode::SOEFD, 50),
        // std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-7.txt", 310063, ImprovementStrategyCode::SALOe, 50),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-8.txt", 303148, ImprovementStrategyCode::SOEFD, 50),
        // std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-8.txt", 303148, ImprovementStrategyCode::SALOe, 50),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-9.txt", 305305, ImprovementStrategyCode::SOEFD, 50),
        // std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-9.txt", 305305, ImprovementStrategyCode::SALOe, 50),
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-10.txt", 314864, ImprovementStrategyCode::SOEFD, 50),
        // std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Random/p500-100-10.txt", 314864, ImprovementStrategyCode::SALOe, 50),
    };
    /*
    std::vector<std::tuple<std::string, int, ImprovementStrategyCode, InitialPoolStrategyCode, EvolutionStrategyCode, int>> list_of_run_settings = {
        std::make_tuple(std::string(PROJECT_ROOT_PATH) + "/instance/Small Set (38 instances)/p500-5-3.txt", 16816, ImprovementStrategyCode::SAe, InitialPoolStrategyCode::RCL, EvolutionStrategyCode::FSS, 20),
    };*/

    Recorder* bulkRecorder = new Recorder("p500.txt", "SOEFD1Point5_test_ProbChange", true);

    for (const auto& run_settings : list_of_run_settings) {
        bulkRecorder->clearTimeResults();
        std::string filename;
        int knownbest;
        ImprovementStrategyCode improvementStrategyCode;
        int total_runs;

        std::tie(filename, knownbest, improvementStrategyCode, total_runs) = run_settings;
        char filename_charlist[1000];
        std::strcpy(filename_charlist, filename.c_str());

        std::vector<Partition> generatedSolutions;
        std::string configCode = "";
        switch (improvementStrategyCode) {
        case ImprovementStrategyCode::SA:
            configCode += "SA-";
            break;
        case ImprovementStrategyCode::SALO:
            configCode += "SALO-";
            break;
        case ImprovementStrategyCode::SALOe:
            configCode += "SALOe-";
            break;
        case ImprovementStrategyCode::SALOoE:
            configCode += "SALOoE-";
            break;
        case ImprovementStrategyCode::SOEFD:
            configCode += "SOEFD-";
            break;
        }

        Recorder* recorder = new Recorder(filename_charlist, configCode, false);
        bulkRecorder->writeLine("\n" + filename + ":\n" + configCode);

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
        int found_best = 0;

        // TODO: correct generator and seeds for bulk runs
        while (run_cnt < total_runs) {
            RandomGenerator* randomGenerator = new RandomGenerator(param_seed + run_cnt);
            printf("Seed: %d\n", param_seed + run_cnt);
            clearResult(&finalBest);
            clock_t starttime = clock();
            recorder->writeLine("Seed: " + std::to_string(param_seed + run_cnt));
            recorder->setStartTime(starttime);

            ImprovementStrategy* improvementStrategy = nullptr;
            switch (improvementStrategyCode) {
            case ImprovementStrategyCode::SA:
                improvementStrategy = new SimulatedAnnealingImprovement(knownbest, param_minpercent, param_tempfactor, param_sizefactor, bulkRecorder, randomGenerator);
                break;
            case ImprovementStrategyCode::SALO:
                improvementStrategy = new SaloImprovement(knownbest, param_minpercent, param_tempfactor, param_sizefactor, bulkRecorder, randomGenerator);
                break;
            case ImprovementStrategyCode::SALOe:
                improvementStrategy = new SaloExtendedImprovement(knownbest, param_minpercent, param_tempfactor, param_sizefactor, bulkRecorder, randomGenerator);
                break;
            case ImprovementStrategyCode::SALOoE:
                improvementStrategy = new SaloOverEdgesImprovement(knownbest, param_minpercent, param_tempfactor, param_sizefactor, bulkRecorder, randomGenerator);
                break;
            case ImprovementStrategyCode::SOEFD:
                improvementStrategy = new SaloOverEdgesForcedDualImprovement(knownbest, param_minpercent, param_tempfactor, param_sizefactor, bulkRecorder, randomGenerator);
                break;
            case ImprovementStrategyCode::SALODN:
                improvementStrategy = new SaloDualNeighborImprovement(knownbest, param_minpercent, param_tempfactor, param_sizefactor, bulkRecorder, randomGenerator);
            }

            // build Partition
            Partition* childPartition = new Partition(graph.getNodeCount());
            int* initpart = new int[nnode];
            int sum = 0;
            for (int i = 0; i < nnode; i++) {
                initpart[i] = i + 1;
                sum += graph.getMatrix()[i][i];
            }
            childPartition->buildPartition(initpart);
            childPartition->setValue(sum);
            delete[] initpart;

            improvementStrategy->setEnvironment(graph);

            improvementStrategy->setStart(*childPartition);
            improvementStrategy->calibrateTemp();

            improvementStrategy->improveSolution(*childPartition, clock(), 500, &finalBest, 0);
            generatedSolutions.push_back(improvementStrategy->getBestPartition());
            totaltime = (double)(clock() - starttime) / CLOCKS_PER_SEC;

            if (finalBest.best_val > bestInAll) {
                bestInAll = finalBest.best_val;
            }
            sumtime += finalBest.best_foundtime;
            sumres += finalBest.best_val;
            sumiter += finalBest.best_generation;

            reportResult();
            verifySolution(finalBest.best_partition, &graph);

            run_cnt++;

            delete improvementStrategy;
            delete randomGenerator;
        }
        int numPairs = 0;
        int totalDistance = 0;
        int maxDistance = INT_MIN;
        int minDistance = INT_MAX;
        int totalScore = 0;
        int maxScore = INT_MIN;
        int minScore = INT_MAX;
        for (int i = 0; i < generatedSolutions.size() - 1; i++) {
            int score = generatedSolutions[i].getValue();
            if (score < minScore) minScore = score;
            if (score > maxScore) maxScore = score;
            totalScore += score;

            for (int j = i + 1; j < generatedSolutions.size(); j++) {
                numPairs++;
                int distance = generatedSolutions[i].calculateDistance(generatedSolutions[i].getPvertex(), generatedSolutions[j].getPvertex());
                totalDistance += distance;
                if (distance < minDistance) minDistance = distance;
                if (distance > maxDistance) maxDistance = distance;
            }
        }

        float avgScore = totalScore / generatedSolutions.size();
        float avgDistance = totalDistance / numPairs;

        bulkRecorder->writeLine("numRuns: " + std::to_string(generatedSolutions.size()));

        bulkRecorder->writeLine("score (min/max/avg): " + std::to_string(minScore) + ";" + std::to_string(maxScore) + ";" + std::to_string(avgScore));
        bulkRecorder->writeLine("distance (min/max/avg): " + std::to_string(minDistance) + ";" + std::to_string(maxDistance) + ";" + std::to_string(avgDistance));
        bulkRecorder->writeLine("average time: " + std::to_string(sumtime / run_cnt));
        bulkRecorder->writeTimeResults();
        bulkRecorder->writeLine("-------------------------------------------------------------");
        // recorder->writeTimeResults();
        // recorder->createTimeResultsFiles();

        // verifySolution(finalBest.best_partition, &graph);

        delete recorder;
        delete[] bestInAlllPartition;
    }

    delete bulkRecorder;

    return 0;
}

int main(int argc, char** argv) {
    // return normal_run(argc, argv);
    //return benchmark_improvement(argc, argv);
    return bulk_run(argc, argv);
}
