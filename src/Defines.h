#ifndef DEFINES_H_
#define DEFINES_H_
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <algorithm>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define getcwd _getcwd
#define PATH_MAX 260
#else
#include <unistd.h>
#endif
using namespace std;

#define DEBUG_SWT 0
#define MIN(a,b) ((a)<(b)?(a):(b))
#define PARENT(idx) ((idx-1)/2)
#define LEFT(idx) (2*idx+1)
#define RIGHT(idx) (2*idx+2)
#define EMPTY_IDX 0

extern const int MAX_CHAR;
const int MAX_VAL = 999999999;
extern const float CONST_E;

/****************Declaration of all the parameters**************/
//configurations
extern char param_filename[1000];
extern int param_knownbest;
extern int param_time;		     //the max time for memetic procedure, unit: seconds
extern int param_seed;
extern int param_max_generations;//not used
//5 parameters
extern int param_sizefactor;     //theta_size of SA
extern double param_tempfactor;  //theta_cool of SA
extern double param_minpercent;  //theta_minper of SA
extern double param_shrink;      //shrink percent in crossover
extern int param_pool_size;      //size of population

#endif /* DEFINES_H_ */
