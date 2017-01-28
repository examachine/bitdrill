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

#include "fim_all.hh"

#include "items.hh"
#include "transaction.hh"
#include "database.hh"
#include "candidates.hh"
#include "frequents.hh"
#include "utils.hh"
#include "direct_count.hh"
#include "vertical.hh"

using namespace KDCI;

namespace KDCI {
template <class T, class T1>
void DCP_iter(int iter, int max_trans_len, 
	      dci_items& counters, 
	      DCP_candidates<T,T1>& c, set_of_frequents<T,T1>& next_freq,
	      DCI_vertical_dataset<T>& DCI_dataset);

template <class T, class T1>
void DCI_iter_diffuse(int iter, dci_items& counters, 
		     set_of_frequents<T,T1>& freq, 
		     set_of_frequents<T,T1>& next_freq, 
		     DCI_vertical_dataset<T>& DCI_dataset);

template <class T, class T1>
void DCI_iter_diffuse_key(int iter, dci_items& counters, 
		     set_of_frequents<T,T1>& freq, 
		     set_of_frequents<T,T1>& next_freq, 
		     DCI_vertical_dataset<T>& DCI_dataset);

template <class T, class T1>
void DCI_iter_compact(int iter, dci_items& counters, 
		    set_of_frequents<T,T1>& freq, 
		    set_of_frequents<T,T1>& next_freq, 
		    DCI_vertical_dataset<T>& DCI_dataset);

template <class T, class T1>
void DCI_iter_compact_key(int iter, dci_items& counters, 
		    set_of_frequents<T,T1>& freq, 
		    set_of_frequents<T,T1>& next_freq, 
		    DCI_vertical_dataset<T>& DCI_dataset);

#define PRUNE_FACTOR_PROJ       0.04
#define PRUNE_FACTOR_NEWLIST    0.8
#define PRUNE_FACTOR_COMPLEXITY 2

const char* TEMPORARY_DATASET = "/tmp/dataset.tmp";
bool write_output;   // dump frequent itemsets to file ? 
char OUTF[256] = ""; // output filename -- this doesn't look good -- exa


// First iteration: counting frequent items	
int first_scan(KDCI::Data *d, dci_items& counters, unsigned int& max_trans_len,
	       bool del_counts)
{
  Chronos time;
  time.StartChronos();

  float min_supp;
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
  ::Transaction t;

  // counting loop
  while(d->getNextTransaction(t) != 0) {    // read next transaction t from db
    for(unsigned int i=0; i<t.size(); i++)  // for each item in t ...
      counters.insert_item(t[i]);           // ... increase corresp. counter
    if (t.size() > max_trans_len)           // keep track of max trans length
      max_trans_len = t.size();
 
    oo.writeTransaction((vector<unsigned int>&)t); // write t to temp file
    
    totalsize += t.size();                  // temp file size
  }
  
  min_supp = 100.0 * (double) counters.min_count /
    (double) oo.get_num_of_trans();

  counters.set_num_of_trans(oo.get_num_of_trans());
  //  remap item identifiers to 1:m1
  int first_freq = counters.remap_items(write_output);

  if (write_output) { // dump frequents to file 
    FSout o(OUTF, 1); // Iter=1
    if(!o.isOpen()) {
      cerr << OUTF << " could not be opened for writing!" << endl;
      exit(1);
    }

    char count_print[32];
    // write empty itemset
    int num_written = sprintf(count_print, "(%d)\n", oo.get_num_of_trans());
    o.printSet(count_print, num_written);

    //  output frequent items 
    for (unsigned int i=first_freq; i < counters.get_m(); i++) {
     
      o.printSet(counters.unmap_ascii[i-first_freq], 
		 counters.unmap_ascii_len[i-first_freq]);
      num_written = sprintf(count_print, "(%d)\n", 
			    (int) counters.get_count(i));
      o.printSet(count_print, num_written);  // print for each suffix[i]
    }
  }

  if (del_counts)
    counters.delete_counts();


#ifdef VERBOSE  
  cout << "# Database statistics:\n#\t" 
       << oo.get_num_of_trans() <<" transactions\n#\t" 
       << counters.get_m() << " different items\n#\t"
       << "Max transaction length = " << max_trans_len <<"\n#\n# ";
  cout << "Min support = " << min_supp << "%    "
       << "Min count   = " << counters.min_count << endl << endl;

  print_statistics("DCP", 1, counters.get_m(), counters.get_m1(), time.ReadChronos());
#else
  nlog << 1 << endl << counters.get_m1() << endl;
  //printf("1\n%d\n",counters.get_m1()); // 1 is the empty set
#endif


  return(counters.get_max_supp());
}


template <class T, class T1>
set_of_frequents<T,T1> *second_scan(int max_trans_len, dci_items& counters, 
				    cand_2_iter<T1>& c, 
				    DCI_vertical_dataset<T>& DCI_dataset)
  // Second iteration with direct count of 2-itemsets. 
  // If possible (enough memory) builds VD on the fly
  // for third and subsequent iterations
{
  Chronos time;
  time.StartChronos(); 

  binFSin<unsigned int> ii(TEMPORARY_DATASET);
  if(!ii.isOpen()) {
    cerr << TEMPORARY_DATASET << " could not be opened!" << endl;
    exit(1);
  }
  
  bool create=false;
  binFSout<T> oo(TEMPORARY_DATASET, create);
  if(!oo.isOpen()) {
    cerr << TEMPORARY_DATASET << " could not be opened!" << endl;
    exit(1);
  }

  int m1 = counters.get_m1();

  KDCI::Transaction<unsigned int> t_in(max_trans_len);
  KDCI::Transaction<T> t_out(max_trans_len);
  
  // 2nd database scan
  // direct count of 2-itemsets
  unsigned int n_tr = 0;
  while(ii.getNextTransaction(t_in)) {
    prune_and_map_and_ord_first_iter(t_in, t_out, counters);
    if (t_out.length >= 2) {
      int x0;
      int index_init;
      for (int t0=0; t0 < (int) t_out.length-1; t0++) {
	x0 = (int) t_out.t[t0];
	index_init = direct_position2_init(x0, m1);
	for (int t1=t0+1; t1 < (int) t_out.length; t1++)
	  c.incr_cand_count(index_init + (int) t_out.t[t1]);
      }
 
      if (DCI_dataset.VD_is_allocated()) { 
	// write the trans in VD on the fly
	DCI_dataset.write_t_in_VD(n_tr, t_out);
	n_tr++;
      }
      else 
	// write Dk
	oo.writeTransaction(t_out);
    }
  }
   
  // output frequent 2-itemsets and set global pruning mask

  if (!DCI_dataset.VD_is_allocated()) {
    // trunc and set global pruning mask only if DCP continues
    oo.trunc();
    counters.set_num_of_trans(oo.get_num_of_trans());
    counters.init_global_pruning();
  } 

  T1 count;
  int  num_freq=0, k=0;
  set_of_frequents<T,T1> *set_freq;
  set_freq = new set_of_frequents<T,T1>(2);
  T t_mapped[2];

  if (write_output) { 
    // dump to file frequent 2-itemsets 
    FSout o(OUTF, 2); // Iter=2
    if(!o.isOpen()) {
      cerr << OUTF << " could not be opened for writing!" << endl;
      exit(1);
    }

    static const int SZ_NUM = 128;
    char cand_unmapped[SZ_NUM * (2+1)]; // +1 for storing count
    int sz1, sz2, num_written;

    // loop over the first item
    for (int i=0; i < m1-1; i++) {
      t_mapped[0] = i;

      memcpy(&cand_unmapped[0], counters.unmap_ascii[i], 
	     counters.unmap_ascii_len[i]);
      sz1 = counters.unmap_ascii_len[i];

    // loop over the second item
      for (int j=i+1; j < m1; j++) {
	t_mapped[1] = j;
      
	memcpy(&cand_unmapped[sz1], counters.unmap_ascii[j], 
	       counters.unmap_ascii_len[j]);
	sz2 = sz1 + counters.unmap_ascii_len[j];
	count = c.get_cand_count(k++);
	if (count >= (T1) counters.min_count){ 
	  // yes, itemset (i,j) is frequent
	  num_freq++;
	  if (!DCI_dataset.VD_is_allocated()) { 
	    // prune only if DCP continues
	    counters.incr_global_pruning((unsigned int) i);	
	    counters.incr_global_pruning((unsigned int) j);
	  }
	  set_freq->add_itemset(t_mapped, count);
	
	  num_written = sprintf(&cand_unmapped[sz2], "(%d)\n", count);
	  o.printSet(cand_unmapped, sz2+num_written);
	}
      }
    }
  }
  else { 
    // DON't dump to file frequent 2-itemsets 

    // loop over the first item
    for (int i=0; i < m1-1; i++) {
      t_mapped[0] = i;
      // loop over the second item
      for (int j=i+1; j < m1; j++) {
	t_mapped[1] = j;
	count = c.get_cand_count(k++);
	if (count >= (T1) counters.min_count){ 
	  // yes, itemset (i,j) is frequent
	  num_freq++;
	  if (!DCI_dataset.VD_is_allocated()) { 
	    // prune only if DCP continues
	    counters.incr_global_pruning((unsigned int) i);	
	    counters.incr_global_pruning((unsigned int) j);
	  }
	  set_freq->add_itemset(t_mapped, count);
// 	  cout << (int)t_mapped[0] << ":" << (int)t_mapped[1] << endl;
	
	}
      }
    }
  }
  
  if (!DCI_dataset.VD_is_allocated())   // prune only if DCP continues
    counters.end_global_pruning(2);     // parameter: K=2

#ifdef VERBOSE
  print_statistics("DCP", 2, m1*(m1-1)/2, num_freq, time.ReadChronos());
#else
  nlog << num_freq << endl;
  //printf("%d\n",num_freq);
#endif
  
  return set_freq;
}


// second and following iterations: 
// the second with direct count, the others with DCP or DCI

template <class T, class T1>
void following_scans(int max_trans_len, dci_items& counters, int max_level)
{
  set_of_frequents<T, T1> *freq;         // frequent itemsets
  int k=2;                               // freq. itemset size 
  DCI_vertical_dataset<T> DCI_dataset;   // vertical dataset
  bool DCI = false;                      // DCI or DCP?
  unsigned int max_count;                
 
  // Check if the in-core vertical dataset VD can be created

  if ( DCI_dataset.VD_can_be_allocated(counters.get_m1(),
				       counters.get_num_of_trans())) {
    DCI_dataset.init_VD();
    DCI = true;
  }
  max_count = counters.get_max_supp();

  // ------------------------------------
  // Second iteration using a prefix table
  // ------------------------------------

  cand_2_iter<T1>   *c;                  // candidates for 2nd iteration
  c = new cand_2_iter<T1>(counters.get_m1());
  freq = second_scan<T, T1>(max_trans_len, counters, *c, DCI_dataset);
  if (freq == NULL)
    return;
  
  if (freq->get_num_frequents() < 3 || max_level==2) {
    // there is nothing else to mine ...
    delete freq;
    return;
  }

  // ------------------------------------
  // third and following iterations
  // ------------------------------------
 
  DCP_candidates<T,T1> *cand;
  cand = new DCP_candidates<T,T1>(k); // allocate cand the first time

  freq->initDA(counters.get_m1());

  // Iterate with DCP until DCI can start
  while (!DCI) { 

    k++;      // increment iter counter

    // Check if the in-core vertical dataset VD can be created
    if (DCI_dataset.VD_can_be_allocated(counters.get_mk(), 
					counters.get_num_of_trans())) {
      // next iter with DCI 
      DCI_dataset.init_VD();
      DCI = true; 
    }

    gen_candidates<T, T1>(*freq, *cand, counters, k, *c);

    if (cand->get_num_candidates() == 0) {
      cout << "no more candidates !\n";
      return;
    }

    if (k==3) {
      delete c;      
      cand->initDA(freq->DA_m);
    }

    DCP_iter<T, T1>(k, max_trans_len, counters, *cand, *freq, DCI_dataset);
    if (freq->get_num_frequents() < k+1 || k==max_level) {
      // nothing to mine ...
      delete cand;
      delete freq;
      return;
    }

  }


  delete cand; // DCI doesn't use candidates

  // ---------------------------------------------
  // Iterations with DCI, i.e. using vertical dataset 
  // and intersections.
  // ---------------------------------------------

  // check the vertical dataset to choose between 
  // sparse or dense optimizations
  // ALSO reorder the columns of the vertical dataset!!
  DCI_dataset.chk_compact_vertical(counters.get_mk());

  // initialize key-pattern flags
  freq->init_keys();

  // create a set of frequent itemsets 
  set_of_frequents<T, T1> *next_freq = new set_of_frequents<T, T1>(k); 
  set_of_frequents<T, T1> *tmp_freq; // pointer used for swapping

  // decide if it is convenient to use the 
  // key-pattern optimization or not 
  bool use_keys = counters.use_key_patterns();

  while(1) {
    k++;      // increment iter counter

    if (use_keys) { 
      // few key patterns are expected
      // it is faster to try to infer itemsets 
      // supports from non-key patterns
      DCI_iter_compact_key<T, T1>(k, counters, *freq, *next_freq, DCI_dataset);
    } else { 
      // too many key patterns are expected
      // it is faster to *count* itemset supports
      // but we can still check if the dataset 
      // is compact or not.
      if (DCI_dataset.is_compact())
	DCI_iter_compact<T, T1>(k, counters, *freq, *next_freq, DCI_dataset);
      else
	DCI_iter_diffuse<T, T1>(k, counters, *freq, *next_freq, DCI_dataset);
    }

    if (next_freq->get_num_frequents() < k || k==max_level) {
      // nothing to mine
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

// performs the current iteration with DCP. 
// If possible it builds VD on the fly
template <class T, class T1>
void DCP_iter(int iter, 
	      int max_trans_len, dci_items& counters, 
	      DCP_candidates<T,T1>& c, set_of_frequents<T,T1>& next_freq,
	      DCI_vertical_dataset<T>& DCI_dataset)
{

  Chronos time;
  time.StartChronos();

  binFSin<T> ii(TEMPORARY_DATASET);
  if(!ii.isOpen()) {
    cerr << TEMPORARY_DATASET << " could not be opened!" << endl;
    exit(1);
  }
  
  bool create=false;
  binFSout<T> oo(TEMPORARY_DATASET, create);
  if(!oo.isOpen()) {
    cerr << TEMPORARY_DATASET << " could not be opened!" << endl;
    exit(1);
  }

  int m1 = counters.get_m1();
  dci_transaction<T> t(max_trans_len, iter, m1);

  bool is_remapped=false;
  if (DCI_dataset.VD_is_allocated()) {
    is_remapped = counters.update_map();   // new mapping of items!!!
  }

  unsigned int n_tr=0;

  while(ii.getNextTransaction(t)) {
    t.prune_global(counters); // t.t_len = 0 or at least iter
    if (t.t_len > 0) {
      t.init_prune_local();
      c.subset_and_count_and_prune_local(t, next_freq);
      t.prune_local(); // t.t_len = 0 or at least iter+1

      if (DCI_dataset.VD_is_allocated()) { // write the trans in VD on the fly
	if (is_remapped) {
	  for (unsigned int h=0; h<t.t_len; h++)
	    t.elements[h] = counters.map[t.elements[h]];
	}
	DCI_dataset.write_t_in_VD(n_tr, t);
	n_tr++;
      }
      else
	oo.writeTransaction(t); // write pruned trans
    }
  }
  
  if (!DCI_dataset.VD_is_allocated()) {// trunc and prune only if DCP continues
    oo.trunc();
    counters.set_num_of_trans(oo.get_num_of_trans());
    counters.init_global_pruning();
  } 

  // Dump frequents
  next_freq.reset(iter);

  T1 count;
  int  num_freq=0;
  T *v, *v_remapped;

  v = new T[iter];
  v_remapped = new T[iter];

  int ind;
  T1 tmp_c;


  c.init_read_itemsets();
  while ((ind=c.read_next_itemset(v, tmp_c)) != -1) {
    if ((count = c.get_count(ind)) >= (T1) counters.min_count) {
      num_freq++;

      if (!DCI_dataset.VD_is_allocated()) { // prune only if DCP continues
	for (int i=0; i<iter; i++) {
	  counters.incr_global_pruning((unsigned int) v[i]);
	}
	next_freq.add_itemset(v, count);
	
      }
      else {
	if (is_remapped) {
	  for (int i=0; i<iter; i++) 
	    v_remapped[i] = (T) counters.map[v[i]]; // re-map items
	  next_freq.add_itemset(v_remapped, count);
	}
	else
	  next_freq.add_itemset(v, count);
      }
    }
  }
  
  delete [] v;
  delete [] v_remapped;

  if (DCI_dataset.VD_is_allocated()  && is_remapped)
    counters.update_unmap(write_output);   // new mapping of items!!!

  if (write_output) { // dump to file frequent itemsets 
    FSout o(OUTF, iter);
    if(!o.isOpen()) {
      cerr << OUTF << " could not be opened for writing!" << endl;
      exit(1);
    }

    next_freq.dump_itemsets(counters, o);
  }

  if (!DCI_dataset.VD_is_allocated()) // prune only if DCP continues
    counters.end_global_pruning(iter);
 

#ifdef VERBOSE
  print_statistics("DCP", iter, c.get_num_candidates(), 
		   num_freq, time.ReadChronos());
#else
  nlog << num_freq << endl;
  //printf("%d\n",num_freq);
#endif
}




// performs the current iteration with DCI 
// by using the optimizations for sparse datasets
template <class T, class T1>
void DCI_iter_diffuse(int iter,dci_items& counters, 
		     set_of_frequents<T,T1>& previous_freq, 
		     set_of_frequents<T,T1>& next_freq, 
		     DCI_vertical_dataset<T>& DCI_dataset)
{
  Chronos time;
  time.StartChronos();

  static bool pruning_flag = true;

  pruning_flag = ! pruning_flag;

  next_freq.reset(iter);

  if (!previous_freq.init_gen_cand())
    return;

  static bool first_order=false;

  T *cand;
  T *CACHE_items;

  cand = new T[iter];
  CACHE_items = new T[iter];
  CACHE_items[0] = counters.get_m1() - 1; // init CACHE - surely different !!!

  int num_freq = 0;
  int num_cand = 0;
  int cand_type;
  int count;


  previous_freq.get_prefix(cand);
  previous_freq.get_suffix(&cand[iter - 2]);

  num_cand++;
  cand_type = NEW_PREFIX; 
 
  DCI_statistics stats;
  stats.reset_stats();

  DCI_dataset.reset_prune_mask();
  DCI_dataset.init_cache(iter);
 
  counters.init_flag_item();
  counters.init_first_item_counts();

  while (1) {
    int start;
    if (cand_type == NEW_PREFIX)
      start = 0;
    else
      start = iter - 2;

    int prefix_len;
    for (prefix_len = start; prefix_len < iter-1; prefix_len++) {
      if (cand[prefix_len] != CACHE_items[prefix_len])
	break;
    }
    
    for (int i = prefix_len; i < iter; i++) { // copy to cache
      CACHE_items[i] = cand[i];
    }

    if (DCI_dataset.candidate_is_frequent_diffuse(cand, prefix_len, iter, 
					  counters.min_count, count, 
					  stats, pruning_flag)) {
      num_freq++;
      next_freq.add_itemset(cand, (T1) count);

      if (pruning_flag) {
	for (int i = 0; i < iter; i++) 
	  counters.flag_item[cand[i]] = true;
	counters.first_item_counts[cand[0]]++;
      }
      
    }
 
    cand_type = previous_freq.next_cand();
    if (cand_type == END_GEN_CAND) 
      break;
    else if (cand_type == NEW_SUFFIX) 
      previous_freq.get_suffix(&cand[iter-2]);
    else {
      previous_freq.get_prefix(cand);
      previous_freq.get_suffix(&cand[iter-2]);
    }
    num_cand++;   
  }

  int COMPLEXITY_INTERSECTION, COMPLEXITY_PRUNING;
  int new_tid_list_size;

  if (pruning_flag) {
    int sz_list_proj;  // aggiunto
    // aggiunto parametro
    stats.get_stats(counters.get_active_items(), num_cand, iter,  
		    DCI_dataset.get_tid_list_size(),  sz_list_proj,
		    COMPLEXITY_INTERSECTION,
		    COMPLEXITY_PRUNING);

    new_tid_list_size = DCI_dataset.check_pruning();
    
    if ((sz_list_proj > PRUNE_FACTOR_PROJ*DCI_dataset.get_tid_list_size()) && 
	(new_tid_list_size<DCI_dataset.get_tid_list_size()*PRUNE_FACTOR_NEWLIST) &&
	(COMPLEXITY_PRUNING < COMPLEXITY_INTERSECTION/PRUNE_FACTOR_COMPLEXITY)) {
      DCI_dataset.prune_VD (new_tid_list_size, counters);
      //      cout << "ORDERING\n";
      DCI_dataset.order_bits_diffuse(counters);
      first_order = true;
    }
    
    
    //      stats.get_stats(counters.get_active_items(), num_cand, iter, 
    //  		    DCI_dataset.get_tid_list_size(), 
    //  		    COMPLEXITY_INTERSECTION, 
    //  		    COMPLEXITY_PRUNING);
    
    //      new_tid_list_size = DCI_dataset.check_pruning();
    
    //      if ((new_tid_list_size < DCI_dataset.get_tid_list_size() * 0.8) && 
    //  	(COMPLEXITY_PRUNING < COMPLEXITY_INTERSECTION/2)) {
    
    //        DCI_dataset.prune_VD (new_tid_list_size, counters);
    //        DCI_dataset.order_bits_sparse(counters);
    //        first_order = true;
    //      }
  }
  
  if (first_order == false) {
    DCI_dataset.order_bits_diffuse(counters);
    first_order = true;
  }
  
  delete [] cand;
  delete [] CACHE_items;

  if (write_output) { // dump to file frequent itemsets 
    FSout o(OUTF, iter);
    if(!o.isOpen()) {
      cerr << OUTF << " could not be opened for writing!" << endl;
      exit(1);
    }
    next_freq.dump_itemsets(counters, o);
  }

#ifdef VERBOSE
  print_statistics("DCIs", iter, num_cand, num_freq, time.ReadChronos());
#else
  nlog << num_freq << endl;
  //printf("%d\n",num_freq);
#endif

  return;
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

  if (!previous_freq.init_gen_cand())
    return;

  static bool first_order=false;

  T *cand, *cand_subset;
  T *CACHE_items;
  T key_pair[2];
  T1 count_pair[2];

  cand = new T[iter];
  cand_subset = new T[iter-1];
  CACHE_items = new T[iter];
  CACHE_items[0] = counters.get_m1() - 1; // init CACHE - surely different !!!

  int num_freq = 0;
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

  while (1) {

    if ((key_pair[0] != (T) - 1)    ||    (key_pair[1] != (T) - 1)) {// cand is surely not a key pattern 
      one_search++;
      
      if (cand[iter-2] == key_pair[0]) { // the key pattern is the first generator
        num_freq++;
        next_freq.add_itemset(cand, (T1) count_pair[1], cand[iter-2]);      
	if (!first_order) {
	  for (int i = 0; i < iter; i++) 
	    counters.flag_item[cand[i]] = true;
	  counters.first_item_counts[cand[0]]++;
	}
      }
      else if (cand[iter-1] == key_pair[1]) {// the key pattern is the second generator
        num_freq++;
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
	    if (count != (int) min_count)
	      next_freq.add_itemset(cand, (T1) count, (T) -1);
	    else
	      next_freq.add_itemset(cand, (T1) count, min_key);
	    num_freq++;
	    if (!first_order) {
	      for (int i = 0; i < iter; i++) 
		counters.flag_item[cand[i]] = true;
	      counters.first_item_counts[cand[0]]++;
	    }
   
	  }
	}
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

  if (write_output) { // dump to file frequent itemsets 
    FSout o(OUTF, iter);
    if(!o.isOpen()) {
      cerr << OUTF << " could not be opened for writing!" << endl;
      exit(1);
    }
    next_freq.dump_itemsets(counters, o);
  }

#ifdef VERBOSE
  print_statistics("DCIsk", iter, num_cand, num_freq, time.ReadChronos());
//    cout << "one search : " << one_search << " ("<< ((float) one_search)/num_cand*100 << ")"<< endl;
#else
  nlog << num_freq << endl;
  //printf("%d\n",num_freq);
#endif

  return;
}



// performs the current iteration with DCI 
// by using the optimizations for dense datasets
template <class T, class T1>
void DCI_iter_compact_key(int iter,dci_items& counters, 
				       set_of_frequents<T,T1>& previous_freq, 
				       set_of_frequents<T,T1>& next_freq, 
				       DCI_vertical_dataset<T>& DCI_dataset)
{

  Chronos time;
  time.StartChronos();


  next_freq.reset(iter);

  if (!previous_freq.init_gen_cand())
    return;

  T *cand, *cand_subset;
  T *CACHE_items;
  T key_pair[2];
  T1 count_pair[2];

  cand = new T[iter];
  cand_subset = new T[iter-1];

  CACHE_items = new T[iter];
  CACHE_items[0] = counters.get_m1() - 1; // init CACHE - surely different !!!

  int num_freq = 0;
  int num_cand = 0;
  int cand_type;
  int count;

  previous_freq.get_prefix(cand);
  previous_freq.get_suffix(&cand[iter - 2], key_pair, count_pair);

  num_cand++;
  cand_type = NEW_PREFIX; 
 
  DCI_statistics stats;
  stats.reset_stats();

  DCI_dataset.init_cache(iter);

  T key = 0;
  T1 min_count;
  T min_key;
  int one_search=0;
  
  while (1) {
    if ((key_pair[0] != (T) - 1) || (key_pair[1] != (T) - 1)) {
      // cand is surely not a key pattern 
      one_search++;
      
      if (cand[iter-2] == key_pair[0]) { 
	// the key pattern is the first generator
        num_freq++;
        next_freq.add_itemset(cand, (T1) count_pair[1], cand[iter-2]);
      }
      else if (cand[iter-1] == key_pair[1]) {
	// the key pattern is the second generator
        num_freq++;
        next_freq.add_itemset(cand, (T1) count_pair[0], cand[iter-1]);
      }
      else {
	// the key pattern is another subset: we must find it
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
          next_freq.add_itemset(cand, (T1) count, key);
        }
        
      } 
      
    } else {

      if (count_pair[0] < count_pair[1]) { 
	// remember min_count and corresponding key between generators
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

      for (int del=iter-3; del>=0; del--) { 
	// look for the subset with minimum count
	int j = 0;
	for (int i=0; i<iter; i++)
	  if (i != del)
	    cand_subset[j++] = cand[i];

	if (previous_freq.find_one_subset(cand_subset, other_key, count)==0){
	  pruned = true;
	  break;
	  // a subset is infrequent, prune the cand and take the next one
	}
	 	
	if (other_key == (T) -1) {
	  // remember min_count and corresponding key
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
	    next_freq.add_itemset(cand, (T1) count, other_key);
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
	  
	  if (DCI_dataset.candidate_is_frequent_compact(cand, 
						      prefix_len, iter, 
						      (int) counters.min_count, 
						      count, 
						      stats)) {
	    
	    num_freq++;
	    if (count != (int) min_count)
	      next_freq.add_itemset(cand, (T1) count, (T) -1);
	    else
	      next_freq.add_itemset(cand, (T1) count, min_key);
	  }
	}
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

  delete [] cand;
  delete [] cand_subset;
  delete [] CACHE_items;

  if (write_output) { // dump to file frequent itemsets 
    FSout o(OUTF, iter);
    if(!o.isOpen()) {
      cerr << OUTF << " could not be opened for writing!" << endl;
      exit(1);
    }
    next_freq.dump_itemsets(counters, o);
  }
  
#ifdef VERBOSE
  print_statistics("DCIdk", iter, num_cand, num_freq, time.ReadChronos());
//    cout << "one search : " << one_search << " ("<< ((float) one_search)/num_cand*100 << ")"<< endl;
#else
  nlog << num_freq << endl;
  //printf("%d\n",num_freq);
#endif
  return;
}

// performs the current iteration with DCI by using the optimizations for dense datasets
template <class T, class T1>
void DCI_iter_compact(int iter,dci_items& counters, 
				       set_of_frequents<T,T1>& previous_freq, 
				       set_of_frequents<T,T1>& next_freq, 
				       DCI_vertical_dataset<T>& DCI_dataset)
{

  Chronos time;
  time.StartChronos();


  next_freq.reset(iter);

  if (!previous_freq.init_gen_cand())
    return;

  T *cand;
  T *CACHE_items;

  cand = new T[iter];
  CACHE_items = new T[iter];
  CACHE_items[0] = counters.get_m1() - 1; // init CACHE - surely different !!!

  int num_freq = 0;
  int num_cand = 0;
  int cand_type;
  int count;

  previous_freq.get_prefix(cand);
  previous_freq.get_suffix(&cand[iter - 2]);

  num_cand++;
  cand_type = NEW_PREFIX; 
 
  DCI_statistics stats;
  stats.reset_stats();

  DCI_dataset.init_cache(iter);

  while (1) {
    int start;
    if (cand_type == NEW_PREFIX)
      start = 0;
    else
      start = iter - 2;

    int prefix_len;
    for (prefix_len = start; prefix_len < iter-1; prefix_len++) {
      if (cand[prefix_len] != CACHE_items[prefix_len])
	break;
    }
    
    for (int i = prefix_len; i < iter; i++) { // copy to cache 
      CACHE_items[i] = cand[i];
    }
    //DCI_dataset.set_is_included_flags(cand, prefix_len, iter);


    if (DCI_dataset.candidate_is_frequent_compact(cand, 
						prefix_len, iter, 
						(int) counters.min_count, 
						count, 
						stats)) {

      num_freq++;
      next_freq.add_itemset(cand, (T1) count);
    }

    cand_type = previous_freq.next_cand();
    if (cand_type == END_GEN_CAND) 
      break;
    else if (cand_type == NEW_SUFFIX) 
      previous_freq.get_suffix(&cand[iter-2]);
    else {
      previous_freq.get_prefix(cand);
      previous_freq.get_suffix(&cand[iter-2]);
    }
    num_cand++;   
  }
  
  delete [] cand;
  delete [] CACHE_items;

  if (write_output) { // dump to file frequent itemsets 
    FSout o(OUTF, iter);
    if(!o.isOpen()) {
      cerr << OUTF << " could not be opened for writing!" << endl;
      exit(1);
    }
    next_freq.dump_itemsets(counters, o);
  }
  
#ifdef VERBOSE
  print_statistics("DCId", iter, num_cand, num_freq, time.ReadChronos());
#else
  nlog << num_freq << endl;
  //printf("%d\n",num_freq);
#endif
  return;
}


// mine all frequent item sets
dci_items* kdci_all(Transaction_Set& ts, int min_count, string out,
	     int max_level, bool clear_ts)
{
  Chronos all_time;
  all_time.StartChronos();

  Data *d;                        // database object used only for the 
  d = new Data(ts);          // first scan  

  if (out.size()==0)
    write_output = false;
  else {
    sprintf(OUTF, "%s", out.c_str());
    write_output = true;
  }

  dci_items *counters = new dci_items(min_count);  // counters for singleton

  // **************************************************
  // First iteration 
  // **************************************************
  unsigned int max_support;  // the same as nr_of_trans
  unsigned int max_trans_len;

  max_support = first_scan(d, *counters, max_trans_len);

  // m1 is the number of items (singleton) 
  // that are found to be frequent
  int m1 = counters->get_m1();

  delete d;

  if (max_level==1) {
    return counters;
  }

  if (clear_ts)
    ts.clear_memory();

  if (max_support == 0 || m1 < 2) {
#ifdef VERBOSE
    // there is nothing to mine...
    cout << "Total time: " << all_time.ReadChronos() << endl;
#endif
    return counters;
  }

  
  // **************************************************
  // Second and Following iterations 
  // since we now know how many distinct items are frequent
  // and their maximum support, we can optimize the amount
  // of memory used to store itemsets and their counters
  // **************************************************
  

  m1++; // we need one extra element to store the key-pattern flag (-1)

  if (m1 < 256   &&    max_support < 256)
    following_scans<unsigned char, unsigned char>
      (max_trans_len, *counters, max_level);
  else if (m1 < 256  && max_support < 256*256)
    following_scans<unsigned char, unsigned short int>
      (max_trans_len, *counters, max_level);  
  else if (m1 < 256  && max_support >= 256*256)
    following_scans<unsigned char, unsigned int>
      (max_trans_len, *counters, max_level);  
  else if (m1 < 256*256  && max_support < 256)
    following_scans<unsigned short int, unsigned char>
      (max_trans_len, *counters, max_level);  
  else if (m1 >= 256*256  && max_support < 256)
    following_scans<unsigned int, unsigned char>
      (max_trans_len, *counters, max_level);  
  else if (m1 < 256*256  && max_support < 256*256)
    following_scans<unsigned short int, unsigned short int>
      (max_trans_len, *counters, max_level);
  else if (m1 < 256*256  && max_support >= 256*256)
    following_scans<unsigned short int, unsigned int>
      (max_trans_len, *counters, max_level);
  else if (m1 >= 256*256 && max_support < 256*256)
    following_scans<unsigned int, unsigned short int>
      (max_trans_len, *counters, max_level);
  else
    following_scans<unsigned int, unsigned int>
      (max_trans_len, *counters, max_level);

#ifdef VERBOSE	
  cout << "Total time: " << all_time.ReadChronos() << endl;
#endif
#ifdef PROFILE
  cout << "candidate check count: " << candidate_check_count << endl;
#endif
  unlink(TEMPORARY_DATASET);
  return counters;
}


} // namespace

