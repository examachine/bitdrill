//
//
// C++ Interface for module: Hash_Trie
//
// Description: A trie that uses hash maps for indexing pointers
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef Hash_Trie_Interface
#define Hash_Trie_Interface

#include "generic/General.hxx"
#include <ext/hash_map>
#include <boost/pool/pool_alloc.hpp>
//#include <ext/pool_allocator.h>
using namespace __gnu_cxx;

template <typename Symbol>
class Hash_Trie
{
public:

//   typedef hash_map<Symbol, Hash_Trie<Symbol>*,
// 		   hash<Symbol>, equal_to<Symbol>,
// 		   boost::pool_allocator< pair<Symbol, Hash_Trie<Symbol>* > > 
// 		   > Hash_Map;

  typedef hash_map<Symbol, Hash_Trie<Symbol>* > Hash_Map;

  Hash_Trie<Symbol>() : endmark(false) {
  }

  ~Hash_Trie<Symbol>() {
    clear();
  }

  bool empty() {
    return subtries.empty(); 
  }

  void clear() {
    // deallocate recursively
    typename Hash_Map::iterator it = subtries.begin();
    for (; it != subtries.end(); it++) {
      delete it->second;
    }
    subtries.clear();
    endmark = false;
  }

  void insert(const vector<Symbol>& s) {
    insert_iterator(s.begin(), s.end());
  }

  void insert_iterator(typename vector<Symbol>::const_iterator first,
		       typename vector<Symbol>::const_iterator last) {
    if (first==last) {
      endmark = true;
      return;
    }
    else {
      // check if first character exists
      Hash_Trie<Symbol>* subtrie = find_symbol(*first);
      if (subtrie)
	// tail recursion
	subtrie->insert_iterator(++first, last);
      else
	insert_newbranch(first, last);
    }
  }

  void insert_newbranch(typename vector<Symbol>::const_iterator first,
			typename vector<Symbol>::const_iterator last) {
    if (first==last) {
      endmark = true;
      return;
    }
    else {
      Hash_Trie<Symbol>* subtrie = new Hash_Trie<Symbol>();
      subtries[*first] = subtrie;
      // I love tail recursion
      subtrie->insert_newbranch(++first, last);
    }
  }

  bool query(const vector<Symbol>& s) {
    return query_iterator(s.begin(), s.end());
  }

  bool query_iterator(typename vector<Symbol>::const_iterator first,
		      typename vector<Symbol>::const_iterator last) {
    if (first==last) {
      return endmark;
    }
    else {
      // check if first character exists
      Hash_Trie<Symbol>* subtrie = find_symbol(*first);
      if (subtrie)
	return subtrie->query_iterator(++first, last);
      else
	return false;
    }
  }

  friend class Walk;

  // DFS walk in trie
  struct Walk
  {
    Walk(Hash_Trie<Symbol>* t) {
      trie_stack.push(t);
      iter_stack.push(t->subtries.begin());
    }

    Walk(const Walk& w)
      : trie_stack(w.trie_stack), iter_stack(w.iter_stack),
	current_string(w.current_string) {
    }

    bool next() {
      if (trie_stack.empty()) {
	return false;
      }
      do {
	typename Hash_Trie<Symbol>::Hash_Map::iterator it = iter_stack.top();
	if (it == trie_stack.top()->subtries.end() ) {
	  // subbranch over. pop back.
	  trie_stack.pop();
	  iter_stack.pop();
	  current_string.pop_back();
	  // advance iterator to explore remaining subbranches
	  it = iter_stack.top();
	  it++;
	  if (current_string.size()==0 && it == trie_stack.top()->subtries.end() ) {
	    trie_stack.pop();
	    iter_stack.pop();
	    return false;
	  }

	  iter_stack.pop();
	  iter_stack.push(it);
	}
	else {
	  // add arc symbol
	  current_string.push_back(it->first);
	  //cout << "cur str = " << current_string << endl;
	  // depth first
	  trie_stack.push(it->second);
	  iter_stack.push(it->second->subtries.begin());
	}
      }
      while (trie_stack.empty() || !trie_stack.top()->endmark);
      return !trie_stack.empty();
    }

    vector<Symbol>& ref() {
      return current_string;
    }

    bool end() {
      return trie_stack.empty();
    }

    stack< Hash_Trie<Symbol>* > trie_stack;
    stack< typename Hash_Trie<Symbol>::Hash_Map::iterator > iter_stack; 
    vector<Symbol> current_string;

  };

  void print(vector<Symbol> str = vector<Symbol>()) {
    // print elements in DFS order
    if (endmark) {
      cout << "elt: " << str << endl;
    }
    typename Hash_Map::iterator it = subtries.begin();
    for (; it != subtries.end(); it++) {
      vector<Symbol> str2 = str;
      str2.push_back(it->first);
      it->second->print(str2);
    }
  }


private:

  Hash_Trie* find_symbol(Symbol symbol) {
    typename Hash_Map::iterator it = subtries.find(symbol);
    if (it!=subtries.end()) {
      return subtries[symbol];
    }
    else
      return 0;
  }

  Hash_Map subtries;

  bool endmark;
};


#endif
