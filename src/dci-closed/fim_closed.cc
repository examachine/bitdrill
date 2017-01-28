// Copyright (C) 2003 salvatore orlando <salvatore.orlando@unive.it>
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
#include <iostream>
#include <cmath>

using namespace std;
#include <stdio.h>

#include "items.hh"
#include "transaction.hh"
#include "database.hh"
#include "candidates.hh"
#include "frequents.hh"
#include "utils.hh"
#include "direct_count.hh"
#include "vertical.hh"
#include "closure.hh"
#include "writer.hh"

namespace DCIClosed {

#define TEMPORARY_DATASET "/tmp/dataset.tmp"
#define PRUNE_FACTOR_PROJ       0.04
#define PRUNE_FACTOR_NEWLIST    0.8
#define PRUNE_FACTOR_COMPLEXITY 2

string OUTF; // output file
bool write_output;   // dump frequent itemsets to file ? 


unsigned int* DCP_ITEMSETS_BY_LENGTH;
unsigned int DCP_0_ITMESETS;
unsigned int DCP_1_ITEMSETS;
unsigned int DCP_MAX_ITEMSET_LENGTH;


int first_scan(Data *d, dci_items& counters, unsigned int& max_trans_len);

template <class T, class T1>
void following_scans(int max_trans_len, dci_items& counters);

template <class T, class T1>
set_of_frequents<T,T1> *second_scan(int max_trans_len, dci_items& counters, 
				    cand_2_iter<T1>& c, 
				    DCI_vertical_dataset<T>& DCI_dataset);


template <class T, class T1>
void  DCI_iter_diffuse_key(int iter,dci_items& counters, 
			   set_of_frequents<T,T1>& previous_freq, 
			   set_of_frequents<T,T1>& next_freq, 
			   DCI_vertical_dataset<T>& DCI_dataset);


// mine closed itemsets from transaction database ts with support mincount
// if a filename out is given, the patterns are written to that file
// if max_level is non-zero, levels up to max_level are mined
// if clear_ts is true, the transaction set is cleared right after
// reading it and transferring to dci structures
void mine_closed(Transaction_Set& ts, int min_count, string out,
		 bool clear_ts=false)
{
  Chronos all_time;

  all_time.StartChronos();

  Data *d;                        // database object used only for the 
  d = new Data(ts);          // first scan

  //dci_items* counters= new dci_items(min_count);  // counters for singleton
  dci_items counters(min_count);  // counters for singleton
    
  if (out.size()==0)
    write_output=false;
  else {
    OUTF = out;
    write_output = true;
  }

  // **************************************************
  // First scan
  // **************************************************
  unsigned int max_support;  // the same as nr_of_trans
  unsigned int max_trans_len;

  max_support = first_scan(d, counters, max_trans_len);

  // m1 is the number of items (singleton) 
  // that are found to be frequent
  int m1 = counters.get_m1();

  delete d;

  //if (max_level==1) {		// if we are told to mine only 1-items
  //  return counters;
  //}

  if (clear_ts)			// won't be needed afterwards!
    ts.clear_memory();

  if (max_support == 0 || m1 < 2) {
    // there is nothing to mine...
    // cout << "Total time: " << all_time.ReadChronos() << endl;
    plog << "1" << endl <<"1"<<endl;
    // dump to files !!!!!!!!
    return;
    //return 0;
  }
  
  // **************************************************
  // Second and Following iterations 
  // since we now know how many distinct items are frequent
  // and their maximum support, we can optimize the amount
  // of memory used to store itemsets and their counters
  // **************************************************
  

  if (m1 < 256   &&    max_support < 256)
    following_scans<unsigned char, unsigned char>(max_trans_len, counters);
  else if (m1 < 256  && max_support < 256*256)
    following_scans<unsigned char, unsigned short int>(max_trans_len, counters);  
  else if (m1 < 256  && max_support >= 256*256)
    following_scans<unsigned char, unsigned int>(max_trans_len, counters);  
  else if (m1 < 256*256  && max_support < 256)
    following_scans<unsigned short int, unsigned char>(max_trans_len, counters);  
  else if (m1 >= 256*256  && max_support < 256)
    following_scans<unsigned int, unsigned char>(max_trans_len, counters);  
  else if (m1 < 256*256  && max_support < 256*256)
    following_scans<unsigned short int, unsigned short int>(max_trans_len, counters);
  else if (m1 < 256*256  && max_support >= 256*256)
    following_scans<unsigned short int, unsigned int>(max_trans_len, counters);
  else if (m1 >= 256*256 && max_support < 256*256)
    following_scans<unsigned int, unsigned short int>(max_trans_len, counters);
  else
    following_scans<unsigned int, unsigned int>(max_trans_len, counters);

#ifdef MY_STATS
  plog << " - Total time: " << all_time.ReadChronos() << endl;
#endif


  unlink(TEMPORARY_DATASET);
  //return 0;
}


/** First iteration: counting frequent items	
  * frequent singletons are mined,
	* this will be used to prune the datasaet later.
	*/
int first_scan(Data *d, dci_items& counters, unsigned int& max_trans_len)
{
#ifdef MY_STATS
  FIRST_SCAN_TIME.StartChronos();
#endif

  //float min_supp;
  bool create=true;

  // create a temporary binary representation of the dataset
  // on disk. we will keep writing on the same file during 
  // all subsequent scans
  binFSout<unsigned int> oo(TEMPORARY_DATASET, create);
  if(!oo.isOpen()) {
    cerr << TEMPORARY_DATASET << " could not be opened!" << endl;
    exit(1);
  }

  int totalsize=0;
  max_trans_len = 0;
  vector<unsigned int> t;

  // counting loop
  while(d->getNextTransaction(t)) {    // read next transaction t from db
    for(unsigned int i=0; i<t.size(); i++){  // for each item in t ...
      counters.insert_item(t[i]);           // ... increase corresp. counter
    }

    if (t.size() > max_trans_len)           // keep track of max trans length
      max_trans_len = t.size();
 
    oo.writeTransaction(t);                 // write t to temp file
    
    totalsize += t.size();                  // temp file size
  }
  
  /*
    min_supp = 100.0 * (double) counters.min_count /
    (double) oo.get_num_of_trans();
  */

  counters.set_num_of_trans(oo.get_num_of_trans());

  //  remap item identifiers to 1:m1
  counters.remap_items(write_output);
 


#ifdef MY_STATS
  FIRST_SCAN_TIME.StopChronos();
#endif


  return(counters.get_max_supp());
}



/** second and following iterations: 
  * First a pruned dataset is stored in the vertical format
	* and after the depth first iteration starts.
	*/
template <class T, class T1>
void following_scans(int max_trans_len, dci_items& counters)
{
  set_of_frequents<T, T1> *freq;         // frequent itemsets
  int k=2;                               // freq. itemset size 
  DCI_vertical_dataset<T> DCI_dataset(&counters);   // vertical dataset
  unsigned int max_count;                

  // Check if the in-core vertical dataset VD can be created
  if (DCI_dataset.VD_can_be_allocated(counters.get_m1(),
				      counters.get_num_of_trans())) {
    DCI_dataset.init_VD();
  }
  

  max_count = counters.get_max_supp();


  // ------------------------------------
  // Second iteration using a prefix table
  // ------------------------------------
  // ---------------------------------------------------
  // Prune the DB and convert to the vertical format.
  // dataset is supposed to be dense,
  // columns are reodered and sect eq is created.
  // ---------------------------------------------------

  cand_2_iter<T1>   *c;                  // candidates for 2nd iteration
  c = new cand_2_iter<T1>(counters.get_m1());
  freq = second_scan<T, T1>(max_trans_len, counters, *c, DCI_dataset);
 
	
  if (freq == NULL) {
    // ---------------------------------------------------
    // DENSE DATASET
    // ---------------------------------------------------
    ItemsetWriter<T>* writer = new ItemsetWriter<T>(&counters, OUTF.c_str());
    DCI_dataset.DClose_first_iter(writer);

    counters.delete_counts();
    delete writer;
  } else {
    // ---------------------------------------------------
    // SPARSE DATASET
    // ---------------------------------------------------
    if (freq->get_num_frequents() < 3) {
      int ok=0;
      if (freq->get_num_frequents()>0) {
	FSout o(OUTF.c_str(), 3);
	if(!o.isOpen()) {
	  cerr << OUTF.c_str() << " could not be opened for writing!" << endl;
	  exit(1);
	}
	freq->init_to_output();
	freq->dump_itemsets(counters, o, ok);
      }
      plog << DCP_0_ITMESETS + DCP_1_ITEMSETS + ok << endl;
      plog << DCP_0_ITMESETS << endl;
      plog << DCP_1_ITEMSETS << endl;
      if (ok>0) plog << ok << endl;
      delete freq;
      return;
    }

    // ---------------------------------------------
    // Iterations with DCI, i.e. using vertical dataset 
    // and intersections.
    // ---------------------------------------------

    freq->initDA(counters.get_m1());
    // initialize key-pattern flags
    freq->init_keys();

    // create a set of frequent itemsets 
    set_of_frequents<T, T1> *next_freq = new set_of_frequents<T, T1>(k); 
    set_of_frequents<T, T1> *tmp_freq; // pointer used for swapping

    ITEMSETS_BY_LENGTH = new unsigned int[counters.get_m1()+1];
    ITEMSETS_BY_LENGTH[0] = DCP_0_ITMESETS;
    ITEMSETS_BY_LENGTH[1] = DCP_1_ITEMSETS;

    while(1) {
      k++;      // increment iter counter
      DCI_iter_diffuse_key<T,T1>(k, counters, *freq, *next_freq, DCI_dataset);

      if (next_freq->get_num_frequents() < k) {
	MAX_ITEMSET_LENGTH = k-1;
	if (next_freq->get_num_frequents()>0) {
	  MAX_ITEMSET_LENGTH = k;
	  FSout o(OUTF.c_str(), k+1);
	  if(!o.isOpen()) {
	    cerr << OUTF.c_str() << " could not be opened for writing!" << endl;
	    exit(1);
	  }
	  int ok;
	  next_freq->init_to_output();
	  next_freq->dump_itemsets(counters, o, ok);

	  ITEMSETS_BY_LENGTH[k] = ok;

	}

	unsigned int tot_items = 0;
	for (unsigned int i=0; i<=MAX_ITEMSET_LENGTH; i++)
	  tot_items += ITEMSETS_BY_LENGTH[i];
	plog << tot_items << endl;
	for (unsigned int i=0; i<=MAX_ITEMSET_LENGTH; i++)
	  plog << ITEMSETS_BY_LENGTH[i] << endl;


	delete [] ITEMSETS_BY_LENGTH;
	delete freq;
	delete next_freq;
	return;
      }
      else {
	// swap sets of frequent itemsets 
	tmp_freq = freq;
	freq = next_freq;
	next_freq = tmp_freq;
      }
    }
  }

}


	/** Second iteration with direct count of 2-itemsets. 
		* If possible (enough memory) builds VD on the fly
    * for third and subsequent iterations.
		* if after the pruning one transaction has only one element
		* it's not saved 'cos useless for speth first iterations.
		* N.B.: use COUNTER for 1-itemset count!!!!
		*/
// !!! RETURN NULL IF THE DATASET IS DENSE
// !!! RETURN SET_OF_FREQUENTS IF THE DATASET IS SPARSE
template <class T, class T1>
set_of_frequents<T,T1> *second_scan(int max_trans_len, dci_items& counters, 
				    cand_2_iter<T1>& c, 
				    DCI_vertical_dataset<T>& DCI_dataset)
  // Second iteration with direct count of 2-itemsets. 
  // If possible (enough memory) builds VD on the fly
  // for third and subsequent iterations
{
#ifdef MY_STATS
  SECOND_SCAN_TIME.StartChronos(); 
#endif
  // int num = 0;

  binFSin<unsigned int> ii(TEMPORARY_DATASET);
  if(!ii.isOpen()) {
    cerr << TEMPORARY_DATASET << " could not be opened!" << endl;
    exit(1);
  }

  int m1 = counters.get_m1();
  Transaction<unsigned int> t_in(max_trans_len);
  Transaction<T> t_out(max_trans_len);

  // cout << counters.get_m1() << endl;

  // database scan
  unsigned int n_tr = 0;
  while(ii.getNextTransaction(t_in)) {
    // !!! N.B. : items are not sorted, for now !!!
    prune_and_map_and_ord_first_iter(t_in, t_out, counters);

    // only transaction useful from the second iteration are saved
    if (t_out.length >= 2) {

      // incr candidate count
      int x0;
      int index_init;
      for (int t0=0; t0 < (int) t_out.length-1; t0++) {
	x0 = (int) t_out.t[t0];
	index_init = direct_position2_init(x0, m1);
	for (int t1=t0+1; t1 < (int) t_out.length; t1++)
	  c.incr_cand_count(index_init + (int) t_out.t[t1]);
      }

      // write the trans in VD on the fly
      DCI_dataset.write_t_in_VD(n_tr, t_out);
      n_tr++;
    }
  }

  set_of_frequents<T,T1> *set_freq;

  if ( DCI_dataset.chk_compact_vertical(counters.get_m1()) == COMPACT_DATASET ) {
    // cout << "dense" << endl;
    set_freq = NULL;
  }
  else {
    T1 count;
    int  num_freq=0, k=0;
    set_freq = new set_of_frequents<T,T1>(2);
    T t_mapped[2];

    vector<bool> to_output(counters.get_m1());
    for (unsigned int i=0; i<to_output.size(); i++)
      to_output[i]=true;

    // loop over the first item
    for (int i=0; i < m1-1; i++) {
      t_mapped[0] = i;
      // loop over the second item
      for (int j=i+1; j < m1; j++) {
	t_mapped[1] = j;
	count = c.get_cand_count(k++);
	if (count >= (T1) counters.min_count){ 
	  // yes, itemset (i,j) is frequent
	  set_freq->add_itemset(t_mapped, count);

	  // check closure
	  if (count == (T1)counters.get_count(i+counters.first_freq)){
	    to_output[i] = false;
	  }
	  if (count == (T1)counters.get_count(j+counters.first_freq)){
	    to_output[j] = false;
	  }
	}
      }
    }
	    
    char count_print[32];
    int num_written=0;

    FSout o(OUTF.c_str(), 1); // Iter=1
    if(!o.isOpen()) {
      cerr << OUTF.c_str() << " could not be opened for writing!" << endl;
      exit(1);
    }

    DCP_0_ITMESETS = 0;
    DCP_1_ITEMSETS = 0;
    bool empty_closed = counters.get_num_of_trans() != counters.get_count(counters.get_m()-1);

    if (empty_closed) {
      // write empty itemset
      DCP_0_ITMESETS++;
      num_written = sprintf(count_print, "(%d)\n", counters.get_num_of_trans());
      o.printSet(count_print, num_written);
    }

    //		if (empty_closed)
    //			printf("1\n"); // 1 is the empty set

    for (unsigned int i=counters.first_freq; i < counters.get_m(); i++) {
      if (to_output[i-counters.first_freq]) {
	num_freq++;
	o.printSet(counters.unmap_ascii[i-counters.first_freq], 
		   counters.unmap_ascii_len[i-counters.first_freq]);
	num_written = sprintf(count_print, "(%d)\n", 
			      (int) counters.get_count(i));
	o.printSet(count_print, num_written);  // print for each suffix[i]
	DCP_1_ITEMSETS++;
      } 
    }


    //		printf("%d\n",num_freq);
	  
    counters.delete_counts();
  }

  
#ifdef MY_STATS
  SECOND_SCAN_TIME.StopChronos();
#endif

  return set_freq;
}




// performs the current iteration with DCI by using the optimizations for sparse datasets and key patterns
template <class T, class T1>
void  DCI_iter_diffuse_key(int iter,dci_items& counters, 
			   set_of_frequents<T,T1>& previous_freq, 
			   set_of_frequents<T,T1>& next_freq, 
			   DCI_vertical_dataset<T>& DCI_dataset)
{

    
  Chronos time;
  time.StartChronos();

  next_freq.reset(iter);
  int num_freq = 0;

  if (!previous_freq.init_gen_cand()) {
    FSout o(OUTF.c_str(), iter);
    if(!o.isOpen()) {
      cerr << OUTF.c_str() << " could not be opened for writing!" << endl;
      exit(1);
    }

    previous_freq.dump_itemsets(counters, o, num_freq);
    printf("%d\n",num_freq);
    return;
  }

  static bool first_order=false;

  T *cand, *cand_subset;
  T *CACHE_items;
  T key_pair[2];
  T1 count_pair[2];

  cand = new T[iter];
  cand_subset = new T[iter-1];
  CACHE_items = new T[iter];
  CACHE_items[0] = counters.get_m1() - 1; // init CACHE - surely different !!!


  int num_cand = 0;
  int cand_type;
  int count;
  
  previous_freq.get_prefix(cand);
  previous_freq.get_suffix(&cand[iter - 2], key_pair, count_pair);

  num_cand++;
  cand_type = NEW_PREFIX; 
 
  DCI_statistics stats;
  stats.reset_stats();

  DCI_dataset.reset_prune_mask();
  DCI_dataset.init_cache(iter);
 
  counters.init_flag_item();
  counters.init_first_item_counts();
  T key = 0;
  T1 min_count;
  T min_key;
  int one_search=0;

  int itemset_count;
  previous_freq.init_to_output();

  while (1) {
    itemset_count=0;

    if ((key_pair[0] != (T) - 1)    ||    (key_pair[1] != (T) - 1)) {// cand is surely not a key pattern 
      one_search++;
      
      if (cand[iter-2] == key_pair[0]) { // the key pattern is the first generator
        num_freq++;
	itemset_count = count_pair[1];
        next_freq.add_itemset(cand, (T1) count_pair[1], cand[iter-2]);      
	if (!first_order) {
	  for (int i = 0; i < iter; i++) 
	    counters.flag_item[cand[i]] = true;
	  counters.first_item_counts[cand[0]]++;
	}
      }
      else if (cand[iter-1] == key_pair[1]) {// the key pattern is the second generator
        num_freq++;
	itemset_count = count_pair[0];
        next_freq.add_itemset(cand, (T1) count_pair[0], cand[iter-1]);
	if (!first_order) {
	  for (int i = 0; i < iter; i++) 
	    counters.flag_item[cand[i]] = true;
	  counters.first_item_counts[cand[0]]++;
	}
      }
      else {// the key pattern is another subset: we must find it
        if (key_pair[0] != (T) -1)
          key=key_pair[0];
        else
          key=key_pair[1];
	
        int j=0;
        for (int i=0; i<iter; i++)
          if (cand[i] != key)
            cand_subset[j++] = cand[i];
        
        T tmp_key;
        if (previous_freq.find_one_subset(cand_subset, tmp_key, count)) {
          num_freq++;
	  itemset_count = count;
          next_freq.add_itemset(cand, (T1) count, key);
	  if (!first_order) {
	    for (int i = 0; i < iter; i++) 
	      counters.flag_item[cand[i]] = true;
	    counters.first_item_counts[cand[0]]++;
	  }
        }
        
      } 
      
    } else {

      if (count_pair[0] < count_pair[1]) { // remember min_count and corresponding key between generators
	min_count = count_pair[0];
	min_key = cand[iter - 1];	   
      }
      else {
	min_count = count_pair[1];
	min_key = cand[iter - 2];
      }
      
      T other_key;
      bool is_key_pattern = true;
      bool pruned = false;
      
      for (int del=iter-3; del>=0; del--) { // look for the subset with minimum count
	int j = 0;
	for (int i=0; i<iter; i++)
	  if (i != del)
	    cand_subset[j++] = cand[i];

	if (previous_freq.find_one_subset(cand_subset, other_key, count) == 0) {
	  pruned = true;
	  break; // a subset is infrequent, prune the cand and take the next one
	}
	 	
	if (other_key == (T) -1) {// remember min_count and corresponding key
	  if (min_count >= (T1) count) {
	    min_count = count;
	    min_key = cand[del];
	  }
	} else {// cand is not a key pattern
	  is_key_pattern = false;
	  int j1=0;
	  for (int i=0; i<iter; i++)
	    if (cand[i] != other_key)
	      cand_subset[j1++] = cand[i];
	  
	  T tmp_key; // check if the associated subset is frequent
	  if (previous_freq.find_one_subset(cand_subset, tmp_key, count)) {
	    num_freq++;
	    itemset_count = count;
	    next_freq.add_itemset(cand, (T1) count, other_key);
	    if (!first_order) {
	      for (int i = 0; i < iter; i++) 
		counters.flag_item[cand[i]] = true;
	      counters.first_item_counts[cand[0]]++;
	    }
	    break;
	  }
	}
      }
      if (!pruned && is_key_pattern) // we must count candidate support!
	{
	  int prefix_len;
	  for (prefix_len = 0; prefix_len < iter-1; prefix_len++) {
	    if (cand[prefix_len] != CACHE_items[prefix_len])
	      break;
	  }
    
	  for (int i = prefix_len; i < iter; i++) { // copy to cache
	    CACHE_items[i] = cand[i];
	  }

	  if (DCI_dataset.candidate_is_frequent_diffuse(cand, prefix_len, iter, 
							counters.min_count, count, 
							stats, !first_order)) {
	    if (count != (int) min_count) {
	      next_freq.add_itemset(cand, (T1) count, (T) -1);
	      itemset_count = 0;
	    }
	    else
	      next_freq.add_itemset(cand, (T1) count, min_key);
	    itemset_count = count;
	    num_freq++;
	    if (!first_order) {
	      for (int i = 0; i < iter; i++) 
		counters.flag_item[cand[i]] = true;
	      counters.first_item_counts[cand[0]]++;
	    }
   
	  }
	}
    }


    // check every subset
    if (itemset_count!=0)
      for (int del=iter-1; del>=0; del--) { // look for the subset with minimum count
	int j = 0;
	for (int i=0; i<iter; i++)
	  if (i != del)
	    cand_subset[j++] = cand[i];
	
	previous_freq.check_closed(cand_subset, itemset_count);
      }

    // generate next candidate
    cand_type = previous_freq.next_cand();
    if (cand_type == END_GEN_CAND) 
      break;
    else if (cand_type == NEW_PREFIX) 
      previous_freq.get_prefix(cand);
    previous_freq.get_suffix(&cand[iter-2], key_pair, count_pair);
    num_cand++;   
  }

  if (first_order == false) {
    DCI_dataset.order_bits_diffuse(counters);
    first_order = true;
  }
    
  delete [] cand;
  delete [] cand_subset;
  delete [] CACHE_items;

  FSout o(OUTF.c_str(), iter);
  if(!o.isOpen()) {
    cerr << OUTF.c_str() << " could not be opened for writing!" << endl;
    exit(1);
  }

  previous_freq.dump_itemsets(counters, o, num_freq);
  //printf("%d\n",num_freq);
  ITEMSETS_BY_LENGTH[iter-1] = num_freq;

  return;
}

}




