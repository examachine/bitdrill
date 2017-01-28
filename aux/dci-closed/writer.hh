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

#ifndef __WRITER_H
#define __WRITER_H


#include "database.hh"
#include "items.hh"
#include <iostream>
#include <stdio.h>

template <class T>
class ItemsetWriter{
public:
  /** counstructor */
  ItemsetWriter(dci_items* ii, char* file_name);
  /** destructor */
  ~ItemsetWriter();

  void Write(close_itemset<T, int> &itemset);
  inline void Write(char* str, int len) { 
  out->printSet(str, len);
  };

private:
  dci_items* counters;
  char* itemset_unmapped;
  int*  itemset_unmapped_index;
  T*    cache;
  static const int MAX_LEN = 1000;
  static const int SZ_NUM = 50;
  FSout* out;
};

template <class T>
ItemsetWriter<T>::ItemsetWriter(dci_items* ii, char* file_name){
  counters = ii;

  cache = new T[MAX_LEN];
  cache[0] = counters->m1;

  itemset_unmapped = new char[SZ_NUM * MAX_LEN]; // +1 for storing count
  itemset_unmapped_index = new int[MAX_LEN];
  itemset_unmapped_index[0] = 0;
  out = new FSout(file_name);
}

template <class T>
ItemsetWriter<T>::~ItemsetWriter(){
  delete [] itemset_unmapped;
  delete [] itemset_unmapped_index;
  delete [] cache;
  delete out;
}

template <class T>
void ItemsetWriter<T>::Write(close_itemset<T, int> &itemset){ 
//   unsigned int div;
//   int ind, num_written;

//   for (div=0; div<itemset.lung; div++)
//     if (itemset.items[div] != cache[div])
// 	    break;

//   for (;div<itemset.lung; div++){
//     cache[div] = itemset.items[div];
//     ind = itemset_unmapped_index[div];
//     memcpy(&itemset_unmapped[ind], counters->unmap_ascii[cache[div]], counters->unmap_ascii_len[cache[div]]);
//     itemset_unmapped_index[div+1] = ind + counters->unmap_ascii_len[cache[div]];
//   }

//   ind = itemset_unmapped_index[div];
//   num_written = sprintf(&itemset_unmapped[ind], "(%d)\n", (int)itemset.count);

//   cache[div] = counters->get_m1();
  
//   out->printSet(itemset_unmapped, ind+num_written);

}
#endif
