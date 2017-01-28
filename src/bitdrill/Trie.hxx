//
//
// C++ Interface for module: Trie
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef Trie_Interface
#define Trie_Interface

#include "generic/General.hxx"
#include "Vertical_DB.hxx"

#include <boost/pool/pool_alloc.hpp>
#include <ext/slist>
using namespace __gnu_cxx;
//#include "Pool_Allocator.hxx"

//BUG: inserts a redundant node at the leaf 
template <typename Symbol>
class Trie
{
public:

  typedef slist< pair<Symbol,Trie*> > Trie_List;

  typedef Trie<Symbol> Self;

  typedef boost::fast_pool_allocator< pair<Symbol, Trie* > > Alloc;

  typedef boost::fast_pool_allocator< Self > Alloc2;
  //typedef pool_allocator< pair<Symbol, Trie* > > Alloc;

  static Self def_node;

  // constructor
  Trie<Symbol>() : endmark(false) {
  }

  // destructor
  ~Trie<Symbol>() {
    // deallocate recursively
    typename Trie_List::iterator it = subtries.begin();
    for (; it != subtries.end(); it++) {
      //delete it->second;
      Alloc2 a;
      a.destroy(it->second);
      a.deallocate(it->second);
    }
    //subtries.clear();
    endmark = false;
  }

  bool empty() const {
    return subtries.empty(); 
  }

  int num_children() const {
    return subtries.size();
  }

  slist< pair<Symbol,Trie*>, Alloc >* get_subtries() const {
    return &subtries;
  }

  const Self *get_last_subtrie() const {
    // iterate slist and get last subtrie, 0 if no subtries.
    const Self* last = 0;
    typename Trie_List::const_iterator it = subtries.begin();
    for (; it != subtries.end(); it++) {
      last = it->second;
    }
    return last;
  }

  void clear() {
    // deallocate recursively
    typename Trie_List::iterator it = subtries.begin();
    for (; it != subtries.end(); it++) {
      //delete it->second;
      Alloc2::destroy(it->second);
      Alloc2::deallocate(it->second);
    }
    subtries.clear();
    endmark = false;
  }

  void insert(const vector<Symbol>& s) {
    insert_iterator(s.begin(), s.end());
  }

  template <typename It>
  void insert_iterator(It first,
		       It last) {
    if (first==last) {
      endmark = true;
      return;
    }
    else {
      // check if first character exists
      Self* subtrie = find_symbol(*first);
      if (subtrie)
	// tail recursion
	subtrie->insert_iterator(++first, last);
      else
	insert_newbranch(first, last);
    }
  }

  template<typename It>
  // vector<Symbol>::const_iterator
  void insert_newbranch(It first,
			It last) {
    if (first==last) {
      endmark = true;
      return;
    }
    else {
      //Self* subtrie = new Self();
      Alloc2 a;
      Self* subtrie = a.allocate();
      a.construct(subtrie, def_node);

      subtries.push_front(pair<Symbol, Trie*>(*first, subtrie));
      // I love tail recursion
      subtrie->insert_newbranch(++first, last);
    }
  }

  bool query(const vector<Symbol>& s) const {
    return query_iterator(s.begin(), s.end());
  }

  template <typename Iterator>
  bool query_iterator(Iterator first,
		      Iterator last) const {
    if (first==last) {
      return endmark;
    }
    else {
      // check if first character exists
      Self* subtrie = find_symbol(*first);
      if (subtrie)
	return subtrie->query_iterator(++first, last);
      else
	return false;
    }
  }

  bool check_supports(const vector<Symbol>& itemset) {
    /*
    //TODO: iterate supports without copying anything
    struct Support_Iterator
    {
      Support_Iterator(const vector<int>& itemset) {
	vit = itemset.begin();
      };
      vector<int>::const_iterator vit;
    };
    */
    vector<Symbol> support;
    support.resize(itemset.size()-1);
    for (typename vector<Symbol>::const_iterator it = itemset.begin();
	 it!=itemset.end(); it++) {
      copy(itemset.begin(), it, support.begin());
      copy(it+1, itemset.end(), support.begin() + 
	   (it - itemset.begin()));
      if (!query(support))
	return false;
    }
    return true;
  }


  void print(ostream& out = nlog, vector<Symbol> str = vector<Symbol>()) const {
    // print elements in DFS order
    if (endmark) {
      out << "elt: " << str << endl;
    }
    typename Trie_List::const_iterator it = subtries.begin();
    for (; it != subtries.end(); it++) {
      vector<Symbol> str2 = str;
      str2.push_back(it->first);
      it->second->print(out, str2);
    }

  }


  int size() {
    int s = 0;
    // print elements in DFS order
    if (endmark)
      s++;
    typename Trie_List::iterator it = subtries.begin();
    for (; it != subtries.end(); it++)
       s += it->second->size();
    return s;
  }

  void encode(vector<int>& M) const {
    // encode recursively
    if (endmark)
      M.push_back(0); // end mark
    else
      M.push_back(1); // no end mark
    if (subtries.size()>0) {
      typename Trie_List::const_iterator it = subtries.begin();
      for (; it != subtries.end(); it++) {
	M.push_back(2); // recursively encoding a child
	M.push_back(it->first); // arc symbol
	it->second->encode(M);
      }
    }
    M.push_back(3); // finish coding this node
  }

  vector<int>::const_iterator decode_it(vector<int>::const_iterator it, 
		 vector<int>::const_iterator end) {
    if (it!=end) {
      int code = *it++;
      if (code==0)
	endmark = true;
      code = *it++;
      while (code==2) { // decode a child
	int symbol = *it++;
	// merge information into existing trie
	Self* subtrie = find_symbol(symbol);
	if (!subtrie) {
	  Alloc2 a;
	  subtrie = a.allocate();
	  a.construct(subtrie, def_node);
	  subtries.push_front(pair<Symbol, Trie*>(symbol, subtrie));
	}
	it = subtrie->decode_it(it, end);
	code = *it++; // leave it at the beginning of next chunk
      }
      assert(code==3); // look for end code
    }
    return it;
  }

  void decode(const vector<int>& M) {
    decode_it(M.begin(), M.end());
  }

  friend class Walk;

  // BUG: other walks may crash when trie is empty
  // DFS walk in trie
  struct Walk
  {
    Walk(Trie<Symbol>* t) {
      if (!t->empty()) {
	trie_stack.push_back(t);
	iter_stack.push_back(t->subtries.begin());
      }
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
	typename Trie<Symbol>::Trie_List::const_iterator it = iter_stack.back();
	if (it == trie_stack.back()->subtries.end() ) {
	  // subbranch over. pop back.
	  trie_stack.pop_back();
	  iter_stack.pop_back();
	  current_string.pop_back();
	  // advance iterator to explore remaining subbranches
	  it = iter_stack.back();
	  it++;
	  if (current_string.size()==0 && it == trie_stack.back()->subtries.end() ) {
	    trie_stack.pop_back();
	    iter_stack.pop_back();
	    return false;
	  }

	  iter_stack.pop_back();
	  iter_stack.push_back(it);
	}
	else {
	  // add arc symbol
	  current_string.push_back(it->first);
	  //cout << "cur str = " << current_string << endl;
	  // depth first
	  trie_stack.push_back(it->second);
	  iter_stack.push_back(it->second->subtries.begin());
	}
      }
      while (trie_stack.empty() || !trie_stack.back()->endmark);
      return !trie_stack.empty();
    }
    const vector<Symbol>& ref() const {
      return current_string;
    }

    bool end() const {
      return trie_stack.empty();
    }

    vector<const Self*> trie_stack;
    vector< typename Self::Trie_List::const_iterator > iter_stack; 
    vector<Symbol> current_string;

  };

  friend class Walk_K;

  // DFS walk in trie up to depth k
  struct Walk_K
  {
    Walk_K(const Trie<Symbol>* t, unsigned int _k)
      : k(_k) {
      trie_stack.push_back(t);
      iter_stack.push_back(t->subtries.begin());
    }

    Walk_K(const Walk_K& w)
      : k(w.k), trie_stack(w.trie_stack), iter_stack(w.iter_stack),
	current_string(w.current_string)  {
    }

    bool next() {
      if (trie_stack.empty()) {
	return false;
      }
      do {
	typename Trie<Symbol>::Trie_List::const_iterator it = iter_stack.back();
	if (it == trie_stack.back()->subtries.end() || 
	    current_string.size() == k) {
	  // subbranch over. pop back.
	  trie_stack.pop_back();
	  iter_stack.pop_back();
	  current_string.pop_back();
	  // advance iterator to explore remaining subbranches
	  if (iter_stack.empty())
	    return false;
	  it = iter_stack.back();
	  it++;
	  if (current_string.size()==0 && it == trie_stack.back()->subtries.end() ) {
	    trie_stack.pop_back();
	    iter_stack.pop_back();
	    return false;
	  }
	  iter_stack.pop_back();
	  iter_stack.push_back(it);
	}
	else if (current_string.size() < k) {
	  // add arc symbol
	  current_string.push_back(it->first);
	  //cout << "cur str = " << current_string << endl;
	  // depth first
	  trie_stack.push_back(it->second);
	  iter_stack.push_back(it->second->subtries.begin());
	}
      }
      while (trie_stack.empty() || current_string.size() < k);
      return !trie_stack.empty();
    }

    const vector<Symbol>& ref() const {
      return current_string;
    }

    const Trie<Symbol>* ref_node() const {
#ifdef DEBUG
      assert(!trie_stack.empty());
#endif
      return trie_stack.back();
    }

    bool end() {
      return trie_stack.empty();
    }

    unsigned int k;
    vector<const Self*> trie_stack;
    vector< typename Self::Trie_List::const_iterator > iter_stack; 
    vector<Symbol> current_string;

  };


  friend class Walk_Tidlist;

  // DFS walk in trie
  struct Walk_Tidlist
  {
    Walk_Tidlist(const Trie<Symbol>* t, const Vertical_DB& db, int k)
      : vertical_db(db), sentinel(0)
    {
      trie_stack.push_back(t);
      iter_stack.push_back(t->subtries.begin());
      tidlist_cache.resize(k); 
      for (int i=0; i<k; i++) {
	tidlist_cache[i].reset(vertical_db.get_num_txn(), 
			       vertical_db.count[vertical_db.freq_to_item(0)]);
      }
    }

    Walk_Tidlist(const Walk_Tidlist& w)
      : trie_stack(w.trie_stack), iter_stack(w.iter_stack),
	current_string(w.current_string), tidlist_cache(w.tidlist_cache),
	vertical_db(w.vertical_db), sentinel(0) {
    }

    Walk_Tidlist(const Walk_K& w, const Vertical_DB& db, int depth, 
		 const Self* _sentinel = 0)
      : trie_stack(w.trie_stack), iter_stack(w.iter_stack),
	current_string(w.current_string), vertical_db(db),
	sentinel(_sentinel) {
      tidlist_cache.resize(depth);

      // reconstruct cache
      for (unsigned int depth = 0; depth<current_string.size(); depth++) {
	if (depth==0)
	  tidlist_cache[0] = vertical_db.item(current_string[0]);
	else if (depth>0) {
	  tidlist_cache[depth-1].intersect
	    (vertical_db.item(current_string[depth]), tidlist_cache[depth] );
	}
      }
    }

    ~Walk_Tidlist() {
      
    }

    bool next() {
      if (trie_stack.empty() || trie_stack.back()==sentinel) {
	return false;
      }
      do {
	typename Trie<Symbol>::Trie_List::const_iterator it = iter_stack.back();
	if (it == trie_stack.back()->subtries.end() ) {
	  // subbranch over. pop back.
	  trie_stack.pop_back();
	  iter_stack.pop_back(); 
	  current_string.pop_back();
	  // advance iterator to explore remaining subbranches
	  it = iter_stack.back();
	  it++;
	  if (current_string.size()==0 && it == trie_stack.back()->subtries.end() ) {
	    trie_stack.pop_back();
	    iter_stack.pop_back();
	    return false;
	  }
	  iter_stack.pop_back();
	  iter_stack.push_back(it);
	}
	else {
	  // add arc symbol
	  current_string.push_back(it->first);
	  //cout << "cur str = " << current_string << endl;
	  // depth first
	  trie_stack.push_back(it->second);
	  iter_stack.push_back(it->second->subtries.begin());
	  int depth = current_string.size();
	  if (depth==1) {
	    tidlist_cache[0] = vertical_db.item(it->first);
	  }
	  else {
	    //tidlist_cache[depth-1] = tidlist_cache[depth-2] & 
	    //  vertical_db.item(it->first);
	    tidlist_cache[depth-2].intersect(vertical_db.item(it->first),
					     tidlist_cache[depth-1] );
	  }
	}
      }
      while (trie_stack.empty() || !trie_stack.back()->endmark);
      return !trie_stack.empty();
    }

    const vector<Symbol>& ref() const {
      return current_string;
    }

    const Tid_List& tidlist() {
      return tidlist_cache[current_string.size()-1];
    }

    bool end() const {
      return trie_stack.empty() || trie_stack.back()==sentinel;
    }

    vector< const Self*> trie_stack;
    vector< typename Self::Trie_List::const_iterator > iter_stack; 
    vector<Symbol> current_string;
    vector< Tid_List > tidlist_cache;
    const Vertical_DB& vertical_db;
    const Self* sentinel;
  };


  friend class Candidate_Walk;

  // class to walk candidates
  struct Candidate_Walk
  {
    Candidate_Walk(const Walk& w)
      : parent(w.trie_stack[w.trie_stack.size()-2]),
	it(w.iter_stack[w.iter_stack.size()-2]) { }

    Candidate_Walk(const Walk_Tidlist& w)
      : parent(w.trie_stack[w.trie_stack.size()-2]),
	it(w.iter_stack[w.iter_stack.size()-2]) { }

    Candidate_Walk(Self *p, typename Self::Trie_List::iterator i)
      : parent(p), it(i) {}

    bool next() {
      it++;
      return ! end();
    }

    Symbol ref() {
      return it->first;
    }

    bool end() {
      return (it == parent->subtries.end());
    }

    const Self *parent; // Parent of k-1 length itemset
    typename Self::Trie_List::const_iterator it; // iterator    
  };


  static void release_mem() {
    boost::singleton_pool<boost::pool_allocator_tag, sizeof(pair<Symbol, Trie* >)>::release_memory();
    boost::singleton_pool<boost::pool_allocator_tag, sizeof(Self)>::release_memory();
  }


private:

  Trie* find_symbol(Symbol symbol) const {
    typename slist< pair<Symbol,Trie*> >::const_iterator it = subtries.begin();
    for (; it!=subtries.end(); it++) {
      if (symbol==it->first)
	return it->second;
    }
    return 0;
  }

  slist< pair<Symbol,Trie*>, Alloc > subtries;
  bool endmark;


};

template <class T> Trie<T> Trie<T>::def_node; // static member of Trie :/

template <class T>
inline ostream& operator<<(ostream& out, const Trie<T>&t) {
  t.print(out);
  return out;
}
#endif
