//
//
// C++ Interface for module: FP_Trie
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FP_Trie_Interface
#define FP_Trie_Interface

#include "generic/General.hxx"
#include "Pattern.hxx"

// This exists because g++ has a horrible bug
// and partly because the design would have to be really twisted
// for certain workarounds...

// hacked version of Multi_Trie that allows us to use pointers properly...
// ahem, and handle parent links of course :P

// the mere existence of this class shows us that C++ is basically
// not more advanced than Assembly in terms of abstraction.

// Index must have access to its first element via first()
// and a function strip() and length()  must be defined on it
template <typename Symbol, typename String>
class FP_Trie_Impl
{
public:

  class Node {
  public:
    Symbol symbol;
    int count;
    Node* next_node;
    Node* parent;
    Node() {}
    Node(const Symbol & s)
      : symbol(s), count(1), parent(0) {}
    Node(const Symbol & s, Node* & c)
      : symbol(s), count(1), next_node(c), parent(0) {}
    Node(int i, int c) : symbol(i), count(c) {}
  };

  struct Node_Info
  {
    Node_Info() {}
    Node_Info(Node & node) : symbol(node.symbol), count(node.count) {}
    Node_Info(int i, int c) : symbol(i), count(c) {}
    int symbol;
    int count;
  };

  typedef vector<Node_Info> Path;

  typedef vector<FP_Trie_Impl*> Subtries;
  typedef map< int, Node* > Item_Table;

  // construct
  FP_Trie_Impl() {}
  FP_Trie_Impl(Symbol s) : node(s) {}
  FP_Trie_Impl(Symbol s, int c) : node(s,c) {}

  // destruct
  ~FP_Trie_Impl() {
    for (typename Subtries::iterator i=children.begin();
	 i!=children.end(); i++) {
      delete *i;
    }
  }

  // is this fp trie empty?
  bool is_empty() {
    return children.size()==0;
  }
  
  bool have_symbol(const String& string) {
    return node.symbol == first(string);
  }

  //  bool have_symbol(const Pattern& string) {
  //  return node.symbol == first(string);
  // }

  void insert_item_table(Item_Table& item_table, Node& node) {
      // attach node link
      if (item_table.find(node.symbol) == item_table.end()) {
	// if that's not in the table
	node.next_node = 0;
	item_table[node.symbol] = &node;
      }
      else {
	// okay, this node isn't linked in, let's put it
	node.next_node = item_table[node.symbol];
	item_table[node.symbol] = &node;
      }
  }

  FP_Trie_Impl* create_trie(const String& string,
			    Item_Table& item_table)
  {
    assert(!empty(string));
    Symbol sym = first(string);
    String stripped = strip(string);
    if (empty(stripped)) {
      FP_Trie_Impl* trie = new FP_Trie_Impl(sym);
      insert_item_table(item_table, trie->node);
      return trie;
    }
    else {
      FP_Trie_Impl* trie = new FP_Trie_Impl(sym);
      trie->children.push_back(create_trie(stripped, item_table));
      Node & node = trie->children.back()->node;
      node.parent = &trie->node;
      insert_item_table(item_table, trie->node);
      return trie;
    }
  }

  // returns the subtrie node into which it has been inserted
  Node* insert(const String& string, Item_Table& item_table) {
    if (empty(string))
      return 0;
    else if (have_symbol(string)) {

      // increment count of this node
      node.count++;

      // shortcut: we've already got it
      if (length(string)==1)
	return &node;

      String rest = strip(string);
      // trie (he he) to insert to children
      // and if successful return true
      for (typename Subtries::iterator i=children.begin(); i!=children.end();
	   i++) {
	Node* node = (*i)->insert(rest, item_table); 
	if (node)
	  return node;
      }
      // nobody has it, let's create that freaking subtrie
      // which consists of a single path
      children.push_back(create_trie(rest, item_table));
      children.back()->node.parent = &node;
      return children.back()->query(rest);
    }
    return 0;
  }

  FP_Trie_Impl* create_trie(const Pattern& string,
			    Item_Table& item_table)
  {
    assert(!string.symbol_set.empty());
    Symbol sym = string.symbol_set.front();
    Pattern stripped = strip(string);
    if (empty(stripped.symbol_set)) {
      FP_Trie_Impl* trie = new FP_Trie_Impl(sym, string.count);
      insert_item_table(item_table, trie->node);
      return trie;
    }
    else {
      FP_Trie_Impl* trie = new FP_Trie_Impl(sym, string.count);
      trie->children.push_back(create_trie(stripped, item_table));
      Node & node = trie->children.back()->node;
      node.parent = &trie->node;
      insert_item_table(item_table, trie->node);
      return trie;
    }
  }

  // insert a pattern
  bool insert_pattern(const Pattern& string, Item_Table& item_table) {
    if (string.symbol_set.empty())
      return false;
    else if (have_symbol(string.symbol_set)) {

      // increase count of this node
      node.count += string.count;

      // shortcut: we've already got it
      if (length(string.symbol_set)==1)
	return true;

      Pattern rest = strip(string);
      // trie (he he) to insert to children
      // and if successful return true
      for (typename Subtries::iterator i=children.begin(); i!=children.end();
	   i++) {
	if ((*i)->insert_pattern(rest, item_table))
	  return true;
      }
      // nobody has it, let's create that freaking subtrie
      // which consists of a single path
      children.push_back(create_trie(rest, item_table));
      children.back()->node.parent = &node;
      return true;
    }
    else
      return false;
  }

  Node* query(const String& string) {
    assert(!empty(string));
    if (have_symbol(string)) {

      // we've found it
      if (length(string)==1)
	return &node;

      String rest = strip(string);

      //scan children to see if they have got it
      for (typename Subtries::iterator
	     i=children.begin(); i!=children.end(); i++) {
	if ((*i)->have_symbol(rest))
	  return (*i)->query(rest);
      }
      return 0;
    }
    else {
      // nobody's got it
      return 0;
    }
  }

  const Node& get_node() const { return node; }
  const Subtries& get_children() const { return children;}

  bool single_path()
  {
    if (children.size()==0)
      return true;
    else if (children.size()==1)
      return children[0]->single_path();
    else
      return false;
  }

  void get_single_path(Path& path)
  {
    if (node.parent) {			// don't add root node to path
      path.push_back(Node_Info(node));
    }
    if (children.size()==0) {
      return;
    }
    else {
      children[0]->get_single_path(path);
    }
  }

  void print_path(ostream& out,
		  Path& path)
  {
    out << "[ ";
    int count = path.back().count;
    for (typename Path::const_iterator i = path.begin();
	 i!=path.end(); i++) {
      const Node_Info& node_info= *i;
      out << node_info.symbol << " "; // << ":" << node_info.count << " ";
    }
    out << " ] " << count;
  }

  void print_dbg_path (ostream& out,
		       Path& path)
  {
    for (typename Path::const_iterator i = path.begin();
	 i!=path.end(); i++) {
      const Node_Info& node_info= *i;
      out << (char)node_info.symbol << ":" << node_info.count << " ";
    }
  }

private:
  Node node;
  Subtries children;
};

template <typename Sym, typename Str>
ostream& operator <<(ostream& out, const FP_Trie_Impl<Sym,Str>& tree)
{
  out << "[ (" << tree.get_node().symbol << ":"
      << tree.get_node().count << ") ";
  for (typename FP_Trie_Impl<Sym,Str>::Subtries::const_iterator
	 i=tree.get_children().begin(); i!=tree.get_children().end(); i++) {
    out << **i;
  }
  out << " ]";
  return out;
}

template <typename Sym, typename Str>
void dot_graph_recursive(ostream& out,
			 const FP_Trie_Impl<Sym,Str>& tree,
			 string node = "n")
{
  char symbol = (char)tree.get_node().symbol;
  ostringstream label;
  if (symbol == -1)
    label << "\"root\"";
  else
    label << "\"("
	  << tree.get_node().symbol << ":"
	  << tree.get_node().count << ")\"";
  out << node << " [ label = " << label.str() << " ];" << endl;
  int child_no = 0;
  // generate tree structure
  for (typename FP_Trie_Impl<Sym,Str>::Subtries::const_iterator
	 i=tree.get_children().begin();
       i!=tree.get_children().end(); i++, child_no++) {
    ostringstream node_child;
    node_child << node << child_no;
    out << node << " -> " << node_child.str() << ";" << endl;
    dot_graph_recursive(out, **i, node_child.str());
  }
}

template <typename Sym, typename Str>
void dot_graph(ostream& out, const FP_Trie_Impl<Sym,Str>& tree)
{
  out << "digraph G { " << endl;
  dot_graph_recursive(out, tree);
  out << "}" << endl;
}


#endif
