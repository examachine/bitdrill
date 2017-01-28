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
 
#ifndef __ITEMS_H 
#define __ITEMS_H 
 
// threshold on support to discriminate  
// between dense and sparse databases 
#define SUPP_THRESHOLD     0  
#define AVG_SUPP_THRESHOLD 20  
 
 
#include <iostream> 
#include <vector> 
#include <string> 
#include <stdio.h> 
 
#include "my_defs.hh" 

using namespace std; 

typedef	union {
	int			d[2];	/* 2 Doubleword (32-bit) values */
	unsigned int		ud[2];	/* 2 Unsigned Doubleword */
	short			w[4];	/* 4 Word (16-bit) values */
	unsigned short		uw[4];	/* 4 Unsigned Word */
	char			b[8];	/* 8 Byte (8-bit) values */
	unsigned char		ub[8];	/* 8 Unsigned Byte */
	float			s[2];	/* Single-precision (32-bit) value */
} __attribute__ ((aligned (8))) mmx_t;	/* On an 8-byte (64-bit) boundary */

typedef union {
        unsigned int                    f[4];  /* Single-precision (32-bit) value */
}  __attribute__ ((aligned (16))) sse_t;                                      /* On a 16 byte (128-bit) boundary */

typedef sse_t tid_list;

 
 
class dci_item { 
public: 
  dci_item(unsigned int i=0, unsigned int c=0) {id=i; count=c;} 
  unsigned int id; 
  unsigned int count; 
}; 
 
 
struct AscendingItemsSort 
{ 
  bool operator() (const dci_item& a, const dci_item& b) { 
    return (a.count < b.count); 
  } 
}; 
 
struct DescendingItemsSort 
{ 
  bool operator() (const dci_item& a, const dci_item& b) { 
    return (a.count > b.count); 
  } 
}; 
 
 
class dci_items { 
public: 
  dci_items(int min_c, unsigned int sel_sampling_th) { 
    acc = new vector<dci_item>; 
    min_count = min_c; 
    max_supp=0; 
    selective_sampling_th = sel_sampling_th;
    to_be_removed = 0;
  } 
  ~dci_items() { 
  } 
 
  inline void insert_item(unsigned int id); 
  void remap_items(void); 
  void delete_counts() {delete acc;}; 

  inline unsigned int get_supp(int id) {return (*acc)[id].count;} 
 
  unsigned int min_count; 
  unsigned int m1;                // # of remaining items after iteration 1 
  unsigned int m;                 // # of different items in database 
  unsigned int max_supp;          // maximum support
  vector<dci_item> *acc;          // item counters 
 
  // selective sampling threshold from
  // "A Sampling­based Framework For Parallel Data Mining"
  unsigned int selective_sampling_th; 
  unsigned int to_be_removed;
}; 
 
 
 
inline void dci_items::insert_item(unsigned int id) 
{ 
  if (id >= (*acc).size()) { 
    (*acc).resize(id+1); 
  } 
 

  (*acc)[id].count++; 

} 
 
 
template <class T>
class close_item { 
public: 
  T id; 
  int count; 
  tid_list* head_list; 
  tid_list* tail_list; 
  bool isSectionIncluded; 
  int sect_eq_num_bit; 
 
#ifdef INCLUDED_BYTES 
  unsigned int included; 
#endif 
 
};

template <class T, class T1>
class POST_item { 
public: 
  POST_item (T i=0, T1 inc=0) {id = i; included=inc; } 
  T id; 
  //  int count; 
  T1 included; 
};


template <class T, class T1>
class close_itemset { 
public: 

  T* items; 
  T lung; 
  T1 count; 
  tid_list* head_list; 
  tid_list* tail_list; 
 
  bool isSectionIncluded; 
  int sect_eq_num_bit; 
  unsigned int bytes_included_sect_eq; 

  unsigned int min_included;
  unsigned int old_min_included;
  int old_bits_included;
  int bits_included;

  void printTofile(FILE * out) {
    fwrite( &lung, sizeof(T), 1, out);
    fwrite( &count, sizeof(T1), 1, out);
    fwrite(  items, sizeof(T), lung, out);
  }

  void printTofile(FILE * out, T* unmap) {
    fwrite( &lung, sizeof(T), 1, out);
    fwrite( &count , sizeof(T1), 1, out);
    for (T i=0; i<lung; i++)
      fwrite( &( unmap[items[i]] ) , sizeof(T), 1, out);
  }
}; 
 
#endif 
