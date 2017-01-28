// Copyright (C) 2003 salvatore orlando <salvatore.orlando@unive.it> 
// Copyright (C) 2005 claudio lucchese  <clucches@dsi.unive.it> 
// University of Venice, Ca' Foscari, Dipartimento di Informatica, (VE) Italy 
// Istituto di Scienza e Tecnologia dell'Informazione, ISTI - CNR, (PI) Italy 
//   
// This program is free software; you can redistribute it and/or modify 
// it under the terms of the GNU General Public License as published by 
// the Free Software Foundation; either version 2 of the License, or 
// (at your option) any later version. 
//  
// This program is distributed in the hope that it will be useful, 
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
// GNU General Public License for more details. 
//  
// You should have received a copy of the GNU General Public License 
// along with this program; if not, write to the Free Software  
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. 

#include <vector> 
#include <algorithm> 
#include <functional> 
#include <cmath> 
 
using namespace std;


#include "utils.hh" 

#include "my_defs.hh"

#include "stat.hh"
#include "print.hh"

#include "first_scan.hh"
#include "second_scan.hh"
#include "third_scan.hh"
#include "fourth_scan.hh"
#include "outputunmap.hh"

#include "items.hh" 
#include "transaction.hh" 
#include "database.hh" 
#include "memory.hh" 
 
 
 
template <class T, class T1> 
void following_scans(unsigned int max_trans_len, dci_items& counters, char* output_file, float partition_size, unsigned int no_of_threads); 

long double* ITEMSETS_BY_LENGTH;
unsigned int MAX_ITEMSET_LENGTH;
long double TOT_ITEMSETS = 0; 

extern Chronos GLOBAL_CLOCK;
 
int main(int argc, char ** argv) 
{ 
  synch_init();
  GLOBAL_CLOCK.StartChronos();
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setting Parameters
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if(argc<4 || argc>7) {
    synch_printf( "%s\n\n", "Usage: fim_closed <input_file> <min_count> <output> [#threads = 1] [max_memory_in_MB = 1GB] [selective sampling = 0]");
    exit(1);
  }

  // start chronos
  Chronos all_time;
  all_time.StartChronos();
 
  char*          dataset_name  = argv[1];
  unsigned int      min_count  = atoi(argv[2]);
  char*           output_file  = argv[3]; 
  float        partition_size  = 1024*1024*1024; // default is 1GB
  unsigned int  no_of_threads  = 1;              // default is 1 threads
  unsigned int sel_sampling_th = 0;              // default is no sampling

  if (argc>=5) {
    no_of_threads = atoi(argv[4]);
    if (no_of_threads == 0) fatal_error("At least one thread is needed.");
  } 
  if (argc>=6) {
    partition_size = atof(argv[5])*1024*1024;  // to bytes
    if (partition_size == 0.0) 
      fatal_error("Partition size must be greater than 0.");
  }
  if (argc>=7) {
    sel_sampling_th = atoi(argv[6]);
    if (sel_sampling_th <0 || sel_sampling_th>100) 
      fatal_error("Selective sampling threshold must be between 0 and 100");
  }
  if (min_count == 0) fatal_error("Minimum count must be greater than 0");



  // **************************************************
  // info
  // **************************************************
#ifdef VERBOSE
  synch_printf("%s", 
	       "# ################################################################################ #\n"
	       "#                                                                                  #\n"
	       "# ## DCI_CLOSED, and algorithm for mining frequent closed itemsets.             ## #\n"
	       "# ##                                                                            ## #\n"
	       "# ## Copyright (C) 2005 Claudio Lucchese clucches@dsi.unive.it                  ## #\n"
	       "# ## University of Venice, Ca' Foscari, Dipartimento di Informatica, (VE) Italy ## #\n"
	       "# ## Copyright (C) 2003 Salvatore Orlando orlando@dsi.unive.it                  ## #\n"
	       "# ## University of Venice, Ca' Foscari, Dipartimento di Informatica, (VE) Italy ## #\n"
	       "# ## Copyright (C) 2003 Raffaele Perego perego@isti.cnr.it                      ## #\n"
	       "# ## Istituto di Scienza e Tecnologia dell'Informazione, ISTI - CNR, (PI) Italy ## #\n"
	       "# ##                                                                            ## #\n"
	       "# ## DCI_CLOSED comes with ABSOLUTELY NO WARRANTY; this is free software,       ## #\n"
	       "# ## and you are welcome to redistribute it under certain conditions.           ## #\n"
	       "# ## Please see http://www.gnu.org/copyleft/gpl.html for details                ## #\n"
	       "#\n"
	       "# ################################################################################ #\n"
	       "#\n"
	       "#\n"
	       "# ## Verbose mode : ON\n"
#ifdef MY_STATS
	       "# ## Stat report : ON\n" 
#else
	       "# ## Stat report : OFF\n"
#endif
#ifdef USE_SSE
#ifdef USE_MMX
	       "# ## MMX (64-bit) optimizations : ON\n" 
#else
	       "# ## SSE2 (128-bit) optimizations : ON\n"
#endif
#else
	       "# ## SSE2 optimizations : OFF\n"
#endif
#ifdef AVOID_TRANS_CHECK
	       "# ## Check duplicate singletons in transactions : OFF\n"
#else
	       "# ## Check duplicate singletons in transactions : ON\n"
#endif
	       "# ## Input Dataset size is expressed as:\n"
	       "# ## DataSize :  #items * #transactions (expected #transactions)\n"
	       "#\n"
	       "#\n"
	       "# ## Mining for frequent closed itemsets.\n");

  synch_printf("# ##  -   input dataset : %s\n", dataset_name);
  synch_printf("# ##  - minimum support : %d\n", min_count);
  synch_printf("# ##  -     output file : %s\n", output_file);
  synch_printf("# ##  -    memory limit : %.2f MB\n", partition_size/1024/1024);
  synch_printf("# ##  -  no. of threads : %d\n", no_of_threads);
  synch_printf("# ##  -   sel. sampling : %d%\n", sel_sampling_th);
  switch (PARTITIONING_POLICY) {
  case STATIC_STATIC:
    synch_printf("# ##  - parallelization : SSN\n", no_of_threads);
    break;
  case STATIC_DINAMIC:
    synch_printf("# ##  - parallelization : SDN\n", no_of_threads);
    break;
  case DINAMIC_DINAMIC:
    synch_printf("# ##  - parallelization : DDN\n", no_of_threads);
    break;
  case STATIC_STATIC_P:
    synch_printf("# ##  - parallelization : SSP\n", no_of_threads);
    break;
  case STATIC_DINAMIC_P:
    synch_printf("# ##  - parallelization : SDP\n", no_of_threads);
    break;
  case DINAMIC_DINAMIC_P:
    synch_printf("# ##  - parallelization : DDP\n", no_of_threads);
    break;
  }

#endif


  // ************************************************** 
  // First scan 
  // ************************************************** 
  unlink (output_file);

  // counters for singleton 
  dci_items counters(min_count, sel_sampling_th);  
  
  unsigned int max_trans_len = 0; 
  unsigned int n_tr = 
    first_scan(dataset_name, counters, max_trans_len, output_file); 
  unsigned int m1 = counters.m1;

  // Initialize Itemsets stats
  ITEMSETS_BY_LENGTH = new long double [m1+1];
  if (!ITEMSETS_BY_LENGTH) 
    fatal_error("Could not allocate memory of itemsets counts.");

  for (unsigned int i=0; i<m1+1; i++)
    ITEMSETS_BY_LENGTH[i]=0;
  MAX_ITEMSET_LENGTH = (m1>0) ? 1 : 0;

  // Output the empty set if frequent and closed
  if ( n_tr >= counters.min_count &&
       n_tr != counters.max_supp) {
    ITEMSETS_BY_LENGTH[0] = 1;
    FILE* out = fopen(output_file,"ab");
    if (!out) fatal_error("Could not open output file.");

    fprintf(out, "(%d)\n", n_tr);
    fclose (out);
  } 

  if ( m1 > 1) { 
    // ************************************************** 
    // Second and Following iterations  
    // since we now know how many distinct items are frequent 
    // and their maximum support, we can optimize the amount 
    // of memory used to store itemsets and their counters 
    // ************************************************** 
    if (m1 < 256   &&    n_tr < 256)       
      following_scans<unsigned char, unsigned char>(max_trans_len, counters, output_file, partition_size, no_of_threads); 
    else if (m1 < 256  && n_tr < 256*256) 
      following_scans<unsigned char, unsigned short int>(max_trans_len, counters, output_file, partition_size, no_of_threads);   
    else if (m1 < 256  && n_tr >= 256*256) 
      following_scans<unsigned char, unsigned int>(max_trans_len, counters, output_file, partition_size, no_of_threads);   
    else if (m1 < 256*256  && n_tr < 256) 
      following_scans<unsigned short int, unsigned char>(max_trans_len, counters, output_file, partition_size, no_of_threads);   
    else if (m1 >= 256*256  && n_tr < 256) 
      following_scans<unsigned int, unsigned char>(max_trans_len, counters, output_file, partition_size, no_of_threads);   
    else if (m1 < 256*256  && n_tr < 256*256) 
      following_scans<unsigned short int, unsigned short int>(max_trans_len, counters, output_file, partition_size, no_of_threads); 
    else if (m1 < 256*256  && n_tr >= 256*256) 
      following_scans<unsigned short int, unsigned int>(max_trans_len, counters, output_file, partition_size, no_of_threads); 
    else if (m1 >= 256*256 && n_tr < 256*256) 
      following_scans<unsigned int, unsigned short int>(max_trans_len, counters, output_file, partition_size, no_of_threads); 
    else 
      following_scans<unsigned int, unsigned int>(max_trans_len, counters, output_file, partition_size, no_of_threads); 
  }


  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Output Statistics on Itemsets length
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  for (unsigned int i=0; i<=MAX_ITEMSET_LENGTH; i++) {
    synch_printf("%d\n", (int) ITEMSETS_BY_LENGTH[i]);
    TOT_ITEMSETS += ITEMSETS_BY_LENGTH[i];
  }
  delete [] ITEMSETS_BY_LENGTH; 
  
  synch_printf( "#\n"
		"# ## Number of  closed itemsets: %d \n"
		"# ## Elapsed Time: %.2f secs. \n"
		"#\n", 
		(int) TOT_ITEMSETS, all_time.ReadChronos() );

 
  unlink(TEMPORARY_DATASET); 
  unlink(PARTITIONS_FILE);
  unlink(UNMAP_FILE);
  unlink(MAPPED_OUTPUT);
  //  Check_memory::consistency_check();
 
 
  // OUTPUT STATISTICS 
#ifdef MY_STATS 
  printStat();
#endif 

  return 0; 
} 
 
 

 
 
/** second and following iterations:  
 * First a pruned dataset is stored in the vertical format 
 * and after the depth first iteration starts. 
 */ 
template <class T, class T1> 
void following_scans(unsigned int max_trans_len, dci_items& counters, char* output_file, float partition_size, unsigned int no_of_threads) 
{ 
  // --------------------------------------------------
  // Create Mapping, Unmapping, and supports vector
  // --------------------------------------------------
  // reset mapping
  int* map = new int[counters.m];
  if (!map) fatal_error("Could not allocate memory for singletons mapping");

  for (unsigned int i=0; i < counters.m; i++) 
    map[i] = -1;

  // open output file for mapping
  FILE* map_file = fopen(UNMAP_FILE,"wb");
  if (!map_file) fatal_error("Coulnd not open singletons unmapping file for writing.");


  // create a new smaller count vector
  T1* singletons_support = new T1[counters.m1];
  if (!singletons_support) fatal_error("Could not allocate memory of singletons supports");
  
  int start = counters.m - counters.m1;
  for (unsigned int i=0; i < counters.m1; i++) {
    // output unmapping
    fwrite  (&((*(counters.acc))[i+start].id),sizeof(unsigned int),1,map_file);
    // store map & supp
    map[(*(counters.acc))[i+start].id] = i;
    singletons_support[i] = (*(counters.acc))[i+start].count;
  }

  fclose(map_file);

  // remove old stats
  counters.delete_counts();

 
  // ------------------------------------ 
  // Second Scan : Calculate Partitions
  // ------------------------------------ 
 
  unlink(PARTITIONS_FILE);

  unsigned int n_part = 
    second_scan<T,T1>(max_trans_len, counters.m1, map, singletons_support, counters.min_count, output_file, partition_size); 

  delete [] map;
  delete [] singletons_support;

  // ------------------------------------ 
  // Third Scan : Mine
  // ------------------------------------ 

  third_scan<T,T1>(n_part, counters.m1, counters.min_count, max_trans_len, no_of_threads);
  
  // ------------------------------------ 
  // Merge-Sort : Remove Duplicates
  // ------------------------------------ 
  //  if (n_part > 1)
  fourth_scan<T, T1> (partition_size, n_part, no_of_threads, MAPPED_OUTPUT);


  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Output and unmap itemsets
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  output_unmap<T, T1>(MAPPED_OUTPUT, UNMAP_FILE, output_file, (T) counters.m1, n_part, no_of_threads);


} 
 



