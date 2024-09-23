#include "Statistic.h"

void clearResult(BestSolutionInfo* sts) {
	sts->best_generation = 0;
	sts->best_val = -MAX_VAL;
	sts->best_foundtime = 0.0;
}