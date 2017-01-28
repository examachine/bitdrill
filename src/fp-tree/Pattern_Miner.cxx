//
//
// C++ Implementation for module: Pattern_Miner
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "Pattern_Miner.hxx"

#include "fp-tree/FP_Tree.hxx"
#include "transaction-db/Transaction_Set.hxx"
#include "2-items/Count.hxx"

Pattern_Miner::Pattern_Miner(const string& name,
			     Transaction::File_Format format,
			     bool dbg)
  : db_name(name), ts(my_ts), my_ts(name, format), 
    relative_support(false),
    given_counter(false), debug(dbg)
{
}

Pattern_Miner::Pattern_Miner(Transaction_Set &_ts, bool dbg)
  : ts(_ts), relative_support(false), given_counter(false), debug(dbg)
{
}

Pattern_Miner::Pattern_Miner(Transaction_Set &_ts, TS_Counter &ctr,
			     bool dbg)
  : ts(_ts), 
    relative_support(false), counter(&ctr),
    given_counter(true), debug(dbg)
{
}

void Pattern_Miner::discover_relative(float sup, const string& out)
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

void Pattern_Miner::discover_absolute(int sup, const string& out)
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

void Pattern_Miner::discover()
{
  map<int,int> pat_counts;	// length -> cardinality map for patterns

  TS_Counter* counter;
  if (!given_counter) {
    counter = new TS_Counter(ts,true); // construct serial counter
  }
  else
    counter = this->counter;

  // determine count and frequent items
  nlog << "counting large items in " << db_name << endl;
  Wall_Time t_s;
  if (!given_counter) {
    // count 1 items
    if (relative_support)
      counter->count_items_relative(real_support, 1);
    else
      counter->count_items_absolute(support, 1);
  }
  nlog << "|F| = " << counter->F.size() << " ";
  if (debug) {
    nlog << endl << "F = " << counter->F
	 << " count = " << counter->count << endl;
  }
  Wall_Time t_f;
  nlog << "[" << t_f - t_s << " sec.] " << endl;

  // construct fp tree
  nlog << "constructing fp-tree ";
  FP_Tree fp_tree(counter->F, counter->count, pat_counts, debug);
  Transaction_Set::Scanner* scanner = ts.gen_scanner();
  while (!scanner->end()) {
    Transaction t_i;
    if (scanner->read(t_i)) {
      fp_tree.insert(t_i);
    }
  }
  delete scanner;
  Wall_Time t_d;
  nlog << "[" << t_d - t_f << " sec.] " << endl;

  // discover patterns
  nlog << "discovering frequent patterns ";
  support = counter->support;	// counter knows absolute support
  if (write_output) {
    ofstream pattern_out(pattern_filename.c_str());
    pattern_out << "(" << counter->num_transactions << ")" << endl;
    fp_tree.discover_patterns(support, &pattern_out);
  }
  else {
    fp_tree.discover_patterns(support);
  }
  Wall_Time t_e;
  nlog << "[" << t_e - t_d << " sec.] " << endl;
  nlog << "end run [" << t_e - t_s << " sec.] " << endl;
  if (write_output)
    fp_tree.print_pattern_counts(nlog);

  if (!given_counter)
    delete counter;

}
