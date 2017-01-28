//
//
// C++ Interface for module: Miner
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef Miner_Interface
#define Miner_Interface

#include "generic/General.hxx"
#include "transaction-db/Transaction_Set.hxx"
#include "Vertical_DB.hxx"
#include "Pattern.hxx"

class TS_Counter;

class Miner
{  
public:

  Miner(const string & db_name, Transaction::File_Format format, 
	bool dbg = false);
  Miner(Transaction_Set & ts, bool dbg = false);
  Miner(Transaction_Set &_ts, TS_Counter &ctr, bool dbg = false);

  virtual ~Miner() {}

  void discover_relative(float real_support, const string &out_name = "",
			 int max_level = 0);
  void discover_absolute(int abs_support, const string &out_name = "",
			 int max_level = 0);

  void discover(Vertical_DB& db, int support, Pattern_Set& freq, 
		Pattern_Set& out, int max_level = 0);

  static void gen_candidates(Pattern_Set& freq, Pattern_Set& cand, 
			     int max_level = 0);

  void discover_relative_cand(float real_support, 
                              Pattern_Set& cand,
                              const string &out_name = "", 
                              int max_level = 0);
  void discover_absolute_cand(int abs_support, 
                              Pattern_Set& cand,
                              const string &out_name = "",              
                              int max_level = 0);

 
  void discover_cand(Vertical_DB& db, int support, Pattern_Set& cand, 
                     Pattern_Set& out, int max_level = 0);
  void discover_cand_serial(Vertical_DB& db, int support, 
                            Pattern_Set& cand, 
                            Pattern_Set& out, int max_level = 0); 

  static void write_pattern(ostream& pattern_out, const vector<int>& itemset, int frequency);
protected:
  virtual void discover();
  virtual void discover_cand(Pattern_Set& cand);

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
  int max_level;


};


#endif
