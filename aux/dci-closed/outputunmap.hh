#ifndef __OUTPUTUNMAP_HH
#define __OUTPUTUNMAP_HH

#include "my_defs.hh"
#include "stat.hh"

extern long double* ITEMSETS_BY_LENGTH;
extern unsigned int MAX_ITEMSET_LENGTH;

template <class T, class T1>
void output_unmap( char *mapped, char* map, char* output, T m1,  unsigned int num_of_parts, unsigned int no_of_threads );

#endif
