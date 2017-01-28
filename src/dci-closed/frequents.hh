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

#ifndef __DCICLOSED_FREQUENTS_H
#define __DCICLOSED_FREQUENTS_H

#include "direct_count.hh"
#include "items.hh"
#include "database.hh"

namespace DCIClosed {

const int NEW_PREFIX = 1;
const int NEW_SUFFIX = 2;
const int END_GEN_CAND = 0;


template <class T, class T1>
class set_of_frequents : public set_of_itemsets<T,T1> {
public:
  set_of_frequents(int iter) : set_of_itemsets<T, T1>(iter) {}
  int get_num_frequents() {return this->num_itemsets;}

  void dump_itemsets(dci_items& counters, FSout& o);
	void dump_itemsets(dci_items& cc, FSout& o, int &isets);
  //  void dump_itemsets_bin(dci_items& cc, FSout& o);

  void printf_freq();

  int init_gen_cand();
  void get_prefix(T *v);
  void get_suffix(T *v);
  void get_suffix(T *v, T *keys, T1 *counts);
  int next_cand();

  int find_subsets(T *v);
  int find_one_subset(T *v, T& key, int& count);

	void check_closed(T *v, int count);

	/** retrives the key and minimum support between
	  * all the subsets but the generators.
		* key is set to -1, if no non key patterns are found
		*   otherwise it's set to the key flag of the first non key pattern found.
		* min_count is set to the support of the first non key pattern found.
		* 0 is returned if an infrequent subset is fooud, 1 otherwise.
  	*/
  // int find_subsets(T *v, T& key, int& min_count);


  /** Direct acces structure for DCP in the 3th iteration */
  vector<int> DA;
	/** size of di direct access structure (unnecessary ???) */
  int DA_m;
	/** Initializes the direct access structure used for
	  * a fast DCP in the 3rd iteration.
		* for each itemset(i0, i1) its position is stored.
		*/
	void initDA(int m);

	void reset(int iter){
		set_of_itemsets<T,T1>::reset(iter);
		DA.resize(0);
		DA_m = 0;
	}



	vector<bool> to_output;
	void init_to_output(void) {
		to_output.resize(this->num_itemsets);
		for (int i=0; i<this->num_itemsets; i++)
			to_output[i] = true;
	}

private:
   unsigned int i1, i2;


  inline int my_bsearch(T *v, int del, int start);
  inline int my_bsearch(T *v, int del, int start, int end);
}; 

/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

template <class T, class T1>
int set_of_frequents<T, T1>::init_gen_cand() {
  if (this->num_itemsets == 0) 
    return 0;

  this->curr_prefix = 0;
  this->init_sect = 0;
  this->end_sect = this->ind_section[this->curr_prefix];

  while (this->curr_prefix < this->ind_section.size()) 
    if(this->end_sect - this->init_sect < 1) {
      this->curr_prefix++;
      this->init_sect = this->end_sect+1;
      this->end_sect = this->ind_section[this->curr_prefix];
    } else {
      i1=this->init_sect;
      i2=i1+1;
      return 1;
    }
  
  return 0;
}


template <class T, class T1>
void set_of_frequents<T,T1>::get_prefix(T *v) {
  int ind_prefix = this->curr_prefix * (this->k-1);
  for (int i=0; i<this->k-1; i++)
    v[i] = this->prefixes[ind_prefix + i];
}

template <class T, class T1>
void set_of_frequents<T, T1>::get_suffix(T *v) {
  v[0] = this->suffixes[i1];  
  v[1] = this->suffixes[i2];
  // plog << "xxxxxx (" << this->curr_prefix << " - "
  //     << this->ind_section[this->curr_prefix] << ") " << i1 << " " << i2;
}

template <class T, class T1>
void set_of_frequents<T, T1>::get_suffix(T *v, T *key_pair, T1 *counts) {
  v[0] = this->suffixes[i1];  
  v[1] = this->suffixes[i2];
  
  key_pair[0] = this->keys[i1];
  key_pair[1] = this->keys[i2];
  counts[0] = this->counters[i1];
  counts[1] = this->counters[i2];
  // plog << "xxxxxx (" << this->curr_prefix << " - "
  //     << this->ind_section[this->curr_prefix] << ") " << i1 << " " << i2;
}


template <class T, class T1>
inline int set_of_frequents<T, T1>::next_cand() {
  i2++;
  if (i2 > this->end_sect) {
    i1++;
    i2 = i1+1;
  }
  if (i2 <= this->end_sect)
    return NEW_SUFFIX;
  else {
    this->curr_prefix++;
    while (this->curr_prefix < this->ind_section.size()) {
      this->init_sect = this->end_sect+1;
      this->end_sect = this->ind_section[this->curr_prefix];
      if(this->end_sect - this->init_sect < 1) {
	this->curr_prefix++;
      } else {
	i1=this->init_sect;
	i2=i1+1;
	return NEW_PREFIX;
      }
    }

    return END_GEN_CAND; 
  }
}


/*
template <class T, class T1>
inline int set_of_frequents<T, T1>::find_subsets(T *v, T& key, int& min_count){
	T* subset= new T[this->k-1];
	int j, found, supp;
	T min_key;

	min_count = INT_MAX;
	min_key = (T)-1;
	
	// for each subset 
  for (int del=0; del < this->k-1; del++) {
		// build the subset
		j=0;
		for (int i=0; j<k; i++)
			if (i!=del)
				subset[j++] = v[i];

		// retrieve subset
		found = find_one_subset(subset, min_key, supp);
		if (found==0) {				// infrequent subset found
			delete [] subset;
			return 0;
		}
	
		// if the subset is not a key pattern
		if (min_key != (T) -1) {
			key = min_key;
			min_count = supp;

			delete [] subset;
			return 1;
		} 
		// if the subset is a key patter
		else {
			if (supp < min_count) {
				key = min_key;
				min_count = supp;
			}
		}

	} // end for

	delete [] subset;
	return 1;
}

*/

template <class T, class T1>
inline int set_of_frequents<T, T1>::find_subsets(T *v)
{
  int ret, start=0;

  for (int del=this->k-2; del >= 0; del--) {
    ret = my_bsearch(v, del, start);

    // plog << "ret " << ret << "- del=" << del << endl;
    if (ret == -1)
      return 0;
    else {
      int from, to;
      to = this->ind_section[ret];
      if (ret == 0)
	from = 0;
      else
	from = this->ind_section[ret-1]+1;
      
      int j;
      // plog << "f:" << from << " t:" << to << endl;
      for(j=from; j <= to; j++) {
	if (v[this->k] == this->suffixes[j])
	  break;
	else if (v[this->k] < this->suffixes[j])
	  return 0;
      }
      if (j <= to)  // elem found
	start = ret+1;
      else
	return 0;
    }

  }
  return 1;
}





template <class T, class T1>
inline int set_of_frequents<T,T1>::my_bsearch(T *v, int del, int start, int end)
{
  int in, fin, j, pivot;
  int sz=this->k-1;
  int i;

  in  = start;
  fin = end;
  
  while(fin >= in) {
    pivot = (in+fin) >> 1;
    j = pivot*sz;

    for(i=0; i < sz+1;  i++) {
      if (i != del) {  
				if (this->prefixes[j] == v[i]) 
					{ j++; continue; }	
				if (this->prefixes[j] > v[i])	
					{ fin = pivot-1; break; }
				if (this->prefixes[j] < v[i])
					{ in = pivot+1; break; }
      }
    }
    if (i==sz+1)
      return pivot;
  }
  return -1;
}








template <class T, class T1>
inline int set_of_frequents<T, T1>::find_one_subset(T *v, T& key, int& count)
{
  int in, fin, j, pivot=0;
  int sz_prefix=this->k-1;

  in = 0;
  fin= this->ind_section.size() - 1;
  
  bool found = false;

  while(fin >= in) {
    pivot = (in+fin) >> 1;
    j = pivot * sz_prefix;

    int i;
    for(i=0; i < sz_prefix;  i++) {
      if (this->prefixes[j] == v[i]) 
	{ j++; continue; }	
      if (this->prefixes[j] > v[i])	
	{ fin = pivot-1; break; }
      if (this->prefixes[j] < v[i])
	{ in = pivot+1; break; }
    }
    if (i==sz_prefix) {
      found = true;
      break;
    }
  }


    
  if (found) {
  
    int from, to;
    to = this->ind_section[pivot];
  
    if (pivot == 0)
      from = 0;
    else
      from = this->ind_section[pivot-1]+1;
      
    // plog << "f:" << from << " t:" << to << endl;

    if (to - from < 10) { // linear search
      for(j=from; j <= to; j++) {
	if (v[this->k-1] == this->suffixes[j]) {
	  key = this->keys[j];
	  count = (int) this->counters[j];
	  return 1;
	}
      }
    } else { // binary search

      while(to >= from) {
	pivot = (to + from) >> 1;
	if (v[this->k-1] == this->suffixes[pivot]) {
	  key = this->keys[pivot];
	  count = (int) this->counters[pivot];
	  return 1;
	}
	if (v[this->k-1] < this->suffixes[pivot])
	  to = pivot-1;
	else
	  from = pivot+1;
      }
    }
  }
  
  return 0;
}



template <class T, class T1>
inline 	void set_of_frequents<T, T1>::check_closed(T *v, int count)
{
  int in, fin, j, pivot=0;
  int sz_prefix=this->k-1;

  in = 0;
  fin= this->ind_section.size() - 1;
  
  bool found = false;

  while(fin >= in) {
    pivot = (in+fin) >> 1;
    j = pivot * sz_prefix;

    int i;
    for(i=0; i < sz_prefix;  i++) {
      if (this->prefixes[j] == v[i]) 
	{ j++; continue; }	
      if (this->prefixes[j] > v[i])	
	{ fin = pivot-1; break; }
      if (this->prefixes[j] < v[i])
	{ in = pivot+1; break; }
    }
    if (i==sz_prefix) {
      found = true;
      break;
    }
  }


    
  if (found) {
  
    int from, to;
    to = this->ind_section[pivot];
  
    if (pivot == 0)
      from = 0;
    else
      from = this->ind_section[pivot-1]+1;
      
    // plog << "f:" << from << " t:" << to << endl;

    if (to - from < 10) { // linear search
      for(j=from; j <= to; j++) {
	if (v[this->k-1] == this->suffixes[j]) {
		if (count == (int)this->counters[j])
			to_output[j]=false;
//	  key = this->keys[j];
//	  count = (int) this->counters[j];
	  return;
	}
      }
    } else { // binary search

      while(to >= from) {
	pivot = (to + from) >> 1;
	if (v[this->k-1] == this->suffixes[pivot]) {
		if (count == (int)this->counters[pivot])
			to_output[pivot]=false;
//	  key = this->keys[pivot];
//	  count = (int) this->counters[pivot];
	  return;
	}
	if (v[this->k-1] < this->suffixes[pivot])
	  to = pivot-1;
	else
	  from = pivot+1;
      }
    }
  }
  
  return;
}

template <class T, class T1>
inline int set_of_frequents<T,T1>::my_bsearch(T *v, int del, int start)
{
  int in, fin, j, pivot;
  int sz=this->k-1;

  in = start;
  fin= this->ind_section.size() - 1;
  
  while(fin >= in) {
    pivot = (in+fin) >> 1;
    j = pivot*sz;

    int i;
    for(i=0; i < sz+1;  i++) {
      if (i != del) {  
	if (this->prefixes[j] == v[i]) 
	  { j++; continue; }	
	if (this->prefixes[j] > v[i])	
	  { fin = pivot-1; break; }
	if (this->prefixes[j] < v[i])
	  { in = pivot+1; break; }
      }
    }
    if (i==sz+1)
      return pivot;
  }
  return -1;
}


template <class T, class T1>
void set_of_frequents<T, T1>::dump_itemsets(dci_items& cc, FSout& o, int &isets) {
	isets = 0;
	int kk = 0;
  if (this->num_itemsets == 0) {
    return;
  }
  
  //print_size();
  char *itemset_unmapped;
  int *itemset_unmapped_index;
  T* cache;

  cache = new T[this->k];
  cache[0] = cc.get_m1();

  const int SZ_NUM = 16;
  itemset_unmapped = new char[SZ_NUM * (this->k+1)]; // +1 for storing count
  itemset_unmapped_index = new int[this->k+1];
  itemset_unmapped_index[0] = 0;

  this->curr_prefix = 0;
  this->init_sect = 0;
  this->end_sect = this->ind_section[this->curr_prefix];

  while (1) {
    int ind_prefix = this->curr_prefix * (this->k-1);
    int i_cache;

    int i;
    for (i=0, i_cache=ind_prefix; i_cache<ind_prefix+(this->k-1); i++, i_cache++)
      if (this->prefixes[i_cache] != cache[i])
	break;

    
    int ind, num_written;
    for (int j=i_cache; j<ind_prefix+(this->k-1); j++, i++) {
      cache[i] = this->prefixes[j];
      ind = itemset_unmapped_index[i];
      memcpy(&itemset_unmapped[ind], cc.unmap_ascii[this->prefixes[j]], cc.unmap_ascii_len[this->prefixes[j]]);
      itemset_unmapped_index[i+1] = ind + cc.unmap_ascii_len[this->prefixes[j]];
    }

    for (unsigned int h=this->init_sect; h<=this->end_sect; h++) {
      ind = itemset_unmapped_index[i];
      memcpy(&itemset_unmapped[ind], cc.unmap_ascii[this->suffixes[h]], cc.unmap_ascii_len[this->suffixes[h]]);
      ind = ind + cc.unmap_ascii_len[this->suffixes[h]];
      num_written = sprintf(&itemset_unmapped[ind], "(%d)\n", 
			    (int) this->counters[h]);

			if (to_output[kk++]){
				isets++;
				o.printSet(itemset_unmapped, ind+num_written);// print for each suffix[i]
			}
    }


    
    this->curr_prefix++;
    if (this->curr_prefix == this->ind_section.size()) 
      break;
    this->init_sect = this->end_sect + 1;
    this->end_sect = this->ind_section[this->curr_prefix];
  }


  delete [] itemset_unmapped;
  delete [] itemset_unmapped_index;
  delete [] cache;
}


template <class T, class T1>
void set_of_frequents<T, T1>::dump_itemsets(dci_items& cc, FSout& o) {
  if (this->num_itemsets == 0) {
    return;
  }
  
  //print_size();
  char *itemset_unmapped;
  int *itemset_unmapped_index;
  T* cache;

  cache = new T[this->k];
  cache[0] = cc.get_m1();

  const int SZ_NUM = 16;
  itemset_unmapped = new char[SZ_NUM * (this->k+1)]; // +1 for storing count
  itemset_unmapped_index = new int[this->k+1];
  itemset_unmapped_index[0] = 0;

  this->curr_prefix = 0;
  this->init_sect = 0;
  this->end_sect = this->ind_section[this->curr_prefix];

  while (1) {
    int ind_prefix = this->curr_prefix * (this->k-1);
    int i_cache;

    int i;
    for (i=0, i_cache=ind_prefix; i_cache<ind_prefix+(this->k-1); i++, i_cache++)
      if (this->prefixes[i_cache] != cache[i])
	break;

    
    int ind, num_written;
    for (int j=i_cache; j<ind_prefix+(this->k-1); j++, i++) {
      cache[i] = this->prefixes[j];
      ind = itemset_unmapped_index[i];
      memcpy(&itemset_unmapped[ind], cc.unmap_ascii[this->prefixes[j]], cc.unmap_ascii_len[this->prefixes[j]]);
      itemset_unmapped_index[i+1] = ind + cc.unmap_ascii_len[this->prefixes[j]];
    }

    for (unsigned int h=this->init_sect; h<=this->end_sect; h++) {
      ind = itemset_unmapped_index[i];
      memcpy(&itemset_unmapped[ind], cc.unmap_ascii[this->suffixes[h]], cc.unmap_ascii_len[this->suffixes[h]]);
      ind = ind + cc.unmap_ascii_len[this->suffixes[h]];
      num_written = sprintf(&itemset_unmapped[ind], "(%d)\n", 
			    (int) this->counters[h]);

      o.printSet(itemset_unmapped, ind+num_written);// print for each suffix[i]
    }
    
    this->curr_prefix++;
    if (this->curr_prefix == this->ind_section.size()) 
      break;
    this->init_sect = this->end_sect + 1;
    this->end_sect = this->ind_section[this->curr_prefix];
  }


  delete [] itemset_unmapped;
  delete [] itemset_unmapped_index;
  delete [] cache;

	/*
	init_read_itemsets();
	T* itemset = new T[this->k];
	int ind;
	T1 ccc;

	while ((ind = read_next_itemset(itemset, ccc))!= -1) {
		for (int i=0; i<k; i++)
			plog << (int) itemset[i] << " ";
		plog << "\t (" << (int) ccc << ")";
		plog << "\t k:" << (int) this->keys[ind] << endl;
	}
	delete [] itemset;
	*/
}


// template <class T, class T1>
// void set_of_frequents<T, T1>::dump_itemsets_bin(dci_items& cc, FSout& o) {
//   if (this->num_itemsets == 0) {
//     return;
//   }

//   T* cache;
//   unsigned int *iset;
//   cache = new T[this->k];
//   iset = new unsigned int[this->k];
//   cache[0] = cc.get_m1();

//   this->curr_prefix = 0;
//   this->init_sect = 0;
//   this->end_sect = this->ind_section[this->curr_prefix];

//   while (1) {
//     int ind_prefix = this->curr_prefix * (this->k-1)


//     int i_cache;

//     int i;
//     for (i=0, i_cache=ind_prefix; i_cache<ind_prefix+(this->k-1); i++, i_cache++)
//       if (this->prefixes[i_cache] != cache[i])
// 	break;

//     for (int j=i_cache; j<ind_prefix+(this->k-1); j++, i++) {
//       cache[i] = this->prefixes[j];
//       iset[i] = cc.unmap[this->prefixes[j]];
//     }

//     for (unsigned int n=this->init_sect; n<=this->end_sect; n++) {
//       iset[i] = cc.unmap[this->suffixes[this->k]];
//       o.printSet_bin(k, iset, (int) this->counters[n]);// print for each suffix[i]
//     }
    
//     this->curr_prefix++;
//     if (this->curr_prefix == this->ind_section.size()) 
//       break;
//     this->init_sect = this->end_sect + 1;
//     this->end_sect = this->ind_section[this->curr_prefix];
//   }


//   delete [] cache;
//   delete [] iset;
// }


template <class T, class T1>
void set_of_frequents<T, T1>::initDA(int m) { 
	DA_m = m;
	DA.resize(m*(m-1)/2);
  for (int i=0; i<(m*(m-1)/2); i++)  
		DA[i]=-1;

	int p = 0; /* prefix */
	int s = 0; /* suffix */
	while ( s<(int)this->suffixes.size() ){
		DA[direct_position2(this->prefixes[p],this->suffixes[s],m)] = s;
		if ( s == this->ind_section[p] )
			p++;
		s++;
	}
}

/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
     debug functions
   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
/*

template <class T, class T1>
void set_of_frequents<T, T1>::pprint(){
	plog << endl;
	plog << "frequents: " << endl;
	plog << "iteration    = " << k            << endl;
	plog << "this->num_itemsets = " << this->num_itemsets << endl;
	plog << "num_this->prefixes = " << this->prefixes.size()/(this->k-1) << endl;
	plog << " this->prefixes: ";
	int h=0;
	for (unsigned int i=0; i<this->prefixes.size(); i++){
		if ((i%(this->k-1))==0) {
			plog << endl << h++ <<": ";
		}
		plog << "  " << (int) this->prefixes[i];
	}
	plog << endl;

	plog << "num:this->suffixes = " << this->suffixes.size()       << endl;
	plog << " this->suffixes: " << endl;
	for (unsigned int i=0; i< this->suffixes.size(); i++) {
		plog << i << ": " << (int) this->suffixes[i] << endl;
	}
	plog << endl;

	plog << " this->counters: " << endl;
	for (unsigned int i=0; i< this->counters.size(); i++) {
		plog << i << ": " << (int) this->counters[i] << endl;
	}
	plog << endl;

	plog << " this->ind_section: " << endl;
	for (unsigned int i=0; i<this->ind_section.size(); i++) {
		plog << "  " << this->ind_section[i];
	}
	plog << endl;

	plog << " dcp candidates." << endl;
}

*/

}
#endif
