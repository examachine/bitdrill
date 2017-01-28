//
//
// C++ Implementation for module: Miner
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "Miner.hxx"
#include "Vertical_DB.hxx"
#include "Trie.hxx"
#include "Bit_Vector.hxx"

#include "transaction-db/Transaction_Set.hxx"
#include "2-items/Count.hxx"

Miner::Miner(const string& name, Transaction::File_Format format, 
	     bool dbg)
  : db_name(name), ts(my_ts), my_ts(name, format), 
    relative_support(false),
    given_counter(false), debug(dbg), max_level(0)
{
}

Miner::Miner(Transaction_Set &_ts, bool dbg)
  : ts(_ts), relative_support(false), given_counter(false), 
    debug(dbg), max_level(0)
{
}

Miner::Miner(Transaction_Set &_ts, TS_Counter &ctr, bool dbg)
  : ts(_ts), relative_support(false), counter(&ctr),
    given_counter(true), debug(dbg), max_level(0)
{
}

void Miner::discover_relative(float sup, const string& out, int ml)
{
  if (out != "") {
    write_output = true;
    pattern_filename = out;
  }
  else write_output = false;

  real_support = sup;
  relative_support = true;
  max_level = ml;
  discover();
}

void Miner::discover_absolute(int sup, const string& out, int ml)
{
  if (out != "") {
    write_output = true;
    pattern_filename = out;
  }
  else write_output = false;

  // determine count and frequent items
  support = sup;
  relative_support = false;
  max_level = ml;

  discover();
}

void Miner::discover()
{
  TS_Counter* counter;
  if (!given_counter) {
    counter = new TS_Counter(ts, true);
  }
  else
    counter = this->counter;

  // determine count and frequent items
  nlog << "counting large two items in " << db_name << " using kDCI" << endl;
  Wall_Time t_s;
  if (!given_counter) {
    // count 1 items
    if (relative_support)
      counter->count_items_relative(real_support, 2);
    else
      counter->count_items_absolute(support, 2);
  }
  nlog << "|F| = " << counter->F.size() << " ";
  if (debug) {
    nlog << endl << "F = " << counter->F
	 << " count = " << counter->count << endl;
  }
  Wall_Time t_f;
  nlog << "[" << t_f - t_s << " sec.] " << endl;

  // TODO: if there are few frequent items, then use kdci to 
  // continue mining level 2, otherwise use bitdrill

  // construct vertical representation
  nlog << "constructing vertical db representation";
  Vertical_DB vertical_db(counter->F, counter->count, counter->num_items, 
			  counter->num_transactions, ts);
  Wall_Time t_d;
  nlog << "[" << t_d - t_f << " sec.] " << endl;
#ifdef DEBUG
  nlog << vertical_db << endl;
#endif

  // discover patterns
  nlog << "discovering frequent patterns ";
  support = counter->support;	// counter knows absolute support
  ofstream* pattern_out = 0;
  if (write_output) {
    pattern_out = new ofstream(pattern_filename.c_str());
    *pattern_out << "(" << counter->num_transactions << ")" << endl;
  }

  // output 1 items
  for (int i=0; i<(int)counter->F.size(); i++) {
    int item = vertical_db.freq_to_item(i);
    if (pattern_out) 
      *pattern_out << item << " (" 
		   << counter->count[item] << ")" << endl;
  }

  if (max_level && 2 > max_level) {
    nlog << "max level 1 achieved" << endl;
    return;
  }

  // insert and output 2-items
  int k = 3; // level to mine
  Trie<int> *freq = new Trie<int>, *freq_next = new Trie<int>;
  Graph& F2 = counter->F2;

  for (unsigned int u=0; u<F2.num_vertices(); u++) {
    vector<int> x(2);
    for (Graph::Adjacency::iterator it=F2.adjacency[u].begin();
	 it!=F2.adjacency[u].end(); it++) {
      Edge& e = *it;
      unsigned int v = e.neighbor;
      if (u<v) {
	x[0] = counter->f_item[u]; 
	x[1] = counter->f_item[v];
	vertical_db.reorder(x);
	freq->insert(x);
	if (pattern_out)
	  write_pattern(*pattern_out, x, e.weight);
      }
    }
  }

  while (!freq->empty()) {
    if (max_level && k > max_level) {
      nlog << "max level " << max_level << " achieved" << endl;
      break;
    }
    nlog << "mining level " << k << endl;

    vector<int> candidate_itemset;
    candidate_itemset.resize(k);
    int num_freq = 0;

    Trie<int>::Walk_Tidlist walk(freq, vertical_db, k);

    while (!walk.end()) {
      if (walk.next()) {
	const vector<int>& itemset = walk.ref();

	// construct tidlist intersection incrementally while walking
	// and make use of partial results to speed up computation
	const Tid_List& intersection = walk.tidlist();

#ifdef DEBUG
 	nlog << "walking itemset=" << itemset 
	     << ", tidlist=" << intersection << endl;
#endif
	
	Trie<int>::Candidate_Walk cand_walk(walk);
	
	while (!cand_walk.end()) {
	  if (cand_walk.next()) {
	    int diff = cand_walk.ref();	    
#ifdef DEBUG
//  	    nlog << "diff=" << diff << endl;
#endif
	    //TODO: write a merge iterator
	    merge(itemset.begin(), itemset.end(), 
		  &diff, (&diff)+1, 
		  candidate_itemset.begin(), 
		  vertical_db.greater_freq);
	    if (!freq->check_supports(candidate_itemset))
	      continue;
	    // no need to check if candidate_itemset has been inserted
	    // before, because each candidate itemset is generated only
	    // once

#ifdef DEBUG
 	    nlog << "candidate " << candidate_itemset << endl;
#endif
	    // count the occurence of union in database
	    int frequency = intersection.count_intersection
	      (vertical_db.item(diff), support);
#ifdef DEBUG
 	    nlog << "frequency=" << frequency << endl;
#endif	    
	    if (frequency >= support) {
	      freq_next->insert(candidate_itemset);
	      num_freq++;
	      if (pattern_out)
		write_pattern(*pattern_out, candidate_itemset, frequency);
	    }
	    
	  }
	}
      }
    }

    // swap double buffer
    delete freq;
    //Trie<int>::release_mem();
    freq = freq_next;
    freq_next = new Trie<int>;

    if (num_freq>0) {
      cout << num_freq << endl;
      nlog << num_freq << " frequent itemsets mined" << endl;
    }
    k++;
  }

  delete freq_next;

  Wall_Time t_e;
  nlog << "[" << t_e - t_d << " sec.] " << endl;
  nlog << "end run [" << t_e - t_s << " sec.] " << endl;

  if (write_output)
    delete pattern_out;
    
  if (!given_counter)
    delete counter;
}


// discover frequent patterns, starting from a given set of frequent
// patterns, up to given maximum level
void Miner::discover(Vertical_DB& vertical_db, int support, 
		     Pattern_Set& freq_patterns,
		     Pattern_Set& out, int max_level)
{
  Wall_Time t_s;
  nlog << "discovering frequent patterns from vertical db " << endl << flush;

  int k = freq_patterns.max_length+1; // level to mine
  Trie<int> *freq = new Trie<int>, *freq_next = new Trie<int>;
  vector<int> x(k-1);
  for (list<Pattern>::const_iterator i=freq_patterns.bysize[k-1].begin();
       i!=freq_patterns.bysize[k-1].end(); i++) {
    x = i->itemset;
    vertical_db.reorder(x);
    freq->insert(x);
#ifdef DEBUG
    nlog << "inserted " << x << endl;
#endif
  }
  int total_num_freq = 0;
  while (!freq->empty()) {
    if (max_level && k > max_level) {
      nlog << "max level " << max_level << " achieved" << endl;
      break;
    }
    nlog << "mining level " << k << endl;
    vector<int> candidate_itemset;
    candidate_itemset.resize(k);
    int num_cand = 0;
    int num_pruned = 0;
    int num_freq = 0;

    Trie<int>::Walk_Tidlist walk(freq, vertical_db, k);

    while (!walk.end()) {
      if (walk.next()) {
	const vector<int>& itemset = walk.ref();

	// construct tidlist intersection incrementally while walking
	// and make use of partial results to speed up computation
	const Tid_List& intersection = walk.tidlist();

#ifdef DEBUG
 	nlog << "walking itemset=" << itemset << endl;
#endif
	
	Trie<int>::Candidate_Walk cand_walk(walk);
	
	while (!cand_walk.end()) {
	  if (cand_walk.next()) {
	    int diff = cand_walk.ref();	    
#ifdef DEBUG
//  	    nlog << "diff=" << diff << endl;
#endif
	    //TODO: write a merge iterator
	    merge(itemset.begin(), itemset.end(), 
		  &diff, (&diff)+1, 
		  candidate_itemset.begin(), 
		  vertical_db.greater_freq);
	    if (!freq->check_supports(candidate_itemset)) {
	      num_pruned++;
	      continue;
	    }
	    // no need to check if union_itemset has been inserted
	    // before, because each candidate itemset is generated only
	    // once
#ifdef DEBUG
 	    nlog << "candidate " << candidate_itemset << endl;
#endif
	    num_cand++;
	    // count the occurence of union in database
	    int frequency = intersection.count_intersection
	      (vertical_db.item(diff), support);
	    if (frequency >= support) {
	      // TODO: order in decreasing frequency
	      //vertical_db.reorder(union_itemset);
	      freq_next->insert(candidate_itemset);
	      num_freq++;
	      out.push_back(Pattern(candidate_itemset, frequency));
	    }
	    
	  }
	}
      }
    }

    // swap double buffer
    delete freq;
    //Trie<int>::release_mem();
    freq = freq_next;
    freq_next = new Trie<int>;

    if (num_freq>0) {
      cout << num_freq << endl;
      nlog << num_freq << " frequent itemsets mined"
	   << ", " << num_pruned << " pruned, " << num_cand << " candidates "
	   << endl;
    }

    k++;
    total_num_freq += num_freq;
  };

  delete freq_next;

  nlog << "total_num_freq=" << total_num_freq << endl;
  Wall_Time t_e;
  nlog << "end run [" << t_e - t_s << " sec.] " << endl;
}


void Miner::gen_candidates(Pattern_Set& freq_patterns, 
			   Pattern_Set& out, int max_level)
{
  Vertical_DB::Greater_Freq greater_freq(freq_patterns.count_vec);

  Wall_Time t_s;
  nlog << "generating candidate itemsets from a set of frequent patterns " << endl << flush;

  int k = freq_patterns.max_length+1; // level to mine
  Trie<int> *freq = new Trie<int>, *freq_next = new Trie<int>;
  vector<int> x(k-1);
  for (list<Pattern>::const_iterator i=freq_patterns.bysize[k-1].begin();
       i!=freq_patterns.bysize[k-1].end(); i++) {
    x = i->itemset;
    // sort by decreasing frequency
    sort(x.begin(), x.end(), greater_freq);
    freq->insert(x);
#ifdef DEBUG
    nlog << "inserted " << x << endl;
#endif
  }
  int total_num_cand = 0;
  while (!freq->empty()) {
    if (max_level && k > max_level) {
      nlog << "max level " << max_level << " achieved" << endl;
      break;
    }
    nlog << "generating candidates at level " << k << endl << flush;
    vector<int> candidate_itemset;
    candidate_itemset.resize(k);
    int num_cand = 0;
    int num_pruned = 0;

    Trie<int>::Walk walk(freq);

    while (!walk.end()) {
      if (walk.next()) {
	const vector<int>& itemset = walk.ref();

#ifdef DEBUG
 	nlog << "walking itemset=" << itemset << endl;
#endif
	
	Trie<int>::Candidate_Walk cand_walk(walk);
	
	while (!cand_walk.end()) {
	  if (cand_walk.next()) {
	    int diff = cand_walk.ref();	    
#ifdef DEBUG
//  	    nlog << "diff=" << diff << endl;
#endif
	    //TODO: write a merge iterator
	    merge(itemset.begin(), itemset.end(), 
		  &diff, (&diff)+1, 
		  candidate_itemset.begin(), 
		  greater_freq);
	    if (!freq->check_supports(candidate_itemset)) {
	      num_pruned++;
	      continue;
	    }
	    // no need to check if union_itemset has been inserted
	    // before, because each candidate itemset is generated only
	    // once
#ifdef DEBUG
 	    nlog << "candidate " << candidate_itemset << endl;
#endif
	    num_cand++;
	    // count the occurence of union in database
	    freq_next->insert(candidate_itemset);
	    //TODO: insert minimum known freq
	    out.push_back(Pattern(candidate_itemset, 0));
	    
	  }
	}
      }
    }

    // swap double buffer
    delete freq;
    //Trie<int>::release_mem();
    freq = freq_next;
    freq_next = new Trie<int>;

    if (num_cand>0) {
      cout << num_cand << endl;
      nlog << num_pruned << " pruned, " << num_cand << " candidates "
	   << endl << flush;
    }

    k++;
    total_num_cand += num_cand;
  };

  delete freq_next;

  nlog << "total_num_cand=" << total_num_cand << endl;
  Wall_Time t_e;
  nlog << "end run [" << t_e - t_s << " sec.] " << endl;

}


void Miner::write_pattern(ostream& pattern_out, const vector<int>& itemset, int frequency)
{
  for (vector<int>::const_iterator i=itemset.begin();
       i!=itemset.end(); i++) {
    pattern_out << *i << ' ';
  }
  pattern_out << '(' << frequency << ')' << endl;
}



void Miner::discover_relative_cand(float sup, Pattern_Set& cand,
                                   const string& out, int ml)
{
  if (out != "") {
    write_output = true;
    pattern_filename = out;
  }
  else write_output = false;

  real_support = sup;
  relative_support = true;
  max_level = ml;
  discover_cand(cand);
}

void Miner::discover_absolute_cand(int sup, Pattern_Set& cand,
                                   const string& out, int ml)
{
  if (out != "") {
    write_output = true;
    pattern_filename = out;
  }
  else write_output = false;

  // determine count and frequent items
  support = sup;
  relative_support = false;
  max_level = ml;

  discover_cand(cand);
}

void Miner::discover_cand(Pattern_Set& cand)
{
  TS_Counter* counter;
  if (!given_counter)
    counter = new TS_Counter(ts, true);
  else
    counter = this->counter;

  // determine count and frequent items
  nlog << "counting large items in " << db_name << " using kDCI" << endl;
  Wall_Time t_s;
  if (!given_counter) {
    // count 1 items
    if (relative_support)
      counter->count_items_relative(real_support, 2);
    else
      counter->count_items_absolute(support, 2);
  }
  nlog << "|F| = " << counter->F.size() << " ";
  if (debug) {
    nlog << endl << "F = " << counter->F
	 << " count = " << counter->count << endl;
  }
  Wall_Time t_f;
  nlog << "[" << t_f - t_s << " sec.] " << endl;

  // construct vertical representation
  nlog << "constructing vertical db representation";
  Vertical_DB vertical_db(counter->F, counter->count, 
                          counter->num_items, 
			  counter->num_transactions, ts);
  Wall_Time t_d;
  nlog << "[" << t_d - t_f << " sec.] " << endl;
#ifdef DEBUG
  nlog << vertical_db << endl;
#endif
  
  // discover patterns
  nlog << "discovering frequent patterns ";
  support = counter->support;	// counter knows absolute support

  Pattern_Set out;
  discover_cand(vertical_db, support, cand, out, max_level);

  //output patterns
  if (write_output) {
    ofstream pattern_out(pattern_filename.c_str());
    pattern_out << "(" << counter->num_transactions << ")" << endl;
    for (list<Pattern>::const_iterator i=out.begin();
         i!=out.end(); i++) {
      write_pattern(pattern_out, i->itemset, i->count);
    }
  }

}

// discover frequent patterns, considering only pre-generated
// set of candidates given to 
void Miner::discover_cand(Vertical_DB& vertical_db, int support, 
                          Pattern_Set& cand_patterns,
                          Pattern_Set& out, int max_level)
{
  Wall_Time t_s;
  nlog << "discovering frequent patterns from vertical db " << endl << flush;

  int min_level = cand_patterns.min_length(); // level to mine
  if (!max_level)
    max_level = cand_patterns.max_length;
  else
    max_level = min(max_level, cand_patterns.max_length);
  int k = min_level;
  
  int total_num_freq = 0;
  Trie<int> freq; 

  // mine level-wise using candidate set only
  while (max_level && k <= max_level) {

    nlog << "mining level " << k << endl;
    vector<int> candidate_itemset;
    candidate_itemset.resize(k);
    int num_cand = 0;
    int num_freq = 0;

    // construct a prefix-tree for candidates
    Trie<int> cand;
    vector<int> x(k);
    for (list<Pattern>::const_iterator i=cand_patterns.bysize[k].begin();
         i!=cand_patterns.bysize[k].end(); i++) {
      x = i->itemset;
      vertical_db.reorder(x);
      cand.insert(x);
#ifdef DEBUG
      nlog << "inserted " << x << endl;
#endif
    }

    Trie<int>::Walk_Tidlist walk(&cand, vertical_db, k);

    while (!walk.end()) {
      
      if (walk.next()) {
	const vector<int>& itemset = walk.ref();
	// construct tidlist intersection incrementally while walking
	// and make use of partial results to speed up computation
	const Tid_List& intersection = walk.tidlist();
#ifdef DEBUG
 	nlog << "walking itemset=" << itemset << endl;
#endif
        num_cand++;
        // count the occurence of union in database
        int frequency = intersection.count();
#ifdef DEBUG
 	nlog << "frequency=" << frequency << endl;
#endif
        if (frequency >= support) {
          // TODO: order in decreasing frequency
          //vertical_db.reorder(union_itemset);
          freq.insert(itemset);
          num_freq++;
          out.push_back(Pattern(itemset, frequency));
        }
	   
      }
    }

    k++;
    total_num_freq += num_freq;

    if (num_freq>0) {
      cout << total_num_freq << endl;
      nlog << num_freq << " frequent itemsets mined"
           << ", " << num_cand << " candidates " << endl;
    }
  };

  if (total_num_freq>0) {
    cout << total_num_freq << endl;
    nlog << total_num_freq << " frequent itemsets mined in total "
         << endl;
  }

  nlog << "total_num_freq=" << total_num_freq << endl;
  Wall_Time t_e;
  nlog << "end run [" << t_e - t_s << " sec.] " << endl;
}


// discover frequent patterns, considering only pre-generated
// set of candidates given to 
void Miner::discover_cand_serial(Vertical_DB& vertical_db, int support, 
                                 Pattern_Set& cand_patterns,
                                 Pattern_Set& out, int max_level)
{
  Wall_Time t_s;
  nlog << "discovering frequent patterns from vertical db " << endl << flush;

  int min_level = cand_patterns.min_length(); // level to mine
  if (!max_level)
    max_level = cand_patterns.max_length;
  int k = min_level;

  Trie<int> *freq = new Trie<int>, *freq_next = new Trie<int>;
  
  int total_num_freq = 0;

  // mine the first level using the first level of candidates only
  {
    nlog << "mining level " << k << endl;
    vector<int> candidate_itemset;
    candidate_itemset.resize(k);
    int num_cand = 0;
    int num_freq = 0;

    Trie<int> cand;
    vector<int> x(k);
    for (list<Pattern>::const_iterator i=cand_patterns.bysize[k].begin();
         i!=cand_patterns.bysize[k].end(); i++) {
      x = i->itemset;
      vertical_db.reorder(x);
      cand.insert(x);
#ifdef DEBUG
      nlog << "inserted " << x << endl;
#endif
    }

    Trie<int>::Walk_Tidlist walk(&cand, vertical_db, k);

    while (!walk.end()) {
      
      if (walk.next()) {
	const vector<int>& itemset = walk.ref();
	// construct tidlist intersection incrementally while walking
	// and make use of partial results to speed up computation
	const Tid_List& intersection = walk.tidlist();
#ifdef DEBUG
 	nlog << "walking itemset=" << itemset << endl;
#endif
        num_cand++;
        // count the occurence of union in database
        int frequency = intersection.count();
#ifdef DEBUG
 	nlog << "frequency=" << frequency << endl;
#endif
        if (frequency >= support) {
          // TODO: order in decreasing frequency
          //vertical_db.reorder(union_itemset);
          freq->insert(itemset);
          num_freq++;
          out.push_back(Pattern(itemset, frequency));
        }
	   
      }
    }
    
    if (num_freq>0) {
      cout << num_freq << endl;
      nlog << num_freq << " frequent itemsets mined"
	   << ", " << num_cand << " candidates " << endl;
    }

    k++;
    total_num_freq += num_freq;
  };

  // now we have the freq's for the first level, continue mining

  while (!freq->empty()) {
    if (max_level && k > max_level) {
      nlog << "max level " << max_level << " achieved" << endl;
      break;
    }
    nlog << "mining level " << k << endl;
    vector<int> candidate_itemset;
    candidate_itemset.resize(k);
    int num_cand = 0;
    int num_pruned = 0;
    int num_freq = 0;

    Trie<int> cand;
    vector<int> x(k);
    for (list<Pattern>::const_iterator i=cand_patterns.bysize[k].begin();
         i!=cand_patterns.bysize[k].end(); i++) {
      x = i->itemset;
      vertical_db.reorder(x);
      cand.insert(x);
#ifdef DEBUG
      nlog << "inserted " << x << endl;
#endif
    }

    Trie<int>::Walk_Tidlist walk(freq, vertical_db, k);

    while (!walk.end()) {
      if (walk.next()) {
	const vector<int>& itemset = walk.ref();

	// Construct tidlist intersection incrementally while walking
	// and make use of partial results to speed up computation
	const Tid_List& intersection = walk.tidlist();

#ifdef DEBUG
 	nlog << "walking itemset=" << itemset << endl;
#endif

	Trie<int>::Candidate_Walk cand_walk(walk);
	
	while (!cand_walk.end()) {
	  if (cand_walk.next()) {
	    int diff = cand_walk.ref();	    
#ifdef DEBUG
//  	    nlog << "diff=" << diff << endl;
#endif
	    //TODO: write a merge iterator
	    merge(itemset.begin(), itemset.end(), 
		  &diff, (&diff)+1, 
		  candidate_itemset.begin(), 
		  vertical_db.greater_freq);
	    if (!cand.query(candidate_itemset) ||
                !freq->check_supports(candidate_itemset)) {
	      num_pruned++;
	      continue;
	    }
      
	    // no need to check if union_itemset has been inserted
	    // before, because each candidate itemset is generated only
	    // once
#ifdef DEBUG
 	    nlog << "candidate " << candidate_itemset << endl;
#endif
	    num_cand++;
	    // count the occurence of union in database
	    int frequency = intersection.count_intersection
	      (vertical_db.item(diff), support);
	    if (frequency >= support) {
	      // TODO: order in decreasing frequency
	      //vertical_db.reorder(union_itemset);
	      freq_next->insert(candidate_itemset);
	      num_freq++;
	      out.push_back(Pattern(candidate_itemset, frequency));
	    }
	    
	  }
	}
      }
    }

    // swap double buffer
    delete freq;
    //Trie<int>::release_mem();
    freq = freq_next;
    freq_next = new Trie<int>;

    if (num_freq>0) {
      cout << num_freq << endl;
      nlog << num_freq << " frequent itemsets mined"
	   << ", " << num_pruned << " pruned, " 
           << num_cand << " candidates " << endl;
    }

    k++;
    total_num_freq += num_freq;
  };

  delete freq_next;

  nlog << "total_num_freq=" << total_num_freq << endl;
  Wall_Time t_e;
  nlog << "end run [" << t_e - t_s << " sec.] " << endl;
}

