//
//
// C++ Interface for module: Pattern
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef Pattern_Interface
#define Pattern_Interface

#include "generic/General.hxx"

typedef vector<int> Itemset;

// A pattern is a set of items with a count associated
struct Pattern
{
  Pattern() {}
  Pattern(const Itemset& _itemset, int cnt)
    : itemset(_itemset), count(cnt) {}

  Itemset itemset;
  int count;
  bool read(istream&);

  inline static void write_pattern(ofstream& out, const vector<int>& itemset, int freq)
  {
    for (vector<int>::const_iterator i=itemset.begin();
	 i!=itemset.end(); i++)
      out << *i << ' ';
    out << '(' << freq << ')' << endl;
  }

  void write(ofstream& out) {
    write_pattern(out, itemset, count);
  }

};

inline ostream & operator <<(ostream& out, const Pattern & pat) {
  for (vector<int>::const_iterator i=pat.itemset.begin();
       i!=pat.itemset.end(); i++)
    out << *i << " ";
  out << '(' << pat.count << ')';
  return out;
}

// A pattern of length 1
struct Pattern1
{
  int symbol;
  int count;
};

inline ostream & operator <<(ostream& out, const Pattern1 & pat) {
  out << pat.symbol << "(" << pat.count << ")";
  return out;
}

class Pattern_Set : public list<Pattern>
{
public:
  Pattern_Set()
    : num_items(0), max_length(0), num_transactions(0) {}
  Pattern_Set(Pattern_Set& freq, vector<int>& part_vec, int part);
  Pattern_Set(Pattern_Set& freq, vector< vector<int> >& dist_vec, int proc);

  ~Pattern_Set() {}

  void read(const string& fname);
  void read(ifstream& in);
  void write(const string& fname, bool bysize=true) const;
  void write(ofstream& out, bool bysize=true) const;
  void compute();
  void decompose_by_size();
  int min_length();

  set<int> F;
  map<int, int> count;
  vector<int> count_vec;
  int num_items;
  int max_length;
  int num_transactions;

  vector< list<Pattern> > bysize;

};

#endif
