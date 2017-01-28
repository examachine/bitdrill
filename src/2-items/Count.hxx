//
//
// C++ Interface for module: Count
//
// Description:
//
//
// Author: exa
//
// Copyright: See COPYING file that came with this distribution
//
//

#ifndef Count_Interface
#define Count_Interface

#include "generic/General.hxx"

#include "graph/Graph.hxx"
#include "graph/Mtx_Graph.hxx"
#include "graph/Vertex_Separator.hxx"
#include "graph/Graph_Metis_Converter.hxx"
#include "transaction-db/Transaction_Set.hxx"

// Configuration switches
// GRAPH_2ITEMS
// USE_KDCI

#define USE_KDCI

#ifdef USE_KDCI
#include "kdci/fim_all.hh"
#endif

class TS_Counter {
public:

  enum Load_Estimate_Func { datumsize=0, approx3patterns=1,
			    f2weight=2, fpgrowthload=3,
			    kkstar=4, linearloadgraph=5,
                            };

  static string temp_filename_prefix;
  string temp_filename;

  TS_Counter(Transaction_Set& ts, bool serial = false);
  TS_Counter(const string& prefix, bool serial = false);
  ~TS_Counter();
//   TS_Counter(TS_Counter& counter, set<int> items);

  void count_items_relative(float sup, int level);
  void count_items_absolute(int sup, int level);

  int num_items;
  int num_transactions;
  int total_num_transactions;
  float real_support;
  int support;
  int local_support;
  int level;
  bool serial;

  void count_items_serial();

  // vertical projection from existing counter object
  void project_items_from(TS_Counter& count, set<int> items);

  void count_1_items();
  void count_1_items_serial();
  void count_1_items_local(vector<int>& count);
  void large_items();
  void exchange_1_items();
  void sum_1_items();
  void count_2_items();
  void count_2_items_local();
  void exchange_2_items(Undir_Mtx_Graph* total_two_items=0);
  bool freq_item(int i);
  void prune_2_items(Undir_Mtx_Graph* total_two_items=0);

  vector<int> count_local;
  vector<int> count;

  set<int> F;			// frequent 1-items

#ifndef GRAPH_2ITEMS
  Undir_Mtx_Graph* two_items;	// local two items graph with mtx rep
  vector<int> f_item;		// forward map freq -> item
  vector<int> item_f;		// inverse map
#else
  Graph two_items;		// local two items graph with adjlist rep
#endif
  Graph F2;			// accumulated frequent 2-items graph
  int F2_edges;			// number of edges in total_F2;
  Graph F2_load;		// load estimate of 2-items graph

  // stats
  void compute_stats();
  void print_db_stats(ostream& out);
  void print_load_estimate(ostream& out);
  int total_tlen;		// total txn length
  double avg_tlen;		// average txn length
  double dev_tlen;
  int F2_edge_weight;		// F2 edge weight
  double F2_sparsity;		// F2 sparsity
  double avg_F2_deg;		// average degree of f2 graph
  double dev_F2_deg;
  double avg_F2_edge_weight;	// average F2 edge weight
  double dev_F2_edge_weight;
  int min_F2_deg, max_F2_deg;

  string db_prefix;

private:
  Transaction_Set transaction_set_ext;

#ifdef USE_KDCI
  KDCI::dci_items *dci_counters;
  unsigned int max_support;  // the same as nr_of_trans
  unsigned int max_trans_len;
#endif

public:
  Transaction_Set & transaction_set;

  // make a rough load estimate of this dataset
  double estimate_load_all(Load_Estimate_Func f);

  // make a rough load estimate of this vertical projection of dataset
  double estimate_load(Load_Estimate_Func f, set<int>& A);

  int size_items(set<int>& A);	// how large under itemset A?
  int approx_3patterns(set<int>& A); // make a rough estimate of number of 3 patterns
  double estimate_fpgrowth_load(set<int>& A); // make a rough estimate of number of 3 items
  double kkstar_load(set<int>& A);
  double linear_load_graph(set<int>& A);
  double f2_weight(set<int>& A);

private:
  void count_items();
  bool relative_support;
};

inline bool TS_Counter::freq_item(int i)
{
  return count[i]>=support;
}

#endif
