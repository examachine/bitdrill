//
//
// C++ Interface for module: Symbol
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef Symbol_Interface
#define Symbol_Interface

#include "generic/General.hxx"

inline int blank(const vector<int>& str) {
  return -1;
}

inline int first(const vector<int>& str) {
  return str[0];
}

inline vector<int> strip(vector<int> str) {
  vector<int>::iterator b = str.begin();
  str.erase(b, b+1);
  return str;
}

inline bool empty(const vector<int>& str) {
  return str.empty();
}

inline int length(const vector<int>& str) {
  return str.size();
}

inline void add_prefix(vector<int>& str, int prefix) {
  str.insert(str.begin(), prefix);
}


#endif
