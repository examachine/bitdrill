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
  initialize(argv[0], true);		// serial application

  cerr << " freq-bitdrill-cand -- all frequent itemset mining with given candidate set" << endl;
  cerr << "Bitdrill (C) by Eray Ozkural <erayo@cs.bilkent.edu.tr>" << endl;
  cerr << endl;

  Options options;
  BoolOption verbose("verbose", "v");
  options.add(&verbose);

  vector<string> args = get_options(argc, argv, options);

  if (args.size()<3 || args.size()>5) {
    cout << "usage: " << argv[0]
	 << " [--verbose | -v] ascii-db-file threshold cand-file [output-file] [max-level]" << endl;
    return 1;
  }

  string db_name(args[0]);
  
  istringstream is(args[1]);
  double sup;
  is >> sup;

  string candpat_fn = args[2];

  Miner miner(db_name, Transaction::ascii);
  // set verbosity

  //TODO: min_level

  // read candidate patterns
  Pattern_Set cand;
  cerr << "Reading candidate patterns from " << candpat_fn << endl;
  ifstream candpat(candpat_fn.c_str());
  if (candpat.good())
    cand.read(candpat);
  else {
    cerr << "Error opening file " << candpat_fn << endl;
      return 2;
  }
  
  string output_name;
  if (args.size()>=4)
    output_name = args[3];
  int max_level = 0; // means mine all
  if (args.size()>=5) {
    max_level = atoi(args[4].c_str());
    cerr << "mining up to level " << max_level << endl;
  }
  if (sup < 1.0)
    miner.discover_relative_cand(sup, cand, output_name, max_level);
  else
    miner.discover_absolute_cand(int(sup), cand, output_name, max_level);
  
  return 0; // success
}
