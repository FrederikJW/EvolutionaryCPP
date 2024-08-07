#include "Defines.h"
#include "MemeticRun.h"
#include "strategies/InitialPoolBuilder.h"
#include "strategies/MergeDivideCrossover.h"
#include "strategies/SimulatedAnnealingImprovement.h"
#include "strategies/RCLInitStrategy.h"
#include "strategies/SaloExtendedImprovement.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <climits>
// #include <unistd.h>

int nnode;
int nedge;
int** matrix;

char param_filename[1000] = "instance/rand500-100.txt";
int param_knownbest = 309125;
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

void showUsage() {
    std::cerr << "usage: [-f <file path>] [-t <run time>] [-g <seed>] [-v <best solution>] "
        << "[-x <max generation>] [-b <theta size>] [-c <theta cool>] [-d <theta minper>] "
        << "[-s <shrink ratio>] [-p <pool size>]\n" << std::endl;
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

void clearResult(BestSolutionInfo* sts) {
    sts->best_generation = 0;
    sts->best_val = -MAX_VAL;
    sts->best_foundtime = 0.0;
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

int rcl_test(int argc, char** argv) {
    // loading graph
    Graph graph;
    graph.load(param_filename);
    nnode = graph.getNodeCount();

    finalBest.best_partition = new Partition(nnode);
    clearResult(&finalBest);

    Population population(param_pool_size);

    int generationCnt = 0;

    ImprovementStrategy* improvementStrategy = new SimulatedAnnealingImprovement(param_knownbest, param_minpercent, param_tempfactor, param_sizefactor);
    InitialPoolStrategy* RCLStrategy = new RCLInitStrategy();
    RCLStrategy->buildInitialPool(&finalBest, population, graph, improvementStrategy, param_time, &generationCnt);

    return 0;
}

int normal_run(int argc, char** argv) {
    fout = NULL;

    Graph graph;

    readParameters(argc, argv);

    srand(param_seed);

    fout = setupRecordFile();

    // use graph.load
    graph.load(param_filename);
    nnode = graph.getNodeCount();
    finalBest.best_partition = new Partition(nnode);

    for (int i = 0; i < argc; i++) {
        fprintf(fout, "%s ", argv[i]);
    }
    fprintf(fout, "\n");
    fprintf(fout, "idx\t best_v\t npat\t find_t\t find_i\t ttl_t\t  ttl_i\n");

    int run_cnt = 0;
    float sumtime = 0.0;
    int sumres = 0;
    int sumiter = 0;
    int bestInAll = -MAX_VAL;
    int* bestInAlllPartition = new int[nnode];

    while (run_cnt < 1) {
        clearResult(&finalBest);
        clock_t starttime = clock();

        ImprovementStrategy* improvementStrategy = new SaloExtendedImprovement(param_knownbest, param_minpercent, param_tempfactor, param_sizefactor);
        CrossoverStrategy* mergeDevideCrossover = new MergeDivideCrossover(param_shrink);
        InitialPoolStrategy* initialPoolstrategy = new RCLInitStrategy();

        MemeticRun memeticRun(mergeDevideCrossover, initialPoolstrategy, improvementStrategy, param_max_generations, param_time);
        memeticRun.run(&finalBest, graph, &totalgen, param_pool_size);

        totaltime = (double)(clock() - starttime) / CLOCKS_PER_SEC;
        fprintf(fout, "%-d\t %-d\t %-d\t %-.2f\t %-d\t %-.2f\t %-d\n", run_cnt + 1, finalBest.best_val, finalBest.best_partition->getBucketSize() - 1,
            finalBest.best_foundtime, finalBest.best_generation, totaltime, totalgen);

        if (finalBest.best_val > bestInAll) {
            bestInAll = finalBest.best_val;
        }
        sumtime += finalBest.best_foundtime;
        sumres += finalBest.best_val;
        sumiter += finalBest.best_generation;

        reportResult();

        run_cnt++;
    }
    fprintf(fout, "best result: %d\n", bestInAll);
    fprintf(fout, "average time: %.2f\n", sumtime / run_cnt);
    fprintf(fout, "average result: %.2f\n", (float)sumres / run_cnt);
    fprintf(fout, "average best iteration: %d\n", sumiter / run_cnt);
    fclose(fout);
    delete[] bestInAlllPartition;

    return 0;
}

int main(int argc, char** argv) {
    return normal_run(argc, argv);
}
