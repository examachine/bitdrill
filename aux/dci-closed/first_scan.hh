#ifndef __FIRST_SCAN_H
#define __FIRST_SCAN_H

#include "items.hh" 

unsigned int first_scan( char *dataset_name, 
			 dci_items& counters, 
			 unsigned int& max_trans_len, 
			 char* output_file);
#endif
