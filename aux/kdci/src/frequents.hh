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

#ifndef __FREQUENTS_H
#define __FREQUENTS_H

#include "direct_count.hh"
#include "items.hh"
#include "database.hh"


const int NEW_PREFIX = 1;
const int NEW_SUFFIX = 2;
const int END_GEN_CAND = 0;


template <class T, class T1>
class set_of_frequents : public set_of_itemsets<T,T1> {
public:
  set_of_frequents(int iter) : set_of_itemsets<T, T1>(iter) {}
  int get_num_frequents() {return num_itemsets;}

  void dump_itemsets(dci_items& counters, FSout& o);
  //  void dump_itemsets_bin(dci_items& cc, FSout& o);

  void printf_freq();

  int init_gen_cand();
  void get_prefix(T *v);
  void get_suffix(T *v);
  void get_suffix(T *v, T *keys, T1 *counts);
  int next_cand();

  int find_subsets(T *v);
  int find_one_subset(T *v, T& key, int& count);


  /** Direct acces structure for DCP in the 3th iteration */
  vector<int> DA;
	/** size of di direct access structure (unnecessary ???) */
  int DA_m;
	/** Initializes the direct access structure used for
	  * a fast DCP in the 3rd iteration.
		* for each itemset(i0, i1) its position is stored.
		*/
	void initDA(int m);



private:

  unsigned int i1, i2;

  inline int my_bsearch(T *v, int del, int start);
}; 

#endif
