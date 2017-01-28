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
  cerr << "freq -- sequential frequency mining" << endl;
  cerr << "FP-Growth* by Eray Ozkural <erayo@cs.bilkent.edu.tr>" << endl;
  cerr << endl;

  if (argc!=3 && argc!=4) {
    cout << "usage: " << argv[0]
	 << " ascii-db-file threshold [output-file]" << endl;
    exit(-1);
  }

  string db_name(argv[1]);
  
  istringstream is(argv[2]);
  double sup;
  is >> sup;

  if (argc==4) {
    string output_name(argv[3]);
    Pattern_Miner miner(db_name, Transaction::ascii);
    if (sup < 1.0)
      miner.discover_relative(sup, output_name);
    else
      miner.discover_absolute(int(sup), output_name);
  }
  else {
    Pattern_Miner miner(db_name, Transaction::ascii);
    if (sup < 1.0)
      miner.discover_relative(sup);
    else
      miner.discover_absolute(int(sup));
  }
  
  return 0; // success
}
