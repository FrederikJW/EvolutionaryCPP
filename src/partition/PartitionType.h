#ifndef PARTITIONTYPE_H
#define PARTITIONTYPE_H

#include <vector>
#include <time.h>

using namespace std;

extern FILE* fout;

typedef struct {
	int* ppos;    /*ppos[pid] is the position of partition "pid" in "pbkt"*/
	int* pbkt;    /*Elements from pbkt[0] to pbkt[pbkt_size-1] include all the partition ids*/
	int pbkt_size;/*The number of partitions*/
	int* pcnt;    /*The number of vertices in each partition*/
	int* pvertex; /*The partition of each vertex, for example, v is in partition pvertex[v]*/
}PartitionType;

extern void printPartition(PartitionType* ppt, int gnnode, FILE* fout);
extern PartitionType* allocatePartitionData(int gnnode);
extern void buildPartition(PartitionType* ppt, int* vpart, int gnnode);
extern void copyPartition(PartitionType* dest, PartitionType* source, int gnnode);
extern void disposePartition(PartitionType* ppt);
extern void updatePartition(PartitionType* ppt, int v, int target);
extern int calculateDistance(int* p1, int* p2, int gnnode);
extern void generateRandList(int* randlist, int len);
extern void swapAry(int* ary, int idx1, int idx2);
extern int size_inter_section(vector<int>* v1, vector<int>* v2);
extern int calculateMaxMatch(int* p1, int n_part1, int* p2, int n_part2);

// Function declarations for managing PartitionType objects
void copyPartition(PartitionType* dest, const PartitionType* src, int size);

// Other function declarations possibly used in Population and strategies
int calculateMaxMatch(const int* pvertex1, int size1, const int* pvertex2, int size2);
void buildPartition(PartitionType* partition, const int* pchild, int size);

// Mock definitions for lsdata and related functions
struct LSData {
    PartitionType* ppt_best;
    int fbest;
};

extern LSData* lsdata;
extern double initTemp;
extern clock_t start_time;
extern double best_time;

void setLSStart(PartitionType* partition, int val);
void annealingSearch(double temp, clock_t start, double best_time);

#endif // PARTITIONTYPE_H
