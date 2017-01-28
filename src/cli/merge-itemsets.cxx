//
// C++ Implementation
//
// Description: Merge two disjoint itemsets with separately mined
// frequent itemsets, starting from any level k.
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "generic/General.hxx"
#include "transaction-db/Transaction_Set.hxx"
#include "generic/Getopt.hxx"
#include "parallel/Parallel.hxx"

#include "2-items/Count.hxx"
#include "bitdrill/Miner.hxx"
#include "bitdrill/Vertical_DB.hxx"
#include "bitdrill/Pattern.hxx"
#include "bitdrill/Merge_Itemsets.hxx"

int main(int argc, char *argv[])
{
  initialize(argv[0]);		// serial application

  cout << argv[0] <<  " -- merge two disjoint itemsets of a transaction database" << endl;

  if (/*argc!=5 && */argc!=7) {
    cout << "usage: " << argv[0] 
	 << " <db> <freq itemset1> <freq itemset2> <support> [<freq_itemset_uptok> <k>]" << endl;
    cout << "where <freq itemset1> and <freq itemset2> are all frequent "
	 << " patterns found in two disjoint itemsets of <db> with <support>."
	 << "<k> is the level to start merging from, and <freq_itemset_uptok> "
	 << "is the pattern file for frequent itemsets up to level k-1." 
         << endl;
    exit(-1);
  }
  string dbname(argv[1]);
  string freq1name(argv[2]);
  string freq2name(argv[3]);
  istringstream is(argv[4]);
  int support;
  is >> support;
  // TODO: make the following args optional
  string freqname(argv[5]);
  int k;
  {
    istringstream is(argv[6]);
    is >> k;
  }
  Pattern_Set freq;
  freq.read(freqname);
  Pattern_Set freq1;
  freq1.read(freq1name);
  Pattern_Set freq2;
  freq2.read(freq2name);

  cout << "reading transaction set " << dbname << endl;
  Transaction_Set db(dbname);
  TS_Counter counter(db, true);
  counter.count_items_absolute(support, 1);
// construct vertical representation
  nlog << "constructing bitmap representation";
  Vertical_DB vertical_db(counter.F, counter.count, counter.num_items, 
			  counter.num_transactions, db);

  ostringstream os;
  os << dbname << ".out.merge." << support; 
  ofstream out(os.str().c_str());
  merge_itemsets(freq, k, freq1, freq2, vertical_db, support, &out);
  return 0;
  
}
