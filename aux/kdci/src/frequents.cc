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

#include "frequents.hh"

template class set_of_frequents<unsigned int, unsigned int>;
template class set_of_frequents<unsigned int, unsigned short int>;
template class set_of_frequents<unsigned int, unsigned char>;

template class set_of_frequents<unsigned short int, unsigned int>;
template class set_of_frequents<unsigned short int, unsigned short int>;
template class set_of_frequents<unsigned short int, unsigned char>;

template class set_of_frequents<unsigned char, unsigned int>;
template class set_of_frequents<unsigned char, unsigned short int>;
template class set_of_frequents<unsigned char, unsigned char>;


template <class T, class T1>
int set_of_frequents<T, T1>::init_gen_cand() {
  if (num_itemsets == 0) 
    return 0;

  curr_prefix = 0;
  init_sect = 0;
  end_sect = ind_section[curr_prefix];

  while (curr_prefix < ind_section.size()) 
    if(end_sect - init_sect < 1) {
      curr_prefix++;
      init_sect = end_sect+1;
      end_sect = ind_section[curr_prefix];
    } else {
      i1=init_sect;
      i2=i1+1;
      return 1;
    }
  
  return 0;
}


template <class T, class T1>
void set_of_frequents<T,T1>::get_prefix(T *v) {
  int ind_prefix = curr_prefix * (k-1);
  for (int i=0; i<k-1; i++)
    v[i] = prefixes[ind_prefix + i];
}

template <class T, class T1>
void set_of_frequents<T, T1>::get_suffix(T *v) {
  v[0] = suffixes[i1];  
  v[1] = suffixes[i2];
  // cout << "xxxxxx (" << curr_prefix << " - "
  //     << ind_section[curr_prefix] << ") " << i1 << " " << i2;
}

template <class T, class T1>
void set_of_frequents<T, T1>::get_suffix(T *v, T *key_pair, T1 *counts) {
  v[0] = suffixes[i1];  
  v[1] = suffixes[i2];
  
  key_pair[0] = keys[i1];
  key_pair[1] = keys[i2];
  counts[0] = counters[i1];
  counts[1] = counters[i2];
  // cout << "xxxxxx (" << curr_prefix << " - "
  //     << ind_section[curr_prefix] << ") " << i1 << " " << i2;
}


template <class T, class T1>
inline int set_of_frequents<T, T1>::next_cand() {
  i2++;
  if (i2 > end_sect) {
    i1++;
    i2 = i1+1;
  }
  if (i2 <= end_sect)
    return NEW_SUFFIX;
  else {
    curr_prefix++;
    while (curr_prefix < ind_section.size()) {
      init_sect = end_sect+1;
      end_sect = ind_section[curr_prefix];
      if(end_sect - init_sect < 1) {
	curr_prefix++;
      } else {
	i1=init_sect;
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

  for (int del=k-2; del >= 0; del--) {
    ret = my_bsearch(v, del, start);

    // cout << "ret " << ret << "- del=" << del << endl;
    if (ret == -1)
      return 0;
    else {
      int from, to;
      to = ind_section[ret];
      if (ret == 0)
	from = 0;
      else
	from = ind_section[ret-1]+1;
      
      int j;
      // cout << "f:" << from << " t:" << to << endl;
      for(j=from; j <= to; j++) {
	if (v[k] == suffixes[j])
	  break;
	else if (v[k] < suffixes[j])
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
  int sz_prefix=k-1;

  in = 0;
  fin= ind_section.size() - 1;
  
  bool found = false;

  while(fin >= in) {
    pivot = (in+fin) >> 1;
    j = pivot * sz_prefix;

    int i;
    for(i=0; i < sz_prefix;  i++) {
      if (prefixes[j] == v[i]) 
	{ j++; continue; }	
      if (prefixes[j] > v[i])	
	{ fin = pivot-1; break; }
      if (prefixes[j] < v[i])
	{ in = pivot+1; break; }
    }
    if (i==sz_prefix) {
      found = true;
      break;
    }
  }


    
  if (found) {
  
    int from, to;
    to = ind_section[pivot];
  
    if (pivot == 0)
      from = 0;
    else
      from = ind_section[pivot-1]+1;
      
    // cout << "f:" << from << " t:" << to << endl;

    if (to - from < 10) { // linear search
      for(j=from; j <= to; j++) {
	if (v[k-1] == suffixes[j]) {
	  key = keys[j];
	  count = (int) counters[j];
	  return 1;
	}
      }
    } else { // binary search

      while(to >= from) {
	pivot = (to + from) >> 1;
	if (v[k-1] == suffixes[pivot]) {
	  key = keys[pivot];
	  count = (int) counters[pivot];
	  return 1;
	}
	if (v[k-1] < suffixes[pivot])
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
  int sz=k-1;

  in = start;
  fin= ind_section.size() - 1;
  
  while(fin >= in) {
    pivot = (in+fin) >> 1;
    j = pivot*sz;

    int i;
    for(i=0; i < sz+1;  i++) {
      if (i != del) {  
	if (prefixes[j] == v[i]) 
	  { j++; continue; }	
	if (prefixes[j] > v[i])	
	  { fin = pivot-1; break; }
	if (prefixes[j] < v[i])
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
  if (num_itemsets == 0) {
    return;
  }
  
  //print_size();
  char *itemset_unmapped;
  int *itemset_unmapped_index;
  T* cache;

  cache = new T[k];
  cache[0] = cc.get_m1();

  const int SZ_NUM = 16;
  itemset_unmapped = new char[SZ_NUM * (k+1)]; // +1 for storing count
  itemset_unmapped_index = new int[k+1];
  itemset_unmapped_index[0] = 0;

  curr_prefix = 0;
  init_sect = 0;
  end_sect = ind_section[curr_prefix];

  while (1) {
    int ind_prefix = curr_prefix * (k-1);
    int i_cache;

    int i;
    for (i=0, i_cache=ind_prefix; i_cache<ind_prefix+(k-1); i++, i_cache++)
      if (prefixes[i_cache] != cache[i])
	break;

    
    int ind, num_written;
    for (int j=i_cache; j<ind_prefix+(k-1); j++, i++) {
      cache[i] = prefixes[j];
      ind = itemset_unmapped_index[i];
      memcpy(&itemset_unmapped[ind], cc.unmap_ascii[prefixes[j]], cc.unmap_ascii_len[prefixes[j]]);
      itemset_unmapped_index[i+1] = ind + cc.unmap_ascii_len[prefixes[j]];
    }

    for (unsigned int h=init_sect; h<=end_sect; h++) {
      ind = itemset_unmapped_index[i];
      memcpy(&itemset_unmapped[ind], cc.unmap_ascii[suffixes[h]], cc.unmap_ascii_len[suffixes[h]]);
      ind = ind + cc.unmap_ascii_len[suffixes[h]];
      num_written = sprintf(&itemset_unmapped[ind], "(%d)\n", 
			    (int) counters[h]);

      o.printSet(itemset_unmapped, ind+num_written);// print for each suffix[i]
    }


    
    curr_prefix++;
    if (curr_prefix == ind_section.size()) 
      break;
    init_sect = end_sect + 1;
    end_sect = ind_section[curr_prefix];
  }


  delete [] itemset_unmapped;
  delete [] itemset_unmapped_index;
  delete [] cache;
}


// template <class T, class T1>
// void set_of_frequents<T, T1>::dump_itemsets_bin(dci_items& cc, FSout& o) {
//   if (num_itemsets == 0) {
//     return;
//   }

//   T* cache;
//   unsigned int *iset;
//   cache = new T[k];
//   iset = new unsigned int[k];
//   cache[0] = cc.get_m1();

//   curr_prefix = 0;
//   init_sect = 0;
//   end_sect = ind_section[curr_prefix];

//   while (1) {
//     int ind_prefix = curr_prefix * (k-1)

;
//     int i_cache;

//     int i;
//     for (i=0, i_cache=ind_prefix; i_cache<ind_prefix+(k-1); i++, i_cache++)
//       if (prefixes[i_cache] != cache[i])
// 	break;

//     for (int j=i_cache; j<ind_prefix+(k-1); j++, i++) {
//       cache[i] = prefixes[j];
//       iset[i] = cc.unmap[prefixes[j]];
//     }

//     for (unsigned int n=init_sect; n<=end_sect; n++) {
//       iset[i] = cc.unmap[suffixes[k]];
//       o.printSet_bin(k, iset, (int) counters[n]);// print for each suffix[i]
//     }
    
//     curr_prefix++;
//     if (curr_prefix == ind_section.size()) 
//       break;
//     init_sect = end_sect + 1;
//     end_sect = ind_section[curr_prefix];
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
	while ( s<(int)suffixes.size() ){
		DA[direct_position2(prefixes[p],suffixes[s],m)] = s;
		if ( s == ind_section[p] )
			p++;
		s++;
	}
}
