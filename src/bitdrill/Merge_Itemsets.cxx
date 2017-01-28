#include "Merge_Itemsets.hxx"
#include "bitdrill/Trie.hxx"

//TODO bool items_in_both_parts(const vector<int>& itemset,
// 			  const vector<bool>& part)
// { 
// }

bool check_merge_supports(const Trie<int>* freq_prev, 
			  const vector<int>& itemset,
			  const vector<bool>& part)
{
#ifdef DEBUG
  assert(itemset.size()>=3); // not used for smaller itemsets
#endif
  //TODO: optimize
  vector<int> support;
  support.resize(itemset.size()-1);
  for (vector<int>::const_iterator it = itemset.begin();
       it!=itemset.end(); it++) {
    copy(itemset.begin(), it, support.begin());
    copy(it+1, itemset.end(), support.begin() + 
	 (it - itemset.begin()));
    bool items_in_one_part = true;
    bool items_in_which_part = part[support[0]];
    for (vector<int>::const_iterator it = ++support.begin();
	 items_in_one_part && it!=support.end(); it++) {
      if (part[*it] != items_in_which_part)
	items_in_one_part = false;
    }
    
    if (!items_in_one_part) {
#ifdef DEBUG
      nlog << "checking support " << support << endl;
#endif
      if (!freq_prev->query(support))
	return false;
    }
  }
  return true;
}

// merge level k
void merge_itemsets(const Pattern_Set& freq_patterns, unsigned int k, 
		    const Pattern_Set& freq1_in, const Pattern_Set& freq2_in, 
		    const Vertical_DB& vertical_db, 
		    int support, ofstream* out) {
  assert (k>=2);
  assert ((unsigned int)freq_patterns.max_length >= k-1);
  int num_items = freq_patterns.num_items;
  const vector<int>& F1 = vertical_db.F;  
#ifdef DEBUG
  nlog << "freq1_in.bysize = " << freq1_in.bysize << endl;
  nlog << "freq2_in.bysize = " << freq2_in.bysize << endl;
  nlog << "F1 = " << F1 << endl;
  int num_freq_items = F1.size();
  nlog << num_freq_items << " frequent items in read pattern sets" << endl;
#endif
  
  vector<int> F1_1; // frequent items
  vector<int> F1_2;
  copy(freq1_in.F.begin(), freq1_in.F.end(), back_inserter(F1_1));
  copy(freq2_in.F.begin(), freq2_in.F.end(), back_inserter(F1_2));
#ifdef DEBUG
  nlog << "F1_1 = " << F1_1 << endl;
  nlog << "F1_2 = " << F1_2 << endl;
#endif  
  int freq1_size = freq1_in.max_length+1;
  int freq2_size = freq2_in.max_length+1; 
  Trie<int> freq1[freq1_size], freq2[freq2_size];
  for (int l=1; l<=freq1_in.max_length; l++) {
    for (list<Pattern>::const_iterator i=freq1_in.bysize[l].begin();
	 i!=freq1_in.bysize[l].end(); i++) {
      vector<int> x(i->itemset);
      vertical_db.reorder(x);
      freq1[l].insert(x);
    }
#ifdef DEBUG
    nlog << "freq1[" << l << "]" << freq1[l] << endl;
#endif
  }
  for (int l=1; l<=freq2_in.max_length; l++) {
    for (list<Pattern>::const_iterator i=freq2_in.bysize[l].begin();
	 i!=freq2_in.bysize[l].end(); i++) {
      vector<int> x(i->itemset);
      vertical_db.reorder(x);
      freq2[l].insert(x);
    }
#ifdef DEBUG
    nlog << "freq2[" << l << "]" << freq2[l] << endl;
#endif
  }


  int total_num_freq = 0;
  vector<bool> in_F1_1(num_items, false);
  for (vector<int>::const_iterator i=F1_1.begin(); i!=F1_1.end(); i++)
    in_F1_1[*i] = true;
  vector<bool> in_F1(num_items, false);
  for (vector<int>::const_iterator i=F1.begin(); i!=F1.end(); i++)
    in_F1[*i] = true;

  Trie<int> *freq_prev = new Trie<int>;

  // insert previously mined frequent itemsets at level k-1
  {
#ifdef PROFILE
    int pruned_prev = 0;
#endif
    for (list<Pattern>::const_iterator i=freq_patterns.bysize[k-1].begin();
	 i!=freq_patterns.bysize[k-1].end(); i++) {
      const vector<int>& x = i->itemset;
      // insert only patterns that are in F     
      // insert only patterns that have items in both parts
      bool inside = true;
      for (vector<int>::const_iterator it = x.begin();
	   inside && it!=x.end(); it++)
	if (!in_F1[*it])
	  inside = false;
      if (inside) {
	bool items_in_one_part = true;
	bool items_in_which_part = in_F1_1[x[0]];
	for (vector<int>::const_iterator it = ++x.begin();
	     items_in_one_part && it!=x.end(); it++) {
	  if (in_F1_1[*it] != items_in_which_part)
	    items_in_one_part = false;
	}
	if (!items_in_one_part) {
	  vector<int> candidate(x);
	  vertical_db.reorder(candidate);
	  freq_prev->insert(candidate);
	}
#ifdef PROFILE
	else
	  pruned_prev++;
#endif
      }
    }
#ifdef PROFILE
    nlog << "pruned_prev = " << pruned_prev << endl;  
#endif
  }
#ifdef DEBUG
  nlog << "freq_prev = " << *freq_prev << endl;
#endif


  while (!freq_prev->empty() || k==2) {
    Wall_Time t_s;
    nlog << "merging level " << k << endl;
    int num_pruned = 0, num_pruned2 = 0, num_candidates = 0;
#ifdef PROFILE
    int num_ones = 0, num_both_parts = 0;
    nlog << "|freq_prev| = " << freq_prev->size() << endl;
#endif
    vector<int> candidate(k);

    Trie<int> candidates; // TODO: length k

    if (k==2) {
      // assume F1_1 and F1_2 are disjoint
#ifdef DEBUG
      nlog << endl << "F1_1=" << F1_1 << endl;
      nlog << "F1_2=" << F1_2 << endl;
#endif
      for (vector<int>::iterator x=F1_1.begin();
	   x!=F1_1.end();x++) {
	for (vector<int>::iterator y=F1_2.begin();
	     y!=F1_2.end();y++) {
	  candidate[0] = *x;
	  candidate[1] = *y;
	  vertical_db.reorder(candidate);
	  candidates.insert(candidate);
	  num_candidates++;
	}
      }
    }
    else {
      // k>=3

      // first generate candidates based on frequent 
      // itemsets discovered during previous level of merging
      Trie<int>::Walk walk(freq_prev); //TODO: use Walk_K?
      vector<int> candidate1(k);
      vector<int> candidate2(k);
      vector<int> cond_candidate(k-2);
      Trie<int> freq_cond_1[F1.size()]; // for one of each F1_1
      Trie<int> freq_cond_2[F1.size()]; // for one of each F1_2
      while (!walk.end()) {
	if (walk.next()) {
	  const vector<int>& prefix = walk.ref();

#ifdef DEBUG
	  nlog << "walking k-1 prefix=" << prefix << endl;
#endif
	  Trie<int>::Candidate_Walk cand_walk(walk);
	  while (!cand_walk.end()) {
	    if (cand_walk.next()) {
	      int diff = cand_walk.ref();
#ifdef DEBUG
 	      nlog << "diff=" << diff << endl;
#endif

	      merge(prefix.begin(), prefix.end(), 
		    &diff, (&diff)+1, 
		    candidate.begin(), 
		    vertical_db.greater_freq);

	      // split candidate
	      candidate1.resize(0);
	      candidate2.resize(0);
	      for (vector<int>::const_iterator i=candidate.begin();
		   i!=candidate.end(); i++) {
		int item = *i;
		if (in_F1_1[item])
		  candidate1.push_back(item);
		else
		  candidate2.push_back(item);
	      }


#ifdef DEBUG
	      nlog << "splitting candidate" << candidate
		   << ", candidate1=" << candidate1
		   << ", candidate2=" << candidate2 << endl;
#endif
	      // must have items in both itemsets
	      // TODO: try to do this while walking prefix of candidate?
	      int len1 = candidate1.size();
	      int len2 = candidate2.size();
	      if (len1==0 || len2==0) {
		num_pruned2++;
		continue;
	      }

#ifdef PROFILE
	      num_both_parts++;
	      if (len1==1 || len2==1) {
		num_ones++;
		continue;
	      }
#endif
	      // pruning based on previously mined frequent itemsets
	      if (1 < len1 && len1 < freq1_size) {
		if (!freq1[len1].query(candidate1)) {
		  num_pruned2++;
		  continue;
		}
	      }

	      if (1 < len2 && len2 < freq2_size) {
		if (!freq2[len2].query(candidate2)) {
		  num_pruned2++;
		  continue;
		}
	      }

	      // downward closure pruning
 	      if (k>=3 && !freq_prev->check_supports(candidate)) {
 		num_pruned++;
 		continue;
 	      }

	      num_candidates++;
 	      candidates.insert(candidate);
#ifdef DEBUG
	      nlog << "candidate " << candidate << endl;
#endif
	    }
	  }

	  // now preparing conditional freq itemsets
	  int num_items_1 = 0, last_item_1=0, 
	    num_items_2 = 0, last_item_2=0;
	  for (vector<int>::const_iterator it = prefix.begin(); 
	       it!=prefix.end(); it++) {
	    int item = *it;
	    if (in_F1_1[item]) {
	      num_items_1++;
	      last_item_1 = item;
	    }
	    else {
	      num_items_2++;
	      last_item_2 = item;
	    }
	  }
	  if (num_items_1==1) {
	    int item = last_item_1;
	    int freq_ix = vertical_db.item_to_freq(item);
	    set_difference(prefix.begin(), prefix.end(), 
			   &item, (&item)+1, 
			   cond_candidate.begin(), 
			   vertical_db.greater_freq);
	    freq_cond_1[freq_ix].insert(cond_candidate);
#ifdef DEBUG
	    nlog << "freq_cond_1 for item " << item << " " << cond_candidate << endl;
	    nlog << "freq_ix=" << freq_ix << endl;
#endif
	  }
	  if (num_items_2==1) {
	    int item = last_item_2;
	    int freq_ix = vertical_db.item_to_freq(item);
	    set_difference(prefix.begin(), prefix.end(), 
			   &item, (&item)+1, 
			   cond_candidate.begin(), 
			   vertical_db.greater_freq);
	    freq_cond_2[freq_ix].insert(cond_candidate);
#ifdef DEBUG
	    nlog << "freq_cond_2 for item " << item << " " << cond_candidate << endl;
	    nlog << "freq_ix=" << freq_ix << endl;
#endif
	  }

	}  
      } // freq_prevwalk

#ifdef DEBUG
      nlog << "generating conditional candidates" << endl;
#endif

      vector<int> prev_candidate(k-1);
      for (vector<int>::const_iterator x=F1_1.begin();
	   x!=F1_1.end();x++) {
	int item = *x;
	int fix = vertical_db.item_to_freq(item);
#ifdef DEBUG
	nlog << "item=" << item   
	     << ", freq_cond_1[fix]=" << freq_cond_1[fix] << endl;
#endif
	Trie<int>::Walk walk(&freq_cond_1[fix]);
	while (!walk.end()) {
	  if (walk.next()) {
	    const vector<int>& prefix = walk.ref();
	    Trie<int>::Candidate_Walk cand_walk(walk);
	    while (!cand_walk.end()) {
	      if (cand_walk.next()) {
		int diff = cand_walk.ref();
		merge(prefix.begin(), prefix.end(), 
		      &diff, (&diff)+1, 
		      prev_candidate.begin(), 
		      vertical_db.greater_freq);
		// k-1 itemset must be present in freq2
		if ( k-1<freq2_size && !freq2[k-1].query(prev_candidate))
		  num_pruned2++;
		else {
		  merge(prev_candidate.begin(), prev_candidate.end(), 
			&item, (&item)+1, 
			candidate.begin(), 
			vertical_db.greater_freq);
		  if (k>=3 && !check_merge_supports(freq_prev, candidate, 
						    in_F1_1)) {
		    num_pruned++;
		    continue;
		  }
		  else {
		    num_candidates++;
		    candidates.insert(candidate);
#ifdef DEBUG
		    nlog << "candidatex " << candidate << endl;
#endif
		  }
		}
	      }
	    }
	  }
	}
      }


      for (vector<int>::const_iterator x=F1_2.begin();
	   x!=F1_2.end();x++) {
	int item = *x;
	int fix = vertical_db.item_to_freq(item);
#ifdef DEBUG
	nlog << "item=" << item   
	     << ", freq_cond_2[fix]=" << freq_cond_2[fix] << endl;
#endif
	Trie<int>::Walk walk(&freq_cond_2[fix]);
	while (!walk.end()) {
	  if (walk.next()) {
	    const vector<int>& prefix = walk.ref();
	    Trie<int>::Candidate_Walk cand_walk(walk);
	    while (!cand_walk.end()) {
	      if (cand_walk.next()) {
		int diff = cand_walk.ref();
		merge(prefix.begin(), prefix.end(), 
		      &diff, (&diff)+1, 
		      prev_candidate.begin(), 
		      vertical_db.greater_freq);
		// k-1 itemset must be present in freq1
#ifdef DEBUG
		nlog << "prev_candidate = " << prev_candidate << endl;
#endif
		if (k-1<freq1_size && !freq1[k-1].query(prev_candidate))
		  num_pruned2++;
		else {
		  merge(prev_candidate.begin(), prev_candidate.end(), 
			&item, (&item)+1, 
			candidate.begin(), 
			vertical_db.greater_freq);
		  if (k>=3 && !check_merge_supports(freq_prev, candidate, 
						    in_F1_1)) {
		    num_pruned++;
		    continue;
		  }
		  else {
		    num_candidates++;
		    candidates.insert(candidate);
#ifdef DEBUG
		    nlog << "candidatey " << candidate << endl;
#endif
		  }
		}
	      }
	    }
	  }
	}
      }
      
    } // else

    nlog << "pruned " << num_pruned << ", pruned2 " << num_pruned2 
	 << " candidates, "  << num_candidates 
	 << " candidates remaining [" <<  Wall_Time()-t_s << " sec]" << endl;

    if (candidates.empty())
      break;

    delete freq_prev;
    freq_prev = new Trie<int>;

    // do a pass over transaction set
    
    int num_freq = 0;

    Trie<int>::Walk_Tidlist walk(&candidates, vertical_db, k);

    Wall_Time t_s2;
    while (!walk.end()) {
      if (walk.next()) {
	const vector<int>& itemset = walk.ref();

	// construct tidlist intersection incrementally while walking
	// and make use of partial results to speed up computation
	const Tid_List& intersection = walk.tidlist();
	int frequency = intersection.count();
	if (frequency >= support) {
	  // output frequent itemset
	  num_freq++;
 	  if (out)
	    Pattern::write_pattern(*out, itemset, frequency);
	  freq_prev->insert(itemset);
#ifdef DEBUG
	  nlog << "frequent " << itemset << " , freq: " << frequency << endl;
#endif
	}
      }
    }

    nlog << num_freq << " frequent itemsets discovered " 
	 << Wall_Time()-t_s2 << " sec]" << endl;

    k++;
    total_num_freq += num_freq;
  } // while 1
  nlog << endl << "frequent itemsets exhausted" << endl << "end run" << endl;

  nlog << "total_num_freq=" << total_num_freq << endl;

  delete freq_prev;

};


void merge_itemsets_old(Pattern_Set& freq_patterns,
		    unsigned int k, Pattern_Set& freq1, Pattern_Set& freq2, 
		    Vertical_DB& vertical_db, 
		    int support, ofstream* out) {
  assert (k>=2);

#ifdef DEBUG
  nlog << "freq1.bysize = " << freq1.bysize << endl;
  nlog << "freq2.bysize = " << freq2.bysize << endl;
  vector<int>& F1 = vertical_db.F;  
  int num_freq_items = F1.size();
  nlog << "F1 = " << F1 << endl;
  nlog << num_freq_items << " frequent items in read pattern sets" << endl;
#endif
 
  Trie<int> *freq_prev = new Trie<int>;

  vector<int> x(k-1);
  Trie<int> *f_first_minus_1 = new Trie<int>;
  for (list<Pattern>::const_iterator i=freq_patterns.bysize[k-1].begin();
       i!=freq_patterns.bysize[k-1].end(); i++) {
    x = i->itemset;
    vertical_db.reorder(x);
    f_first_minus_1->insert(x);
  }
  
  bool first = true;
  int total_num_freq = 0;
  while (1) {
    Wall_Time t_s;
    nlog << "merging level " << k << endl;
    int pruned = 0, num_candidates = 0;
    vector<int> z(k);

    Trie<int> candidates; // TODO: length k

    if (k==2) {
      vector<int> F1_1; // frequent items
      vector<int> F1_2;
      //TODO: better way to find F1 ahh
      copy(freq1.F.begin(), freq1.F.end(), back_inserter(F1_1));
      copy(freq2.F.begin(), freq2.F.end(), back_inserter(F1_2));
//       // assume F1_1 and F1_2 are disjoint
//       merge(F1_1.begin(), F1_1.end(), 
// 	    F1_2.begin(), F1_2.end(), 
// 	    back_inserter(F1));
#ifdef DEBUG
      nlog << endl << "F1_1=" << F1_1 << endl;
      nlog << "F1_2=" << F1_2 << endl;
#endif
      for (vector<int>::iterator x=F1_1.begin();
	   x!=F1_1.end();x++) {
	for (vector<int>::iterator y=F1_2.begin();
	     y!=F1_2.end();y++) {
	  z[0] = *x; //x->itemset[0];
	  z[1] = *y; //y->itemset[0];
	  vertical_db.reorder(z);
	  candidates.insert(z);
	  num_candidates++;
	}
      }
    }
    else {
      
      for (unsigned int xlen=1; xlen<k; xlen++) {
	unsigned int ylen = k - xlen;
	if (xlen >= freq1.bysize.size())
	  continue; // no frequent items with that length
	for (list<Pattern>::iterator xs=freq1.bysize[xlen].begin();
	     xs!=freq1.bysize[xlen].end(); xs++) {
	  if (ylen >= freq2.bysize.size())
	    continue; // no frequent items with that length
	  for (list<Pattern>::iterator ys=freq2.bysize[ylen].begin();
	       ys!=freq2.bysize[ylen].end(); ys++) {

	    copy(xs->itemset.begin(), xs->itemset.end(), &z[0]);
	    copy(ys->itemset.begin(), ys->itemset.end(), &z[xlen]);
	    vertical_db.reorder(z);

	    bool prune = false;
	    
	    if (first) {
	      if (!f_first_minus_1->check_supports(z)) {
		prune = true;
		pruned++;
		continue;
	      }
	    }

	    // pruning across parts of the candidate itemset, 
	    // has meaning only if both parts have 2 or more items
	    if (!prune && xlen > 1) {
	      for (vector<int>::iterator i=xs->itemset.begin(); 
		   i!=xs->itemset.end(); i++) {
		int item = *i;
		Transaction zprev;
		zprev.resize(k-1);
		remove_copy(&z[0], &z[k], &zprev[0], item);
		if (!first && !freq_prev->query(zprev)) {
		  pruned++;
		  prune = true;
		  break;
		}
	      }
	    } // if xlen

	    if (!prune && ylen > 1) {
	      for (vector<int>::iterator i=ys->itemset.begin(); 
		   i!=ys->itemset.end(); i++) {
		int item = *i;
		Transaction zprev;
		zprev.resize(k-1);
		remove_copy(&z[0], &z[k], &zprev[0], item);
		if (!first && !freq_prev->query(zprev)) {
		  pruned++;
		  prune = true;
		  break;
		}
	      }
	    } // if ylen
	    
	    if (!prune) {
	      candidates.insert_iterator(z.begin(), z.end());
	      num_candidates++;
	    }

	  } // ys !=
	  
	} // xs !=
      } // for xlen

    } // else

    nlog << "pruned " << pruned << " candidates, "  << num_candidates 
	 << " candidates remaining [" <<  Wall_Time()-t_s << " sec]" << endl;

    if (first)
      delete f_first_minus_1;
 
    if (candidates.empty())
      break;

    delete freq_prev;
    freq_prev = new Trie<int>;

    // do a pass over transaction set
    
    int num_freq = 0;

    Trie<int>::Walk_Tidlist walk(&candidates, vertical_db, k);

    Wall_Time t_s2;
    while (!walk.end()) {
      if (walk.next()) {
	const vector<int>& itemset = walk.ref();

	// construct tidlist intersection incrementally while walking
	// and make use of partial results to speed up computation
	const Tid_List& intersection = walk.tidlist();
	int frequency = intersection.count();
	if (frequency >= support) {
	  // TODO: order in decreasing frequency
	  //vertical_db.reorder(union_itemset);
	  //output itemset
	  num_freq++;
 	  if (out)
	    Pattern::write_pattern(*out, itemset, frequency);
	  freq_prev->insert(itemset);
#ifdef DEBUG
	  nlog << "frequent " << itemset << " , freq: " << frequency << endl;
#endif
	}
      }
    }

    nlog << num_freq << " frequent itemsets discovered " 
	 << Wall_Time()-t_s2 << " sec]" << endl;

    k++;
    first = false;
    total_num_freq += num_freq;
  } // while 1
  nlog << endl << "frequent itemsets exhausted" << endl << "end run" << endl;

  nlog << "total_num_freq=" << total_num_freq << endl;

  if (out)
    out->close();
  delete freq_prev;

};


