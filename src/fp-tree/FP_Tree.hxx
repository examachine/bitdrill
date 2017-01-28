//
//
// C++ Interface for module: FP_Tree
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FP_Tree_Interface
#define FP_Tree_Interface

#include "generic/General.hxx"
// #include "structure/Multi_Trie.hxx"
#include "FP_Trie.hxx"
#include "Symbol.hxx"
#include "transaction-db/Transaction.hxx"

// This is trie interface
// template <typename Symbol, typename String>
// class FP_Tree : public FP_Trie_Impl<Symbol, String>

class FP_Tree : public FP_Trie_Impl<int, vector<int> >
{
public:
  typedef int Symbol;
  typedef vector<int> String;

  typedef map< int, Node* > Item_Table;

  typedef FP_Trie_Impl<Symbol, String> Base;

  FP_Tree(const set<int> &f, const vector<int> & c,
	  map<int,int> & counts, bool dbg=false);

  FP_Tree(int num_items, map<int,int>& counts, bool dbg=false);

  Node* insert(const Transaction& t);

  Node* query(const Transaction& t);

  void discover_patterns(int epsilon, ostream* pattern_out = 0);

  void test_path(const vector<int>& string);

  Node* node_list(int item) {
    return item_table[item];    
  }

  const Item_Table & get_item_table() {
    return item_table;
  }
  void print_node_links(ostream& out);
  void print_pattern_counts(ostream& out);

private:
  // compare items according to their counts
  struct Greater_Freq
  {
    Greater_Freq(const vector<int>& c) : count(c) {}

    bool operator() (int i, int j) {
      return count[i] > count[j];
    }
    const vector<int> & count;
  };
  Greater_Freq greater_freq;

  Node* query_ordered(vector<int> string);
  Node* insert_ordered(vector<int> string);
  void fp_growth(int epsilon, const Pattern& alpha,
		 ostream* pattern_out);
  FP_Tree* cons_conditional_fp_tree(int epsilon, int symbol);
  void count_symbols(int epsilon, int symbol, set<int> &f, vector<int> & c);
  void count_prefix_path(Node* node, vector<int>& count);
  void clean_prefix_path_marks(vector<Node*> &node_vec);
  void insert_cond_patterns(FP_Tree* fp_tree, Node* symbol_list);
  void insert_cond_pattern(FP_Tree* fp_tree, Node & node);
  void get_pattern(Node& node, Pattern& pat);
  void insert(const Pattern& pat);
  int minimum_count(const Path& path);

  Item_Table item_table;
  const set<int>& F;
  vector<int> own_count;
  const vector<int>& count;
  vector<int> F_vec;
  vector<int> in_F;
  set<int> own_F;
  map<int,int>& pattern_counts;
  bool debug;
  void sort_decreasing_freq(vector<int>& t_i);
  void quiksort(vector<int>& A, int p, int r);
  int partition(vector<int>& A, int p, int r);
  void count_quiksort(vector<int>& A, int p, int r);
  int count_partition(vector<int>& A, int p, int r);
  void prune_pattern(Pattern& pat);
  void prune_path(Path& path, int epsilon);

  class node_infrequent {
  public:
    node_infrequent(int e) : epsilon(e) {}
    bool operator() (Node_Info& x) {
      return x.count < epsilon; 
    }
    const int epsilon;
  };

  map<int,int> counts;

  void print_combinations(const Path& path, const Pattern& prefix,
			  ostream* pattern_out);
  void dbg_print_tree(const Pattern& pat);
};

inline void FP_Tree::print_node_links(ostream& out)
{
  for (Item_Table::iterator item_list = item_table.begin();
       item_list != item_table.end(); item_list++) {
    const Node* item_node = item_list->second;
    //int symbol = item_node->symbol;
    //cout << " " << symbol << endl;
    while (item_node) {
      out << item_node->symbol << ":"
	  << item_node->count << " ";    
      item_node = item_node->next_node;	
    }
    out << endl;
  }
}


#endif
