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

#include "Miner2.hxx"
#include "Vertical_DB.hxx"
//#include "Trie.hxx"
#include "Dense_Itemsets.hxx"
#include "transaction-db/Transaction_Set.hxx"
#include "2-items/Count.hxx"
#include "parallel/Parallel.hxx"

Miner2::Miner2(const string& name,
	     Transaction::File_Format format,
	     bool dbg)
  : db_name(name), ts(my_ts), my_ts(name, format), 
    relative_support(false),
    given_counter(false), debug(dbg)
{
}

Miner2::Miner2(Transaction_Set &_ts, bool dbg)
  : ts(_ts), relative_support(false), given_counter(false), debug(dbg)
{
}

Miner2::Miner2(Transaction_Set &_ts, TS_Counter &ctr,
			     bool dbg)
  : ts(_ts), 
    relative_support(false), counter(&ctr),
    given_counter(true), debug(dbg)
{
}

void Miner2::discover_relative(float sup, const string& out)
{
  if (out != "") {
    write_output = true;
    pattern_filename = out;
  }
  else write_output = false;

  real_support = sup;
  relative_support = true;

  discover();
}

void Miner2::discover_absolute(int sup, const string& out)
{
  if (out != "") {
    write_output = true;
    pattern_filename = out;
  }
  else write_output = false;

  // determine count and frequent items
  support = sup;
  relative_support = false;

  discover();
}

// is a-b 1 elt?
// int one_diff(const vector<int>& a,  const vector<int>& b)
// {
  
// }

void Miner2::discover()
{
  TS_Counter* counter;
  if (!given_counter) {
    counter = new TS_Counter(ts);
  }
  else
    counter = this->counter;

  // determine count and frequent items
  nlog << "counting large items in " << db_name << " ";
  Wall_Time t_s;
  if (!given_counter) {
    // count 1 items
    if (relative_support)
      counter->count_items_relative(real_support, 1, true);
    else
      counter->count_items_absolute(support, 1, true);
  }
  nlog << "|F| = " << counter->F.size() << " ";
  if (debug) {
    nlog << endl << "F = " << counter->F
	 << " count = " << counter->count << endl;
  }
  Wall_Time t_f;
  nlog << "[" << t_f - t_s << " sec.] " << endl;

  // construct vertical representation
  nlog << "constructing bitmap representation";
  Vertical_DB vertical_db(counter->F, counter->count, counter->num_items, 
			  counter->num_transactions, ts);
  Wall_Time t_d;
  nlog << "[" << t_d - t_f << " sec.] " << endl;

  // discover patterns
  nlog << "discovering frequent patterns ";
  support = counter->support;	// counter knows absolute support
  ofstream* pattern_out = 0;
  if (write_output) {
    pattern_out = new ofstream(pattern_filename.c_str());
    *pattern_out << "(" << counter->num_transactions << ")" << endl;
  }

  // mine here
  int k = 2; // level to mine
  Dense_Itemsets *freq = new Dense_Itemsets(vertical_db), 
    *freq_next = new Dense_Itemsets(vertical_db);
  for (int i=0; i<counter->F.size(); i++) {
    boost::dynamic_bitset<> itemset(counter->F.size());
    itemset.set(i);
    int item = vertical_db.freq_to_item(i);
    if (pattern_out) 
      *pattern_out << item << " (" 
		   << counter->count[item] << ")" << endl;
#ifdef DEBUG
    cout << "inserting " << itemset << endl;
#endif
    freq->insert(itemset);
  }
  freq->sort();


  do {
    boost::dynamic_bitset<> int2;
    boost::dynamic_bitset<> intersection;
    boost::dynamic_bitset<> diff_itemset(k);
    boost::dynamic_bitset<> union_itemset(k);
    int num_freq = 0;
    
    const Dense_Itemsets::Itemset_Vector& itemset_vec = 
      freq->ref_itemset_vec();
    
    Dense_Itemsets::Itemset_Vector::const_iterator walk;
    for (walk=itemset_vec.begin(); walk!=itemset_vec.end(); walk++) {
      const boost::dynamic_bitset<> & itemset = **walk;
      vertical_db.tidlist_freq(itemset, intersection);
	
      Dense_Itemsets::Itemset_Vector::const_iterator walk2;
	
      for (walk2=walk; walk2!=itemset_vec.end(); walk2++) {
	const boost::dynamic_bitset<>& itemset2 = **walk2;
	// do itemset and itemset2 union in k items?
	diff_itemset = itemset2 - itemset;
	if (diff_itemset.count()==1) {
	  //cout << "candidate " << union_itemset 
	  //<< " freq: " 	<< frequency << endl;
	  union_itemset = itemset | itemset2;
#ifdef DEBUG
// 	  cout << "!" << itemset << itemset2 
// 	       << diff_itemset << union_itemset << endl;
#endif
	  if (!freq_next->find(&union_itemset)) {		
	    // if so count the occurence of union in database
	    int2 = intersection & vertical_db.item_freq(diff_itemset.find_first());
	    int frequency = int2.count();
	    
	    if (frequency >= support) {
	      // order in decreasing fre	quency
	      //vertical_db.reorder(union_itemset);
	      freq_next->insert(union_itemset);
	      num_freq++;
#ifdef DEBUG
	      //cout << union_itemset << endl;
#endif
	      if (pattern_out) {
		boost::dynamic_bitset<>::size_type pos;
		for (pos=union_itemset.find_first(); 
		     pos!=boost::dynamic_bitset<>::npos;
		     pos=union_itemset.find_next(pos)) 
		  *pattern_out << vertical_db.freq_to_item(pos) << ' ';
		*pattern_out << '(' << frequency << ')' << endl;
	      }
	    }
	  }
	}
      }

    }

    // swap double buffer
    delete freq;
    freq = freq_next;
    freq->sort();
    freq_next = new Dense_Itemsets(vertical_db);

    if (num_freq>0)
      cout << num_freq << endl;

    k++;
  } while (!freq->empty());

  delete freq_next;

  Wall_Time t_e;
  nlog << "[" << t_e - t_d << " sec.] " << endl;
  nlog << "end run [" << t_e - t_s << " sec.] " << endl;

  if (write_output)
    delete pattern_out;
    
  if (!given_counter)
    delete counter;
}
