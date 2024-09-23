#ifndef STATISTIC
#define STATISTIC
#include "partition/Partition.h"
#include "Defines.h"

/***************************Statistic***************************/
typedef struct ST_Stats {
	Partition* best_partition;

	int best_val; 			//largest objective value
	double best_foundtime;  //The first time best_obj was found
	int best_generation;    //The iteration when best_obj was found

}BestSolutionInfo;

void clearResult(BestSolutionInfo* sts);

//extern void clearResult(BestSolutionInfo* sts);
//extern void printResult(BestSolutionInfo *sts, int index);

#endif // STATISTIC