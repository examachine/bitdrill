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


#include "candidates.hh"

template class DCP_candidates<unsigned int, unsigned int>;
template class DCP_candidates<unsigned int, unsigned short int>;
template class DCP_candidates<unsigned int, unsigned char>;

template class DCP_candidates<unsigned short int, unsigned int>;
template class DCP_candidates<unsigned short int, unsigned short int>;
template class DCP_candidates<unsigned short int, unsigned char>;

template class DCP_candidates<unsigned char, unsigned int>;
template class DCP_candidates<unsigned char, unsigned short int>;
template class DCP_candidates<unsigned char, unsigned char>;





template void gen_candidates(set_of_frequents<unsigned int, unsigned int> &, 
			     DCP_candidates<unsigned int, unsigned int>&, 
			     dci_items&, int, cand_2_iter<unsigned int>&);
template void gen_candidates(set_of_frequents<unsigned int, unsigned short int> &, 
			     DCP_candidates<unsigned int, unsigned short int>&, 
			     dci_items&, int, cand_2_iter<unsigned short int>&);
template void gen_candidates(set_of_frequents<unsigned int, unsigned char> &, 
			     DCP_candidates<unsigned int, unsigned char>&, 
			     dci_items&, int, cand_2_iter<unsigned char>&);

template void gen_candidates(set_of_frequents<unsigned short int, unsigned int>&, 
			     DCP_candidates<unsigned short int, unsigned int>&, 
			     dci_items&, int, cand_2_iter<unsigned int>&);
template void gen_candidates(set_of_frequents<unsigned short int, unsigned short int>&, 
			     DCP_candidates<unsigned short int, unsigned short int>&, 
			     dci_items&, int, cand_2_iter<unsigned short int>&);
template void gen_candidates(set_of_frequents<unsigned short int, unsigned char>&, 
			     DCP_candidates<unsigned short int, unsigned char>&, 
			     dci_items&, int, cand_2_iter<unsigned char>&);
template void gen_candidates(set_of_frequents<unsigned char, unsigned int>&, 
			     DCP_candidates<unsigned char, unsigned int>&, 
			     dci_items&, int, cand_2_iter<unsigned int>&);
template void gen_candidates(set_of_frequents<unsigned char, unsigned short int>&, 
			     DCP_candidates<unsigned char, unsigned short int>&, 
			     dci_items&, int, cand_2_iter<unsigned short int>&);
template void gen_candidates(set_of_frequents<unsigned char, unsigned char>&, 
			     DCP_candidates<unsigned char, unsigned char>&, 
			     dci_items&, int, cand_2_iter<unsigned char>&);




template <class T, class T1>
void DCP_candidates<T,T1>::init_prefix_table(int m)  {
  m1=m;
  sz = m1*(m1-1)/2+1; // one more element
  buf.resize(sz);

  for (int i=0; i<sz; i++)
    buf[i] = -1;

  int old0 = 0;
  int old1 = 0;
  int sz_cand = prefixes.size();
    
  for (int i=0; i < sz_cand; i += k-1) {
    int cur0 = prefixes[i];
    int cur1 = prefixes[i+1];
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

  //  bzero(counters, num_cand*sizeof(T1));

}





template <class T, class T1>
inline void DCP_candidates<T,T1>::scan_candidates3(dci_transaction<T>& t, 
					   int t0, int t1,
					   set_of_frequents<T,T1>& freq)
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
				counters[j + h]++;
				n_match++;
				t.incr_prune_local((unsigned int) t2);
			}
	//	}
	}
    t.incr_prune_local((unsigned int) t0, n_match);
    t.incr_prune_local((unsigned int) t1, n_match);
}





template <class T, class T1>
inline void DCP_candidates<T,T1>::scan_candidates(dci_transaction<T>& t, 
					   int t0, int t1,
					   int start, int end)
{
  int i;
  while (start<end) { 
    int item;
    item = prefixes[start+2];
    if (t.direct_access[item] != 0) {
      if (t.direct_access[item] < (T) (k - 2)) { // we can no longer 
	break;                                   // find matching candidates 
      }
    }
    else {
      start += k-1;   
      continue;
    }

    for (i=1; i < k-3; i++) {
      item = prefixes[start+2+i];
      if (t.direct_access[item] < (T) (k - 2 - i))
	break;
    }


    if (i == k-3) { // the prefixes are the same

      int to = ind_section[start/(k-1)];
      int from;
      if (start == 0) 
	from = 0;
      else
 	from = ind_section[start/(k-1)-1] + 1;



      int n_match=0;

      for (int j=from; j<=to; j++) {
	item = suffixes[j];
	if (t.direct_access[item] != 0) {
	  n_match++;
	  t.incr_prune_local((unsigned int) t.t_len - t.direct_access[item]);
	  counters[j]++; // increment count of the candidate
	}
      }

      if (n_match > 0) {
	t.incr_prune_local((unsigned int) t0, n_match);
	t.incr_prune_local((unsigned int) t1, n_match);

	for (int j=0; j < k-3; j++) {
	  item = prefixes[start+2+j];
	  t.incr_prune_local((unsigned int) t.t_len -  t.direct_access[item],
			     n_match);
	}

      }
      
    }
    start += k-1;   
  }  
}


template <class T, class T1>
inline void 
DCP_candidates<T,T1>::subset_and_count_and_prune_local(dci_transaction<T>& t, set_of_frequents<T,T1>& freq)
{
  int t0, t1;

  t.set_direct_access(); // importante!!

  if (k == 3) { /* generate all possible subsets of k elements */
    for (t0=0; t0<= (int) t.t_len-3; t0++) {
      for (t1=t0+1; t1<= (int) t.t_len-2; t1++) {
		scan_candidates3(t, t0, t1, freq);
      }
    }
  } else {     
    for (t0=0; t0<= (int) t.t_len-k; t0++) {
      int x0, cand_acc_init;
      x0 = t.elements[t0];
      cand_acc_init = direct_position2_init(x0, m1);
      for (t1=t0+1; t1<= (int) t.t_len-k+1; t1++) {
	int start, end, cand_acc;
	
	cand_acc = cand_acc_init + t.elements[t1];
	start = buf[cand_acc];
	end = buf[cand_acc+1];
	if (end==start) continue;

	scan_candidates(t, t0, t1, start, end);
      }
    }   
  }

  t.reset_direct_access(); // IMPORTANTE !!
}





// void DCP_candidates<T,T1>::remap_all_candidates(dci_items& counters) 
// {
//   for (unsigned int i=0; i<prefixes.size(); i++)
//     if (counters.unmap[prefixes[i]] == -1)
//       cout << "errore";
//     else
//       prefixes[i] = (T) counters.map[prefixes[i]];
//   for (unsigned int i=0; i<suffixes.size(); i++)
//     if (counters.unmap[suffixes[i]] == -1)
//       cout << "errore";
//     else
//       suffixes[i] = (T) counters.map[suffixes[i]];
// }


template <class T, class T1>
void gen_candidates(set_of_frequents<T,T1>& set_freq, 
		    DCP_candidates<T,T1>& cand_set,
		    dci_items& counters, 
		    int iter, cand_2_iter<T1>& c)
{
  int m1 = counters.get_m1();


  cand_set.reset(iter);

  if (set_freq.init_gen_cand()) {    

    T *cand;
    cand = new T[iter];

    set_freq.get_prefix(cand);
    set_freq.get_suffix(&cand[iter - 2]);
    
    int n_cand=0, pruned=0;
    while (1) {
      n_cand++;

      if (iter == 3) { /*
	int index = direct_position2((cand[1]), (cand[2]), m1);
	if ((unsigned int) c.get_cand_count(index) < counters.min_count)
	  pruned++;
	else */
	  cand_set.add_itemset(cand, 0);
      }
      else {
	if (! set_freq.find_subsets(cand)) // k>3
	  pruned++;
	else
	  cand_set.add_itemset(cand, 0);
      }
      
      int ret = set_freq.next_cand();
      if (ret == END_GEN_CAND) 
	break;
      else if (ret == NEW_SUFFIX) 
	set_freq.get_suffix(&cand[iter-2]);
      else {
	set_freq.get_prefix(cand);
	set_freq.get_suffix(&cand[iter-2]);
      }
    }

    delete [] cand;
    cand_set.init_prefix_table(m1);


    //cout << "n_cand:" << n_cand-pruned << "\t(pruned:" << pruned << ")\t";
  }
}

template <class T, class T1>
void DCP_candidates<T,T1>::initDA(int m) { 
	  DA_m = m;
    DA.resize(m*(m-1)/2+1);
    for (int i=0; i<(m*(m-1)/2+1); i++)  
		  DA[i]=-1;
	  DA[direct_position2(prefixes[0], prefixes[1], DA_m)] = 0;
	  for (int i=2; i<(int)prefixes.size(); i+=2) 
  		DA[direct_position2(prefixes[i], prefixes[i+1], DA_m)] = ind_section[i/2-1]+1;
  }

/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
     debug functions
   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
template <class T, class T1>
inline void DCP_candidates<T,T1>::pprint(){
	cout << endl;
	cout << " dcp candidates: " << endl;
	cout << "iteration    = " << k            << endl;
	cout << "num_itemsets = " << num_itemsets << endl;
	cout << "num_prefixes = " << prefixes.size()/(k-1) << endl;
	cout << " prefixes: ";
	int h=0;
	for (unsigned int i=0; i<prefixes.size(); i++){
		if ((i%(k-1))==0) {
			cout << endl << h++ <<": ";
		}
		cout << "  " << (int) prefixes[i];
	}
	cout << endl;

	cout << "num:suffixes = " << suffixes.size()       << endl;
	cout << " suffixes: " << endl;
	for (unsigned int i=0; i< suffixes.size(); i++) {
		cout << i << ": " << (int) suffixes[i] << endl;
	}
	cout << endl;

	cout << " counters: " << endl;
	for (unsigned int i=0; i< counters.size(); i++) {
		cout << i << ": " << (int) counters[i] << endl;
	}
	cout << endl;

	cout << " ind_section: " << endl;
	for (unsigned int i=0; i<ind_section.size(); i++) {
		cout << "  " << ind_section[i];
	}
	cout << endl;

	cout << " dcp candidates." << endl;
}


