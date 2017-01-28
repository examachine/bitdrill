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

#ifndef __KDCI_FREQUENTS_H
#define __KDCI_FREQUENTS_H

#include "direct_count.hh"
#include "items.hh"
#include "database.hh"

namespace KDCI {

const int NEW_PREFIX = 1;
const int NEW_SUFFIX = 2;
const int END_GEN_CAND = 0;

template <class T, class T1>
class set_of_frequents : public set_of_itemsets<T,T1> {
public:
  typedef set_of_itemsets<T,T1> base;

  set_of_frequents(int iter) : set_of_itemsets<T, T1>(iter) {}
  int get_num_frequents() {return base::get_num_itemsets();}

  void dump_itemsets(dci_items& counters, FSout& o);
  //  void dump_itemsets_bin(dci_items& cc, FSout& o);

  void printf_freq();

  int init_gen_cand();
  void get_prefix(T *v);
  void get_suffix(T *v);
  void get_suffix(T *v, T *keys, T1 *counts);
  int next_cand();

  int find_subsets(T *v);
  int find_one_subset(T *v, T& key, int& count);


  /** Direct acces structure for DCP in the 3th iteration */
  vector<int> DA;
	/** size of di direct access structure (unnecessary ???) */
  int DA_m;
	/** Initializes the direct access structure used for
	  * a fast DCP in the 3rd iteration.
		* for each itemset(i0, i1) its position is stored.
		*/
	void initDA(int m);



private:

  unsigned int i1, i2;

  inline int my_bsearch(T *v, int del, int start);
}; 


  /*
template class set_of_frequents<unsigned int, unsigned int>;
template class set_of_frequents<unsigned int, unsigned short int>;
template class set_of_frequents<unsigned int, unsigned char>;

template class set_of_frequents<unsigned short int, unsigned int>;
template class set_of_frequents<unsigned short int, unsigned short int>;
template class set_of_frequents<unsigned short int, unsigned char>;

template class set_of_frequents<unsigned char, unsigned int>;
template class set_of_frequents<unsigned char, unsigned short int>;
template class set_of_frequents<unsigned char, unsigned char>;
  */

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
  // cout << "xxxxxx (" << this->curr_prefix << " - "
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
  // cout << "xxxxxx (" << this->curr_prefix << " - "
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


template <class T, class T1>
inline int set_of_frequents<T, T1>::find_subsets(T *v)
{
  int ret, start=0;

  for (int del=this->k-2; del >= 0; del--) {
    ret = my_bsearch(v, del, start);

    // cout << "ret " << ret << "- del=" << del << endl;
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
      // cout << "f:" << from << " t:" << to << endl;
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
      
    // cout << "f:" << from << " t:" << to << endl;

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
}


// template <class T, class T1>
// void set_of_frequents<T, T1>::dump_itemsets_bin(dci_items& cc, FSout& o) {
//   if (this->num_itemsets == 0) {
//     return;
//   }

//   T* cache;
//   unsigned int *iset;
//   cache = new T[k];
//   iset = new unsigned int[k];
//   cache[0] = cc.get_m1();

//   this->curr_prefix = 0;
//   this->init_sect = 0;
//   this->end_sect = this->ind_section[this->curr_prefix];

//   while (1) {
//     int ind_prefix = this->curr_prefix * (k-1)

;
//     int i_cache;

//     int i;
//     for (i=0, i_cache=ind_prefix; i_cache<ind_prefix+(k-1); i++, i_cache++)
//       if (this->prefixes[i_cache] != cache[i])
// 	break;

//     for (int j=i_cache; j<ind_prefix+(k-1); j++, i++) {
//       cache[i] = this->prefixes[j];
//       iset[i] = cc.unmap[this->prefixes[j]];
//     }

//     for (unsigned int n=this->init_sect; n<=this->end_sect; n++) {
//       iset[i] = cc.unmap[this->suffixes[k]];
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
	DA.resize(m*(m-1)/2+1);
  for (int i=0; i<(m*(m-1)/2+1); i++)  
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


} // namespace

#endif
