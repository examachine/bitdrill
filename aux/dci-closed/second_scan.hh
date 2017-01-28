#ifndef __SECOND_SCAN_HH
#define __SECOND_SCAN_HH

#include "items.hh" 

template <class T, class T1> 
unsigned int second_scan( unsigned int &max_trans_len, 
		  unsigned int m1,
		  int* map,
		  T1* singletons_support,
		  T1 min_count,
		  char* output_file,
		  float limit);


#endif
