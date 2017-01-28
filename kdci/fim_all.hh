#ifndef __KDCI_FIM_ALL_HH
#define __KDCI_FIM_ALL_HH

#include <iostream>
#include <vector>
#include <cmath>
#include <cstdio>
using namespace std;

#include "transaction-db/Transaction_Set.hxx"

//#include "items.hh"
//#include "transaction.hh"
//#include "database.hh"
//#include "candidates.hh"
//#include "frequents.hh"
//#include "utils.hh"
//#include "direct_count.hh"
//#include "vertical.hh"

namespace KDCI {

  class dci_items;
  class Data;
  template<typename T> class DCI_vertical_dataset;
  template<typename T, typename T1> class set_of_frequents;
  template<typename T1> class cand_2_iter;

  extern bool write_output;
  extern const char* TEMPORARY_DATASET;

  dci_items* kdci_all(::Transaction_Set& ts, int min_count, string out,
		      int level=0, bool clear_ts=false);

  int first_scan(KDCI::Data *d, dci_items& counters, unsigned int& max_trans_len,
		 bool del_counts = false);

  template <class T, class T1>
  void following_scans(int max_trans_len, dci_items& counters, int max_level);

  template <class T, class T1>
  set_of_frequents<T,T1> *second_scan(int max_trans_len, dci_items& counters, 
				      cand_2_iter<T1>& c, 
				      DCI_vertical_dataset<T>& DCI_dataset);

}

#endif
