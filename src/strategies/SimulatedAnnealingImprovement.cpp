#include "SimulatedAnnealingImprovement.h"
#include "../Defines.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>

// TODO: move these parameter to somewhere else
#define EMPTY_IDX 0

SimulatedAnnealingImprovement::~SimulatedAnnealingImprovement() {
    disposeEnvironment();
}

void SimulatedAnnealingImprovement::improveSolution(Partition& solution, clock_t startTime, int maxSeconds, BestSolutionInfo *frt, int generation_cnt) {
    recorder->enter("improve_solution");
    setStart(solution);
    search(startTime, maxSeconds);
    selectBetter(frt, startTime, generation_cnt);
    recorder->exit("improve_solution");
    printf("Child has been raised to by SA %d\n", lsdata->fbest);
}

void SimulatedAnnealingImprovement::setEnvironment(Graph& graph) {
    if (lsdata == nullptr)
        lsdata = new SA_RT_Data;
    int gnnode = graph.getNodeCount();
    lsdata->gmatrix = graph.getMatrix();
    lsdata->gnnode = gnnode;

    lsdata->ppt = new Partition(gnnode);
    lsdata->ppt_best = new Partition(gnnode);

    lsdata->gammatbl = new int* [gnnode];
    for (int i = 0; i < gnnode; ++i) {
        lsdata->gammatbl[i] = new int[gnnode + 1];
    }
}

void SimulatedAnnealingImprovement::setStart(Partition& startSol) {
    int startObjValue = startSol.getValue();
    lsdata->ppt->copyPartition(startSol);
    lsdata->fcurrent = startObjValue;

    lsdata->ppt_best->copyPartition(startSol);
    lsdata->fbest = startObjValue;

    buildCurGamma(startSol.getPvertex());
}

void SimulatedAnnealingImprovement::disposeEnvironment() {
    delete lsdata->ppt;
    delete lsdata->ppt_best;

    for (int i = 0; i < lsdata->gnnode; ++i) {
        delete[] lsdata->gammatbl[i];
    }
    delete[] lsdata->gammatbl;

    delete lsdata;
    lsdata = nullptr;
}

void SimulatedAnnealingImprovement::buildCurGamma(const int* pvertex) {
    int lsnnode = lsdata->gnnode;
    int test_sum = 0;
    for (int i = 0; i < lsnnode; ++i) {
        std::memset(lsdata->gammatbl[i], 0, sizeof(int) * (lsnnode + 1));
        for (int j = 0; j < lsnnode; ++j) {
            lsdata->gammatbl[i][pvertex[j]] += lsdata->gmatrix[i][j];
            test_sum += lsdata->gmatrix[i][j];
        }
    }

    /*
    for (int i = 0; i < lsnnode; ++i) {
        for (int j = 0; j < lsnnode; ++j) {
            std::cout << lsdata->gammatbl[i][j] << " ";
        }
        std::cout << std::endl;
    }*/
}

void SimulatedAnnealingImprovement::updateCurGamma(int u, int src, int dest) {
    // Alias pointers to reduce dereferencing and respect constness
    int** gammatbl = lsdata->gammatbl;
    const int* const* gmatrix = lsdata->gmatrix;
    const int* gmatrix_row_u = gmatrix[u];

    // Cache gnnode to avoid repeated access
    int gnnode = lsdata->gnnode;

    for (int i = 0; i < gnnode; ++i) {
        int* gamma_row = gammatbl[i];
        int delta = gmatrix_row_u[i];

        gamma_row[src] -= delta;
        gamma_row[dest] += delta;
    }
}


// algorithm gets stuck on certain instances when trying to move nodes back and forth with gamma 0
void SimulatedAnnealingImprovement::calibrateTemp() {
    double lt = 1.0, ut = 2000.0;
    double tempTolerate = 0.05;
    double _temp = (lt + ut) / 2;

    while (true) {
        int iter = 0;
        int accpCnt = 0;
        pureDescent();
        int L = lsdata->gnnode * lsdata->ppt->getBucketSize() * sizefactor;
        while (true) {
            int i = (*mGenerator)() % lsdata->gnnode;
            int bestpid = -1;
            int bestdelta = -MAX_VAL;
            for (int k = 0; k < lsdata->ppt->getBucketSize(); ++k) {
                int pid = lsdata->ppt->getBucket()[k];
                if (pid == lsdata->ppt->getPvertex()[i] || (lsdata->ppt->getPcount()[lsdata->ppt->getPvertex()[i]] == 1 && pid == EMPTY_IDX))
                    continue;
                int delta = lsdata->gammatbl[i][pid] - lsdata->gammatbl[i][lsdata->ppt->getPvertex()[i]];
                if (delta > bestdelta) {
                    bestdelta = delta;
                    bestpid = pid;
                }
            }
            if (bestdelta > 0) {
                changeCurSolution(i, bestpid);
                lsdata->fcurrent += bestdelta;
                accpCnt++;
            }
            else {
                double prob = std::exp((double)bestdelta / _temp);
                if ((*mGenerator)() % 1000 < prob * 1000) {
                    changeCurSolution(i, bestpid);
                    lsdata->fcurrent += bestdelta;
                    accpCnt++;
                }
            }
            iter++;
            if (iter > L)
                break;
        }
        double accpProb = (double)accpCnt / L;
        if (accpProb > 0.5 + tempTolerate) {
            ut = _temp;
            _temp = (lt + ut) / 2;
        }
        else if (accpProb < 0.5 - tempTolerate) {
            lt = _temp;
            _temp = (lt + ut) / 2;
        }
        else {
            printf("Accept probability %.3f, Calibrate temp %.2f\n\n", accpProb, _temp);
            break;
        }
    }

    temp = _temp;
}

void generateRandList(int* randlist, int len, RandomGenerator* generator) {
    int idx = 0;
    assert(randlist != NULL);

    for (idx = 0; idx < len; idx++) {
        randlist[idx] = idx;
    }
    for (idx = 0; idx < len; idx++) {//swap
        int randid = (*generator)() % len;
        int tmp = randlist[idx];
        randlist[idx] = randlist[randid];
        randlist[randid] = tmp;
    }
}

void SimulatedAnnealingImprovement::pureDescent() {
    int* randlst = new int[lsdata->gnnode];
    bool improved = true;

    while (improved) {
        improved = false;
        generateRandList(randlst, lsdata->gnnode, mGenerator);

        for (int i = 0; i < lsdata->gnnode; ++i) {
            int currentnode = randlst[i];
            for (int k = 0; k < lsdata->ppt->getBucketSize(); ++k) {
                int curpart = lsdata->ppt->getBucket()[k];
                int gain = lsdata->gammatbl[currentnode][curpart] - lsdata->gammatbl[currentnode][lsdata->ppt->getPvertex()[currentnode]];
                if (gain > 0) {
                    improved = true;
                    changeCurSolution(currentnode, curpart);
                    lsdata->fcurrent += gain;
                }
            }
        }
    }
    if (lsdata->fcurrent > lsdata->fbest) {
        lsdata->ppt_best->copyPartition(*lsdata->ppt);
        lsdata->fbest = lsdata->fcurrent;
    }
    delete[] randlst;
}

double SimulatedAnnealingImprovement::fastExp(double x)
{
    union {
        long long int i;
        double d;
    } tmp;

    tmp.i = static_cast<long long int>(1512775 * x + 1072632447);
    tmp.i <<= 32;
    return tmp.d;
}

void SimulatedAnnealingImprovement::search(clock_t startTime, int maxSeconds) {
    double T = temp;
    int frozenCounter = 0;
    int accpCnt = 0;
    int saitr = 0;
    int test_counter = 0;

    const size_t gnnode = lsdata->gnnode;
    const size_t memSize = sizeof(int) * gnnode;

    int* vecTmpBest = new int[gnnode];
    pureDescent();

    int L = gnnode * lsdata->ppt->getBucketSize() * sizefactor;

    std::memcpy(vecTmpBest, lsdata->ppt->getPvertex(), sizeof(int) * lsdata->gnnode);

    const int TIME_CHECK_INTERVAL = 10000;
    int time_check_counter = 0;

    std::uniform_real_distribution<double> uni_dist(0.0, 1.0);
    std::uniform_int_distribution<int> node_dist(0, lsdata->gnnode - 1);

    while (true) {
        int i = node_dist(*mGenerator);

        int* pvertex = lsdata->ppt->getPvertex();
        int* pcount = lsdata->ppt->getPcount();
        size_t bucketSize = lsdata->ppt->getBucketSize();
        int* const bucket = lsdata->ppt->getBucket();

        int current_partition = pvertex[i];
        int current_partition_count = pcount[current_partition];

        int bestpid = -1;
        int bestdelta = -MAX_VAL;

        int* const gamma_i = lsdata->gammatbl[i];
        const int gamma_i_current = gamma_i[current_partition];
        test_counter++;

        for (int k = 0; k < bucketSize; ++k) {
            int pid = bucket[k];
            if (pid == current_partition || (current_partition_count == 1 && pid == EMPTY_IDX))
                continue;
            int delta = gamma_i[pid] - gamma_i_current;
            if (delta > bestdelta) {
                bestdelta = delta;
                bestpid = pid;
            }
        }

        if (bestdelta >= 0) {
            changeCurSolution(i, bestpid);
            lsdata->fcurrent += bestdelta;
            accpCnt++;
        }
        else {
            double prob = fastExp((double)bestdelta / T);
            if (uni_dist(*mGenerator) < prob) {
                changeCurSolution(i, bestpid);
                lsdata->fcurrent += bestdelta;
                accpCnt++;
            }
        }

        pvertex = lsdata->ppt->getPvertex();

        if (lsdata->fcurrent > lsdata->fbest) {
            std::memcpy(vecTmpBest, pvertex, memSize);
            lsdata->fbest = lsdata->fcurrent;
        }

        saitr++;
        if (saitr > L) {
            saitr = 0;
            T *= tempfactor;
            if (accpCnt * 100 < static_cast<int>(minpercent * L)) {
                frozenCounter++;
            }
            if (frozenCounter > 5) {
                break;
            }
            accpCnt = 0;
        }

        time_check_counter++;
        if (time_check_counter >= TIME_CHECK_INTERVAL) {
            time_check_counter = 0;
            if ((double)(clock() - startTime) / CLOCKS_PER_SEC >= maxSeconds) {
                break;
            }
        }
    }

    printf("SA iterations: %d", test_counter);

    // TODO: fix this: how to get value?
    lsdata->ppt_best->buildPartition(vecTmpBest);
    lsdata->ppt_best->setValue(lsdata->fbest);
    delete[] vecTmpBest;
}

void SimulatedAnnealingImprovement::selectBetter(BestSolutionInfo* frt, clock_t start_time, int generation_cnt) {
    if (lsdata->fbest > frt->best_val) {
        frt->best_partition->copyPartition(*(lsdata->ppt_best));
        frt->best_val = lsdata->fbest;
        frt->best_foundtime = (double)(clock() - start_time) / CLOCKS_PER_SEC;
        frt->best_generation = generation_cnt;
        if (frt->best_val >= knownbest) {
            return;
        }
    }
}

int SimulatedAnnealingImprovement::getBestObjective() {
    return lsdata->fbest;
}

Partition SimulatedAnnealingImprovement::getBestPartition() {
    return *lsdata->ppt_best;
}

int SimulatedAnnealingImprovement::decideTarget(int dest) {
    if (dest == EMPTY_IDX) {
        dest = lsdata->ppt->getBucket()[lsdata->ppt->getBucketSize()];
        lsdata->ppt->setBucketSize(lsdata->ppt->getBucketSize() + 1);
    }
    return dest;
}

int SimulatedAnnealingImprovement::changeCurSolution(int u, int dest) {
    int src = lsdata->ppt->getPvertex()[u];
    int target = decideTarget(dest);
    updateCurGamma(u, src, target);
    lsdata->ppt->updatePartition(u, target);
    return target;
}
