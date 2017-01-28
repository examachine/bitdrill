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

#ifndef __KDCI_VERTICAL_H
#define __KDCI_VERTICAL_H

#include <stdint.h>

#include "transaction.hh"
#include "tid_list_macros.hh"
#include "memory.hh"

namespace KDCI {

const int COMPACT_DATASET  = 1;
const int DIFFUSE_DATASET = 0;

extern int candidate_check_count;

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
    // req: call after checking with VD_can_be_allocated which
    // sets some variables

    if (VD_rows==0) {
      cerr << "number of rows 0" << endl;
    }
    
    uint64_t num_entries = VD_rows*tid_list_size;

    VD = new unsigned int[num_entries];
#ifdef DEBUG
    cerr << "allocated VD of size " << num_entries  
	 << " (" << VD_rows << " X " << tid_list_size << ")\n";
    cerr << "VD_size = " << VD_size << endl;
#endif
    bzero(VD,num_entries*sizeof(unsigned int));
    prune_mask = new unsigned int[tid_list_size];
    and_index = new int[tid_list_size+2];
  }
  
  bool candidate_is_frequent_diffuse(T *cand, int prefix_len, int iter, 
					   int min_count, int& count,
					   DCI_statistics& stats,
					   bool pruning);
  bool candidate_is_frequent_compact(T *cand, int prefix_len, int iter, 
				     int min_count, int& count,
				     DCI_statistics& stats);


  inline void reset_prune_mask() {
     bzero(prune_mask,tid_list_size*sizeof(unsigned int));
     //bzero(flag_item,VD_rows*sizeof(unsigned int));
     //bzero(first_items,VD_rows*sizeof(unsigned int));
  }

  // the two following were inline
  void write_t_in_VD (unsigned int n_tr, dci_transaction<T>& t);
  void write_t_in_VD (unsigned int n_tr, KDCI::Transaction<T>& t);

  inline bool is_compact() {
    if (dataset_kind == COMPACT_DATASET)
      return true;
    else 
      return false;
  }

  void prune_VD (int new_tid_list_size, dci_items& c);
  inline int check_pruning ();
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
  uint64_t VD_size;
  uint64_t tid_list_size;
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


template <class T>
inline bool DCI_vertical_dataset<T>::
candidate_is_frequent_diffuse(T *cand, 
			     int prefix_len,
			     int iter, 
			     int min_count, 
			     int& count, 
			     DCI_statistics& stats,
			     bool pruning)
{
#ifdef PROFILE
  candidate_check_count++;
#endif

  unsigned int *pp, *pp1;
  unsigned int *curr_cache;

  if ((prefix_len == 0) || (prefix_len == 1)) { 
    // cache content cannot be reused, perform 
    // first intersection and store it in cache
    // remember runs of 0's in and_index

    curr_cache = CACHE_tidlist + tid_list_size;
    pp = VD + (cand[0] * tid_list_size);
    pp1 = VD + (cand[1] * tid_list_size);

    and_tid_list_set_index(curr_cache, pp, pp1, and_index, tid_list_size);

    stats.n_and_reduced = and_index[1];

    if (pruning) {
      stats.nn_and++;
      stats.num_and += tid_list_size;
    }

    prefix_len = 2;
  }




  curr_cache = CACHE_tidlist + (prefix_len * tid_list_size);
  
  if (stats.n_and_reduced < 0.9 * tid_list_size) {
    for (int k = prefix_len; k < iter - 1; k++) {
      // performs intersections by exploiting 
      // runs of 0's stored in and_index

      pp = VD + (cand[k] * tid_list_size);
      and_tid_list_use_index(curr_cache, curr_cache - tid_list_size, pp, 
			     and_index, tid_list_size);
      curr_cache += tid_list_size;
    }
    // performs last intersection  by exploiting 
    // runs of 0's stored in and_index.
    // Counts 1's while intersecting
    pp = VD + (cand[iter - 1] * tid_list_size);
    count = and_tid_list_count_1_use_index(curr_cache, 
					   curr_cache - tid_list_size,  
					   pp, and_index, tid_list_size);
    
    if (pruning) {
      stats.n_and_index += (iter-prefix_len);
      stats.num_and += (iter-prefix_len)*stats.n_and_reduced;
    }
  } else {
    for (int k = prefix_len; k < iter - 1; k++) {
      // performs intersections by exploiting 
      // runs of 0's stored in and_index
      
      pp = VD + (cand[k] * tid_list_size);
      and_tid_list(curr_cache, curr_cache - tid_list_size, pp, tid_list_size);
      curr_cache += tid_list_size;
    }
    // performs last intersection  by exploiting 
    // runs of 0's stored in and_index.
    // Counts 1's while intersecting
    pp = VD + (cand[iter - 1] * tid_list_size);
    count = and_tid_list_count_1(curr_cache, curr_cache - tid_list_size,
				 pp, tid_list_size);
    if (pruning) {
      stats.nn_and += (iter-prefix_len);
      stats.num_and += (iter-prefix_len)*tid_list_size;
    }
  } 

  


  
  if (count >= min_count) {
    // the candidate is frequent !
    // set pruning and reordering masks
    //for (int i = 0; i < iter; i++)
    //  flag_item[cand[i]] = 1;
    //first_items[cand[0]] += 1;
    if (pruning) {
      if (stats.n_and_reduced < 0.9 * tid_list_size)
	or_tid_list_use_index(prune_mask, prune_mask,  
			      CACHE_tidlist + ((iter - 1) * tid_list_size), 
			      and_index, tid_list_size);
      else
	or_tid_list(prune_mask, prune_mask,  
		    CACHE_tidlist + ((iter - 1) * tid_list_size), 
		    tid_list_size);
	
      stats.n_or_index++;

    }
    return true;
  }
  else 
    return false;
}











template <class T> inline bool 
DCI_vertical_dataset<T>::candidate_is_frequent_compact(T *cand, 
						     int prefix_len,
						     int iter, 
						     int min_count, 
						     int& count, 
						     DCI_statistics& stats)
{
  unsigned int *pp, *pp1;
  bool use_section = false;
  
  if (prefix_len == 0) {
    if (is_included(cand[0])) { 
      // all items in the EQ sect !
      num_bit_set_init = n_bit_set_eq;
      use_section = true;
    }
    prefix_len++;
  } 
  else {
    if (is_included(cand[prefix_len - 1]))
      // almost an item in the EQ sect !
      use_section = true;
  }

  unsigned int *curr_cache;

  if (prefix_len == 1) {

    curr_cache = CACHE_tidlist + tid_list_size;
    
    if (!use_section) {
      pp = VD + (cand[0] * tid_list_size);
      pp1 = VD + (cand[1] * tid_list_size);


      if (!is_included(cand[1])) { 
	// both are NOT included in the EQ section: normal intersection
	and_tid_list(curr_cache, pp, pp1, tid_list_size);
      } else { 
	// the second is included in the EQ section
	// intersect and count up to sz_sect_eq 
	num_bit_set_init = and_tid_list_count_1(curr_cache, pp, pp1, 
						sz_sect_eq); 

	  
	use_section = true;
	and_tid_list_use_section(curr_cache, pp, pp1, 
				 sz_sect_eq, tid_list_size);
      }
    } else {
      pp = VD + (cand[0] * tid_list_size);
      pp1 = VD + (cand[1] * tid_list_size);
      
      and_tid_list_use_section(curr_cache, pp,pp1, sz_sect_eq, tid_list_size);
    }

    prefix_len++;

  }



  
  int k = prefix_len;
  curr_cache = CACHE_tidlist + (k * tid_list_size);

  if (!use_section) {
    
    for (; k < iter - 1; k++) {
      pp = VD + (cand[k] * tid_list_size);
    
      if (!is_included(cand[k])) { 
	// cand[k] is not included in the EQ section
	and_tid_list(curr_cache, curr_cache - tid_list_size, 
		     pp, tid_list_size);
	curr_cache += tid_list_size; 
	// increment cache pointer

      } else { 
	// cand[k] is the fisrt included in the EQ section
	num_bit_set_init = and_tid_list_count_1(curr_cache, 
						curr_cache - tid_list_size, 
						pp, sz_sect_eq); 
	// partial counting !!!!!!!
	use_section = true;
	break;
      }
    }
  }

  if (use_section) {
      for (; k < iter - 1; k++) {
	pp = VD + (cand[k] * tid_list_size);
	and_tid_list_use_section(curr_cache, curr_cache - tid_list_size, 
				 pp, sz_sect_eq, tid_list_size);
	curr_cache += tid_list_size; // increment cache pointer
      }
  }

  // last intersection
  // Counts 1's while intersecting
  pp = VD + (cand[iter - 1] * tid_list_size);
  if (use_section) {
    count = and_tid_list_count_1_use_section(curr_cache, 
					     curr_cache - tid_list_size,
					     pp, sz_sect_eq, tid_list_size);
    count += num_bit_set_init; 
  } else {
    count = and_tid_list_count_1(curr_cache, curr_cache - tid_list_size,
				 pp, tid_list_size);
  }
  
  
  if (count >= min_count) 
    // cand is frequent !
    return true;
  else 
    return false;
}


template <class T>
int DCI_vertical_dataset<T>::chk_compact_vertical(int n_frequent_items)
  // heuristic used to determine if in the vertical dataset
  // there is a region where tid lists resamble one with the other
  // 
  // the area of the region of similarity must be above a given 
  // threshold for the dataset to be dense. 

{
  int start, end, cc=0, perc=0;
  unsigned int *mask, *last_row, *result;

  float perc_eq_limit[] = { 95.0, 90.0, 85.0, 80.0, 75.0, 
			    70.0, 65.0, 60.0};
  int max_perc_ind = sizeof(perc_eq_limit) / sizeof(float);
  int curr_perc = 0;

  int curr_perc_OK = 0;
  
  int start_OK, perc_OK;

  mask = new unsigned int [tid_list_size];
  result = new unsigned int [tid_list_size];

  for (unsigned int i=0; i<tid_list_size; i++)
    mask[i] = 0xffffffff;
  
  end = n_frequent_items;
  
  last_row = &VD[(end-1) * tid_list_size];

  start_OK = end-1;
  perc_OK = 0;

  start = end-2;
 

  while(start >= 0) {
    // ----------------------------------------------------
    // find the percentage of identical elements in 
    // the last start-end tidlists (most frequent items).
    float perc_tmp;
    int cc_tmp;
    bzero(result, tid_list_size*sizeof(unsigned int)); 
	
    for (unsigned int i=0; i<tid_list_size; i++) {
      result[i] = ~(VD[start * tid_list_size + i] ^ last_row[i]);
      result[i] = (result[i] & mask[i]);
    }
    cc_tmp = count_1_bits(result, tid_list_size);
    perc_tmp =  100.0 * (float) cc_tmp / 
      (float) (tid_list_size*sizeof(int)*8);
    // ----------------------------------------------------
    
    bool found = false;

    // ----------------------------------------------------
    // now test all percentage range limits 
    // given in perc_eq_limit
    // ----------------------------------------------------
    while (curr_perc < max_perc_ind) {

      if (perc_tmp >= perc_eq_limit[curr_perc]) {
	// we found the percentage range
	// we store the partial result in mask
	// and go on with another item
	memcpy((void *) mask, (void *) result, tid_list_size*sizeof(int));
	cc=cc_tmp;

	perc_OK = (int) perc_tmp;
	start_OK = start;

	found = true;
	curr_perc_OK = curr_perc;

	start--;
	break;
      } else if ((end-start) < n_frequent_items/7)
	// percentage is lower
	// we consider another range if the number of 
	// tidlists considered so far is not too big
	curr_perc++;
      else
	// percentage is low and there are no more enough 
	// items to consider. 
	break;
    }
    
    // we know which is the percentage range of superposition
    // among tidlists. if there are other items to consider 
    // we go on, otherwise we break the loop
    if (!found)
      break;
    

  }
  
  start = start_OK;
  perc = perc_OK;

  int limit_num_freq_eq;
  if (perc > 80)
    limit_num_freq_eq = n_frequent_items/7;
  else if (perc > 75)
    limit_num_freq_eq = n_frequent_items/6;
  else if (perc > 70)
    limit_num_freq_eq = n_frequent_items/5;
  else if (perc > 65)
    limit_num_freq_eq = n_frequent_items/4;
   else if (perc > 60)
    limit_num_freq_eq = n_frequent_items/3;
   else 
    limit_num_freq_eq = n_frequent_items/2;
 
  if (perc == 0 || (end-start) < limit_num_freq_eq) {
    delete [] mask;
    delete [] result;
    dataset_kind = DIFFUSE_DATASET;
    return dataset_kind;
  }
  
  start_sect_eq=start;
  end_sect_eq=end;
  sz_sect_eq = cc / (sizeof(int)*8);

  reorder_bits_compact(n_frequent_items, mask);
  
  delete [] mask;
  delete [] result;
  dataset_kind = COMPACT_DATASET;
  return dataset_kind;

}




template <class T>
int DCI_vertical_dataset<T>::check_pruning () 
{
  
  int num_bit_set;
  
  num_bit_set = count_1_bits(prune_mask, tid_list_size);
  int acc_bit = tid_list_size * sizeof(unsigned int) * 8 - num_bit_set;

  int new_sz_bit = (sizeof(unsigned int) * 8 * tid_list_size) - acc_bit;
  int new_tid_list_size = new_sz_bit / (sizeof(unsigned int) * 8);
  if ((new_sz_bit % (sizeof(unsigned int) * 8)) != 0)
    new_tid_list_size++;

  return new_tid_list_size;

}


template <class T>
void DCI_vertical_dataset<T>::prune_VD (int new_tid_list_size, dci_items& c) 
{
  unsigned char *m = (unsigned char *) prune_mask;

  for (unsigned int i = 0; i < VD_rows; i++) {
    // c.flag_item[] indicates the rows corresponding to active items
    if (c.flag_item[i] == 0)
      continue;

    unsigned char *o = (unsigned char *) &VD[i * tid_list_size];
    unsigned char *n = (unsigned char *) &VD[i * new_tid_list_size];

    unsigned int i_b = 0;
    unsigned int i_B = 0;


    for (unsigned int j = 0; j < tid_list_size * sizeof(unsigned int); j++)
      for (int k = 0; k < 8; k++) {
        if ((m[j] & mask_byte(k)) != 0) {
          if ((o[j] & mask_byte(k)) != 0)
            n[i_B] = n[i_B] | mask_byte(i_b);
          else
            n[i_B] = n[i_B] & (~mask_byte(i_b));
	  
          i_b = (i_b + 1) % 8;
          if (i_b == 0)
            i_B++;
        }
      }

    if (i_b != 0) {
      while (i_b < 8) {
        n[i_B] = n[i_B] & (~mask_byte(i_b));
        i_b++;
      }

      i_B++;
    }

    while (i_B < new_tid_list_size * sizeof(unsigned int))
      n[i_B++] = 0;
  }

  tid_list_size = new_tid_list_size;

}



template <class T>
void DCI_vertical_dataset<T>::order_bits_diffuse(dci_items& c)
{
  // cout << "ORDERING ...\n";

  int *index_tid = new int[tid_list_size * 8 * sizeof(int)];
  unsigned int *new_list = new unsigned int[tid_list_size];

  T *ordered_freq_items = new T[VD_rows];
  int num_ordered_freq_items=0;

  unsigned int *and_firsts = new unsigned int[tid_list_size];
  bzero(and_firsts, tid_list_size*sizeof(unsigned int));

  for (unsigned int i = 0; i < tid_list_size * sizeof(int) * 8; i++)
    index_tid[i] = -1;

  while (1) {
    int max=0;
    for (unsigned int i = 0; i < VD_rows; i++) {
      if (c.first_item_counts[i] > c.first_item_counts[max])
	max=i;
    }
    
    if (c.first_item_counts[max] != 0) {
       ordered_freq_items[num_ordered_freq_items++] = max;
       c.first_item_counts[max] = 0;

       and_tid_list(and_firsts, and_firsts, (&VD[max * tid_list_size]), tid_list_size);
    }    
    else
        break;
  } 
  

  unsigned int ind=0;

  
  //printf("num_ordered_freq_items=%d\n", num_ordered_freq_items);

  unsigned char *o = (unsigned char *) and_firsts;
  for (unsigned int j = 0; j < tid_list_size * sizeof(int); j++)
    for (int k = 0; k < 8; k++) {
      int tid = j * 8 + k;
      if ((o[j] & mask_byte(k)) != 0)
        if (index_tid[tid] == -1) {
          //if (ind >= tid_list_size * sizeof(unsigned int) * 8)
          //  cout << "error order_bits\n";
          index_tid[tid] = ind++;
        }
    }




  for (int ii = 0; ii < num_ordered_freq_items; ii++) {
    int i = ordered_freq_items[ii];

    unsigned char *o = (unsigned char *) &VD[i * tid_list_size];

    for (unsigned int j = 0; j < tid_list_size * sizeof(unsigned int); j++)
      for (int k = 0; k < 8; k++) {
        int tid = j * 8 + k;
        if ((o[j] & mask_byte(k)) != 0) {
          if (index_tid[tid] == -1) {
            //if (ind >= tid_list_size * sizeof(int) * 8)
            //  cout << "error2 order_bits\n";
	    index_tid[tid] = ind++;
          }
	}
      }
  }

  while (ind < tid_list_size * sizeof(unsigned int) * 8) {
    index_tid[ind] = ind;
    ind++;
  }


  for (unsigned i = 0; i < VD_rows; i++) {
    if (c.flag_item[i] == 0) continue;

    unsigned char *o = (unsigned char *) &VD[i * tid_list_size];
    unsigned char *n = (unsigned char *) new_list;
    bzero(n, tid_list_size * sizeof(unsigned int));

    for (unsigned int j = 0; j < tid_list_size * sizeof(int); j++)
      for (int k = 0; k < 8; k++) {
        int tid = j * 8 + k;
        if ((o[j] & mask_byte(k)) != 0) {
          /*
           * set bit index_tid[tid] 
           */
	  int i_b_glob;
	  int i_b;
	  int i_B;

          i_b_glob = index_tid[tid];
          i_B = i_b_glob / 8;
          i_b = i_b_glob % 8;
          n[i_B] = (n[i_B] | mask_byte(i_b));
        }
      }

    memcpy(o, n, tid_list_size * sizeof(unsigned int));

  }



  delete [] index_tid;
  delete [] new_list;
  delete [] ordered_freq_items;
  delete [] and_firsts;
}



template <class T>
void DCI_vertical_dataset<T>::reorder_bits_compact(int n_frequent_items, 
						 unsigned int *equal)	 
{
    
  unsigned int    *new_list;
  new_list = new  unsigned int[tid_list_size];

  unsigned char   *eq;
  eq = (unsigned char *) equal;
  

  for (int i = 0; i < n_frequent_items; i++) {
    unsigned char   *o;
    unsigned char   *n;
    o = (unsigned char *) &VD[i * tid_list_size];
    n = (unsigned char *) new_list;
    bzero(n, tid_list_size * sizeof(int));

    int tid=0;

    for (unsigned int j = 0; j < tid_list_size * sizeof(int); j++)
      for (unsigned int k = 0; k < 8; k++) {
	if ((eq[j] & mask_byte(k)) != 0) {
	  if ((o[j] & mask_byte(k)) != 0) {
	    int i_b, i_B;
	    
	    i_B = tid / 8;
	    i_b = tid % 8;
	    n[i_B] = (n[i_B] | (0x1 << i_b));
	  }
	  tid++;
	}
      }


    for (unsigned int j = 0; j < tid_list_size * sizeof(int); j++)
      for (unsigned int k = 0; k < 8; k++) {
	if ((eq[j] & mask_byte(k)) == 0) {
	  if ((o[j] & mask_byte(k)) != 0) {
	    int i_b, i_B;
	    i_B = tid / 8;
	    i_b = tid % 8;
	    n[i_B] = (n[i_B] | (0x1 << i_b));
	  }
	  tid++;
	}
      }

    memcpy(o, n, tid_list_size * sizeof(int));
  }

  unsigned int *pp = VD + start_sect_eq * tid_list_size;
  n_bit_set_eq = count_1_bits(pp, sz_sect_eq);

  delete [] new_list;
}






/** checks if the vertical dataset fits into main memory and
  * DCI can start at the following iter
  */
template <class T>  
bool DCI_vertical_dataset<T>::VD_can_be_allocated (unsigned int m, 
						   unsigned int n_trans) {

  tid_list_size = n_trans / (sizeof(int) * 8);  
  if ((n_trans % (sizeof(int) * 8)) != 0)
    tid_list_size++;
  VD_rows = m;
  // careful, the output is 64-bits, the old expression didn't work
  // FIXME: maybe more things should be unit64_t in the structure,
  // like VD_rows?
  uint64_t rows = VD_rows;
  VD_size = rows * tid_list_size;
  VD_size *= sizeof(int);

#ifdef DEBUG
  cerr << "want to allocate size " << VD_size  
       << " (" << VD_rows << " X " << tid_list_size << ")\n";
#endif

  if (VD_size==0) {
    return false; // cannot allocate 0 size
  }

  // maximum amount of memory available for vertical dataset
  // one third of the memory present in the system... too much?
  uint64_t MAX_SIZE_OF_VD = mem_total() / 3;

#ifdef DEBUG
  cout << "max_size =" << MAX_SIZE_OF_VD
       << ", prospective vd size = " << VD_size << std::endl;
#endif

  if (VD_size < MAX_SIZE_OF_VD) { 
    return true;        // VD can be stored in core
  }
  else return false;
}



template <class T>  
inline void DCI_vertical_dataset<T>::write_t_in_VD (unsigned int n_tr, 
						    dci_transaction<T>& t) {
  unsigned int byte_sel;
  unsigned int displ_sel;
  unsigned char *p;

  byte_sel = n_tr / 8;
  displ_sel = n_tr % 8;

  for (int i=0; i<(int) t.t_len; i++) {
    p = (unsigned char *) &VD[tid_list_size * t.elements[i]];
    p[byte_sel] = (p[byte_sel] | (mask_byte(displ_sel)));
  }
}
  
template <class T>  
inline void DCI_vertical_dataset<T>::write_t_in_VD (unsigned int n_tr, 
						    KDCI::Transaction<T>& t) {
  unsigned int byte_sel;
  unsigned int displ_sel;
  unsigned char *p;

  byte_sel = n_tr / 8;
  displ_sel = n_tr % 8;

  for (int i=0; i<(int) t.length; i++) {
    p = (unsigned char *) &VD[tid_list_size * t.t[i]];
    p[byte_sel] = (p[byte_sel] | mask_byte(displ_sel));
  }
}


} // namespace

#endif
