//
//
// C++ Implementation for module: freq-bitdrill
//
// Description: Bitdrill is a new vertical miner
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "generic/Getopt.hxx"
#include "bitdrill/Miner.hxx"

using namespace Getopt;

int main(int argc, char *argv[])
{
  cerr << " freq-bitdrill -- all frequent itemset mining" << endl;
  cerr << "Bitdrill (C) by Eray Ozkural <erayo@cs.bilkent.edu.tr>" << endl;
  cerr << endl;

  Options options;
  BoolOption verbose("verbose", "v");
  options.add(&verbose);
  StringOption freqfn("freq", "f");
  options.add(&freqfn); 

  vector<string> args = get_options(argc, argv, options);

  if (args.size()<2 || args.size()>4) {
    cout << "usage: " << argv[0]
	 << " [--verbose | -v] [--freq | -f ascii-pattern-file] ascii-db-file threshold [output-file] [max-level]" << endl;
    return 1;
  }

  string db_name(args[0]);
  
  istringstream is(args[1]);
  double sup;
  is >> sup;

  Miner miner(db_name, Transaction::ascii);
  // set verbosity

  string output_name;
  if (args.size()>=3)
    output_name = args[2];
  int max_level = 0; // means mine all
  if (args.size()>=4) {
    max_level = atoi(args[3].c_str());
    cerr << "mining up to level " << max_level << endl;
  }
  string freq_fn = freqfn.get_value();
  if (freq_fn.size() > 0) {
    // read frequent patterns
    Pattern_Set freq;
    nlog << "Reading frequent patterns from " << freq_fn << endl;
    ifstream freqpat(freq_fn.c_str());
    if (freqpat.good())
      freq.read(freqpat);
    else {
      nlog << "Error opening file " << freq_fn << endl;
      return 2;
    }
    Transaction_Set ts(db_name); 
    Vertical_DB db(freq.F, freq.count_vec, freq.num_items,
                   freq.num_transactions, ts);
    Pattern_Set out;
    miner.discover(db, int(sup), freq, out, max_level);  
    ofstream pattern_out(output_name.c_str());
    for (list<Pattern>::const_iterator i=out.begin();
         i!=out.end(); i++) {
      Miner::write_pattern(pattern_out, i->itemset, i->count);
    }
  }
  else {
    if (sup < 1.0)
      miner.discover_relative(sup, output_name, max_level);
    else
      miner.discover_absolute(int(sup), output_name, max_level);
  }

  return 0; // success
}
