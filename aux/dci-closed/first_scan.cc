#include "first_scan.hh"

#include <algorithm>
using namespace std;

#include "my_defs.hh"
#include "stat.hh"
#include "database.hh"
#include "utils.hh"
#include "print.hh"

/** First iteration: counting frequent items	 
 * frequent singletons are mined, 
 * this will be used to prune the datasaet later. 
 */ 
unsigned int first_scan( char *dataset_name, 
			 dci_items& counters, 
			 unsigned int& max_trans_len, 
			 char* output_file )
{ 


#ifdef VERBOSE
  Chronos elapsed_time;
  elapsed_time.StartChronos();

  synch_printf( "#\n"
		"# ## STEP 1 . Extract Frequent Items.\n");
#endif

#ifdef MY_STATS
  FIRST_SCAN_TIME.StartChronos(); 
#endif 


  // open input file
  Data * d = new Data(dataset_name);
  if (!d) fatal_error("Could not allocate dataset object.");


  if (!d->isOpen()) fatal_error("Could not open input dataset.");
  
  // create a temporary binary representation of the dataset 
  // on disk. we will keep writing on the same file during  
  // all subsequent scans 
  binFSout<unsigned int> oo(TEMPORARY_DATASET, true); 
  if(!oo.isOpen()) fatal_error("Could not open binary dataset for writing.");

#ifdef VERBOSE
  int totalsize=0; 
#endif

  max_trans_len = 0; 
  vector<unsigned int> t; 

#ifndef AVOID_TRANS_CHECK
  bool duplicates = false;
#endif

 // counting loop 
  while(d->getNextTransaction(t) != 0) {    // read next transaction t from db 

#ifndef AVOID_TRANS_CHECK
    // sort and remove double instances
    sort(t.begin(), t.end());
    vector<unsigned int>::iterator new_end = unique(t.begin(), t.end());
    if (new_end != t.end()) {
      duplicates = true;
      unsigned int ts = new_end-t.begin();
      t.resize(ts);
    }
#endif
 
    for(unsigned i=0; i<t.size(); i++){  // for each item in t ... 
      counters.insert_item(t[i]);           // ... increase corresp. counter 
    } 

    if ( t.size() > max_trans_len)           // keep track of max trans length 
      max_trans_len = t.size(); 
  
    oo.writeTransaction(t);                 // write t to temp file 
     
#ifdef VERBOSE
    totalsize += t.size();             
#endif
  } 
  delete d;

  unsigned int n_tr = oo.get_num_of_trans(); 

#ifdef VERBOSE
#ifndef AVOID_TRANS_CHECK
  if (duplicates) {
    synch_printf( "#\n"
		  "# ## WARNING ! some transactions contain duplicated singletons.\n" 
		  "#\n");
  }
#endif
#endif 

  counters.remap_items();    //  remap item identifiers to 1:m1 
  
#ifdef MY_STATS
  FIRST_SCAN_TIME.StopChronos();
#endif 


#ifdef VERBOSE
  synch_printf( "# ## DataSize : %d * %d = %d KB, dens: %.3e. \n",
		counters.m, n_tr,
		(totalsize + n_tr) * sizeof(unsigned int) /1024, 
		(float)totalsize / (float)(n_tr*counters.m) );
  synch_printf( "# ## Removed by Sel-Sample: %d\n", counters.to_be_removed );
  synch_printf( "# ## Elapsed Time: %.2f secs. \n", elapsed_time.ReadChronos() );
#endif


  return n_tr;
} 
