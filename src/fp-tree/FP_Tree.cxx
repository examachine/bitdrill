//
//
// C++ Implementation for module: FP_Tree
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "FP_Tree.hxx"
#include "math/Choose.hxx"

FP_Tree::FP_Tree(const set<int> &f, const vector<int> & c,
		 map<int,int> &pat_counts, bool dbg)
  :  Base(blank(String())), greater_freq(c), F(f), count(c),
     pattern_counts(pat_counts), 
     debug(dbg)
{
}

FP_Tree::FP_Tree(int num_items, map<int,int>& counts, bool dbg)
  : Base(blank(String())), 
    greater_freq(own_count), F(own_F), own_count(num_items, 0),
    count(own_count), pattern_counts(counts), debug(dbg)
{
}

FP_Tree::Node* FP_Tree::insert(const Transaction& _t) {
//   cerr << "t_in=" << _t << endl;
  Transaction t = _t;
  if (debug) cerr << "insert t " << t ;
  t.prune_not_in(F);
  if (debug) cerr << ", pruned  t " << t ;
  sort_decreasing_freq(t);
  if (debug) cerr << ", ordered = " << t << endl;
  return insert_ordered(t);
}

FP_Tree::Node* FP_Tree::insert_ordered(vector<int> string) {
  Symbol symbol = blank(string);
  add_prefix(string, symbol);
  Node* node = Base::insert(string, item_table);
  assert(node);
  return node;
}

FP_Tree::Node* FP_Tree::query(const Transaction& _t) {
  Transaction t = _t;
  t.prune_not_in(F);
  sort_decreasing_freq(t);
  return query_ordered(t);
}

FP_Tree::Node* FP_Tree::query_ordered(vector<int> string) {
  Symbol symbol = blank(string);
  add_prefix(string, symbol);
  return Base::query(string);
}

void FP_Tree::test_path(const vector<int>& string) {
  Node* node = query(string);
  if (node) {
    while (node) {
      cout << "node " << (char)node->symbol << " : ";
      node = node->parent;
    }
    cout << endl;
  }
}

void FP_Tree::discover_patterns(int epsilon, ostream* pattern_out)
{
  Pattern null;
  null.count = 0;
  if (pattern_out) {
    for (set<int>::iterator it=F.begin(); it!=F.end(); it++) {
      *pattern_out << *it << " (" << count[*it] << ")" << endl;
    }
  }
  fp_growth(epsilon, null, pattern_out);
}

// find the minimum count in a given appt
int FP_Tree::minimum_count(const Path& path)
{
  if (path.size()>=0) {
    Path::const_iterator i = path.begin();
    int min_count = i->count;
    i ++;
    for (; i!=path.end(); i++) {
      const Node_Info& node_info= *i;
      if (node_info.count < min_count)
	min_count = node_info.count;
    }
    return min_count;
  }
  else
    return 0;
}

// FP-Growth algorithm
void FP_Tree::fp_growth(int epsilon, const Pattern& alpha,
			ostream* pattern_out/*, vector<int>& count*/)
{
  dbg_print_tree(alpha);

  if (alpha.symbol_set.size()>0 && alpha.count>epsilon) {
//     *pattern_out << "pattern: " << alpha << endl;
#ifdef DEBUG
    if (debug) cout << "* growing pattern: " << alpha << endl;
#endif
  }

  // if there is a single path
  if (single_path()) {
    // generate a pattern for each combination (denoted as beta)
    // of nodes in path P

    // get path
    Path path;
    get_single_path(path);

    // generate pattern beta union alpha with
    // support = minimum support of nodes in beta

    // instead of generating every combination of beta
    // which is exponential we just output a concise
    // description of that data
    prune_path(path, epsilon);
    if (path.size()>0) {
      // minimum_count(path)>=epsilon
//       *pattern_out << "XPattern (Pattern ";
//       print_path(*pattern_out, path);
//       *pattern_out << ") (" << alpha << ")" << endl;
      if (pattern_out) {
	print_combinations(path, alpha, pattern_out);
	// compute counts;
	int n = path.size();
	int k;
	for (k=1; k<=n; k++) {
	  int l = k + alpha.symbol_set.size();
	  if (pattern_counts.find(l)==pattern_counts.end())
	    pattern_counts[l] = 0;
	  pattern_counts[l] += choose(n, k);
	}
      }
#ifdef DEBUG
      if (debug) {
 	cout << "discovered patterns: 2^(";
 	print_path(cout, path);
 	cout << ") U " << alpha << endl;
      }
#endif
    }
  }
  else {
    // for each item symbol in item table
    for (Item_Table::iterator item_list = item_table.begin();
	 item_list != item_table.end(); item_list++) {
      Node* item_list_head = item_list->second;

      int symbol = item_list_head->symbol;
//       int item_count = item_list_head->count;

      // find patterns containing this symbol
#ifdef DEBUG
      if (debug) {
 	cout << "* mining item " << symbol << endl;
      }
#endif

      // construct conditional FP-tree on symbol

      // generate pattern beta = symbol U alpha
      Pattern beta(alpha);
      beta.symbol_set.push_back(symbol);
      // update beta's count
      beta.count = count[symbol];
#ifdef DEBUG
      if (debug) {
	cout << "beta = " << beta << endl;
      }
#endif
      // output pattern
      int betalen = beta.symbol_set.size();
      if (betalen>1 && beta.count >= epsilon) {
	if (pattern_out) {
	  *pattern_out << beta << endl;
	  if (pattern_counts.find(betalen)==pattern_counts.end()) {
	    pattern_counts[betalen] = 0;
	  }
	  pattern_counts[betalen] = pattern_counts[betalen]+1;
	}
      }

      // construct a conditional fp tree
      FP_Tree* cond_fp_tree = cons_conditional_fp_tree(epsilon, symbol);
      if (!cond_fp_tree->is_empty()) {
	// recursively mine cond_fp_tree
	cond_fp_tree->fp_growth(epsilon, beta, pattern_out);
      }
      delete cond_fp_tree;
    }
  }
}

FP_Tree* FP_Tree::cons_conditional_fp_tree
( int epsilon, int symbol/*, const set<int> &f, const vector<int> & c*/)
{
  Node* symbol_list = item_table[symbol];

  // construct new fp tree
  FP_Tree* fp_tree = new FP_Tree(count.size(), pattern_counts, debug);

  // first count all symbols in the pattern set
  // composed by prefix paths of those symbols
  count_symbols(epsilon, symbol, fp_tree->own_F, fp_tree->own_count);

  // insert prefix paths of all symbols in the fp_tree
  insert_cond_patterns(fp_tree, symbol_list);

  return fp_tree;
}

// count the frequency of all symbols
// in an fp-tree without having to generate
// an intermediate structure!
void FP_Tree::count_symbols(int epsilon, int symbol,
			    set<int> & F,
			    vector<int> & count)
{
  Node* item_node = item_table[symbol];

  // count symbols
#ifdef DEBUG
  if (debug)
    cerr << "count_symbols: " << symbol << endl;
#endif

  // count prefix paths of item
  while (item_node) {
    // count prefix path of that node
    count_prefix_path(item_node, count);
    item_node = item_node->next_node;
  }

#ifdef DEBUG
  if (debug)
    cerr << "count: " << count << endl;
#endif

  // determine frequent symbols
  int curr_symbol = 0;
  for (vector<int>::iterator freq=count.begin(); freq!=count.end();
       freq++, curr_symbol++) {
    if (*freq>=epsilon) {
      F.insert(curr_symbol);
    }
  }
#ifdef DEBUG
  if (debug) {
    cerr << "F' = " << F << endl;
  }
#endif
}

// count the frequency of items in a prefix path
// how does this work?
// this scans the prefix paths of a given node since the pattern
// corresponding to that path has the count of node, simply add that count
// to symbols in the prefix
void FP_Tree::count_prefix_path(Node* node, vector<int>& count)
{
  int prefix_count = node->count;
  node = node->parent;		// exclude starting node, we want prefix
  while (node->parent) {
#ifdef DEBUG
    if (debug) {
      cout << "examining node " << node->symbol << ":"
	   << node->count << endl;
    }
#endif
    count[node->symbol] += prefix_count;
    node = node->parent;
  }
}

// OBSOLETE! ERASED!
void FP_Tree::clean_prefix_path_marks(vector<Node*> &node_vec)
{
  assert(0);			// obsolete
  for (vector<Node*>::iterator i = node_vec.begin();
       i!=node_vec.end(); i++) {
    Node* node = *i;
    assert(node->count <= 0);
    node->count = -node->count;
  }
}

// insert conditional patterns of a symbol
// to form a conditional fp-tree
void FP_Tree::insert_cond_patterns(FP_Tree* fp_tree, Node* symbol_list)
{
  Node* node = symbol_list;
  // insert each prefix path of this node
  while (node) {
    insert_cond_pattern(fp_tree, *node);
    node = node->next_node;
  }
}

void FP_Tree::insert_cond_pattern(FP_Tree* fp_tree, Node & node)
{
  // get pattern of the prefix of node's symbol
  Pattern pattern;
  get_pattern(node, pattern);
 
  // insert that path in the new fp tree
  fp_tree->insert(pattern);

}

// get the pattern starting from node, *and* excluding node
// this corresponds to the conditional pattern base elements
// in Han's paper (called transformed prefix path)
void FP_Tree::get_pattern(Node& node, Pattern& pat)
{
  assert(pat.symbol_set.size()==0);
  // KLUDGE: order doesn't matter
  // -- may impede performance later but do it anyway
  if (node.parent) {
    // pattern count is equal to the leaf count
    pat.count = node.count;	// this is the minimum count in the prefix
#ifdef DEBUG
    if (debug)
      cerr << "getting pattern "
	   << (char)node.symbol << ":" << node.count << endl;
#endif
    Node* curr_node = node.parent; // exclude current node
    while (curr_node->parent) {
      pat.symbol_set.push_back(curr_node->symbol);
      curr_node = curr_node->parent;
    }
  }
  else {
    pat.count=0;
  }
//   cerr << "got pattern " << pat << endl;
    
}

void FP_Tree::insert(const Pattern& _pat)
{
  Pattern pat = _pat;
#ifdef DEBUG
  if (debug) {
    cerr << "insert pattern " << pat << endl;
  }
#endif
  // prune symbols in path that are not frequent
  prune_pattern(pat);
  sort_decreasing_freq(pat.symbol_set);
  //   vector<char> t_c; copy(t.begin(), t.end(), back_inserter(t_c));
#ifdef DEBUG
  if (debug) {
    cerr << "inserted pattern = " << pat << endl;
  }
#endif
  // KLUDGE: but what can I do really? :P
  Symbol symbol = blank(pat.symbol_set);
  add_prefix(pat.symbol_set, symbol);
  Base::insert_pattern(pat, item_table);
}

// sort items in a transaction, first in order of decreasing frequency
// and then sort blocks with the same frequency in order of increasing item
void FP_Tree::sort_decreasing_freq(vector<int>& A)
{
  // 1. sort in order of decreasing frequency!
  count_quiksort(A, 0, A.size()-1);
  if (debug)
    cout << ", count_quiksort(A)=" << A << endl;

  // 2. scan for blocks with same count
  int i = 0;
  int n = A.size()-1;		// last index
  while (i < n) {
    int x = A[i];
    int j = i ;
    // find the last element with same count as x
    do {
      j++;
    } while (j<=n && count[A[j]] == count[x]);
    j--;
    if (j>i)
      quiksort(A,i,j);		// order in increasing indices!
    i++;
  }

}


// quick sort as we know it, sort in increasing order
void FP_Tree::quiksort(vector<int>& A, int p, int r)
{
  if (p < r) {
    int q = partition(A, p, r);
    quiksort(A, p, q);
    quiksort(A, q+1, r);
  }
  
}

int FP_Tree::partition(vector<int>& A, int p, int r)
{
  int x = A[p];
  int i = p - 1;
  int j = r + 1;
  while (1) {
    do {
      j = j - 1;
    } while (A[j] > x);
    do {
      i = i + 1;
    } while (A[i] < x);
    if (i < j)
      swap(A[i], A[j]);
    else
      return j;
  }
}

// sort according to *decreasing* count of items in array A
void FP_Tree::count_quiksort(vector<int>& A, int p, int r)
{
  if (p < r) {
    int q = count_partition(A, p, r);
    count_quiksort(A, p, q);
    count_quiksort(A, q+1, r);
  }
}

int FP_Tree::count_partition(vector<int>& A, int p, int r)
{
  int x = A[p];
  int i = p - 1;
  int j = r + 1;
  while (1) {
    do {
      j = j - 1;
    } while (count[A[j]] < count[x]);
    do {
      i = i + 1;
    } while (count[A[i]] > count[x]);
    if (i<j)
      swap(A[i], A[j]);
    else
      return j;
  }
}

class fast_not_in {
public:
  fast_not_in(const vector<int> &x) : large(x) {}
  bool find(int x, int p, int q) {
    if (p==q)
      return x==large[p];
    else {
      int mid = (int) floor(double(p + q / 2));
      if (x<=large[mid])
	return find(x, p,mid);
      else
	return find(x,mid+1, q);
    }
  }
  bool operator() (int x) {
    return !find(x, 0, large.size()-1); 
  }
  const vector<int>& large;
};

void FP_Tree::prune_pattern(Pattern& pat)
{
  not_in f(F);
  pat.symbol_set.erase
    (remove_if(pat.symbol_set.begin(), pat.symbol_set.end(), f),
     pat.symbol_set.end());
//   fast_not_in f(F_vec);
}

void FP_Tree::prune_path(Path& path, int epsilon)
{
  node_infrequent f(epsilon);
  path.erase
    (remove_if(path.begin(), path.end(), f), path.end());
}

// print numbers of frequent patterns found
void FP_Tree::print_pattern_counts(ostream& out)
{
  pattern_counts[0] = 1;	// number of 0-length patterns is 1
  pattern_counts[1] = F.size();	// 1-items
  int num_patterns = 0;
  for (map<int,int>::iterator i=pattern_counts.begin();
       i!=pattern_counts.end(); i++)
    num_patterns += i->second;
  nlog << num_patterns << endl;
  for (map<int,int>::iterator i=pattern_counts.begin();
       i!=pattern_counts.end(); i++)
    nlog << i->second << endl;
}

// print all combinations in a given path
void FP_Tree::print_combinations(const Path& path, const Pattern& prefix,
				 ostream* out)
{
  list< Path >* C = gen_all_combs(path);

  typedef list< Path >::iterator it;
  for (it i=C->begin(); i!=C->end(); i++) {
    typedef Path::iterator pit;
    // print combination
    for (pit j=i->begin(); j!=i->end(); j++)
      *out << j->symbol << " ";
    // print prefix
    for (vector<int>::const_iterator j=prefix.symbol_set.begin();
	 j!=prefix.symbol_set.end(); j++)
      *out << *j << " ";
    int count;
    if (prefix.symbol_set.size()>0)
      count = min(minimum_count(*i),prefix.count);
    else
      count = minimum_count(*i);
    *out << "(" << count << ")" << endl;
  }
}

void FP_Tree::dbg_print_tree(const Pattern& pat)
{
  if (debug) {
    cout << "** FP-Tree| " << pat << endl;
    ostringstream fn_str;
    fn_str << "fp-tree";
    for (vector<int>::const_iterator i=pat.symbol_set.begin();
	 i!=pat.symbol_set.end(); i++) {
      fn_str << ".";
      if (*i < 0)
	fn_str << "root";
      else
	fn_str << *i;
    }
    fn_str << ".dot";
    string fn = fn_str.str();
    ofstream fp_tree_dot(fn.c_str());
    dot_graph(fp_tree_dot, *this);
    cout << "fp-tree " << *this << endl;
    cout << endl << "fp_tree written to " << fn << endl;
    cout << "item table" << get_item_table() << endl;
    cout << "node links" << endl;
    print_node_links(cout);
  }
}
