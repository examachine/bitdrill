//
//
// C++ Implementation for module: Pattern
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "Pattern.hxx"

bool Pattern::read(istream& in) {
  list<string> tokens;
  // read a list of tokens until EOF
  bool read = false;
  while (!read && !in.eof()) { // read lines until we find a non-empty pattern
    char c;
    do {			// read a line of input (until newline or EOF)
      string token;
      int pos = 0;
      c = in.get();
      while (((c >= '0') && (c <= '9')) || (c=='(') || (c==')') ) {
	token += c;
	c = in.get();
	pos++;
      }
      if (pos) tokens.push_back(token);
    } while (c != '\n' && !in.eof()); // read a line
    read = tokens.size()>=2; // could we read a line of input?
    if (!read && c=='\n')
      tokens.clear();
  }
  if (read) {
    string freq = tokens.back();
    tokens.pop_back();
    sscanf(freq.c_str(), "(%d)", &count);
    for (list<string>::iterator i=tokens.begin();i!=tokens.end();i++) {
      int item = atoi(i->c_str());
      symbol_set.push_back(item);
    }
    sort(symbol_set.begin(), symbol_set.end());
  }
  return read;
}

