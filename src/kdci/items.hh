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

#ifndef __KDCI_ITEMS_H
#define __KDCI_ITEMS_H

#include <iostream>
#include <vector>
#include <string>

#include <stdio.h>
#include <strings.h>

#include "direct_count.hh"

using namespace std;

namespace KDCI {

// threshold on support to discriminate 
// between dense and sparse databases
#define SUPP_THRESHOLD     0 
#define AVG_SUPP_THRESHOLD 20 


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
  void copy_items(std::vector<int> F1);
  void copy_F1(std::vector<int>& F1);
  int remap_items(bool write_output);
  void delete_counts() {delete acc;};
  inline unsigned int get_m() {return m;}
  inline unsigned int get_m1() {return m1;}
  inline unsigned int get_mk() {return mk;}
  inline unsigned int get_count(int id) {return (*acc)[id].count;}
  vector<dci_item>* get_counts() { return acc; }
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

  inline int get_num_itemsets() { return num_itemsets; }
  inline int  get_count(int ind) {return ((int) counters[ind]);}

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

  // following two were inline -- exa
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
	   (int) prefixes.size(), (int) suffixes.size(), (int) counters.size(), (int) ind_section.size(), sz);
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

inline void dci_items::copy_F1(std::vector<int>& F1)
{
  for (unsigned int i=0; i<F1.size(); i++) {
    unsigned int id = i;
    unsigned int count = F1[i];
    if (id >= (*acc).size()) {
      (*acc).resize(id+1);
    }
    (*acc)[id].count = count;
  }
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


template <class T, class T1>
inline void set_of_itemsets<T, T1>::add_itemset(T *v, T1 c){
   
  if (num_itemsets == 0) {
    num_itemsets++;
    prefixes.resize(k - 1);
    for (int i=0; i < k-1; i++)
      prefixes[i] = v[i];

    ind_section.push_back(0);
    suffixes.push_back(v[k-1]);
    counters.push_back(c);
  }
  else {
    // check equality between prefixes
    bool equal=true;
    int ind_last_prefix = prefixes.size()-(k-1);
    
    for (int i=0; i<k-1; i++)
      if (prefixes[ind_last_prefix+i] != v[i]) {
	equal=false;
	break;
      }
      
    if (equal)  {
      // cout << "+++same prefix " << v[0] << "\n";
      num_itemsets++;

      int last_ind = ind_section.size() - 1;
      ind_section[last_ind]++;
      suffixes.push_back(v[k-1]);
      counters.push_back(c);

    }
    else {
      // cout << "+++add section"  << v[0] << "\n";
      
      num_itemsets++;

      int old_sz = prefixes.size();
      prefixes.resize(old_sz + k - 1);
      for (int i=0; i<k-1; i++)
	prefixes[old_sz + i] = v[i];
      
      ind_section.push_back(suffixes.size());
      suffixes.push_back(v[k-1]);
      counters.push_back(c);
    }
  }
}


template <class T, class T1>
inline void set_of_itemsets<T, T1>::add_itemset(T *v, T1 c, T key){   
  add_itemset(v, c);
  keys.push_back(key);
}


template <class T, class T1>  
void set_of_itemsets<T, T1>::init_read_itemsets() {
  curr_itemset=-1; 
  curr_prefix=0;
  init_sect = 0;
  end_sect = ind_section[curr_prefix];
}


template <class T, class T1>  
inline int set_of_itemsets<T, T1>::read_next_itemset(T* v, T1& c) {

  curr_itemset++;

  if (curr_itemset > end_sect) {
    curr_prefix++;
    if (curr_prefix == ind_section.size()) 
      return(-1);

    init_sect = end_sect + 1;
    end_sect = ind_section[curr_prefix];
  } 

  if (curr_itemset == init_sect) {
    int ind_prefix = curr_prefix * (k-1);
    int j=0;
    for (int i=ind_prefix; i<ind_prefix+(k-1); i++)
      v[j++] = prefixes[i];
  } 

  v[k-1] = suffixes[curr_itemset];
  c = counters[curr_itemset];
  
  return curr_itemset;
}


template <class T, class T1>
void set_of_itemsets<T, T1>::printf_itemsets() {
  if (num_itemsets == 0) {
    cerr << "no itemsets!!\n";
    return;
  }

  curr_prefix = 0;
  init_sect = 0;
  end_sect = ind_section[curr_prefix];

  cout << "k=" << k  << endl;
  while (1) {
    cout << "Prefix:" << curr_prefix << "- Section len:" 
	 <<  end_sect-init_sect+1 << "\n   prefix: ";

    int ind_prefix = curr_prefix * (k-1);
    for (int i=ind_prefix; i<ind_prefix+(k-1); i++)
      cout << (int) prefixes[i] << " ";
    
    cout << "\n   (suffixes,counters): ";
    for (unsigned int i=init_sect; i<=end_sect; i++)
      cout << "(" << (int) suffixes[i] 
	   << "," << (int) counters[i] << ") ";
    cout << "\n\n";
    
    curr_prefix++;
    if (curr_prefix == ind_section.size()) 
      break;
    init_sect = end_sect + 1;
    end_sect = ind_section[curr_prefix];
  }
}



template <class T, class T1>
void set_of_itemsets<T, T1>::flag_included_items(dci_items& c)
{
  c.init_flag_item();

  if (num_itemsets == 0)
    return;

  unsigned int curr_prefix = 0;
  unsigned int init_sect = 0;
  unsigned int end_sect = ind_section[curr_prefix];

  while (1) {

    int ind_prefix = curr_prefix * (k-1);
    for (int i=ind_prefix; i<ind_prefix+(k-1); i++)
      c.flag_item[prefixes[i]] = true;
    
    for (unsigned int i=init_sect; i<=end_sect; i++)
      c.flag_item[suffixes[i]] = true;
    
    curr_prefix++;
    if (curr_prefix == ind_section.size()) 
      break;
    init_sect = end_sect + 1;
    end_sect = ind_section[curr_prefix];
  }
  
}


template <class T, class T1>
void set_of_itemsets<T, T1>::set_occurrences_1prefix(dci_items& c)
{
  c.init_first_item_counts();

  if (num_itemsets == 0)
    return;



  unsigned int curr_prefix = 0;
  unsigned int init_sect = 0;
  unsigned int end_sect = ind_section[curr_prefix];

  while (1) {

    int ind_first = curr_prefix * (k-1);

    c.first_item_counts[prefixes[ind_first]] += end_sect - init_sect + 1;
    
    curr_prefix++;
    if (curr_prefix == ind_section.size()) 
      break;
    init_sect = end_sect + 1;
    end_sect = ind_section[curr_prefix];
  }
}


} // namespace

#endif
