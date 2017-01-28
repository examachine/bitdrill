#ifndef COUNT_KDCI
#define COUNT_KDCI

#include "kdci/fim_all.hh"
#include "kdci/items.hh"
#include "kdci/transaction.hh"
#include "kdci/database.hh"
#include "kdci/candidates.hh"
#include "kdci/frequents.hh"
#include "kdci/utils.hh"
#include "kdci/direct_count.hh"
#include "kdci/vertical.hh"

using namespace KDCI;

dci_items* kdci_first_scan(Transaction_Set& ts, int min_count,
			   unsigned int& max_support,
			   unsigned int& max_trans_len)
{
  Data *d;                        // database object used only for the 
  d = new Data(ts);          // first scan  

  dci_items* counters = new dci_items(min_count);  // counters for singleton

  max_support = first_scan(d, *counters, max_trans_len);

  return counters;
}

template <class T, class T1>
void only_second_scan(int max_trans_len, dci_items& counters,
		      Undir_Mtx_Graph& G, vector<int>& item_f)
{
  uint64_t k=2;                               // freq. itemset size 
  DCI_vertical_dataset<T> DCI_dataset;   // vertical dataset
  //bool DCI = false;                      // DCI or DCP?

  //unsigned int max_count;                
  //max_count = counters.get_max_supp();
  
  // ------------------------------------
  // Second iteration using a prefix table
  // ------------------------------------

  int m1 = counters.get_m1();
  cand_2_iter<T1> c(m1);	// candidates for 2nd iteration

  binFSin<unsigned int> ii(TEMPORARY_DATASET);
  if(!ii.isOpen()) {
    cerr << TEMPORARY_DATASET << " could not be opened!" << endl;
    exit(1);
  }
  
  KDCI::Transaction<unsigned int> t_in(max_trans_len);
  KDCI::Transaction<T> t_out(max_trans_len);
  
  // 2nd database scan
  // direct count of 2-itemsets
  //unsigned int n_tr = 0;
  while(ii.getNextTransaction(t_in)) {
    prune_and_map_and_ord_first_iter(t_in, t_out, counters);
    if (t_out.length >= 2) {
      uint64_t x0;
      uint64_t index_init;
      for ( uint64_t t0=0; t0 < (uint64_t) t_out.length-1; t0++) {
	x0 = ( uint64_t) t_out.t[t0];
	index_init = direct_position2_init(x0, m1);
	for ( uint64_t t1=t0+1; t1 < ( uint64_t) t_out.length; t1++)
	  c.incr_cand_count(index_init + ( uint64_t) t_out.t[t1]);
      }
    }
  }
   
  //T1 count;
  //int  num_freq=0;
  k=0;
  //T t_mapped[2];

  // copy to G

#ifdef DEBUG
//   nlog << "item_f" << item_f << endl;
//   G.print();
//   cout << "m1=" << m1 << endl;
#endif

  // loop over the first item
  for (int i=0; i < m1-1; i++) {
    // loop over the second item
    for (int j=i+1; j < m1; j++) {
      int count = c.get_cand_count(k++);
      int u = item_f[counters.unmap[i]];
      int v = item_f[counters.unmap[j]];
      if (u>=0 && v>=0) {
#ifdef DEBUG
// 	nlog << "! " << counters.unmap[i] << " " 
// 	     << counters.unmap[j] << " with count " << count << endl;
// 	nlog << "Adding " << u << " " 
// 	     << v << " with count " << count << endl;
#endif
	G.add(u, v, count);
      }
    }
  }

#ifdef VERBOSE
//   print_statistics("DCP", 2, m1*(m1-1)/2, num_freq, time.ReadChronos());
#endif

#ifdef DEBUG
//   nlog << "afterwards" << endl;
//   G.print();
#endif

}

#endif
