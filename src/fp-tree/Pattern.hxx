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
#include "Symbol.hxx"

typedef vector<int> Symbol_Set;

// A pattern is a set of items with a count associated
struct Pattern
{
  Pattern() {}
  Pattern(const Symbol_Set& symset, int cnt)
    : symbol_set(symset), count(cnt) {}

  Symbol_Set symbol_set;
  int count;
  bool read(istream&);
};

inline Pattern strip(const Pattern& pat) {
  Pattern rest;
  rest.symbol_set=strip(pat.symbol_set);
  rest.count = pat.count;
  return rest;
}

inline ostream & operator <<(ostream& out, const Pattern & pat) {
  for (vector<int>::const_iterator i=pat.symbol_set.begin();
       i!=pat.symbol_set.end(); i++)
    out << *i << " ";
  out << "(" << pat.count << ")";
  return out;
}

// inline istream & operator >>(istream& in, Pattern & pat) {
// }

// A pattern of length 1
struct Pattern1
{
  int symbol;
  int count;
};

inline ostream & operator <<(ostream& out, const Pattern1 & pat) {
  out << pat.symbol << ":" << pat.count;
  return out;
}

class Pattern_Set : public vector<Pattern*>
{
public:
  Pattern_Set() {}
  ~Pattern_Set() {
    for (iterator i=begin(); i!=end(); i++) {
      delete *i;
    }
  }

  Pattern& cons() {
    Pattern* pattern = new Pattern;
    push_back(pattern);
    return *pattern;
  }

};

#endif
