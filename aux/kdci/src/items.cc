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

#include <iostream>
#include <algorithm>

#include "items.hh"
using namespace std;

template class set_of_itemsets<unsigned int, unsigned int>;
template class set_of_itemsets<unsigned int, unsigned short int>;
template class set_of_itemsets<unsigned int, unsigned char>;

template class set_of_itemsets<unsigned short int, unsigned int>;
template class set_of_itemsets<unsigned short int, unsigned short int>;
template class set_of_itemsets<unsigned short int, unsigned char>;

template class set_of_itemsets<unsigned char, unsigned int>;
template class set_of_itemsets<unsigned char, unsigned short int>;
template class set_of_itemsets<unsigned char, unsigned char>;


void dci_items::init_global_pruning()
{
  if (prune_global_mask == NULL)
    prune_global_mask = new unsigned int[m1];
  bzero(prune_global_mask, m1 * sizeof(unsigned int));
}

void dci_items::end_global_pruning(unsigned int k) 
{
  mk=0;
  for(unsigned int i=0; i<m1; i++)
    if (prune_global_mask[i] < k)
      unmap[i] = -1;
    else
      mk++;
}


bool dci_items::update_map() 
{
  // cout << "--m1=" << m1 << endl;
  // cout << "--mk=" << mk << endl;

  if (m1 == mk)
    return false;

  unsigned int j=0;
  for (unsigned int i=0; i<m1; i++)
    if (unmap[i] != -1)
      map[i] = j++;

  return true;
}


bool dci_items::update_unmap(bool write_output) 
{
  //  cout << "==m1=" << m1 << endl;
  // cout << "==mk=" << mk << endl;

  if (m1 == mk)
    return false;


//   for (unsigned int i=0; i<m1; i++) {
//     printf("unmap[%d]->%s++\n", i, unmap_ascii[i]);
//     if (strlen(unmap_ascii[i]) != unmap_ascii_len[i])
//       printf("  diff len %d-%d\n", strlen(unmap_ascii[i]), unmap_ascii_len[i]);
//   }

  unsigned int j=0;
  for (unsigned int i=0; i<m1; i++)
    if (unmap[i] != -1) {
      if (i != j) {
	unmap[j] = unmap[i];

	if (write_output) {
	  int len = unmap_ascii_len[i];
	  memcpy(unmap_ascii[j], unmap_ascii[i], len);
	  unmap_ascii[j][len] = 0;

	  unmap_ascii_len[j] =  unmap_ascii_len[i];
	}
      }
      j++;
    }

  //  if (j != mk)
  //  cout << "ERRORE mk\n";
 
  m1=mk;

//   for (unsigned int i=0; i<mk; i++)
//     printf("unmap[%d]->%s++\n", i, unmap_ascii[i]);


  return true;
}

int dci_items::remap_items(bool write_output)
  // sort items according to their support
  // and maps their id to contiguous and increasing integers.
  // in this function we build a map from original_id to new_id
  // and a reverse "unmap" from new_id to original. 
  // also, we build a map from the new_id to the ascii representation 
  // of the item, so we have it at hand when (if ever) we will have to write
  // frequent itemsets to disk.
{
  m = (*acc).size();
	
  // add the item_id into the pairs; update m1 and avg_supp_perc
  m1=0;

  // counts how many items have support higher than a give threshold
  // and which is their average support. this info is used later to 
  // determine if it is worth using the key-pattern optimization
  avg_supp_at_threshold = 0.;
  int n=0;
  unsigned int count_at_threshold =  (unsigned int) (nr_t*SUPP_THRESHOLD/100.);
  

  for (unsigned int i=0; i < (*acc).size(); i++) {
    (*acc)[i].id = i;
    if ((*acc)[i].count >= min_count) {
      m1++;
      if ((*acc)[i].count > max_supp)
	max_supp = (*acc)[i].count;
      // ----------------------------------------------------
      // average support of items that pass a given threshold
      if ((*acc)[i].count > count_at_threshold) {
	avg_supp_at_threshold += (*acc)[i].count;
	n++;
      }
      // ----------------------------------------------------
    }
  }

  // ----------------------------------------------------
  // average support of items that pass a given threshold
  if (n > 0)
    avg_supp_at_threshold = 100.*avg_supp_at_threshold/n/nr_t;
  else 
    avg_supp_at_threshold = 0.;
  // ----------------------------------------------------

  AscendingItemsSort compare_item;
  sort((*acc).begin(), (*acc).end(), compare_item);

  map   = new int[m];
  unmap = new int[m1];

  for (unsigned int i=0; i < m; i++) 
    map[i] = -1;
  
  int start = m - m1;

  if (write_output) {
    unmap_ascii = new char[m1][8];
    unmap_ascii_len = new unsigned char[m1];
  }

  for (unsigned int i=start; i < m; i++) {
    unmap[i-start] = (*acc)[i].id;
    if (write_output)
      unmap_ascii_len[i-start] = sprintf(&unmap_ascii[i-start][0], 
					 "%d ", (*acc)[i].id);
     
    map[(*acc)[i].id] = i-start;
  }


#ifdef DEBUG
  for (unsigned int i=0; i < m1; i++) 
    cout << "unmap[" << i << "]=" << unmap[i] << endl;
#endif
	
  mk = m1;
  return start;

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





