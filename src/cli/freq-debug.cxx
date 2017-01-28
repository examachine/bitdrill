//
//
// C++ Implementation for module: freq
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//


#include "fp-tree/Pattern_Miner.hxx"

int main(int argc, char *argv[])
{
  cerr << "freq_all_debug -- debug sequential frequency mining" << endl;
  cerr << "FP-Growth* by Eray Ozkural <erayo@cs.bilkent.edu.tr>" << endl;
  cerr << endl;

  if (argc!=3) {
    cout << "usage: " << argv[0] << " db-file threshold" << endl;
    exit(-1);
  }

  string db_name(argv[1]);
  string patterns = db_name + ".patterns";
  
  istringstream is(argv[2]);
  double sup;
  is >> sup;
  
  Pattern_Miner miner(db_name, Transaction::ascii, true);
  if (sup < 1)
    miner.discover_relative(sup, "patterns");
  else
    miner.discover_absolute(int(sup), "patterns");
  
  return 0; // success
}
