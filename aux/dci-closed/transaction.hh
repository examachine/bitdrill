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

template <class ITEM>
class Transaction
{
public:
	
  Transaction(int l) : length(l) {t = new ITEM[l];}
  ~Transaction(){delete [] t;}
  
  ITEM length;
  ITEM* t;
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
				      int* map);


/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

#include <stdio.h>
#include <string.h>

template <class T>
dci_transaction<T>::dci_transaction(unsigned int max_len, unsigned int iter,
				    int m1) {
  max_t_size = max_len;
  t_len = 0;
  elements = new T[max_t_size]; 
  iter_count = iter;
  prune_mask = new unsigned int[max_t_size]; 
  min_suffix_len = iter_count - 2;
  direct_access = new T[m1];
  //bzero(direct_access, m1*sizeof(T));
  memset(direct_access, 0x0, m1*sizeof(T));
}

// template <class T> 
// inline void dci_transaction<T>::prune_local()
// {
//   int j = 0;
//   for (int i = 0; i < (int) t_len; i++)
//     if (prune_mask[i] >= iter_count)
//       elements[j++]=elements[i];
//   if (j > (int) iter_count)
//     t_len = (T) j;
//   else
//     t_len = (T) 0;
// }

// template <class T> 
// inline void dci_transaction<T>::prune_global(dci_items& items)
// {
//   unsigned int j = 0;
//   for (int i = 0; i < (int) t_len; i++)
//     if (items.is_not_global_pruned(elements[i]))
//       elements[j++]=elements[i];
//   if (j >= iter_count)
//     t_len = (T) j;
//   else
//     t_len = (T) 0;
// }


template <class T> 
void quick_sort(T* v, int first, int last)
{
	
  int middle;
  if (first < last) {	  
    T x;
    int i, j;
  
    x = v[first];
    i = first - 1;
    j = last + 1;
  
    while (1) {
      do {
	j--;
      } while (v[j] > x);
      do {
	i++;
      } while (v[i] < x);
      if (i < j) {
	T tmp;
	tmp = v[i];
	v[i] = v[j];
	v[j] = tmp;
      } else {
	middle=j;
	break;
      }
    }
    
    quick_sort<T> (v, first, middle);
    quick_sort<T> (v, middle+1, last);
  }
}
	


template <class T> 
void prune_and_map_and_ord_first_iter(Transaction<unsigned int>& t_in, 
				      Transaction<T>& t_out, 
				      int* map)
{
  T j=0;
  for (unsigned int i=0; i<t_in.length; i++) 
    if (map[t_in.t[i]] != -1)
      t_out.t[j++] = (T) map[t_in.t[i]];
  t_out.length = (T) j;
  if (t_out.length > 1)
    quick_sort<T> (&t_out.t[0], 0, t_out.length - 1);
}





#endif
