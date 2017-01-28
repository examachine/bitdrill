//
//
// C++ Interface for module: Multi_Trie
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef Multi_Trie_Interface
#define Multi_Trie_Interface

#include "generic/General.hxx"

// Generic trie structure
// Index must have access to its first element via first()
// and a function strip() and length()  must be defined on it
template <typename Symbol, typename String, typename Component>
class Multi_Trie_Impl
{
public:
  typedef vector<Multi_Trie_Impl*> Subtries;

  class Node {
  public:
    Symbol symbol;
    int count;
    Component component;
    Node() {}
    Node(const Symbol & s)
      : symbol(s), count(1) {}
    Node(const Symbol & s, const Component & c)
      : symbol(s), count(1), component(c) {}
  };

  // construct
  Multi_Trie_Impl() {}
  Multi_Trie_Impl(const Symbol & s) : node(s) {}
  Multi_Trie_Impl(const Symbol & s, const Component & c) : node(s,c) {}

  // destruct
  ~Multi_Trie_Impl() {
    for (Subtries::iterator i=children.begin(); i!=children.end(); i++) {
      delete *i;
    }
  }

  bool have_symbol(const String& string) {
    return node.symbol == first(string);
  }

  Multi_Trie_Impl* create_trie(const String& string, const Component& component)
  {
    assert(!empty(string));
    Symbol sym = first(string);
    String stripped = strip(string);
    if (empty(stripped)) {
      return new Multi_Trie_Impl(sym, component);
    }
    else {
      Multi_Trie_Impl* trie = new Multi_Trie_Impl(sym);
      trie->children.push_back(create_trie(stripped, component));
      return trie;
    }
  }

  // returns the subtrie node into which it has been inserted
  Node* insert(const String& string, const Component & component) {
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
      for (Subtries::iterator i=children.begin(); i!=children.end();
	   i++) {
	Node* node; 
	if (node=(*i)->insert(rest, component))
	  return node;
      }
      // nobody has it, let's create that freaking subtrie
      // which consists of a single path
      children.push_back(create_trie(rest, component));
      return children.back()->query(rest);
    }
    return 0;
  }

  Node* query(const String& string) {
    assert(!empty(string));
    if (have_symbol(string)) {

      // we've found it
      if (length(string)==1)
	return &node;

      String rest = strip(string);

      //scan children to see if they have got it
      for (Subtries::iterator i=children.begin(); i!=children.end(); i++) {
	if ((*i)->have_symbol(rest))
	  return (*i)->query(rest);
      }
    }
    else {
      // nobody's got it
      return 0;
    }
  }

private:
  Node node;
  Subtries children;
};

// This is trie interface
template <typename Symbol, typename String, typename Component>
class Multi_Trie : public Multi_Trie_Impl<Symbol, String, Component>
{
public:
  typedef Multi_Trie_Impl<Symbol, String, Component> Base;
  Multi_Trie() : Base(blank(String())) {}

  Node* query(String string) {
    Symbol symbol = blank(string);
    add_prefix(string, symbol);
    return Base::query(string);
  }

  Node* insert(String string, const Component & component)
  {
    Symbol symbol = blank(string);
    add_prefix(string, symbol);
    return Base::insert(string, component);
  }

private:
};

#endif
