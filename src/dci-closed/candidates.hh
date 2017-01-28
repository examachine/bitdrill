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

#ifndef __DCICLOSED_CANDIDATES_H
#define __DCICLOSED_CANDIDATES_H

#include <string>

#include "direct_count.hh"
#include "items.hh"
#include "transaction.hh"
#include "frequents.hh"
#include "closure.hh"

namespace DCIClosed {

template <class T> class cand_2_iter {

public:
  cand_2_iter(int m1)  {
    sz = m1*(m1-1)/2;
    buf = new T[sz];
    bzero(buf, sz * sizeof(T));
  }
  ~cand_2_iter() {delete [] buf;}
  inline void incr_cand_count(int i) {buf[i]++;}
  inline int get_cand_count(int i) {return ((int) buf[i]);}
  inline int get_sz() {return sz;}



private:
  T *buf;
  int sz;
  
};

template <class T, class T1>
class DCP_candidates : public set_of_itemsets<T,T1>  {
public:
  DCP_candidates(int iter) : set_of_itemsets<T, T1>(iter) {}

  inline int  get_num_candidates() {return this->num_itemsets;}
  void        init_prefix_table(int m);
  inline void subset_and_count_and_prune_local(dci_transaction<T>& t,
                                                set_of_frequents<T,T1>& freq,
                                                closures<T>* closets);
  inline int  get_count(int ind) {return ((int) this->counters[ind]);}

  void add_itemset(T *v, T1 c, T1 min_supp){
    set_of_itemsets<T,T1>::add_itemset(v, c);
    min_supports.push_back(min_supp);
  }

  //inline void printf_candidates() {printf_itemsets();}

		/** Direct acces structure for DCP in the 3th iteration */
  vector<int> DA;
		/** size of di direct access structure (unnecessary ???) */
  int DA_m;
		/** Initializes the direct access structure used for
			* a fast DCP in the 3rd iteration.
			* for each prefix(c0, c1) the position of the first candidate
			* (c0, c1, _ ) is stored.
			*/
  void initDA(int m);

	void reset(int iter){
		set_of_itemsets<T,T1>::reset(iter);
		DA.resize(0);
		DA_m = 0;
    min_supports.resize(0);
	}


	   /** minimum support of the (k-1)subsets */
	vector<T1> min_supports; 

	/** prints on console all the information stored in the class */
  void pprint();

private:
  int         m1;
  vector<int> buf;
  int         sz;

  inline void scan_candidates3(dci_transaction<T> &t, int t0, int t1,
			       set_of_frequents<T,T1>& freq,
             closures<T>* closets);
  inline void scan_candidates (dci_transaction<T> &t, int t0, int t1,
			       int start, int end,closures<T>* closets);
}; 


template <class T, class T1>
void gen_candidates(set_of_frequents<T,T1>& set_freq,
		    DCP_candidates<T,T1>& cand_set,
		    dci_items& counters, 
		    int iter,
		    cand_2_iter<T1>& c);


/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

template <class T, class T1>
void DCP_candidates<T,T1>::init_prefix_table(int m)  {
  m1=m;
  sz = m1*(m1-1)/2+1; // one more element
  buf.resize(sz);

  for (int i=0; i<sz; i++)
    buf[i] = -1;

  int old0 = 0;
  int old1 = 0;
  int sz_cand = this->prefixes.size();
    
  for (int i=0; i < sz_cand; i += this->k-1) {
    int cur0 = this->prefixes[i];
    int cur1 = this->prefixes[i+1];
    if ((cur0 != old0) || (cur1 != old1)) {
      int dirpos = direct_position2(cur0,cur1,m1);
      //cout << "-->" << cur0 << "," << cur1 << "  dirpos:" 
      //     << dirpos << "  i: " << i << endl;
      buf[dirpos] = i;
      old0 = cur0;
      old1 = cur1;
    }
  }
    
    
  buf[sz-1] = sz_cand;

  old0 = buf[sz-1];
  for (int i = sz - 2; i >= 0; i--){
    if (buf[i] == -1) {
      buf[i] = old0;
    } else {
      old0 = buf[i];
    }
  }    

  //  bzero(this->counters, num_cand*sizeof(T1));

}





template <class T, class T1>
inline void DCP_candidates<T,T1>::scan_candidates3(dci_transaction<T>& t, 
					   int t0, int t1,
					   set_of_frequents<T,T1>& freq,
             closures<T>* closets)
{
	int j = -1;
	int v, h;

	/* base */
	v = DA[direct_position2(t.elements[t0], t.elements[t1], DA_m)];
	if (v == -1)
		return;
	j += v;

	/* offset (c1, c2) */
	j -= freq.DA[direct_position2(t.elements[t0], t.elements[t1], freq.DA_m)];

	/* offset (c1, c3) */
    int n_match=0;
	for (int t2=t1+1; t2<(int)t.t_len; t2++) {
	//	k = freq.DA[direct_position2(t.elements[t1], t.elements[t2], freq.DA_m)];
	//	if ( k!=-1) { 
			h = freq.DA[direct_position2(t.elements[t0], t.elements[t2], freq.DA_m)];
			if ( h != -1) { /* subset check OK */
				this->counters[j + h]++;
				n_match++;
        closets->add_Nclosure((unsigned int)(j+h), &t); 
				// t.incr_prune_local((unsigned int) t2);
			}
	//	}
	}
    // t.incr_prune_local((unsigned int) t0, n_match);
    // t.incr_prune_local((unsigned int) t1, n_match);
}





template <class T, class T1>
inline void DCP_candidates<T,T1>::scan_candidates(dci_transaction<T>& t, 
					   int t0, int t1,
					   int start, int end,
             closures<T>* closets)
{
  int i;
  while (start<end) { 
    int item;
    item = this->prefixes[start+2];
    if (t.direct_access[item] != 0) {
      if (t.direct_access[item] < (T) (this->k - 2)) { // we can no longer 
	break;                                   // find matching candidates 
      }
    }
    else {
      start += this->k-1;   
      continue;
    }

    for (i=1; i < this->k-3; i++) {
      item = this->prefixes[start+2+i];
      if (t.direct_access[item] < (T) (this->k - 2 - i))
	break;
    }


    if (i == this->k-3) { // the this->prefixes are the same

      int to = this->ind_section[start/(this->k-1)];
      int from;
      if (start == 0) 
	from = 0;
      else
 	from = this->ind_section[start/(this->k-1)-1] + 1;



      int n_match=0;

      for (int j=from; j<=to; j++) {
	item = this->suffixes[j];
	if (t.direct_access[item] != 0) {
	  n_match++;
	  t.incr_prune_local((unsigned int) t.t_len - t.direct_access[item]);
	  this->counters[j]++; // increment count of the candidate
    closets->add_Nclosure((unsigned int)(j), &t); 

	}
      }

      if (n_match > 0) {
	t.incr_prune_local((unsigned int) t0, n_match);
	t.incr_prune_local((unsigned int) t1, n_match);

	for (int j=0; j < this->k-3; j++) {
	  item = this->prefixes[start+2+j];
	  t.incr_prune_local((unsigned int) t.t_len -  t.direct_access[item],
			     n_match);
	}

      }
      
    }
    start += this->k-1;   
  }  
}


template <class T, class T1>
inline void 
DCP_candidates<T,T1>::subset_and_count_and_prune_local(dci_transaction<T>& t,
                                                       set_of_frequents<T,T1>& freq,
                                                       closures<T>* closets)
{
  int t0, t1;

  t.set_direct_access(); // importante!!

  if (this->k == 3) { /* generate all possible subsets of k elements */
    for (t0=0; t0<= (int) t.t_len-3; t0++) {
      for (t1=t0+1; t1<= (int) t.t_len-2; t1++) {
		    scan_candidates3(t, t0, t1, freq, closets);
      }
    }
  } else {     
    for (t0=0; t0<= (int) t.t_len-this->k; t0++) {
      int x0, cand_acc_init;
      x0 = t.elements[t0];
      cand_acc_init = direct_position2_init(x0, m1);
      for (t1=t0+1; t1<= (int) t.t_len-this->k+1; t1++) {
	int start, end, cand_acc;
	
	cand_acc = cand_acc_init + t.elements[t1];
	start = buf[cand_acc];
	end = buf[cand_acc+1];
	if (end==start) continue;

	scan_candidates(t, t0, t1, start, end, closets);
      }
    }   
  }

  t.reset_direct_access(); // IMPORTANTE !!
}





// void DCP_candidates<T,T1>::remap_all_candidates(dci_items& this->counters) 
// {
//   for (unsigned int i=0; i<this->prefixes.size(); i++)
//     if (this->counters.unmap[this->prefixes[i]] == -1)
//       cout << "errore";
//     else
//       this->prefixes[i] = (T) this->counters.map[this->prefixes[i]];
//   for (unsigned int i=0; i<this->suffixes.size(); i++)
//     if (this->counters.unmap[this->suffixes[i]] == -1)
//       cout << "errore";
//     else
//       this->suffixes[i] = (T) this->counters.map[this->suffixes[i]];
// }


template <class T, class T1>
void gen_candidates(set_of_frequents<T,T1>& set_freq,
		    DCP_candidates<T,T1>& cand_set,
		    dci_items& counters, 
		    int iter,
		    cand_2_iter<T1>& c)
{
  int m1 = counters.get_m1();

  cand_set.reset(iter);

  if (set_freq.init_gen_cand()) {    

    T*	cand				= new T[iter];
		T1* cand_counts	= new T1[2];

		T* subset		= new T[iter-1];
		T1 min_count, count;

    set_freq.get_prefix(cand);
    set_freq.get_suffix(&cand[iter - 2], cand_counts);
    
    int n_cand=0, pruned=0;
    while (1) {
      n_cand++;
      
      bool infrequent = false;
      min_count = cand_counts[0];

      // look for the subset with minimum count
      for (int del=iter-3; del>=0; del--) { 
        int j = 0;
        for (int i=0; i<iter; i++)
	        if (i != del)
	          subset[j++] = cand[i];

        if (set_freq.find_one_subset(subset, count)==0){
  	      // a subset is infrequent, prune the cand and take the next one
	        infrequent = true;
	        break;
        }
  	    // remember min_count and corresponding key
	      if (min_count > count) 
	        min_count = count;
      }

      if (iter == 3 ) {
        // no pruning
        if (min_count > cand_counts[1])
          min_count = cand_counts[1];

        cand_set.add_itemset(cand, 0, min_count);
      } else {
        if (infrequent)
          pruned++;
        else {
          if (min_count > cand_counts[1])
            min_count = cand_counts[1];

          cand_set.add_itemset(cand, 0, min_count);
        }
      }
       
      int ret = set_freq.next_cand();
      if (ret == END_GEN_CAND) 
				break;
      else if (ret == NEW_SUFFIX) 
				set_freq.get_suffix(&cand[iter - 2], cand_counts);
      else {
				set_freq.get_prefix(cand);
		    set_freq.get_suffix(&cand[iter - 2], cand_counts);
      }
    }

		delete [] subset;
    delete [] cand;
		delete [] cand_counts;
    cand_set.init_prefix_table(m1);


    //cout << "n_cand:" << n_cand-pruned << "\t(pruned:" << pruned << ")\t";
  }
	
}

template <class T, class T1>
void DCP_candidates<T,T1>::initDA(int m) { 
	  DA_m = m;
    DA.resize(m*(m-1)/2);
    for (int i=0; i<(m*(m-1)/2); i++)  
		  DA[i]=-1;
	  DA[direct_position2(this->prefixes[0], this->prefixes[1], DA_m)] = 0;
	  for (int i=2; i<(int)this->prefixes.size(); i+=2) 
  		DA[direct_position2(this->prefixes[i], this->prefixes[i+1], DA_m)] = this->ind_section[i/2-1]+1;
  }

/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
     debug functions
   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
template <class T, class T1>
inline void DCP_candidates<T,T1>::pprint(){
	plog << endl;
	plog << " dcp candidates: " << endl;
	plog << "iteration    = " << this->k << endl;
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


}
#endif
