// class encapsulating functionalities and data of a population; rewritten from https://github.com/hellozhilu/MDMCP/tree/main/code
#include "Population.h"
#include <algorithm>
#include <numeric>
#include <cstdio>
#include <cassert>

Population::Population(int poolSize)
    : obj_max(-MAX_VAL), obj_min(MAX_VAL), dis_min(MAX_VAL), obj_ave(0.0), poolSize(poolSize) { 

}

Population::~Population() {
    disposePopulation();
}

void Population::disposePopulation() {
    if (partitions.size() > 0) {
        for (Partition* partition : partitions) {
            delete partition;
        }
        partitions.clear();
    }
}

void Population::updatePopulation() {
    obj_max = -MAX_VAL;
    obj_min = MAX_VAL;
    dis_min = MAX_VAL;
    double ave_dis = 0.0;

    for (size_t i = 0; i < partitions.size(); ++i) {
        int dis_i2pool = MAX_VAL;
        for (size_t j = 0; j < partitions.size(); ++j) {
            if (i == j) continue;
            int dis_i2j = partitions[i]->calculateMaxMatch(partitions[i]->getPvertex(), partitions[i]->getBucketSize() - 1,
                partitions[j]->getPvertex(), partitions[j]->getBucketSize() - 1);
            if (dis_i2j < dis_i2pool)
                dis_i2pool = dis_i2j;
        }
        distances[i] = dis_i2pool;
        ave_dis += dis_i2pool;
        if (objValues[i] > obj_max)
            obj_max = objValues[i];
        if (objValues[i] < obj_min)
            obj_min = objValues[i];
        if (distances[i] < dis_min)
            dis_min = distances[i];
    }
    obj_ave = ave_dis / partitions.size();
}

int Population::insertPopulationWhenFull(const Partition& partition, int objval) {
    int replace_indv = -1;
    int minDst = partition.getNnode();
    int idxMinDst = -1;
    int worst_obj = MAX_VAL;
    int worst_obj_indv = -1;

    for (int i = 0; i < partitions.size(); ++i) {
        int dist2p = partition.calculateMaxMatch(partition.getPvertex(), partition.getBucketSize() - 1,
            partitions[i]->getPvertex(), partitions[i]->getBucketSize() - 1);
        if (dist2p < minDst) {
            minDst = dist2p;
            idxMinDst = i;
        }
        if (objValues[i] < worst_obj) {
            worst_obj = objValues[i];
            worst_obj_indv = i;
        }
    }

    if (minDst > 0 && objval > worst_obj) {
        replace_indv = worst_obj_indv;
        printf("replace %d because quality %d\n", replace_indv, worst_obj);
    }

    if (replace_indv != -1) {
        partitions[replace_indv]->copyPartition(partition);
        objValues[replace_indv] = objval;
        updatePopulation();
    }

    return replace_indv;
}

int Population::addPopulation(const Partition& partition, int objval) {
    int index = -1;
    if (partitions.size() < static_cast<size_t>(poolSize)) {
        bool add = true;
        for (unsigned int i = 0; i < partitions.size(); i++) {
            Partition* existingPartition = partitions[i];
            int dis = existingPartition->calculateMaxMatch(existingPartition->getPvertex(), existingPartition->getBucketSize() - 1,
                partition.getPvertex(), partition.getBucketSize() - 1);
            if (dis == 0) {
                printf("Solution %d is too close to the previous one %d, (dis %d)\n", objval, objValues[i], dis);
                add = false;
                break;
            }
        }
        if (add) {
            index = partitions.size();
            Partition* partitionCopy = new Partition(partition.getNnode());
            partitionCopy->copyPartition(partition);
            partitions.push_back(partitionCopy);
            objValues.push_back(objval);
            distances.push_back(MAX_VAL);
            if (partitions.size() == static_cast<size_t>(poolSize))
                updatePopulation();
        } // else delete partition
    }
    else {
        index = insertPopulationWhenFull(partition, objval);
        printf("Population: [");
        for (size_t i = 0; i < partitions.size(); ++i) {
            printf("%d-%d, ", objValues[i], distances[i]);
        }
        printf("]\n");
    }

    int obj_sum = std::accumulate(objValues.begin(), objValues.end(), 0);
    obj_ave = obj_sum / static_cast<double>(partitions.size());
    return index;
}

Partition& Population::getPartition(int id) {
    return *partitions[id];
}

double Population::getAverageObjective() const {
    return obj_ave;
}

int Population::getMaxObjective() const {
    return obj_max;
}

int Population::getMinObjective() const {
    return obj_min;
}

int Population::getMinDistance() const {
    return dis_min;
}

int Population::partitionCount() {
    return partitions.size();
}

int Population::getPoolSize() {
    return poolSize;
}

float Population::getAvgDistance() {
    int numPartitions = partitionCount();
    int numPairs = 0;
    int distance = 0;
    for (int i = 0; i < numPartitions - 1; i++) {
        for (int j = i + 1; j < numPartitions; j++) {
            Partition& partition1 = getPartition(i);
            Partition& partition2 = getPartition(j);
            distance += partition1.calculateMaxMatch(partition1.getPvertex(), partition1.getBucketSize() - 1,
                partition2.getPvertex(), partition2.getBucketSize() - 1);
            numPairs++;
        }
    }
    if (numPartitions > 1) {
        return distance / numPairs;
    }
    else {
        return distance;
    }
    
}
