//
//
// C++ Interface for module: Pattern_Miner
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef Pattern_Miner_Interface
#define Pattern_Miner_Interface

#include "generic/General.hxx"
#include "transaction-db/Transaction_Set.hxx"
#include "2-items/Count.hxx"

class Pattern_Miner
{  
public:

  Pattern_Miner(const string & db_name, Transaction::File_Format format,
		bool dbg = false);
  Pattern_Miner(Transaction_Set & ts,
		bool dbg = false);
  Pattern_Miner(Transaction_Set &_ts, TS_Counter &ctr, bool dbg = false);
  void discover_relative(float real_support, const string &out_name = "");
  void discover_absolute(int abs_support, const string &out_name = "");

private:
  void discover();

  string db_name;
  Transaction_Set& ts;
  Transaction_Set my_ts;
  //int num_items;
  float real_support;
  int num_transactions;
  int support;
  bool relative_support;
  TS_Counter* counter;
  bool given_counter;
  bool debug;
  bool write_output;
  string pattern_filename;
};


#endif
