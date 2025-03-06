#include "SaloeCoolImprovement.h"
#include "../Defines.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <vector>
#include <unordered_map>

void SaloeCoolImprovement::search(clock_t startTime, int maxSeconds, int generation_cnt) {
    problem->SALOeCoolSearch();
}

void SaloeCoolImprovement::calibrateTemp() {
    problem->CalibrateCool(10000);
}
