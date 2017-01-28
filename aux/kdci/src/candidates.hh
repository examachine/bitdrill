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

#ifndef __CANDIDATES_H
#define __CANDIDATES_H

#include <string>

#include "direct_count.hh"
#include "items.hh"
#include "transaction.hh"

#include "frequents.hh"

template <class T> class cand_2_iter {

public:
  cand_2_iter(int m1)  {
    sz = m1*(m1-1)/2;
    buf = new T[sz];
    bzero(buf, sz * sizeof(T));
  }
  ~cand_2_iter() {delete [] buf;}
  inline void incr_cand_count(int i) {buf[i]++;}
  inline int get_cand_count(int i) {return ((int) buf[i]);}
  inline int get_sz() {return sz;}

private:
  T *buf;
  int sz;
  
};

template <class T, class T1>
class DCP_candidates : public set_of_itemsets<T,T1>  {
public:
  DCP_candidates(int iter) : set_of_itemsets<T, T1>(iter) {}

  inline int  get_num_candidates() {return num_itemsets;}
  void        init_prefix_table(int m);
  inline void subset_and_count_and_prune_local(dci_transaction<T>& t, set_of_frequents<T,T1>& freq);
  inline int  get_count(int ind) {return ((int) counters[ind]);}

  void printf_candidates() {printf_itemsets();}

  /** Direct acces structure for DCP in the 3th iteration */
  vector<int> DA;
	/** size of di direct access structure (unnecessary ???) */
  int DA_m;
	/** Initializes the direct access structure used for
	  * a fast DCP in the 3rd iteration.
		* for each prefix(c0, c1) the position of the first candidate
		* (c0, c1, _ ) is stored.
		*/
  void initDA(int m);

	/** prints on console all the information stored in the class */
  void pprint();

private:
  int         m1;
  vector<int> buf;
  int         sz;

  inline void scan_candidates3(dci_transaction<T> &t, int t0, int t1,
			       set_of_frequents<T,T1>& freq);
  inline void scan_candidates (dci_transaction<T> &t, int t0, int t1,
			       int start, int end);
}; 


template <class T, class T1>
void gen_candidates(set_of_frequents<T,T1>& set_freq, 
		    DCP_candidates<T,T1>& cand_set,
		    dci_items& counters, 
		    int iter, cand_2_iter<T1>& c);

#endif
