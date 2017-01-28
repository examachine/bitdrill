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

#ifndef __TRANSACTION_H
#define __TRANSACTION_H

#include <vector>
#include "items.hh"

using namespace std;

template <class T>
class Transaction
{
public:
	
  Transaction(int l) : length(l) {t = new T[l];}
  ~Transaction(){delete [] t;}
  
  T length;
  T *t;
};


template <class T>
class dci_transaction {
public:
  dci_transaction(unsigned int max_len, unsigned int iter, int m1);
  ~dci_transaction() {
    delete [] elements;
    delete [] prune_mask;
    delete [] direct_access;
  }

  // void copy_transaction_and_global_prune(Transaction& t, dci_items &ii);

  inline void prune_global(dci_items& items);

  inline void prune_local();

  inline void init_prune_local() {
    for (int i=0; i<(int) t_len; i++)
      prune_mask[i] = 0;
  }

  inline void incr_prune_local(unsigned int elem) {prune_mask[elem]++;}
  inline void incr_prune_local(unsigned int elem, int val) {
    prune_mask[elem] += val;}


  void set_direct_access() {
    for (int i=0; i < (int) t_len; i++)
      direct_access[elements[i]] = t_len - i;
    
    // cout << t_len << endl;
 }

  void reset_direct_access() {
    for (int i=0; i<(int) t_len; i++)
      direct_access[elements[i]] = 0;
  }

	
  T* elements;
  T* direct_access;
  T t_len;

private:
  unsigned int iter_count;
  unsigned int max_t_size;
  pair <int,int> prefix_index;
  unsigned int min_suffix_len;
  unsigned int suffix_len;
  unsigned int *prune_mask;
};

template <class T> 
void quick_sort(T *, int, int);

template <class T> 
void prune_and_map_and_ord_first_iter(Transaction<unsigned int>&, 
				      Transaction<T>&, 
				      dci_items&);


#endif
