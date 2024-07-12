#ifndef PARTITION_H
#define PARTITION_H

#include <vector>
#include <cstdio>
#include <cassert>
#include <algorithm>
#include <cstring>

class Partition {
public:
    Partition(int n);
    ~Partition();

    void buildPartition(int* vpart);
    void copyPartition(const Partition& source);
    void updatePartition(int v, int target);
    void print(FILE* fout) const;
    int calculateDistance(const int* p1, const int* p2) const;
    void setValue(int new_value);
    int getValue() const;
    int* getPvertex() const;
    int* getPcount();
    void setBucketSize(int bucket_size);
    int getBucketSize();
    int* getBucket();
    int sizeIntersection(std::vector<int>* v1, std::vector<int>* v2);
    int calculateMaxMatch(int* p1, int n_part1, int* p2, int n_part2);
    int getNnode();

    // just for testing
    void checkIntegrity();

private:
    void allocate(int n);
    void deallocate();

    int nnode;
    int value;
    int* ppos; // position of partition in the partition bucket
    int* pbkt; // array of all partition identifier
    int pbkt_size; // marks the end of pbkt and is the number of partitions
    int* pcnt; // the size of a partition
    int* pvertex; // in which partition a vertex is
};

#endif // PARTITION_H
