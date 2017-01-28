//
//
// C++ Implementation for module: analyze-ts
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

// #include "estimate-load.hxx"

#include "generic/General.hxx"
#include "transaction-db/Transaction_Set.hxx"
#include "2-items/Count.hxx"
#include "fp-tree/Pattern_Miner.hxx"

#include <string.h>
#include <libgen.h>

int main(int argc, char *argv[])
{
  cout << "transaction set analysis tool" << endl;
  
  if (argc<3 || argc>4) {
    cout << "usage:" << argv[0] << " db threshold [is-absolute]" 
	 << endl;
    
    exit(-1);
  }

  string db_name(argv[1]);

  ostringstream dn_str;
  dn_str << basename(argv[1]) << "-" << argv[2];

  istringstream is2(argv[2]);
  int abs_sup; double real_sup;

  bool absolute;
  if (argc==4) {
    absolute=true;
    is2 >> abs_sup;
  }
  else {
    absolute=false; // yeah, ?: is bad syntax 
    is2 >> real_sup;
  }
    
  initialize(argv[0],true);		// serial exec.

  TS_Counter counter(db_name, true);
  // count 2-items	
  if (absolute)
    counter.count_items_absolute(abs_sup,2);
  else
    counter.count_items_relative(real_sup,2);
  
  // compute stats & report estimate
  counter.compute_stats();
  nlog << "db-stats ";
  counter.print_db_stats(nlog);
  nlog << endl;
  nlog << "load-estimate ";
  counter.print_load_estimate(nlog);
  nlog << endl;

  // write graph
  counter.F2.write_metis(dn_str.str() + ".F2.metis");
    
  counter.F2_load.write_metis(dn_str.str() + "F2-load.metis");

  return 0; // success
}
