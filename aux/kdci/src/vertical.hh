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

#ifndef __VERTICAL_H
#define __VERTICAL_H

#include "transaction.hh"
#include "tid_list_macros.hh"


const int COMPACT_DATASET  = 1;
const int DIFFUSE_DATASET = 0;



static inline unsigned char mask_byte(int k) {
  return (unsigned char) 1 << k;
}

//const static unsigned char mask_byte[8] = { 0x1, 0x2, 0x4, 0x8, 
// 0x10, 0x20, 0x40, 0x80 };





class DCI_statistics {
public:
  void reset_stats() {
    num_and = 0;  
    nn_and = 0;
    n_and_index = 0;
    n_or_index = 0;
  }

  void get_stats(int VD_active_rows, int n_cand, int iter, int tid_list_size,
		 int &sz_list_proj,
		 int& COMPLEXITY_INTERSECTION, int& COMPLEXITY_PRUNING) {
    if (nn_and+n_and_index) {
      num_and = num_and / (double) (nn_and+n_and_index);
      COMPLEXITY_INTERSECTION = (nn_and + n_and_index) * (int) num_and;
      COMPLEXITY_PRUNING = VD_active_rows * tid_list_size;
      sz_list_proj = (int) num_and;
//        cout << "size tid_list reduced=" << (int) num_and
//             << " (" << tid_list_size << ")\n";
    }
  }
  
  void get_stats(int VD_active_rows, int n_cand, int iter, int tid_list_size, 
		 int& COMPLEXITY_INTERSECTION, int& COMPLEXITY_PRUNING) {
    if (nn_and+n_and_index) {
      num_and = num_and / (double) (nn_and+n_and_index);
      
      COMPLEXITY_INTERSECTION = (nn_and + n_and_index) * (int) num_and;
      COMPLEXITY_PRUNING = VD_active_rows * tid_list_size;
      
      //cout << "size tid_list reduced=" << (int) num_and 
      //	   << " (" << tid_list_size << ")\n";
    }  
  }
  
  double num_and;
  int n_and_reduced;
  int nn_and;
  int n_and_index;
  int n_or_index;
private:


};



template <class T>
class DCI_vertical_dataset {
public:
  DCI_vertical_dataset() {
    VD = NULL;
    prune_mask = NULL;
    tid_list_size = 0;
    //flag_item=NULL;
    //first_items=NULL;  
    CACHE_tidlist=NULL;
    and_index = NULL;
  }

  ~DCI_vertical_dataset() {
    if (VD != NULL)
      delete [] VD;
    if (prune_mask != NULL)
      delete [] prune_mask;    
    //if (flag_item != NULL)
    //  delete [] flag_item;
    //if (first_items != NULL)
    //  delete [] first_items;
    if (and_index != NULL)
      delete [] and_index;
    if (CACHE_tidlist != NULL)
      delete [] CACHE_tidlist;
  }
  

  void init_cache(int iter) {
    if (CACHE_tidlist != NULL)
      delete [] CACHE_tidlist;
    CACHE_tidlist = new unsigned int[tid_list_size * iter];

    num_bit_set_init = 0; 
  }
  

  // checks if DCI can start at the following iter
  bool VD_can_be_allocated (unsigned int m, unsigned int n_trans);
 
  inline  bool VD_is_allocated () {
    if (VD != NULL)  return true;
    else return false;
  }

  inline int get_tid_list_size() {
    return tid_list_size;
  }

  void init_VD () {
    VD = new unsigned int[VD_rows*tid_list_size];
    //   cout << "allocated VD of size " << VD_rows*tid_list_size 
    //	 << " (" << VD_rows << " X " << tid_list_size << ")\n";
    bzero(VD,VD_rows*tid_list_size*sizeof(unsigned int));
    prune_mask = new unsigned int[tid_list_size];
    and_index = new int[tid_list_size+2];
  }
  
  inline bool candidate_is_frequent_diffuse(T *cand, int prefix_len, int iter, 
					   int min_count, int& count,
					   DCI_statistics& stats,
					   bool pruning);
  inline bool candidate_is_frequent_compact(T *cand, int prefix_len, int iter, 
					  int min_count, int& count,
					  DCI_statistics& stats);


  inline void reset_prune_mask() {
     bzero(prune_mask,tid_list_size*sizeof(unsigned int));
     //bzero(flag_item,VD_rows*sizeof(unsigned int));
     //bzero(first_items,VD_rows*sizeof(unsigned int));
  }

  inline void write_t_in_VD (unsigned int n_tr, dci_transaction<T>& t);
  inline void write_t_in_VD (unsigned int n_tr, Transaction<T>& t);

  inline bool is_compact() {
    if (dataset_kind == COMPACT_DATASET)
      return true;
    else 
      return false;
  }

  void prune_VD (int new_tid_list_size, dci_items& c);
  int check_pruning ();
  void order_bits_diffuse(dci_items& c);
  int chk_compact_vertical (int n_frequent_items);

  inline bool is_included(T elem) {
    if (elem >=  start_sect_eq) 
      return true;
    else
      return false;	
  }
 
private:
  unsigned int *VD;
  unsigned int VD_size;
  unsigned int tid_list_size;
  unsigned int VD_rows;
  unsigned int *prune_mask;
  //unsigned int *flag_item;
  //unsigned int *first_items;
  int dataset_kind;
  // the CACHE !
  unsigned int *CACHE_tidlist;


  // stores boundaries of the EQ section: used only for dense
  unsigned int start_sect_eq;
  unsigned int end_sect_eq;
  unsigned int sz_sect_eq;
  // store the count for the EQ section: used only for dense
  int n_bit_set_eq;
  // store the current count for the candidate: used only for dense
  int num_bit_set_init; 
  // maintains run of 0's indexes: used only for sparse
  int *and_index; 

  void reorder_bits_compact(int n_frequent_items, unsigned int *equal);
};



#endif
