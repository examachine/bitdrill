#ifdef MY_STATS 

#ifndef __STAT_H
#define __STAT_H

#include "utils.hh"

void printStat(void);

extern long double CLOSURE_BYTE_INCL;
extern long double DUPLICATES_BYTE_INCL;
extern long double AUX_DELTA;
extern long double DUPLICATED;
extern long double BYTE_INCL;
extern long double NUM_FAILED;
extern long double HEAD_NO_INTERSECTION;
extern long double EXT_NO_INTERSECTION;
extern long double HEAD_NO_INCLUSION;
extern long double EXT_NO_INCLUSION;
extern long double BYTE_INTERSECTION;
extern long double PRE;
extern long double PROJECTIONS;
extern long double DUPLICATE_CHECKS;
extern long double MIN_COUNTS_SAVED;
extern long double COUNTS;

extern long double DW;
extern long double DH;
extern long double SEW;
extern long double SEH;

extern Chronos FIRST_SCAN_TIME;
extern Chronos SECOND_SCAN_TIME;
extern Chronos THIRD_SCAN_TIME;
extern Chronos OUTPUT_TIME;
extern Chronos SUPPORT_TIME;
extern Chronos DUPLICATE_TIME;
extern Chronos CLOSURE_TIME;
extern Chronos PROJECTION_TIME;

#endif

#endif
