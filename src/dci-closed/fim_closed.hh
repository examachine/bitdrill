// interface for dci-closed -- exa

#include "transaction-db/Transaction_Set.hxx"

#include "database.hh"
//#include "items.hh"
//#include "vertical.hh"
//#include "candidates.hh"

namespace DCIClosed {

  void mine_closed(::Transaction_Set& ts, int mincount, string out = "",
		   bool clear_ts=false);


  int first_scan(Data *d, dci_items& counters, unsigned int& max_trans_len);

  template <class T, class T1>
  void following_scans(int max_trans_len, dci_items& counters);

  //template <class T, class T1>
  //set_of_frequents<T,T1> *second_scan(int max_trans_len, dci_items& counters, 
  // 				    cand_2_iter<T1>& c, 
  //	  			    DCI_vertical_dataset<T>& DCI_dataset);

}
