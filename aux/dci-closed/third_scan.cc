#include "third_scan.hh"
#include "my_defs.hh"
#include "stat.hh"

#include <pthread.h>

#include "database.hh"
#include "vertical.hh" 
#include "print.hh"
#include "utils.hh"
#include "tid_list_macros.hh"

template <class T,class T1> 
void loadDB( int max_trans_len,
	     T1* conditional_supports,
	     T* conditional_map,
	     unsigned int p_trans,
	     unsigned int p_items,
	     T ub,
	     DCI_VD<T,T1>& DCI_dataset ); 


// instances
template void third_scan<unsigned char, unsigned char>(unsigned int n_part, unsigned int m1, unsigned int min_count, unsigned int max_trans_len, unsigned int no_of_threads );
template void third_scan<unsigned char, unsigned short int>(unsigned int n_part, unsigned int m1, unsigned int min_count, unsigned int max_trans_len, unsigned int no_of_threads );
template void third_scan<unsigned char, unsigned int>(unsigned int n_part, unsigned int m1, unsigned int min_count, unsigned int max_trans_len, unsigned int no_of_threads );

template void third_scan<unsigned short int, unsigned char>(unsigned int n_part, unsigned int m1, unsigned int min_count, unsigned int max_trans_len, unsigned int no_of_threads );
template void third_scan<unsigned short int, unsigned short int>(unsigned int n_part, unsigned int m1, unsigned int min_count, unsigned int max_trans_len, unsigned int no_of_threads );
template void third_scan<unsigned short int, unsigned int>(unsigned int n_part, unsigned int m1, unsigned int min_count, unsigned int max_trans_len, unsigned int no_of_threads );

template void third_scan<unsigned int, unsigned char>(unsigned int n_part, unsigned int m1, unsigned int min_count, unsigned int max_trans_len, unsigned int no_of_threads );
template void third_scan<unsigned int, unsigned short int>(unsigned int n_part, unsigned int m1, unsigned int min_count, unsigned int max_trans_len, unsigned int no_of_threads );
template void third_scan<unsigned int, unsigned int>(unsigned int n_part, unsigned int m1, unsigned int min_count, unsigned int max_trans_len, unsigned int no_of_threads );

template <class T,class T1> 
void third_scan(unsigned int n_part, unsigned int m1, unsigned int min_count, unsigned int max_trans_len, unsigned int no_of_threads){
#ifdef VERBOSE
  Chronos elapsed_time;
  elapsed_time.StartChronos();

  synch_printf( "#\n"
		"# ## STEP 3 . Extract Local Frequent Closed Itemsets.\n");
#endif

#ifdef USE_SSE
  sse_init_masks();
#endif

  // open file for reading partitioning info
  FILE* part_file = fopen(PARTITIONS_FILE,"rb");
  if (!part_file) fatal_error("Could not open partitioning info file for reading.");

  // open per-thread output files
  FILE** output_files = new FILE* [no_of_threads];
  for (unsigned int i=0; i<no_of_threads; i++) {
    char outfilename[30];
    sprintf(&outfilename[0], BIN_ITEMSETS_FILE_FMT, i);
    unlink(outfilename);
    output_files[i] = fopen(outfilename, "ab");
    if (!output_files[i]) fatal_error("Could not open binary itemsets file for writing.");
  }

  // Partition data
  unsigned int lb = 0;
  unsigned int ub = 0;
  unsigned int p_items = 0;
  unsigned int p_trans = 0;
  T* conditional_map = new T[m1];
  if (!conditional_map) fatal_error("Could not allocate memory for conditinal mapping.");
  unsigned int p_id = 0;

  while (fread(&lb, 1, sizeof(unsigned int), part_file) != 0) {
#ifdef VERBOSE
    synch_printf("# ## ## Mining Partition %d. ", p_id+1);
#endif

    // read partition info
    fread(&ub,  1, sizeof(unsigned int), part_file);	
    fread(&p_items,  1, sizeof(unsigned int), part_file);	
    fread(&p_trans,  1, sizeof(unsigned int), part_file);	

    // reset mapping and unmapping    
    T conditional_ub = 0;
    for (unsigned int i=0; i<m1; i++)
      conditional_map[i] = (T)-1;
    T* conditional_unmap = new T [p_items];
    if (!conditional_unmap) fatal_error("Could not allocate memory for conditinal unmapping.");

    for (unsigned int i=0; i<p_items; i++) {
      unsigned int aux;
      fread(&aux, 1, sizeof(unsigned int), part_file);	
      conditional_map[aux] = i;
      conditional_unmap[i] = aux;
      if (conditional_ub == 0 && aux>=ub)
	conditional_ub = i;
    }
    if (conditional_ub == 0)
      conditional_ub = p_items;
    
    // Allocate a new vertical dataset
    T1*  conditional_supports = new T1 [p_items];
    if (!conditional_supports) fatal_error("Could not allocate memory for conditional singletons supports.");
    //bzero( conditional_supports, p_items*sizeof(T1)); 
    memset( conditional_supports, 0x0, p_items*sizeof(T1)); 

    DCI_VD<T,T1> DCI_dataset( min_count, 
			      conditional_supports,
			      p_items, 
			      p_trans,
			      conditional_ub,
			      conditional_unmap );
    
    // load partition
    loadDB<T,T1>( max_trans_len, conditional_supports, conditional_map, 
		  p_trans, p_items, conditional_ub, DCI_dataset);

    // ------------------------------------------------
    // MINE !!!!!!!!!!!!
    // ------------------------------------------------
    
    // reorder dataset
    DCI_dataset.find_section_equal();

    // DCI_dataset.DClose_first_iter(false); 
    Master<T,T1> master(&DCI_dataset, no_of_threads, output_files);
    master.start();
    // ------------------------------------------------

    delete [] conditional_supports;
    delete [] conditional_unmap;
    p_id++;
  }

  
  delete [] conditional_map;
  fclose(part_file);

  for (unsigned int i=0; i<no_of_threads; i++)
    fclose(output_files[i]);
  delete [] output_files;

#ifdef VERBOSE
  synch_printf("# ## Elapsed Time: %.2f secs.\n", elapsed_time.ReadChronos() );
#endif
}


/** Constructes the vertical dataset with a third scan
 *
 */ 
template <class T,class T1> 
void loadDB( int max_trans_len,
	     T1* conditional_supports,
	     T* conditional_map,
	     unsigned int p_trans,
	     unsigned int p_items,
	     T conditional_ub,
	     DCI_VD<T,T1>& DCI_dataset){

#ifdef MY_STATS
  THIRD_SCAN_TIME.StartChronos();  
#endif 

  binFSin<T> ii(TEMPORARY_DATASET); 
  if(!ii.isOpen()) fatal_error("Could not open binary dataset for reading.");
 
  Transaction<T> t_in  (max_trans_len); 
  Transaction<T> t_out (max_trans_len); 
 
  // database scan 
#ifdef VERBOSE
  int totalsize=0; 
#endif
  unsigned int n_tr = 0; 
  while(ii.getNextTransaction(t_in)) { 
    // Skip useless infrequent items and remap them
    t_out.length = 0;
    for (T i=0; i<t_in.length; i++) 
      if (conditional_map[t_in.t[i]] != (T)-1) {
	if ( t_out.length==0 && conditional_map[t_in.t[i]]>=conditional_ub )
	  break;
	t_out.t[t_out.length++] = (T) conditional_map[t_in.t[i]];
	conditional_supports[conditional_map[t_in.t[i]]]++;
      }


    if ( t_out.length > 0 ) { 
      // write the trans in VD on the fly 
      DCI_dataset.write_t_in_VD(n_tr, t_out); 
      n_tr++; 
#ifdef VERBOSE
      totalsize += t_out.length;
#endif
    } 
  } 
  ii.close();

#ifdef MY_STATS
  THIRD_SCAN_TIME.StopChronos();
#endif 

#ifdef VERBOSE
  synch_printf( "DataSize : %d * %d (%d) = %d KB, dens: %.3e. \n",
		p_items, n_tr, p_trans,
		p_trans*p_items/8/1024,
		(float)totalsize / (float)(p_trans*p_items) );
#endif
}
 

/** Constructes the vertical dataset with a third scan
 *
 */ 
