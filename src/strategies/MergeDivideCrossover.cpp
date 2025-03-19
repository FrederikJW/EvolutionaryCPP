// merge divide crossover strategy
#include "MergeDivideCrossover.h"
#include "../Defines.h"
#include <algorithm>
#include <set>
#include <cstdio>
#include <cassert>
#include <random>


int findSet(int* s, int i) {
    int r = s[i];
    while (s[r] != r) {
        r = s[r];
    }
    return r;
}

int unionSet(int* s, int i, int j) {
    int sameset = 0;
    int ri = findSet(s, i);
    int rj = findSet(s, j);
    if (ri < rj) {
        s[rj] = ri;
    }
    else if (ri > rj) {
        s[ri] = rj;
    }
    else {//ri=rj
        sameset = 1;
    }
    return sameset;
}

MergeDivideCrossover::MergeDivideCrossover(float shrink) : shrink(shrink) {}

void MergeDivideCrossover::crossover(Graph& graph, const Partition& parent1, const Partition& parent2, Partition& child, RandomGenerator* generator) {
    const int nnode = graph.getNodeCount();
    int scale = nnode * shrink + (*generator)() % 100;
    while (scale >= nnode)
        scale = nnode * shrink + (*generator)() % 100;

    int** mergeSol = new int* [nnode];
    int* root = new int[nnode];
    for (int i = 0; i < nnode; ++i) {
        mergeSol[i] = new int[nnode];
        std::fill(mergeSol[i], mergeSol[i] + nnode, -1);
        mergeSol[i][i] = 1;
        root[i] = i;
    }

    int vrest = nnode;
    int fixcnt = 0;
    while (vrest > scale) {
        int i = (*generator)() % nnode;
        int j = (*generator)() % nnode;
        while (i == j)
            j = (*generator)() % nnode;
        if (i > j)
            std::swap(i, j);
        if (mergeSol[i][j] == -1) {
            if (parent1.getPvertex()[i] == parent1.getPvertex()[j] && parent2.getPvertex()[i] == parent2.getPvertex()[j]) {
                mergeSol[i][j] = 1;
                if (unionSet(root, i, j) == 0)
                    vrest--;
            }
            else if (parent1.getPvertex()[i] != parent1.getPvertex()[j] && parent2.getPvertex()[i] != parent2.getPvertex()[j]) {
                mergeSol[i][j] = 0;
            }
            fixcnt++;
        }
    }
    printf("scale %d, fix rate %.4f\n", scale, (double)fixcnt * 2 / (nnode * (nnode - 1)));

    int* flaggrp = new int[nnode];
    int* vtxGrpId = new int[nnode];
    std::fill(flaggrp, flaggrp + nnode, -1);
    std::fill(vtxGrpId, vtxGrpId + nnode, -1);

    int gcnt = 0;
    for (int i = 0; i < nnode; ++i) {
        int ri = findSet(root, i);
        if (flaggrp[ri] == -1) {
            flaggrp[ri] = gcnt;
            vtxGrpId[i] = gcnt++;
        }
        else {
            vtxGrpId[i] = flaggrp[ri];
        }
    }

    int** connectMtx = new int* [gcnt];
    int** weightMtx = new int* [gcnt];
    for (int i = 0; i < gcnt; ++i) {
        connectMtx[i] = new int[gcnt];
        weightMtx[i] = new int[gcnt];
        std::fill(connectMtx[i], connectMtx[i] + gcnt, -1);
        std::fill(weightMtx[i], weightMtx[i] + gcnt, 0);
        connectMtx[i][i] = 1;
    }

    for (int i = 0; i < nnode - 1; ++i) {
        for (int j = i + 1; j < nnode; ++j) {
            if (mergeSol[i][j] == 1) {
                assert(vtxGrpId[i] == vtxGrpId[j]);
            }
            else if (mergeSol[i][j] == 0) {
                assert(vtxGrpId[i] != vtxGrpId[j]);
                connectMtx[vtxGrpId[i]][vtxGrpId[j]] = 0;
                connectMtx[vtxGrpId[j]][vtxGrpId[i]] = 0;
            }
            weightMtx[vtxGrpId[i]][vtxGrpId[j]] += graph.getMatrix()[i][j];
            weightMtx[vtxGrpId[j]][vtxGrpId[i]] += graph.getMatrix()[i][j];
        }
    }

    for (int i = 0; i < gcnt; ++i) {
        weightMtx[i][i] /= 2;
    }
    printf("coarse solution partition: %d\n", gcnt);

    int* coarseSol = new int[gcnt];
    int value = child.getValue();
    cliqueCover(weightMtx, connectMtx, gcnt, coarseSol, &value);
    child.setValue(value);

    int* pchild = new int[nnode];
    for (int i = 0; i < nnode; ++i) {
        pchild[i] = coarseSol[vtxGrpId[i]];
    }

    child.buildPartition(pchild);

    delete[] pchild;
    delete[] coarseSol;
    for (int i = 0; i < gcnt; ++i) {
        delete[] connectMtx[i];
        delete[] weightMtx[i];
    }
    delete[] connectMtx;
    delete[] weightMtx;
    delete[] vtxGrpId;
    delete[] flaggrp;
    delete[] root;
    for (int i = 0; i < nnode; ++i) {
        delete[] mergeSol[i];
    }
    delete[] mergeSol;
}

void MergeDivideCrossover::cliqueCover(int** weMat, int** conMat, int n, int* regrp, int* weight) {
    std::set<int> candset;
    int* adjweight = new int[n];
    int grpid = 1;
    *weight = 0;

    for (int i = 0; i < n; ++i) {
        regrp[i] = 0;
    }

    while (true) {
        int curvtx = -1;
        for (int i = 0; i < n; ++i) {
            if (!regrp[i]) {
                curvtx = i;
                break;
            }
        }
        if (curvtx == -1)
            break;

        regrp[curvtx] = grpid;
        candset.clear();
        std::fill(adjweight, adjweight + n, 0);
        for (int i = curvtx + 1; i < n; ++i) {
            if (!regrp[i] && conMat[curvtx][i] == -1) {
                candset.insert(i);
                adjweight[i] = weMat[curvtx][i];
            }
        }

        while (!candset.empty()) {
            int maxw = -MAX_VAL;
            int vmax = -1;
            for (auto itr = candset.begin(); itr != candset.end(); ++itr) {
                if (adjweight[*itr] > maxw) {
                    maxw = adjweight[*itr];
                    vmax = *itr;
                }
            }
            if (maxw < 0)
                break;

            candset.erase(vmax);
            regrp[vmax] = grpid;
            *weight += maxw;

            for (int i = curvtx + 1; i < n; ++i) {
                auto itr = candset.find(i);
                if (itr != candset.end()) {
                    if (conMat[vmax][i] == 0) {
                        candset.erase(itr);
                    }
                    else {
                        adjweight[i] += weMat[vmax][i];
                    }
                }
            }
        }
        grpid++;
    }

    for (int i = 0; i < n; ++i) {
        *weight += weMat[i][i];
    }
    delete[] adjweight;
}
