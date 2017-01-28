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

#include "direct_count.hh"

using namespace std;

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


class dci_items {
 public:
  dci_items(int min_c) {
    prune_global_mask = NULL; acc = new vector<dci_item>;
    min_count = min_c; max_supp=0;
    flag_item = NULL;
    first_item_counts = NULL;
    unmap_ascii= NULL;
    unmap_ascii_len = NULL;
  }
  ~dci_items() {
    if (prune_global_mask != NULL)
      delete [] prune_global_mask; 
    delete [] map; 
    delete [] unmap;
    if (flag_item != NULL)
      delete [] flag_item;
    if (first_item_counts != NULL)
      delete [] first_item_counts;

    if (unmap_ascii != NULL) {
      delete [] unmap_ascii;      
      delete [] unmap_ascii_len;
    }
  }

  int get_active_items() {
    int active=0;
    for (unsigned int i=0; i<m1; i++)
      if (flag_item[i])
	active++;
    return active;
  }


  void init_flag_item() {
    if (flag_item == NULL)
      flag_item = new bool[m1];
    bzero(flag_item, m1 * sizeof(bool));
  }
  void init_first_item_counts() {
    if (first_item_counts == NULL) 
      first_item_counts = new int[m1];
    bzero(first_item_counts, m1 * sizeof(int));
  }


  inline void insert_item(unsigned int id);
  int remap_items(bool write_output);
  void delete_counts() {delete acc;};
  inline unsigned int get_m() {return m;}
  inline unsigned int get_m1() {return m1;}
  inline unsigned int get_mk() {return mk;}
  inline unsigned int get_count(int id) {return (*acc)[id].count;}
  bool update_map();
  bool update_unmap(bool write_output);
  inline unsigned int get_num_of_trans() {return nr_t;}
  inline void set_num_of_trans(unsigned int n) {nr_t = n;}
  inline unsigned int get_max_supp() {return max_supp;}
  inline bool use_key_patterns() {
    return (avg_supp_at_threshold > AVG_SUPP_THRESHOLD);
  }
  void init_global_pruning();
  inline void incr_global_pruning(unsigned int id);
  void end_global_pruning(unsigned int k);
  inline bool is_not_global_pruned(int id);

  int *map;
  int *unmap;

  char (*unmap_ascii)[8];
  unsigned char *unmap_ascii_len;

  unsigned int min_count;

  bool *flag_item;                // active items
  int *first_item_counts;         // n. of occurrence of the same 1-prefix 
                                  // in frequent itemsets

 private:
  unsigned int *prune_global_mask;
  unsigned int m;                 // # of different items in database
  unsigned int m1;                // # of remaining items after iteration 1
  unsigned int mk;                // # of remaining items after iteration k
  unsigned int nr_t;              // # of transactions in database
  unsigned int max_supp;
  double avg_supp_at_threshold;
  vector<dci_item> *acc;          // item counters

};

// --------------------------------------------
// CLASS set_of_itemsets 
// it is used to store a collection of itemsets, 
// we use it both for candidates and for frequent
// itemsets. it adopts a compressed, prefix based 
// representation of the itemsets in the collection
// and provide methods for efficient itemset enumeration
// also keep information on the key-pattern property of 
// an itemset. 
// --------------------------------------------
template <class T, class T1>
class set_of_itemsets {
public:

  set_of_itemsets(int iter) {
    k=iter; 
    num_itemsets=0;
  }
  
  void reset(int iter) {
    //    previous_size = prefixes.size();
    k=iter; 
    num_itemsets=0;
    prefixes.resize(0);
    ind_section.resize(0);
    suffixes.resize(0);
    counters.resize(0);
    keys.resize(0);
    // prefixes.reserve((int) (previous_size * 1.2));
  }

  inline void add_itemset(T *v, T1 c);
  inline void add_itemset(T *v, T1 c, T key);
  void printf_itemsets();
  void flag_included_items(dci_items& c);
  void set_occurrences_1prefix(dci_items& c);

  void init_keys() {
    keys.resize(num_itemsets);
    for (int i=0; i<num_itemsets; i++)
      keys[i] = (T) -1;
  }
  void init_read_itemsets();
  int read_next_itemset(T* v, T1& c);
  
  
  vector<T>   prefixes;    // all different prefixes in the collection of isets
  vector<int> ind_section; // index of the starting point in the suffixes array
  vector<T>   suffixes;    // different suffixes that share the same prefix
  vector<T1>  counters;    // counters for itemsets in the collection
  vector<T>   keys;        // flag for the key-pattern property of an isets


  void print_size()
  {
    int sz;
    sz = (prefixes.size() + suffixes.size()) * sizeof(T) 
      + counters.size() * sizeof(T1)  + ind_section.size() * sizeof(int);
    printf("prefixes.size: %d, suffixes.size %d, counters.size %d, ind_section.size %d. Total %d\n",
	   prefixes.size(), suffixes.size(), counters.size(), ind_section.size(), sz);
  }



protected:
  int k;
  unsigned int curr_itemset;
  unsigned int curr_prefix;
  unsigned int init_sect, end_sect;
  int num_itemsets;
  //  int previous_size;
}; 


inline void dci_items::insert_item(unsigned int id)
{
  if (id >= (*acc).size()) {
	(*acc).resize(id+1);
  }

  (*acc)[id].count++;
}



inline bool dci_items::is_not_global_pruned(int id)
{
    return (unmap[id] != -1);
}


inline void dci_items::incr_global_pruning(unsigned int id)
{
  //  if (id < 0 || id >= m1)
  //  {cerr << "Errore: " << id << endl;}
  prune_global_mask[id]++;
}


#endif
