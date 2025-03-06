#include "SaloCoolImprovement.h"
#include "../Defines.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>
#include <cstring>

// TODO: move these parameter to somewhere else
#define EMPTY_IDX 0

void SaloCoolImprovement::setEnvironment(Graph& graph) {
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

    uni_dist = std::uniform_real_distribution<double>(0.0, 1.0);
    node_dist = std::uniform_int_distribution<int>(0, lsdata->gnnode - 1);

    cooldown_period = 0.3 * gnnode;
    eligible_nodes.reserve(gnnode);
}


void SaloCoolImprovement::calibrateTemp() {
    double lt = 1.0, ut = 2000.0;
    double tempTolerate = 0.05;
    double _temp = (lt + ut) / 2;

    while (true) {
        int iter = 0;
        int accpCnt = 0;
        if (withPureDescent) {
            pureDescent();
        }
        int L = lsdata->gnnode * lsdata->ppt->getBucketSize() * sizefactor;

        eligible_nodes.clear();
        for (int i = 0; i < lsdata->gnnode; ++i) {
            eligible_nodes.push_back(i);
        }
        cooldown_buffer = std::vector<int>(cooldown_period, -1);

        int iteration = 0;
        while (true) {
            int buffer_index = iteration % cooldown_period;
            int cool_node = cooldown_buffer[buffer_index];
            if (cool_node != -1)
                eligible_nodes.push_back(cool_node);

            int rand_i = (*mGenerator)() % eligible_nodes.size();

            int i = eligible_nodes[rand_i];
            eligible_nodes[rand_i] = eligible_nodes.back();
            eligible_nodes.pop_back();

            cooldown_buffer[buffer_index] = i;

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
                double prob = fastExp((double)bestdelta / _temp);
                if ((*mGenerator)() % 1000 < prob * 1000) {
                    changeCurSolution(i, bestpid);
                    lsdata->fcurrent += bestdelta;
                    accpCnt++;
                }
            }
            iter++;
            if (iter > L)
                break;
            iteration++;
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


void SaloCoolImprovement::search(clock_t startTime, int maxSeconds, int generation_cnt) {
    double T = temp;
    int frozenCounter = 0;
    int accpCnt = 0;
    int saitr = 0;
    int test_counter = 0;

    eligible_nodes.clear();
    for (int i = 0; i < lsdata->gnnode; ++i) {
        eligible_nodes.push_back(i);
    }
    cooldown_buffer = std::vector<int>(cooldown_period, -1);

    const size_t gnnode = lsdata->gnnode;
    const size_t memSize = sizeof(int) * gnnode;

    int* vecTmpBest = new int[gnnode];
    if (withPureDescent) {
        pureDescent();
    }

    int L = gnnode * lsdata->ppt->getBucketSize() * sizefactor;

    std::memcpy(vecTmpBest, lsdata->ppt->getPvertex(), sizeof(int) * lsdata->gnnode);

    const int TIME_CHECK_INTERVAL = 10000;
    int time_check_counter = 0;

    int iteration = 0;

    while (true) {
        int buffer_index = iteration % cooldown_period;
        int cool_node = cooldown_buffer[buffer_index];
        if (cool_node != -1)
            eligible_nodes.push_back(cool_node);

        int rand_i = (*mGenerator)() % eligible_nodes.size();

        int i = eligible_nodes[rand_i];
        eligible_nodes[rand_i] = eligible_nodes.back();
        eligible_nodes.pop_back();

        cooldown_buffer[buffer_index] = i;

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
            // printf("current: %d | T: %.3f | %d>%d \n", lsdata->fcurrent, T, accpCnt*100, static_cast<int>(minpercent * L));
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

        // required if problem instance has a lot of neighbouring solutions with same score, which means a lot of relocations with delta 0 which lets SA run infinitly
        // to counter this problem SA will be forcefully terminated if T falls low enough
        if (T < 0.0005)
            break;

        iteration++;
    }

    printf("SA iterations: %d", test_counter);

    lsdata->ppt_best->buildPartition(vecTmpBest);
    lsdata->ppt_best->setValue(lsdata->fbest);
    delete[] vecTmpBest;
}