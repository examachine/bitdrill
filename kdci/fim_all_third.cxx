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

// mine all frequent item sets starting from level 3
int kdci_all_three(Transaction_Set& ts, TS_Counter& counter, string out,
		   int max_level, bool clear_ts)
{
  Chronos all_time;
  all_time.StartChronos();

  if (out.size()==0)
    write_output = false;
  else {
    sprintf(OUTF, "%s", out.c_str());
    write_output = true;
  }

  dci_items counters(min_count);  // counters for singleton

  if (clear_ts)
    ts.clear_memory();
  
  // **************************************************
  // Second and Following iterations 
  // since we now know how many distinct items are frequent
  // and their maximum support, we can optimize the amount
  // of memory used to store itemsets and their counters
  // **************************************************
  

// we need one extra element to store the key-pattern flag (-1)
  int m1 = counter.F.size() + 1;

  if (m1 < 256   &&    max_support < 256)
    third_and_following_scans<unsigned char, unsigned char>
      (max_trans_len, counters, max_level);
  else if (m1 < 256  && max_support < 256*256)
    third_and_following_scans<unsigned char, unsigned short int>
      (max_trans_len, counters, max_level);  
  else if (m1 < 256  && max_support >= 256*256)
    third_and_following_scans<unsigned char, unsigned int>
      (max_trans_len, counters, max_level);  
  else if (m1 < 256*256  && max_support < 256)
    third_and_following_scans<unsigned short int, unsigned char>
      (max_trans_len, counters, max_level);  
  else if (m1 >= 256*256  && max_support < 256)
    third_and_following_scans<unsigned int, unsigned char>
      (max_trans_len, counters, max_level);  
  else if (m1 < 256*256  && max_support < 256*256)
    third_and_following_scans<unsigned short int, unsigned short int>
      (max_trans_len, counters, max_level);
  else if (m1 < 256*256  && max_support >= 256*256)
    third_and_following_scans<unsigned short int, unsigned int>
      (max_trans_len, counters, max_level);
  else if (m1 >= 256*256 && max_support < 256*256)
    third_and_following_scans<unsigned int, unsigned short int>
      (max_trans_len, counters, max_level);
  else
    third_and_following_scans<unsigned int, unsigned int>
      (max_trans_len, counters, max_level);

#ifdef VERBOSE	
  cout << "Total time: " << all_time.ReadChronos() << endl;
#endif

  unlink(TEMPORARY_DATASET);
  return 0;
}



// third and following iterations: 
// used by the entry point that starts at third iteration

template <class T, class T1>
void third_and_following_scans(TS_Counter& counter, dci_items& counters, int max_level)
{
  set_of_frequents<T, T1> *freq;         // frequent itemsets
  int k=2;                               // freq. itemset size 
  DCI_vertical_dataset<T> DCI_dataset;   // vertical dataset
  bool DCI = false;                      // DCI or DCP?
  unsigned int max_count = counters.get_max_supp();
  
  int max_trans_len = counter.max_trans_len;
 
  // Check if the in-core vertical dataset VD can be created

  if ( DCI_dataset.VD_can_be_allocated(counters.get_m1(),
				      counters.get_num_of_trans())) {
    DCI_dataset.init_VD();
    DCI = true;
  }

  set_of_frequents<T,T1> *freq;
  freq = new set_of_frequents<T,T1>(2);

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

    if (next_freq->get_num_frequents() < k) {
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

  KDCITransaction<unsigned int> t_in(max_trans_len);
  KDCITransaction<T> t_out(max_trans_len);
  
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

