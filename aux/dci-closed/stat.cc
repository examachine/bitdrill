#ifndef __STAT_CC
#define __STAT_CC
extern long double TOT_ITEMSETS;

long double CLOSURE_BYTE_INCL		= 0;
long double DUPLICATES_BYTE_INCL	= 0;
long double AUX_DELTA                  = 0;
long double DUPLICATED                 = 0;
long double BYTE_INCL                  = 0;
long double NUM_FAILED			= 0;
long double HEAD_NO_INTERSECTION	= 0;
long double EXT_NO_INTERSECTION	= 0;
long double HEAD_NO_INCLUSION		= 0;
long double EXT_NO_INCLUSION		= 0;
long double BYTE_INTERSECTION		= 0;
long double PRE			= 0;
long double PROJECTIONS		= 0;
long double DUPLICATE_CHECKS           = 0;
long double MIN_COUNTS_SAVED           = 0;
long double COUNTS                     = 0;

long double DW = 0;
long double DH = 0;
long double SEW = 0;
long double SEH = 0;



#include "utils.hh"
Chronos FIRST_SCAN_TIME;
Chronos SECOND_SCAN_TIME;
Chronos THIRD_SCAN_TIME;
Chronos OUTPUT_TIME;
Chronos SUPPORT_TIME;
Chronos DUPLICATE_TIME;
Chronos CLOSURE_TIME;
Chronos PROJECTION_TIME;


#include <iostream>
#include <iomanip>

void printStat(void) {
  cout << scientific << setprecision(8)
       << "#" << endl
       << "# ## Statistics" << endl
       << "# ## . Timings" << endl 
       << "# ## . . 1. SCAN                   time : " << FIRST_SCAN_TIME.ReadChronos() << endl
       << "# ## . . 2. SCAN and DB Building   time : " << SECOND_SCAN_TIME.ReadChronos() << endl
       << "# ## . . 3. SCAN and DB Building   time : " << THIRD_SCAN_TIME.ReadChronos() << endl
       << "# ## . . OUTPUT     time : " << OUTPUT_TIME.ReadChronos() << endl
       << "# ## . . CLOSURE    time : " << CLOSURE_TIME.ReadChronos() << endl
       << "# ## . . SUPPORT    time : " << SUPPORT_TIME.ReadChronos() << endl
       << "# ## . . DUPLICATES time : " << DUPLICATE_TIME.ReadChronos() << endl
       << "# ## . . PROJECTION time : " << PROJECTION_TIME.ReadChronos() << endl
       << "# ## . Dataset" << endl
       << "# ## . . Section equal width  : " << SEW << " / " << DW << endl
       << "# ## . . Section equal height : " << SEH << " / " << DH << endl
       << "# ## . Operations" << endl
       << "# ## . . Word    INTERSECTIONS : " << BYTE_INTERSECTION << endl
       << "# ## . . HEAD no intersections : " << HEAD_NO_INTERSECTION << endl
       << "# ## . . EXT  no intersections : " << EXT_NO_INTERSECTION << endl
       << "# ## . . Word    INCLUSIONS : " << BYTE_INCL << endl
       << "# ## . . HEAD no inclusions : " << HEAD_NO_INCLUSION << endl
       << "# ## . . EXT  no inclusions : " << EXT_NO_INCLUSION << endl
       << "# ## . . " << endl
       << "# ## . . Word Closure Inclusions     : " << CLOSURE_BYTE_INCL << endl
       << "# ## . . Word Avg Closure Inclusions : " << CLOSURE_BYTE_INCL/TOT_ITEMSETS<< endl
       << "# ## . . Word Duplicates Inclusions     : " << DUPLICATES_BYTE_INCL << endl
       << "# ## . . Word Avg Duplicates Inclusions : " << DUPLICATES_BYTE_INCL/(TOT_ITEMSETS+DUPLICATED) << endl
       << "# ## . . bitwise counts        : " << COUNTS << endl
       << "# ## . . saved bitwise counts  : " << MIN_COUNTS_SAVED << endl
       << "# ## . . PRE        Items : " << PRE << endl
       << "# ## . . AVG PRE    Items : " << PRE/(TOT_ITEMSETS+NUM_FAILED) << endl
       << "# ## . . PROJECTIONS  : " << PROJECTIONS << endl
       << "# ## . . DUPLICATE_CHECKS  : " << DUPLICATE_CHECKS << endl
       << "# ## . Sets" << endl
       << "# ## . . CLOSED sets : " << TOT_ITEMSETS << endl
       << "# ## . . DUPLICATED sets : " << DUPLICATED << endl
       << "# ## . . INFREQUENT sets : " << NUM_FAILED-DUPLICATED << endl;
}

#endif


