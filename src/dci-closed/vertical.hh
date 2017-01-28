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

#ifndef __DCICLOSED_VERTICAL_H
#define __DCICLOSED_VERTICAL_H

#include "transaction.hh"
#include "tid_list_macros.hh"
#include "writer.hh"
#include "memory.hh"

namespace DCIClosed {

const int COMPACT_DATASET  = 1;
const int DIFFUSE_DATASET = 0;


#ifdef MY_STATS
	static float BYTE_INCL						= 0;
	static float NUM_CS								= 0;
	static float NUM_FAILED						= 0;
	static float HEAD_INTERSECTION		= 0;
	static float TAIL_INTERSECTION		= 0;
	static float NO_HEAD_INTERSECTION	= 0;
	static float BYTE_INTERSECTION		= 0;
	static float PRE									= 0;
	static float PRUNED_PRE						= 0;
	static float PROJECTIONS					= 0;


	// static float NO_TAIL_INCL = 0;
	Chronos FIRST_SCAN_TIME;
	Chronos SECOND_SCAN_TIME;
	Chronos OUTPUT_TIME;
  Chronos SUPPORT_TIME;
	Chronos DUPLICATE_TIME;
	Chronos CLOSURE_TIME;
	Chronos PROJECTION_TIME;

#endif

static inline unsigned char mask_byte(int k) {
  return (unsigned char) 1 << k;
}

//const static unsigned char mask_byte[8] = { 0x1, 0x2, 0x4, 0x8, 
// 0x10, 0x20, 0x40, 0x80 };

// STATIC MEMORY FOR TID_LIST_STORING
unsigned int* STATIC_TID_LISTS;
unsigned int* ITEMSETS_BY_LENGTH;
unsigned int MAX_ITEMSET_LENGTH;





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
//        plog << "size tid_list reduced=" << (int) num_and
//             << " (" << tid_list_size << ")\n";
    }
  }
  
  void get_stats(int VD_active_rows, int n_cand, int iter, int tid_list_size, 
		 int& COMPLEXITY_INTERSECTION, int& COMPLEXITY_PRUNING) {
    if (nn_and+n_and_index) {
      num_and = num_and / (double) (nn_and+n_and_index);
      
      COMPLEXITY_INTERSECTION = (nn_and + n_and_index) * (int) num_and;
      COMPLEXITY_PRUNING = VD_active_rows * tid_list_size;
      
      //plog << "size tid_list reduced=" << (int) num_and 
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
class DCI_simple_dataset {
public:

  inline bool belongs_to_SE(T elem) {
    if (elem >=  start_sect_eq) 
      return true;
    else
      return false;	
  }

	unsigned int* VD;
//	unsigned int* EQUAL;
  // stores boundaries of the EQ section: used only for dense
  unsigned int start_sect_eq;
  unsigned int end_sect_eq;
  unsigned int sz_sect_eq;
	unsigned int sz_tail;
  // store the count for the EQ section: used only for dense
  int n_bit_set_eq;
  unsigned int tid_list_size;
	
	dci_items* counters;
	ItemsetWriter<T>* writer;


	DCI_simple_dataset<T>* CreateNewDataset(close_itemset<T> &mask_itemset,
																					vector<close_item<T> > &preSet,
																					vector<close_item<T> > &postSet);
	DCI_simple_dataset<T>* CreateNewDataset(unsigned int item);

	bool Check_Inclusion(close_itemset<T> &i, unsigned int j,
												unsigned int &eq, unsigned int &new_eq);

	inline void Intersect_and_Count(	close_itemset<T> &dest,
																		close_itemset<T> &cs,
																		unsigned int item,
																		POST_item<T>* post_lists
																		 );


	void DClose_next_iter(	
													close_itemset<T> &closedSet,
													T* pre_list,
													unsigned int* pre_list_included,
													unsigned int pre_list_lung,
													POST_item<T>* post_list,
													unsigned int post_list_lung);



	int masked_shrink (	unsigned char* dest,
											unsigned char* src,
											unsigned char* mask,
											int* shifted,
											unsigned int n0,
											unsigned int n1);

	void chk_compact_vertical(int n_frequent_items);
  void reorder_bits_compact(int n_frequent_items, unsigned int *equal);
};

template <class T>
class DCI_vertical_dataset {
public:
  DCI_vertical_dataset(dci_items* ii) {
    VD = NULL;
    prune_mask = NULL;
    tid_list_size = 0;
    //flag_item=NULL;
    //first_items=NULL;  
    CACHE_tidlist=NULL;
    and_index = NULL;
		counters = ii;
		threshold = 0.75;

		CACHE_items    = new T[counters->get_m1()];
		CACHE_items[0] = counters->get_m1(); // init CACHE - surely different !!!
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

		delete CACHE_items;
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
    //   plog << "allocated VD of size " << VD_rows*tid_list_size 
    //	 << " (" << VD_rows << " X " << tid_list_size << ")\n";
    bzero(VD,VD_rows*tid_list_size*sizeof(unsigned int));
    prune_mask = new unsigned int[tid_list_size];
    and_index = new int[tid_list_size+2];
  }
  
  inline bool candidate_is_frequent_diffuse(T *cand, int prefix_len, int iter, 
					   int min_count, int& count,
					   DCI_statistics& stats,
					   bool pruning);
  inline bool candidate_is_frequent_compact(T *cand, int iter, 
					  int min_count, int& count,
					  DCI_statistics& stats);


  inline void reset_prune_mask() {
     bzero(prune_mask,tid_list_size*sizeof(unsigned int));
     //bzero(flag_item,VD_rows*sizeof(unsigned int));
     //bzero(first_items,VD_rows*sizeof(unsigned int));
  }

  inline void write_t_in_VD (unsigned int n_tr, dci_transaction<T>& t);
  inline void write_t_in_VD (unsigned int n_tr, DCIClosed::Transaction<T>& t);

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

  void CalcClosure(T* itemset, int length, 
    vector<T>* result, vector<T>& skip_list);

  void BuildHashMap(void);


	void DClose_first_iter(ItemsetWriter<T>* writer);


	/*
	inline unsigned int* getTidList(unsigned int i) {
	};
	inline bool isEqual(unsigned int* i, unsigned int* j);
	bool inClosure(unsigned int* item, unsigned int* itemset) {
		return false;
	}
  */

private:
	dci_items* counters;
  unsigned int *VD;
  unsigned int VD_size;
  unsigned int tid_list_size;
  unsigned int VD_rows;
	int VD_trans;
  unsigned int *prune_mask;
  //unsigned int *flag_item;
  //unsigned int *first_items;
  int dataset_kind;
  // the CACHE !
  unsigned int *CACHE_tidlist;
	T* CACHE_items;


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



  /** Hash Map */
  vector< vector<unsigned int> > hash_map;
  /** offset to find the hash key */
  unsigned int OFFSET;
	float threshold;
};

/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
#include "memory.hh"

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
						     int iter, 
						     int min_count, 
						     int& count, 
						     DCI_statistics& stats)
{
  unsigned int *pp, *pp1;
  bool use_section = false;

	int prefix_len;
  for (prefix_len = 0; prefix_len < iter-1; prefix_len++)
    if (cand[prefix_len] != CACHE_items[prefix_len])
      break;
      
  for (int i = prefix_len; i < iter; i++)  // copy to cache 
    CACHE_items[i] = cand[i];
  CACHE_items[iter] = counters->get_m1();

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


/** heuristic used to determine if in the vertical dataset
  * there is a region where tid lists resamble one with the other
  * 
  * the area of the region of similarity must be above a given 
  * threshold for the dataset to be dense. 
  */

template <class T>
int DCI_vertical_dataset<T>::chk_compact_vertical(int n_frequent_items)
{
  int start, end, cc=0, perc=0;
  unsigned int *mask, *last_row, *result;

  float perc_eq_limit[] = { 95.0, 90.0, 85.0, 80.0, 75.0};
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
      } else if ((end-start) < n_frequent_items/6)
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

	start_sect_eq=start;
	end_sect_eq=end;
	sz_sect_eq = cc / (sizeof(int)*8);
   
	 if (!counters->use_key_patterns()) {
     if (perc == 0 || (end-start) < limit_num_freq_eq) {
       delete [] mask;
				delete [] result;

/*				plog << "sparse" << endl;
				plog << "freq: " << n_frequent_items << endl;
				plog << "perc: " << perc << endl;
				plog << "(end-start)" << (end-start) << endl;
				plog << "lim_num_freq" << limit_num_freq_eq << endl;
				plog << "sz_sect " << sz_sect_eq << endl;
*/
			dataset_kind = DIFFUSE_DATASET;
				return dataset_kind;
			}
	 }
/*  
	plog << "dense" << endl;
	plog << "freq: " << n_frequent_items << endl;
	plog << "perc: " << perc << endl;
	plog << "(end-start)" << (end-start) << endl;
	plog << "lim_num_freq" << limit_num_freq_eq << endl;
	plog << "sz_sect " << sz_sect_eq << endl;
*/

	if (end_sect_eq-start_sect_eq>1)
	  reorder_bits_compact(n_frequent_items, mask);
	else {
		sz_sect_eq = 0; 
		start_sect_eq = end_sect_eq;
	}
  
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
  // plog << "ORDERING ...\n";

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
          //  plog << "error order_bits\n";
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
            //  plog << "error2 order_bits\n";
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
  // plog << "n_trans : " << n_trans << endl;
  VD_trans = (int)n_trans;
  tid_list_size = n_trans / (sizeof(int) * 8);  
  if ((n_trans % (sizeof(int) * 8)) != 0)
    tid_list_size++;
  VD_rows = m;
  VD_size = VD_rows * tid_list_size * sizeof(int);
  
	// always accept nonethless the size
	return true;


  // maximum amount of memory available for vertical dataset
  // one third of the memory present in the system... too much?
  unsigned int MAX_SIZE_OF_VD = mem_total() / 3;

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
						    Transaction<T>& t) {
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

/** Build the Hash Map
  */
template <class T>  
void DCI_vertical_dataset<T>::BuildHashMap(void){
  unsigned char* p; //tid list pointer

  OFFSET = sz_sect_eq/2; // HASH
  if (OFFSET>=tid_list_size) 
    OFFSET = tid_list_size-1;

  // allocate space
  hash_map.resize(256);

  // if the hash_key is 0 all the items match
  hash_map[0].resize(end_sect_eq+1);
  for (unsigned int item=0; item<=end_sect_eq; item++)
    hash_map[0][item] = item;

  // for all the other values of the key
  for (unsigned char hash_key=0; hash_key<255; hash_key++) {
    // for each tid list
    for (unsigned int item=0; item<end_sect_eq; item++) {
      p = (unsigned char*)(VD + item*tid_list_size + OFFSET);
      if (( *p & (hash_key+1) ) == (hash_key+1))
        hash_map[hash_key+1].push_back(item);
    }
    hash_map[hash_key+1].push_back(end_sect_eq);
  }
}


template <class T>  
void DCI_vertical_dataset<T>::CalcClosure( T* itemset, int length,
                                           vector<T>* result,
                                           vector<T>& skip_list){
  unsigned int*   mask;     // result of the tid list intersection
  unsigned int*   tid_list_pointer;
  bool            match;
  unsigned int    skip_index = 0;
  unsigned int    j;      // the item
  int             hash_index=0;
  unsigned char*  hash_key;

  result->resize(0);   // initialize result
  
  //    retrieve the mask from the CACHE
  // get the last element of the CACHE
  mask = CACHE_tidlist + (length-1) * tid_list_size;
  
  // check if the SECT_EQ is envolved
  if ( is_included(itemset[length-1]) ) {       //  if the last item is in SECT_EQ
    if ( is_included(itemset[0]) ) {            //     if all items are in SECT_EQ
      // copy the SECT_EQ to the mask
      memcpy(mask, VD + itemset[0]*tid_list_size, sz_sect_eq*sizeof(unsigned int));
    } else {                                    
      int kk;                                   // find the firt intersection with SECT_EQ
      for( kk=length-2; is_included(itemset[kk]); kk--);
      memcpy(mask, CACHE_tidlist + (kk+1) * tid_list_size, sz_sect_eq*sizeof(unsigned int));
    }
  }

  // get the firt item between the hashed candidates
  hash_key = (unsigned char*)(mask+OFFSET);
  j = hash_map[*hash_key][hash_index];
  // skip the first unuseful elements
  // for (;j<itemset[0]; j=hash_map[*hash_key][++hash_index]);

  //      for eash item accumulate closure
  // outside SECT_EQ
//  for (unsigned int j=0; j<start_sect_eq; j++) {
  for (;j<start_sect_eq; j=hash_map[*hash_key][++hash_index]) {
    // get the tid_list of the candidate item
   	tid_list_pointer = VD + j*tid_list_size;
    // compare the tid_lists
    match=true;

    // if the item is already in the closure
    if (j==skip_list[skip_index]) {
      result->push_back((T)j);
      skip_index++;
//      if (skip_index == skip_list.size())
//        skip_index = 0;
    } 
    // else check whether it's in the closure
    else {
	    for (unsigned int kk=0; match && kk<tid_list_size; kk++) {
	      match = ( *(tid_list_pointer+kk) & *(mask+kk) ) == *(mask+kk);
#ifdef MY_STATS
	      BYTE_INCL++;
#endif
			}
      if (match)
        result->push_back((T)j);
    }
  }
  // inside SECT_EQ
  // first check SECT_EQ
  match=true;
  // if at least one item is already in sect_eq then don't check sect eq
  if (skip_list[skip_index]>=start_sect_eq) {   
    // compare the tid lists
    // get a pointer to the SECT_EQ
    tid_list_pointer = VD + start_sect_eq*tid_list_size;
    for (unsigned int kk=0; match && kk<sz_sect_eq; kk++) {
      match = ( *(tid_list_pointer+kk) & *(mask+kk) ) == *(mask+kk);
      this->num_confr++;
    }
  }
  // if SECT_EQ matches, then go on with the rest of the tid lists
  if (match) {
//    for (unsigned int j=start_sect_eq; j<end_sect_eq; j++) {
    for (;j<end_sect_eq; j=hash_map[*hash_key][++hash_index]) {
      // get the tid_list of the candidate item
   	  tid_list_pointer = VD + j*tid_list_size;
      // compare the tid_lists
      match=true;

      // if the item is already in the closure
      if (j==skip_list[skip_index]) {
        result->push_back((T)j);
        skip_index++;
//        if (skip_index == skip_list.size())
//          skip_index = 0;
      } 
      // else check whether it's in the closure
      else {
        for (unsigned int kk=sz_sect_eq; match && kk<tid_list_size; kk++) {
	        match = ( *(tid_list_pointer+kk) & *(mask+kk) ) == *(mask+kk);
	        this->num_confr++;
	      }
        if (match)
          result->push_back((T)j);
      }
    }
  }
}

// ===================================================================
// ===================================================================
// ===================================================================



template <class T>
struct AscendingSupportCloseItemsSort
{
  bool operator() (const close_item<T>& a, const close_item<T>& b) {
		return ((a.count) < (b.count));
  }
};

template <class T>
struct DescendingSupportCloseItemsSort
{
  bool operator() (const close_item<T>& a, const close_item<T>& b) {
	  return ((a.count) > (b.count));
  }
};





template <class T>  
void DCI_vertical_dataset<T>::DClose_first_iter(ItemsetWriter<T>* writer){

#ifdef MY_STATS
		SUPPORT_TIME.StartChronos();
#endif
	// -----------------------------------
	// Initialize the simple dataset
	// -----------------------------------
	DCI_simple_dataset<T> Data;
	Data.VD											= VD;
//	Data.EQUAL									= VD + start_sect_eq*tid_list_size;
  Data.start_sect_eq					= start_sect_eq;
  Data.end_sect_eq						= end_sect_eq;
  Data.sz_sect_eq							= sz_sect_eq;
  Data.n_bit_set_eq						= n_bit_set_eq;
  Data.tid_list_size					= tid_list_size;
	Data.sz_tail								= tid_list_size-sz_sect_eq;
	Data.counters								= counters;
	Data.writer									= writer;


	ITEMSETS_BY_LENGTH = new unsigned int [end_sect_eq];
	for (unsigned int i=0; i<end_sect_eq; i++)
		ITEMSETS_BY_LENGTH[i]=0;
	MAX_ITEMSET_LENGTH = 0;

	// ----------------------------------------------------
	// Initialize the static memory for storing tid_lists
	// ----------------------------------------------------
	STATIC_TID_LISTS = new unsigned int [tid_list_size*end_sect_eq];
			// to be changed after the projection

		
	// -------------------------------------
	// Initialize New Closure
	// -------------------------------------
	close_itemset<T> new_closure;
	new_closure.items = new T [Data.end_sect_eq];		// items of the closure are
																									//  are accumulated in the same shared memory

	// ---------------------------------------
	// Aux variables
	// --------------------------------------
	// int skip=0;                                 // number of items having the same tid list
	                                            //    of some previous adiacent item
	unsigned int included_words;
	unsigned int dummy;

	// -----------------------------------
	// Initialize PRE and POST lists
	// -----------------------------------

	// Initialize PRE vector
	T* pre_list = new T [Data.end_sect_eq];
	unsigned int* pre_list_included = new unsigned int [Data.end_sect_eq*Data.end_sect_eq];
	unsigned int pre_list_lung = 0;

	// Initialize POST vector
	POST_item<T>* post_list = new POST_item<T> [(end_sect_eq*end_sect_eq)/2+1];
	unsigned int post_list_lung;

#ifdef MY_STATS
		SUPPORT_TIME.StopChronos();
#endif


	// ------------------------------------------------
	// Start Checking Closure
	// ------------------------------------------------
	// for each item... last item dealt separately
	unsigned int post_i=0, pre_i;
	for (; post_i < end_sect_eq-1; post_i++) {
//		plog << "post_i: " << post_i << endl;

		//plog << "First" << endl;


#ifdef MY_STATS
		CLOSURE_TIME.StartChronos();
#endif

	//	skip = 0;									// no item to skip because of equal tid lists.
		post_list_lung = 0;				// empty the postSet size

		// Initialize the new Closure: new_closure = post_I
		new_closure.lung                      = 0;
		new_closure.items[new_closure.lung++] = (T) post_i;
		new_closure.count											= counters->get_count(counters->first_freq+post_i);
		new_closure.isSectionIncluded					= Data.belongs_to_SE(post_i);
		new_closure.head_list									= Data.VD + post_i*Data.tid_list_size;
		new_closure.tail_list									= new_closure.head_list + Data.sz_sect_eq;
		if (new_closure.isSectionIncluded)
			new_closure.sect_eq_num_bit					= Data.n_bit_set_eq;
		else
			new_closure.sect_eq_num_bit					= 0;
		new_closure.bytes_included_sect_eq		= 0;

		// ------------------------------------------------
		// Discard Duplicates
		// ------------------------------------------------

#ifdef MY_STATS
		DUPLICATE_TIME.StartChronos();
#endif

		included_words = 0;
		for(
			pre_i=0;  
			pre_i<post_i &&
			!Data.Check_Inclusion( new_closure,	pre_i, included_words, dummy);	
			pre_i++);

#ifdef MY_STATS
		DUPLICATE_TIME.StopChronos();
#endif

		// if an inclusion was found go with the next post Item
		if (pre_i<post_i){
#ifdef MY_STATS
			NUM_FAILED++;
#endif
			continue;
		}		


		// -----------------------------------------
		// LOOK FOR OTHER ITEMS TO ADD IN CLOSURE
		// -----------------------------------------
		// initialize post_j to the subsequent item
		unsigned int post_j=post_i+1;
		for (; post_j<end_sect_eq; post_j++) {

			included_words = 0;
			if (Data.Check_Inclusion(new_closure, post_j, included_words, dummy)) {
				// add current item to the current closure
				new_closure.items[new_closure.lung++] = (T) post_j;
				// if tid lists are the same, then skip post_j item
				if (new_closure.count == (int)counters->get_count(counters->first_freq+post_j)) {
//					plog << "post_j: " << post_j << endl;
//					skip++;
				}
			} else {
				// if the item is not included then add it to the postSet
				post_list[post_list_lung].id				= post_j;
				post_list[post_list_lung].included	= 0; //included_words;
				post_list_lung++;
			}
		}
#ifdef MY_STATS
		CLOSURE_TIME.StopChronos();
#endif

		// -----------------------------------------
		// Write output
		// -----------------------------------------
//		if (write_output) {
#ifdef MY_STATS
			OUTPUT_TIME.StartChronos();
#endif
			// output
			writer->Write(new_closure);
#ifdef MY_STATS
			OUTPUT_TIME.StopChronos();
#endif
//		}

		ITEMSETS_BY_LENGTH[new_closure.lung]++;
		if (new_closure.lung > MAX_ITEMSET_LENGTH)
			MAX_ITEMSET_LENGTH = new_closure.lung;
#ifdef MY_STATS
		NUM_CS++;
#endif

		// -----------------------------------------
		// RECURSIVE CALL
		// -----------------------------------------
		// if there're items for the recursive call
		if (post_list_lung>0) {

#ifdef MY_STATS
			PRE += pre_list_lung;
#endif


			float np = (float)post_list_lung*post_list_lung / (float)end_sect_eq/ (float)end_sect_eq;
			float sp = (float)(new_closure.count - counters->min_count) / (float)counters->get_count(counters->first_freq+end_sect_eq-1);

			if (np >.1 && sp>.01 && Data.tid_list_size>10) {
			//if (false){

				// Project The Dataset
#ifdef MY_STATS
				PROJECTION_TIME.StartChronos();
				PROJECTIONS++;
#endif
				DCI_simple_dataset<T>* newData;
				newData = Data.CreateNewDataset(post_i);

				new_closure.head_list = newData->VD+post_i*newData->tid_list_size;
				new_closure.tail_list = new_closure.head_list + newData->sz_sect_eq;
				if (new_closure.isSectionIncluded)
					new_closure.sect_eq_num_bit = newData->n_bit_set_eq;

#ifdef MY_STATS
				PROJECTION_TIME.StopChronos();
#endif
				// ------- RECURSIVE CALL ---------
				newData->DClose_next_iter(new_closure, pre_list, pre_list_included, pre_list_lung, post_list, post_list_lung);

#ifdef MY_STATS
				PROJECTION_TIME.StartChronos();
#endif
				delete newData->VD;
				delete newData;

#ifdef MY_STATS
				PROJECTION_TIME.StopChronos();
#endif
			} else {
				// ------- RECURSIVE CALL ---------
				Data.DClose_next_iter(new_closure, pre_list, pre_list_included, pre_list_lung, post_list, post_list_lung);
			}
		}

		// grow pre list
		pre_list[pre_list_lung] = post_i;
		pre_list_included[pre_list_lung] = 0;
		pre_list_lung++;
	} 



	// LAST ELEMENT
	// if we don't have to skip
	if (post_i == end_sect_eq-1 ) {
		//plog << "but last" << endl;

		// reset previous closures and add current item to the current closure
		new_closure.lung = 0;
		new_closure.items[new_closure.lung++] = (T)(end_sect_eq-1);
		new_closure.count = counters->get_count(counters->first_freq+end_sect_eq-1);

//		if (write_output) {
#ifdef MY_STATS
			OUTPUT_TIME.StartChronos();
#endif
			// output
			writer->Write(new_closure);
#ifdef MY_STATS
			OUTPUT_TIME.StopChronos();
#endif
//		}

		ITEMSETS_BY_LENGTH[new_closure.lung]++;
		if (new_closure.lung > MAX_ITEMSET_LENGTH)
			MAX_ITEMSET_LENGTH = new_closure.lung;

#ifdef MY_STATS
		NUM_CS++;
#endif
	}

	// empty set.
	if ((int) counters->get_count(counters->first_freq+end_sect_eq-1) != VD_trans ) {
		//plog << "empy set" << endl;

		// reset previous closures and add current item to the current closure
		new_closure.lung = 0;
		new_closure.count = VD_trans;

//		if (write_output) {
#ifdef MY_STATS
			OUTPUT_TIME.StartChronos();
#endif
			// output
			writer->Write(new_closure);
#ifdef MY_STATS
			OUTPUT_TIME.StopChronos();
#endif
//		}

		ITEMSETS_BY_LENGTH[new_closure.lung]++;
		if (new_closure.lung > MAX_ITEMSET_LENGTH)
			MAX_ITEMSET_LENGTH = new_closure.lung;

#ifdef MY_STATS
		NUM_CS++;
#endif
	}
	
	unsigned int tot_items = 0;
	for (unsigned int i=0; i<=MAX_ITEMSET_LENGTH; i++)
		tot_items += ITEMSETS_BY_LENGTH[i];
	plog << tot_items << endl;
	for (unsigned int i=0; i<=MAX_ITEMSET_LENGTH; i++)
		plog << ITEMSETS_BY_LENGTH[i] << endl;


	
	// ------------------------
	// Free memory
	// ------------------------
	delete [] STATIC_TID_LISTS;
	delete [] post_list;
	delete [] new_closure.items;
	delete [] pre_list;
	delete [] pre_list_included;
	delete [] ITEMSETS_BY_LENGTH;


	// OUTPUT STATISTICS
#ifdef MY_STATS
	plog << " - Statistics" << endl;
	plog << " . Timings" << endl;
	plog << " . . 1. SCAN                   time : " << FIRST_SCAN_TIME.ReadChronos() << endl;
	plog << " . . 2. SCAN and DB Building   time : " << SECOND_SCAN_TIME.ReadChronos() << endl;
	plog << " . . OUTPUT     time : " << OUTPUT_TIME.ReadChronos() << endl;
	plog << " . . CLOSURE    time : " << CLOSURE_TIME.ReadChronos() << endl;
	plog << " . . SUPPORT    time : " << SUPPORT_TIME.ReadChronos() << endl;
	plog << " . . DUPLICATES time : " << DUPLICATE_TIME.ReadChronos() << endl;
	plog << " . . PROJECTION time : " << PROJECTION_TIME.ReadChronos() << endl;
	plog << " . Operations" << endl;
	plog << " . . Byte INCLUSIONS    : " << BYTE_INCL << endl;
	plog << " . . Byte INTERSECTIONS : " << BYTE_INTERSECTION << endl;
	plog << " . . HEAD    intersections : " << HEAD_INTERSECTION << endl;
	plog << " . . NO-HEAD intersections : " << NO_HEAD_INTERSECTION << endl;
	plog << " . . TAIL    intersections : " << TAIL_INTERSECTION << endl;
	plog << " . . PRE        Items : " << PRE << endl;
	plog << " . . PRUNED_PRE Items : " << PRUNED_PRE << endl;
	plog << " . . PROJECTIONS  : " << PROJECTIONS << endl;
	plog << " . Sets" << endl;
	plog << " . . CLOSED sets : " << NUM_CS << endl;
  plog << " . . FAILED sets : " << NUM_FAILED << endl;
	plog << " . Dataset" << endl;
	plog << " . . Section equal width  : " << Data.sz_sect_eq << " / " << Data.tid_list_size << endl;
	plog << " . . Section equal length : " << Data.start_sect_eq << " / " << Data.end_sect_eq << endl;

#endif
}


template <class T>
void DCI_simple_dataset<T>::DClose_next_iter(	 
													close_itemset<T> &closedSet,
													T* pre_list,
													unsigned int* pre_list_included,
													unsigned int pre_list_lung,
													POST_item<T>* post_list,
													unsigned int post_list_lung){

		//plog << "second" << endl;
		//plog << "post    : " << post_list.size() << endl;

	// ------------------------------------------------
	// Duplicate included list
	// ------------------------------------------------
#ifdef MY_STATS
	DUPLICATE_TIME.StartChronos();
#endif
	
	unsigned int* new_pre_list_included = (pre_list_included + pre_list_lung + post_list_lung);
	// memcpy(new_pre_list_included, pre_list_included, sizeof(unsigned int)*pre_list_lung);

#ifdef MY_STATS
	DUPLICATE_TIME.StopChronos();
#endif

	// ------------------------------------------------
	// init closure variables
	// ------------------------------------------------
#ifdef MY_STATS
	CLOSURE_TIME.StartChronos();
#endif

	// reserve memory for intersected postSet items
	POST_item<T>* new_post_list = post_list+post_list_lung;
	unsigned int new_post_list_lung; 

	// initialize new_closure
	close_itemset<T> new_closure;
	new_closure.items												= closedSet.items;
	unsigned int* new_tid_list							= STATIC_TID_LISTS + closedSet.lung*tid_list_size;
																							// to be changed with a level ?!??
	new_closure.head_list										= new_tid_list;
	new_closure.tail_list										= new_tid_list + sz_sect_eq;
	new_closure.isSectionIncluded						= closedSet.isSectionIncluded;
	// isSectionIncluded is set here, and it is used dinamically
	//   in the following to detect were operations involving SECT_EQ

#ifdef MY_STATS
		CLOSURE_TIME.StopChronos();
#endif

	// aux variables
	unsigned int post_i, post_j, pre_i, included;

	// for eash post_i
	for (post_i=0; post_i < post_list_lung; post_i++) {

		// ------------------------------------------------
		// Create a new Generator
		// ------------------------------------------------
		//plog << "Intersect : " << (int)post_list[post_i].id <<endl;

#ifdef MY_STATS
		SUPPORT_TIME.StartChronos();
#endif


		// reset previous closures and add current item to the current closure
	  new_closure.lung												= closedSet.lung;
		new_closure.items[new_closure.lung++]		= post_list[post_i].id;
		// new_closure.sect_eq_num_bit							= closedSet.sect_eq_num_bit;

		// Intersect tid-list
		Intersect_and_Count(new_closure, closedSet, post_i, post_list);

#ifdef MY_STATS
		SUPPORT_TIME.StopChronos();
#endif

		// is it frequent ??
		if (((int)counters->min_count) > new_closure.count  ) {
#ifdef MY_STATS
			NUM_FAILED++;
#endif
			// try with the nest generator
			continue;
		}

		// ------------------------------------------------
		// Discard Duplicates
		// ------------------------------------------------
		//plog << "Duplicates" << endl;

#ifdef MY_STATS
		DUPLICATE_TIME.StartChronos();
#endif

		new_closure.bytes_included_sect_eq = closedSet.bytes_included_sect_eq;
		for(
			pre_i=0;  
			pre_i<pre_list_lung &&
			!Check_Inclusion(		new_closure,
													pre_list[pre_i], 
													pre_list_included[pre_i], 
													new_pre_list_included[pre_i]);	
			pre_i++);

#ifdef MY_STATS
		DUPLICATE_TIME.StopChronos();
#endif

		// if an inclusion was found go with the next post Item
		if (pre_i<pre_list_lung){
#ifdef MY_STATS
			NUM_FAILED++;
#endif
			continue;
		}		


		// ------------------------------------------------
		// Calculate Closure
		// ------------------------------------------------
		//plog << "Closure" << endl;

#ifdef MY_STATS
		CLOSURE_TIME.StartChronos();
#endif

		new_post_list_lung = 0;
		
		for (post_j=post_i+1; post_j<post_list_lung; post_j++) {
			// if an inclusion is found
			
			if ( Check_Inclusion(	new_closure,	
														post_list[post_j].id,
														post_list[post_j].included,
														included)	) {

				// add current item to the current closure
				new_closure.items[new_closure.lung++] = post_list[post_j].id;
			} else {
				new_post_list[new_post_list_lung].id				= post_list[post_j].id;
				new_post_list[new_post_list_lung].included	= included;
				new_post_list_lung++;
			}
		}

#ifdef MY_STATS
		CLOSURE_TIME.StopChronos();
#endif

		ITEMSETS_BY_LENGTH[new_closure.lung]++;
		if (new_closure.lung > MAX_ITEMSET_LENGTH)
			MAX_ITEMSET_LENGTH = new_closure.lung;

#ifdef MY_STATS
		NUM_CS++;
#endif


		// ------------------------------------------------
		// write output
		// ------------------------------------------------
//		if (write_output) {
#ifdef MY_STATS
			OUTPUT_TIME.StartChronos();
#endif
			// output
			writer->Write(new_closure);

#ifdef MY_STATS
			OUTPUT_TIME.StopChronos();
#endif
//		}

		// ------------------------------------------------
		// Recursion
		// ------------------------------------------------
		if (new_post_list_lung>0) {
			//plog << "recursion" << endl;

#ifdef MY_STATS
			PRE+= pre_list_lung;
#endif

			DClose_next_iter(	new_closure, 
												pre_list, 
												new_pre_list_included,
												pre_list_lung,
												new_post_list,
												new_post_list_lung);
		}

		// ---------------------------------------
		// Update Pre List
		// ---------------------------------------

		pre_list[pre_list_lung]										= post_list[post_i].id;
		pre_list_included[pre_list_lung]					= 0;
		pre_list_lung++;
	}
}

template <class T>
inline void DCI_simple_dataset<T>::Intersect_and_Count(
																		close_itemset<T> &dest,
																		close_itemset<T> &cs,
																		unsigned int item,
																		POST_item<T>* post_lists
																		){

	unsigned int max;
	unsigned int start;
	T id = post_lists[item].id;

	if (!cs.isSectionIncluded && dest.isSectionIncluded && belongs_to_SE(id)) {
			// we have already performed intersection operations on sect_eq
			//   during this iteration
    	// DO NOTHING !!!!!!!!!
#ifdef MY_STATS
		NO_HEAD_INTERSECTION++;
#endif
	} else if (cs.isSectionIncluded && belongs_to_SE(id)) {
		// since cs <= id, no intersection
		dest.sect_eq_num_bit	= cs.sect_eq_num_bit;
		dest.head_list				= cs.head_list;
#ifdef MY_STATS
		NO_HEAD_INTERSECTION++;
#endif
	} else {
		
		// copy included bytes without intersection
		max = post_lists[item].included;
		if (max>sz_sect_eq)
			max = sz_sect_eq;

		if (item > 0) 
			start = post_lists[item-1].included;
		else 
			start = 0;

		if (start<max)
			memcpy( dest.head_list	+ start,
							cs.head_list		+ start,
							(max-start) *sizeof(int));

		dest.sect_eq_num_bit = 
			count_1_bits(dest.head_list, max);

		// perform intersection for the rest of the tidlist
		dest.sect_eq_num_bit += and_tid_list_count_1(	
																	dest.head_list					+ max,
																	cs.head_list						+ max,
																	VD + id*tid_list_size		+ max,
																	sz_sect_eq - max);

#ifdef MY_STATS
				HEAD_INTERSECTION++;
				BYTE_INTERSECTION +=(sz_sect_eq-max);
#endif
				
	/*
		dest.sect_eq_num_bit = and_tid_list_count_1(	
																	dest.head_list,
																	cs.head_list,
																	VD + id*tid_list_size,
																	sz_sect_eq);

#ifdef MY_STATS
				HEAD_INTERSECTION++;
				BYTE_INTERSECTION +=(sz_sect_eq);
#endif
				*/
	}

	
	if (post_lists[item].included>sz_sect_eq)
		max = post_lists[item].included - sz_sect_eq;
	else 
		max = 0;

	if (item > 0) 
		start = post_lists[item-1].included;
	else 
		start = 0;

	if (start>sz_sect_eq)
		start -= sz_sect_eq;
	else 
		start = 0;

	if (start<max)
		memcpy( dest.tail_list		+ start,
						cs.tail_list			+ start,
						(max-start)*sizeof(int));

	dest.count = dest.sect_eq_num_bit +
		count_1_bits(dest.tail_list, max);

	dest.count += and_tid_list_count_1(	
		dest.tail_list			+ max,
		cs.tail_list				+ max,
		VD+id*tid_list_size+sz_sect_eq	+ max,
		sz_tail - max);


	dest.isSectionIncluded |= belongs_to_SE(id);

#ifdef MY_STATS
	TAIL_INTERSECTION++;
	BYTE_INTERSECTION +=(sz_tail-max);
#endif
	
/*
	dest.count = dest.sect_eq_num_bit +
		and_tid_list_count_1(	
		dest.tail_list,
		cs.tail_list,
		VD+id*tid_list_size+sz_sect_eq,
		sz_tail);

	dest.isSectionIncluded |= belongs_to_SE(id);

#ifdef MY_STATS
	TAIL_INTERSECTION++;
	BYTE_INTERSECTION +=(sz_tail);
#endif
*/

}
	
template <class T>
bool DCI_simple_dataset<T>::Check_Inclusion(
															close_itemset<T> &i,
															unsigned int j,
															unsigned int &eq,
															unsigned int &new_eq){

	unsigned int kk;

	// initialize tid_list pointers
	unsigned int* tid_list_i = i.head_list;
	unsigned int* tid_list_j = VD+j*tid_list_size;


	// -------------------------
	// section equal
	// -------------------------
	bool match = true;
	if (sz_sect_eq>0 && belongs_to_SE(j)) {
		if (! i.isSectionIncluded) {
			// use additional info about sect_eq
/*
			if (i.bytes_included_sect_eq != UINT_MAX) {
				// inclusion was already performed
				match = (i.bytes_included_sect_eq >= sz_sect_eq);
			} else {
				// first inclusion in sect eq
				kk = eq;
				for (; match && kk<sz_sect_eq; kk++) {
					match = ( *(tid_list_j+kk) & *(tid_list_i+kk) ) == *(tid_list_i+kk);
#ifdef MY_STATS
					BYTE_INCL++;
#endif
				}
				i.bytes_included_sect_eq = kk;
				if (!match)
					i.bytes_included_sect_eq--;
			}
			*/

			if (i.bytes_included_sect_eq > eq)
				kk = i.bytes_included_sect_eq;
			else
				kk = eq;
			if (kk < sz_sect_eq) {
				for (; match && kk<sz_sect_eq; kk++) {
					match = ( *(tid_list_j+kk) & *(tid_list_i+kk) ) == *(tid_list_i+kk);
		#ifdef MY_STATS
					BYTE_INCL++;
		#endif
				}
				i.bytes_included_sect_eq = kk-1;
			}
			else 
				i.bytes_included_sect_eq = kk;

		}
		// else match stays true for section equal
	} else {
		// if j not in sect_eq
		// no section equal optimization
		for (kk=eq; match && kk<sz_sect_eq; kk++) {
			match = ( *(tid_list_j+kk) & *(tid_list_i+kk) ) == *(tid_list_i+kk);
#ifdef MY_STATS
			BYTE_INCL++;
#endif
		}
	}
	
	// -------------------------------
	// tail (outside section equal)
	// -------------------------------
	if (match) {
		//i.isSectionIncluded = true; perche' non funziona ????
		//                            perche' dovrei fare come nelle intersezioni
		//                            quanto capita ?? abbastanza


		tid_list_i = i.tail_list;
		tid_list_j += sz_sect_eq;

		if (eq<=sz_sect_eq)
			kk = 0;
		else
			kk = eq - sz_sect_eq;
		//kk = 0;
		for (; match && kk<sz_tail; kk++) {
			match = ( *(tid_list_j+kk) & *(tid_list_i+kk) ) == *(tid_list_i+kk);
#ifdef MY_STATS
			BYTE_INCL++;
#endif
		}
		new_eq = sz_sect_eq + kk -1;
	}
	else  // if no match in section equal
		new_eq = kk-1;

	return match;
}

template <class T> 
DCI_simple_dataset<T>* DCI_simple_dataset<T>::CreateNewDataset(
																					close_itemset<T> &mask_itemset,
																					vector<close_item<T> > &preSet,
																					vector<close_item<T> > &postSet){
	// INIT
	DCI_simple_dataset<T>* new_Data = new DCI_simple_dataset<T>(*this);
	new_Data->EQUAL					= NULL;
	new_Data->n_bit_set_eq	= 0;
	new_Data->tid_list_size = mask_itemset.count/32+2;
	new_Data->VD = new unsigned int[new_Data->tid_list_size * (preSet.size()+postSet.size())];

	int* shifted;
	unsigned int shifted_i, shifted_val;
	unsigned char* mask;
	unsigned int* vd_pointer;
  unsigned char *dest, *src;
	unsigned int* top_section = NULL;

	// --------------------------------------
	// HEAD PROJECTION
	// --------------------------------------
	shifted = new int [tid_list_size*sizeof(int)*8];
	shifted_i		= 0;
	shifted_val	= 0;
	mask = (unsigned char*) mask_itemset.head_list;
	// build shiftings
	// for each byte transaction
	for (unsigned int i=0; i<sz_sect_eq*sizeof(int); i++) {
		// if the mask_byte != 0
		if (mask[i] != 0) {
			// for each bit
			for (unsigned b=0; b<8; b++) {
				if ((mask[i] & mask_byte(b)) != 0) {
					// if the bit is 1 update stre the new position
					shifted[shifted_i++] = shifted_val++;
				} else {
					// if the bit is 0 than it has to be skipped
					shifted[shifted_i++] = -1;
				}
			}
		} else {
			//if the mask byte is 0 than we have 8 bit to skip
			for (unsigned b=0; b<8; b++) {
				shifted[shifted_i++] = -1;
			}
		}
	}
	// store the new shifted position
	new_Data->sz_sect_eq		= (shifted_val)/32+1;

	// PROJECT
	vd_pointer	= new_Data->VD;
  dest				= (unsigned char*) vd_pointer;
	for (unsigned int i=0; i<preSet.size(); i++) {
		if (new_Data->belongs_to_SE(preSet[i].id)) {
			if (new_Data->EQUAL) {
				preSet[i].head_list				= new_Data->EQUAL;
				preSet[i].sect_eq_num_bit	= new_Data->n_bit_set_eq;
			} else {
				src									= (unsigned char*) (preSet[i].head_list);
				preSet[i].head_list	= (unsigned int*) dest;
				preSet[i].sect_eq_num_bit = masked_shrink(dest, src, mask, shifted, sz_sect_eq*sizeof(int), new_Data->sz_sect_eq*sizeof(int));

				new_Data->EQUAL					= preSet[i].head_list;
				new_Data->n_bit_set_eq	= preSet[i].sect_eq_num_bit;
			}
		} else {
			src									= (unsigned char*) (preSet[i].head_list);
			preSet[i].head_list	= (unsigned int*) dest;
			preSet[i].sect_eq_num_bit = masked_shrink(dest, src, mask, shifted, sz_sect_eq*sizeof(int), new_Data->sz_sect_eq*sizeof(int));
		}

		vd_pointer += new_Data->tid_list_size;
		dest = (unsigned char*) vd_pointer;
	}
	for (unsigned int i=0; i<postSet.size(); i++) {
		
		if (new_Data->belongs_to_SE(postSet[i].id)) {
			if (new_Data->EQUAL) {
				postSet[i].head_list				= new_Data->EQUAL;
				postSet[i].sect_eq_num_bit	= new_Data->n_bit_set_eq;
			} else {
				src										= (unsigned char*) (postSet[i].head_list);
				postSet[i].head_list	= (unsigned int*) dest;
				postSet[i].sect_eq_num_bit = masked_shrink(dest, src, mask, shifted, sz_sect_eq*sizeof(int), new_Data->sz_sect_eq*sizeof(int));

				new_Data->EQUAL					= postSet[i].head_list;
				new_Data->n_bit_set_eq	= postSet[i].sect_eq_num_bit;
				top_section = new_Data->EQUAL;
			}
		} else {
			src										= (unsigned char*) (postSet[i].head_list);
			postSet[i].head_list	= (unsigned int*) dest;
			postSet[i].sect_eq_num_bit = masked_shrink(dest, src, mask, shifted, sz_sect_eq*sizeof(int), new_Data->sz_sect_eq*sizeof(int));
		} 

		vd_pointer+=new_Data->tid_list_size;
		dest = (unsigned char*) vd_pointer;
		postSet[i].included = 0;
	}

	// --------------------------------------
	// TAIL PROJECTION
	// --------------------------------------
	shifted_i		= 0;
	shifted_val	= 0;
	mask = (unsigned char*) mask_itemset.tail_list;
	// build shiftings
	// for each byte transaction
	for (unsigned int i=0; i<(tid_list_size-sz_sect_eq)*sizeof(int); i++) {
		// if the mask_byte != 0
		if (mask[i] != 0) {
			// for each bit
			for (unsigned b=0; b<8; b++) {
				if ((mask[i] & mask_byte(b)) != 0) {
					// if the bit is 1 update stre the new position
					shifted[shifted_i++] = shifted_val++;
				} else {
					// if the bit is 0 than it has to be skipped
					shifted[shifted_i++] = -1;
				}
			}
		} else {
			//if the mask byte is 0 than we have 8 bit to skip
			for (unsigned b=0; b<8; b++) {
				shifted[shifted_i++] = -1;
			}
		}
	}

	// PROJECT
	vd_pointer	= new_Data->VD + new_Data->sz_sect_eq;
  dest				= (unsigned char*) vd_pointer;
	for (unsigned int i=0; i<preSet.size(); i++) {
		src									= (unsigned char*) (preSet[i].tail_list);
		preSet[i].tail_list = (unsigned int*) dest;
		preSet[i].count			= preSet[i].sect_eq_num_bit +
			masked_shrink(dest, src, mask, shifted, (tid_list_size-sz_sect_eq)*sizeof(int), (new_Data->tid_list_size-new_Data->sz_sect_eq)*sizeof(int));

		vd_pointer += new_Data->tid_list_size;
		dest = (unsigned char*) vd_pointer;
	}
	for (unsigned int i=0; i<postSet.size(); i++) {
		src										= (unsigned char*) (postSet[i].tail_list);
		postSet[i].tail_list	= (unsigned int*) dest;
		postSet[i].count			= postSet[i].sect_eq_num_bit +
			masked_shrink(dest, src, mask, shifted, (tid_list_size-sz_sect_eq)*sizeof(int), (new_Data->tid_list_size-new_Data->sz_sect_eq)*sizeof(int));
		
		vd_pointer+=new_Data->tid_list_size;
		dest = (unsigned char*) vd_pointer;
		postSet[i].included = 0;
	}

	new_Data->isFreshProjected = true;
	delete [] shifted;

/*
	for (unsigned int i=0; i<preSet.size(); i++)
		plog <<" " << (int) preSet[i].id;
	plog << " *** ";
	for (unsigned int i=0; i<postSet.size(); i++) 
		plog <<" " << (int) postSet[i].id;
	plog << endl;

	plog << preSet.size() << " : " << postSet.size() << endl;

	new_Data->chk_compact_vertical(preSet.size()+postSet.size());

	// check previous row
	top_section = new_Data->VD+new_Data->tid_list_size*(postSet.size()+preSet.size()-1);
	int diff = 0;
	for (;top_section != new_Data->VD; top_section-=new_Data->tid_list_size) {
		bool ok = true;
		for (unsigned int i=0; ok && i<new_Data->sz_sect_eq; i++)
			ok = *(top_section+i) == *(top_section+i-new_Data->tid_list_size);
		if (ok)
			diff++;
		else break;
	}
	plog << "diff: " << diff << endl;
*/
/*
	unsigned int* unos = new unsigned int[new_Data->tid_list_size];
//	bzero(unos, new_Data->tid_list_size*sizeof(int));
	memcpy(
		unos, 
		new_Data->VD+new_Data->tid_list_size*(preSet.size()+postSet.size()-1),
		new_Data->tid_list_size*sizeof(int));
	unsigned int* p = new_Data->VD+new_Data->tid_list_size*preSet.size();
	for (unsigned int i=0; i<postSet.size()-1; i++) {
		for (unsigned int j=0; j<new_Data->tid_list_size; j++)
			unos[j] = unos[j] & p[j];
		p += new_Data->tid_list_size;
	}
	plog << count_1_bits(unos, new_Data->tid_list_size) << " / " <<  new_Data->tid_list_size *32<< endl;
	delete [] unos;
*/

	return new_Data;
}


template <class T> 
DCI_simple_dataset<T>* DCI_simple_dataset<T>::CreateNewDataset(
																					unsigned int item){
	// INIT
	DCI_simple_dataset<T>* new_Data = new DCI_simple_dataset<T>(*this);
	//new_Data->EQUAL					= NULL;
	new_Data->tid_list_size = counters->get_count(counters->first_freq+item)/32+1;
	new_Data->VD = new unsigned int[new_Data->tid_list_size * end_sect_eq];

	int* shifted;
	unsigned int shifted_i, shifted_val;
	unsigned char* mask;
	unsigned int* vd_pointer;
  unsigned char *dest, *src;

	// --------------------------------------
	// PROJECTION
	// --------------------------------------
	shifted = new int [tid_list_size*sizeof(int)*8];
	shifted_i		= 0;
	shifted_val	= 0;
	mask = (unsigned char*) (VD+item*tid_list_size);
	// build shiftings
	// for each byte transaction
	for (unsigned int i=0; i<tid_list_size*sizeof(int); i++) {
		if (i==sz_sect_eq*sizeof(int)) {
			// store the new shifted position
			new_Data->sz_sect_eq		= (shifted_val)/32;
			new_Data->sz_tail       = new_Data->tid_list_size - new_Data->sz_sect_eq;
		}
		// if the mask_byte != 0
		if (mask[i] != 0) {
			// for each bit
			for (unsigned b=0; b<8; b++) {
				if ((mask[i] & mask_byte(b)) != 0) {
					// if the bit is 1 update store the new position
					shifted[shifted_i++] = shifted_val++;
				} else {
					// if the bit is 0 than it has to be skipped
					shifted[shifted_i++] = -1;
				}
			}
		} else {
			//if the mask byte is 0 than we have 8 bit to skip
			for (unsigned b=0; b<8; b++) {
				shifted[shifted_i++] = -1;
			}
		}
	}


	// PROJECT
	vd_pointer	= new_Data->VD;
  dest				= (unsigned char*) vd_pointer;

	for (unsigned int i=0; i<end_sect_eq; i++) {
		src	= (unsigned char*) (VD+i*tid_list_size);

		masked_shrink(dest, src, mask, shifted, tid_list_size*sizeof(int), new_Data->tid_list_size*sizeof(int));

		vd_pointer += new_Data->tid_list_size;
		dest = (unsigned char*) vd_pointer;
	}

	new_Data->n_bit_set_eq = count_1_bits(new_Data->VD+new_Data->start_sect_eq*new_Data->tid_list_size,new_Data->sz_sect_eq);

	return new_Data;
}


template <class T> 
int DCI_simple_dataset<T>::masked_shrink (	unsigned char* dest,
																						unsigned char* src,
																						unsigned char* mask,
																						int* shifted,
																						unsigned int n0,
																						unsigned int n1){
	unsigned int shifted_i = 0;
	int i_b, i_B;
	bzero(dest, n1);
	int support = 0;
	for (unsigned int j=0; j<n0; j++) {
		unsigned char inter = mask[j] & src[j];
		if (inter!=0) {
			for (unsigned b=0; b<8; b++) {
				if ( (inter & mask_byte(b))!=0 ) {
					support++;
					i_B = shifted[shifted_i] / 8;
					i_b = shifted[shifted_i] % 8;
					dest[i_B] = (dest[i_B] | (0x1 << i_b));
				}
				shifted_i++;
			}
		} else {
			shifted_i+=8;
		}
	}
	return support;
}

template <class T> 
void DCI_simple_dataset<T>::chk_compact_vertical(int n_frequent_items){
  int start, end, cc=0, perc=0;
  unsigned int *mask, *last_row, *result;

  float perc_eq_limit[] = { 95.0, 90.0, 85.0, 80.0, 75.0};
  int max_perc_ind = sizeof(perc_eq_limit) / sizeof(float);
  int curr_perc = 0;

  int curr_perc_OK = 0;
  
  int start_OK, perc_OK;

  mask = new unsigned int [tid_list_size];
  result = new unsigned int [tid_list_size];

  for (unsigned int i=0; i<tid_list_size; i++)
    mask[i] = 0xffffffff;
  
  end = n_frequent_items;

  last_row = &VD[(end-20) * tid_list_size];

  start_OK = end-1;
  perc_OK = 0;

  start = end-1;

  while(start >= 0) {
    // ----------------------------------------------------
    // find the percentage of identical elements in 
    // the last start-end tidlists (most frequent items).
    float perc_tmp;
    int cc_tmp;
    bzero(result, tid_list_size*sizeof(unsigned int)); 
/*	
    for (unsigned int i=0; i<tid_list_size; i++) {
      result[i] = ~(VD[start * tid_list_size + i] ^ last_row[i]);
      result[i] = (result[i] & mask[i]);
    }
*/
    for (unsigned int i=0; i<tid_list_size; i++) {
      result[i] = (~VD[start * tid_list_size + i]) | last_row[i];//(VD[(start+1) * tid_list_size + i]);
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
      } else if ((end-start) < n_frequent_items/6)
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

	plog << " . . Section equal width  : " << sz_sect_eq << " / " << tid_list_size << endl;
	plog << " . . Section equal length : " << end_sect_eq-start_sect_eq << " / " << end_sect_eq << endl;
	plog << " * * Section equal width  : " << cc / (sizeof(int)*8) << " / " << tid_list_size << endl;
	plog << " * * Section equal length : " << end-start << " / " << end << endl;

		/*
  start_sect_eq=start;
  end_sect_eq=end;
  sz_sect_eq = cc / (sizeof(int)*8);
*/

/*
	if (end_sect_eq-start_sect_eq>1)
	  reorder_bits_compact(n_frequent_items, mask);
*/

  delete [] mask;
  delete [] result;
}


template <class T> 
void DCI_simple_dataset<T>::reorder_bits_compact(int n_frequent_items, 
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

}
#endif
