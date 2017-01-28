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

#ifndef __CLOSURES_H
#define __CLOSURES_H

#include "transaction.hh"
#include "items.hh"
#include <iostream>
#include <stdio.h>


namespace DCIClosed {

template <class T>
class closures{
public:
  /** counstructor */
  closures(char* out_filename);
  /** destructor */
  ~closures();

  /** returns the number of entry reserved for closed sets */
  inline unsigned int getNumClosures(void);
  /** prints closed sets of the current iteration */
  void dump(dci_items* counters);

  /** prints closed sets of the current iteration */
  void console(unsigned int pos);


  // DCP_initializations
  // 1-long itemsets closures
  /** reserve num_items entries for the closure of 1-long itemsets */
  void reserve_1closure(unsigned int num_items);
  /** updates the closure of the item-th itemset intersecting with transaction */
  void add_1closure(T item, DCIClosed::Transaction<T>* transaction);
  /** updates the closure all the items in transaction */
  void add_1closure(DCIClosed::Transaction<T>* transaction);

  // 2-long itemsets closures
  /** reserve num_items entries for the closure of 2-long candidates itemsets */
  void reserve_2closure(unsigned int num_itemsets);
  /** updates the closure of the item-th 2-itemset intersecting with transaction */
  void add_2closure(unsigned int pos, DCIClosed::Transaction<T>* transaction);
  /** deletes the closure of the pos-th candidate itemset */
  void delete_2closure(unsigned int pos);

  // n-long itemsets closures
  /** reserve num_items entries for the closure of length-long candidates itemsets */
  void reserve_Nclosure(unsigned int length, unsigned int num_itemsets);
  /** updates the closure of the item-th itemset intersecting with transaction */
  void add_Nclosure(unsigned int pos, dci_transaction<T>* transaction);
  /** deletes the closure of the pos-th candidate itemset */
  void delete_Nclosure(unsigned int pos);

  // DCI_initializations
  /** inistialize count_index */
  void PreDCI(dci_items &counters);
  void setIter(unsigned int iter);

  vector<T>* getPlace(void);
  void getFromPrevious(unsigned int pos, vector<T>& ret);
  void mergeFromPrevious(unsigned int i1, unsigned int i2, vector<T>& ret);

  int checkCountIndex(T* cand, int supp);
  void addCountIndex(int supp);
  void addCountIndex(int supp, unsigned int i);
  void copyAppend(int pos);

  // supports
  /** appendes the support supp the the list of supports */
  inline void addSupport(unsigned int supp);

	// duplicates
  void toBePrinted_2ndIteration();
  inline void toBePrinted(bool val);

private:

  /** Intersects c with elems and the result is stored in c */
  void selfIntersect(vector<T>* c, T* elems, unsigned int len);

  /** check Inclusion */
  bool isIncluded(T* b, vector<T>* a);

  /** all the closures */
  vector< vector<T> > my_closures;
  /** supports of each closure */
  vector<unsigned int> supports;

  /** index to the first closure of a 2-itemset */
  unsigned int start2closures;
  /** index to the first closure of the current iter-itemset */
  unsigned int startNclosures;
  /** index to the first closure of the previous iteration */
  unsigned int startPclosures;
  /** 0 or current iteration if iteration is greater than 2 */
  unsigned int N;

  /** vector for the indixing of closed sets with same support*/
  vector< vector<int> > count_index;
  int min_supp;

  /** to be printed or not, repetitions must not */
  vector<bool> print;

  int checkCountIndex2ndIteration(vector<T> &cand, int supp);


  /** output file */
  FSout* out;
};

/** Constructor
  * Initializes N (current iteration) to 0
  * and output filename
  */
template <class T>
closures<T>::closures(char* out_filename){
  N=0;
  out = new FSout(out_filename, 1);
}

/** Destructor
  * Closes Out file
  */
template <class T>
closures<T>::~closures(){
  delete out;
}


/** Return the number of entries reserve for storing closures.
  * after a complete iteration it's the number of closed sets */
template <class T>
inline unsigned int closures<T>::getNumClosures(void){
  return my_closures.size();
}

/** creates num_items entries for storing closed sets */
template <class T>
void closures<T>::reserve_1closure(unsigned int num_items) {
  if (my_closures.size()<num_items)
    my_closures.resize(num_items);
}

/** intersects (if needed) the transactio with the item-th closed set */
template <class T>
void closures<T>::add_1closure(T item, DCIClosed::Transaction<T>* transaction){
  // closure has already been found
  if (my_closures[item].size() == 1)
    return;

  // there's something to intersect
  if (my_closures[item].size() > 1) { 
    selfIntersect(&(my_closures[item]), transaction->t, transaction->length);
  } 
  // first transaction for the closure
  //    my_closures[item].size() == 0
  else { 
    my_closures[item].resize(transaction->length);
    for(unsigned int j=0; j<transaction->length; j++)
      my_closures[item][j] = transaction->t[j];
  } 
}

/** for each item in the transaction performs a call
  * to the previous function to update closure.
  */
template <class T>
void closures<T>::add_1closure(DCIClosed::Transaction<T>* transaction){
  for (unsigned int i=0; i<transaction->length; i++)
    add_1closure(transaction->t[i], transaction);
}

/** prints the closure of the current iteration.
  * this function has to be called after every iteration,
  * but before items are remapped, otherwise
  * datas in counters->unmap would be wrong.
  * NB: it has to be called for the first time
  * after second iteration.
  */
template <class T>
void closures<T>::dump(dci_items* counters){
  int st;
  if (N==0)    st = 0;
  else         st = startNclosures;
  
  
  char* itemset_unmapped;
  int*  itemset_unmapped_index;
  T*    cache;
  const int MAX_LEN = 40;

  cache = new T[MAX_LEN];
  cache[0] = counters->get_m1();

  const int SZ_NUM = 16;
  itemset_unmapped = new char[SZ_NUM * MAX_LEN]; // +1 for storing count
  itemset_unmapped_index = new int[MAX_LEN];
  itemset_unmapped_index[0] = 0;
  
  unsigned int div;
  int ind, num_written;

  for (unsigned int i=st; i<my_closures.size(); i++) {
    if (print[i]) {
      for (div=0; div<my_closures[i].size(); div++)
        if (my_closures[i][div] != cache[div])
	        break;

      for (;div<my_closures[i].size(); div++){
        cache[div] = my_closures[i][div];
        ind = itemset_unmapped_index[div];
        memcpy(&itemset_unmapped[ind], counters->unmap_ascii[cache[div]], counters->unmap_ascii_len[cache[div]]);
        itemset_unmapped_index[div+1] = ind + counters->unmap_ascii_len[cache[div]];
      }

      ind = itemset_unmapped_index[div];
      num_written = sprintf(&itemset_unmapped[ind], ": %d\n", (int) supports[i]);

      out->printSet(itemset_unmapped, ind+num_written);
    }
  }
    

  delete [] itemset_unmapped;
  delete [] itemset_unmapped_index;
  delete [] cache;
}

/** The pointer to the first entry of 2-itemset closures
  * is set, and additional num_itemsets entries are reserved.
  * (candidates are supposed to be stored now)
  */
template <class T>
void closures<T>::reserve_2closure(unsigned int num_itemsets){
  start2closures = my_closures.size();
  my_closures.resize(my_closures.size()+num_itemsets);
}

/** as add_1closure */
template <class T>
void closures<T>::add_2closure(unsigned int pos, DCIClosed::Transaction<T>* transaction){
  pos += start2closures;
  // closure has already been found
  if (my_closures[pos].size() == 2)
    return;

  // there's something to intersect
  if (my_closures[pos].size() > 2) { 
    selfIntersect(&(my_closures[pos]), transaction->t, transaction->length);
  } 
  // first transaction for the closure
  //    my_closures[item].size() == 0
  else { 
    my_closures[pos].resize(transaction->length);
    for(unsigned int j=0; j<transaction->length; j++)
      my_closures[pos][j] = transaction->t[j];
  } 
}

/** Deletes the pos-th entry after the first 2-itemset closure.
  * it is supposed to be done when we have to delete
  * the closure of an itemset found to be infrequent.
  */
template <class T>
void closures<T>::delete_2closure(unsigned int pos){
  pos += start2closures;
  my_closures.erase(my_closures.begin() + pos);
}

/** Performs an ordered intersection between c and elems,
  * storing the result in c.
  */
template <class T>
void closures<T>::selfIntersect(vector<T>* c,  T* elems, unsigned int len){
  unsigned int p0  = 0;   // pointer to the current closure
  unsigned int p1  = 0;   // pointer to the transaction
  unsigned int res = 0;   // pointer to the result closure

  // loop both the vectors
  while ( p0<c->size() && p1<len) {
    // if the items are equal
    if ( (*c)[p0] == elems[p1] ) {
      // add the item to the closure
      (*c)[res] = (*c)[p0];
      // incr counters
      res++; p0++; p1++;
      } else if ((*c)[p0] < elems[p1] ) {
        p0++;
      } else {
        p1++;
      }
    }
    // resize the closure to the new closure size
    c->erase( c->begin()+res, c->end());
}

/** The current iteration is updates with length,
  * and additional num_itemsets entries are reserved.
  * (candidates are supposed to be stored now)
  */
template <class T>
void closures<T>::reserve_Nclosure(unsigned int length, unsigned int num_itemsets){
  startNclosures = my_closures.size();
  N = length;
  my_closures.resize(my_closures.size()+num_itemsets);
}

/** as add_2closure */
template <class T>
void closures<T>::add_Nclosure(unsigned int pos, dci_transaction<T>* transaction){
  pos += startNclosures;
  // closure has already been found
  if (my_closures[pos].size() == N)
    return;

  // there's something to intersect
  if (my_closures[pos].size() > N) { 
    selfIntersect(&(my_closures[pos]), transaction->elements, transaction->t_len);
  } 
  // first transaction for the closure
  else { 
    my_closures[pos].resize(transaction->t_len);
    for(unsigned int j=0; j<transaction->t_len; j++)
      my_closures[pos][j] = transaction->elements[j];
  } 
}

/** as delete_2closure */
template <class T>
void closures<T>::delete_Nclosure(unsigned int pos){
  pos += startNclosures;
  my_closures.erase(my_closures.begin() + pos);
}

/** appendes the support supp the the list of supports */
template <class T>
inline void closures<T>::addSupport(unsigned int supp){
  supports.push_back(supp);
}
template <class T>
inline void closures<T>::toBePrinted(bool val){
  print.push_back(val);
}

template <class T>
void closures<T>::setIter(unsigned int iter){
  if (iter==3)
    startPclosures = start2closures;
  else
    startPclosures = startNclosures;
  startNclosures = my_closures.size();
  N = iter;

  /*
  // initialize count_index
  for (unsigned int i=0; i<count_index.size(); i++)
    count_index[i].resize(0);
  */

	/*
	if (N==3)
	{
		print.resize(0);
		print.resize(my_closures.size(), true);
	}
*/
}
  
template <class T>
vector<T>* closures<T>::getPlace(void){
  my_closures.resize(my_closures.size()+1);
  return &(my_closures[my_closures.size()-1]);
}

template <class T>
void closures<T>::getFromPrevious(unsigned int pos, vector<T>& ret){
  unsigned int c = startPclosures+pos;
  ret.resize(my_closures[c].size());
  for (unsigned int j=0; j<my_closures[c].size(); j++) 
    ret[j] = my_closures[c][j];
}

template <class T>
void closures<T>::console(unsigned int pos) {
  for (unsigned int j=0; j<my_closures[startPclosures+pos].size(); j++) {
    plog << (int)my_closures[startPclosures+pos][j] << " ";
  }
  plog << endl;
}

template <class T>
void closures<T>::mergeFromPrevious(unsigned int i1, unsigned int i2, vector<T>& ret){
  i1+=startPclosures;
  i2+=startPclosures;
  ret.resize(my_closures[i1].size() + my_closures[i2].size());

  unsigned int p1  = 0;   // pointer to the first closure
  unsigned int p2  = 0;   // pointer to the second transaction
  unsigned int res = 0;   // pointer to the result closure

  // loop both the vectors
  while ( p1<my_closures[i1].size() && p2<my_closures[i2].size() ) {
    // if the items are equal
    if ( my_closures[i1][p1] == my_closures[i2][p2] ) {
      // add the item to the closure
      ret[res] = my_closures[i1][p1];
      // incr counters
      res++; p1++; p2++;
    } else if (my_closures[i1][p1] < my_closures[i2][p2] ) {
      ret[res] = my_closures[i1][p1];
      res++; p1++;
    } else {
      ret[res] = my_closures[i2][p2];
      res++; p2++;
    }
  }

  while (p1<my_closures[i1].size()) {
    ret[res] = my_closures[i1][p1];
    res++; p1++;
  }
  while (p2<my_closures[i2].size()) {
    ret[res] = my_closures[i2][p2];
    res++; p2++;
  }

  ret[res] = 0;

    // resize the closure to the new closure size
//    c->erase( c->begin()+res, c->end());
}

template <class T>
void closures<T>::PreDCI(dci_items &counters){
  count_index.resize(counters.get_max_supp()-counters.min_count+1);
  min_supp = counters.min_count;
}

template <class T>
int closures<T>::checkCountIndex(T* cand, int supp){
  // calc the index
  int ind = supp-min_supp;

  // if there's nothing
  if (count_index[ind].size()==0) {
    return -1;
  }

  // for each index stored
  for (unsigned int i=0; i<count_index[ind].size(); i++) {
    // if an inclusion is found return the closure
    if (isIncluded(cand, &(my_closures[count_index[ind][i]]) ) ) {
      return count_index[ind][i];
    }
  }
  // if no inclusion is found return nothing
  return -1;
}


template <class T>
void closures<T>::addCountIndex(int supp){
  // calc the index
  int ind = supp-min_supp;
  // add the closure
  count_index[ind].push_back(my_closures.size()-1);
}

template <class T>
void closures<T>::addCountIndex(int supp, unsigned int i){
  // calc the index
  int ind = supp-min_supp;
  // add the closure
  count_index[ind].push_back(i);
}


template <class T>
void closures<T>::copyAppend(int pos){
  int c = my_closures.size();
  my_closures.resize(my_closures.size()+1);
  my_closures[c].resize(my_closures[pos].size());
  for (unsigned int i=0; i<my_closures[pos].size(); i++)
    my_closures[c][i] = my_closures[pos][i];
}


  /** check Inclusion */
template <class T>
bool closures<T>::isIncluded(T*a, vector<T>* b){
  unsigned int pa  = 0;   // pointer to the current closure
  unsigned int pb  = 0;   // pointer to the transaction

  // loop both the vectors
  while ( pa<N && pb<b->size()) {
    // if the items are equal
    if ( a[pa] == (*b)[pb] ) {
      // incr counters
      pa++; pb++;
      } else if (a[pa] > (*b)[pb] ) {
        pb++;
      } else {
        return false;
      }
    }

  return (pa==N);
}

template <class T>
void closures<T>::toBePrinted_2ndIteration(){
	for (unsigned int i=0; i<my_closures.size(); i++) {
    int check_closure = checkCountIndex2ndIteration(my_closures[i], supports[i]);
		if (check_closure==-1) {
			addCountIndex(supports[i], i);
			print.push_back(true);
		} else {
			print.push_back(false);
		}
	}
}

template <class T>
int closures<T>::checkCountIndex2ndIteration(vector<T> &cand, int supp){
  // calc the index
  int ind = supp-min_supp;

  // if there's nothing
  if (count_index[ind].size()==0) {
    return -1;
  }

  // for each index stored
  for (unsigned int i=0; i<count_index[ind].size(); i++) {
    // if you fin an equal closure
		bool isEqual=true;
		for (unsigned int j=0; isEqual && j<my_closures[count_index[ind][i]].size(); j++)
			isEqual = (cand[j]==my_closures[count_index[ind][i]][j]);
    if (isEqual) 
      return count_index[ind][i];
  }

  // if no inclusion is found return nothing
  return -1;
}

}

#endif
