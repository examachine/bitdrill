//
//
// C++ Implementation For Module: Pattern
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
  // read a list of tokens until EOF or new line
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
    read = tokens.size()>=1; // could we read a line of input?
    if (!read && c=='\n')
      tokens.clear();
  }
  if (read) {
    string freq = tokens.back();
    sscanf(freq.c_str(), "(%d)", &count);
    tokens.pop_back();
    for (list<string>::iterator i=tokens.begin();i!=tokens.end();i++) {
      int item = atoi(i->c_str());
      itemset.push_back(item);
    }
  }
  return read;
}

// project from a set of patterns for a given part vector and part
Pattern_Set::Pattern_Set(Pattern_Set& freq, vector<int>& part_vec, int part)
{
#ifdef DEBUG
  nlog << "projecting from " << freq << " with part vec " 
       << part_vec << " for part " << part << endl;
#endif
  int max_item = 0;
  max_length = 0;
  F.clear();
  for (iterator it=freq.begin(); it!=freq.end(); it++) {
    Pattern &p = *it;
    {
      max_length = max(max_length, int(p.itemset.size()));
      if (p.itemset.size()==1) {
	int item = p.itemset[0];
	if (part_vec[item] == part) {
	  F.insert(item);
	  max_item = max(max_item, item);
	  count[item] = p.count;
	}
      }
    }
  }
  num_items = max_item + 1;
  num_transactions = freq.num_transactions;

  // form count vector
  count_vec.resize(num_items);
  for (map<int,int>::iterator it=count.begin(); it!=count.end(); it++)
    count_vec[it->first] = it->second;

  // partition by pattern size
  bysize.resize(0); // clear
  bysize.resize(max_length + 1);
  for (list<Pattern>::iterator x=freq.begin(); x!=freq.end(); x++) {
    Pattern & pattern = *x;
    bool subset = true;
    for (vector<int>::iterator it=pattern.itemset.begin();
	 subset && it!=pattern.itemset.end(); it++) {
      if (part_vec[*it] != part)
	subset = false;
    }
    if (subset)
      bysize[x->itemset.size()].push_back(pattern);
  }

#ifdef DEBUG
  nlog << "bysize=" << bysize << endl;
#endif

}

//TODO: faster projection?
// project from a set of patterns for a given distribution vector and proc
// Pattern_Set::Pattern_Set(Pattern_Set& freq, 
//                          vector< vector<int> >& dist_vec, int proc)
// {
// #ifdef DEBUG
//   nlog << "projecting from " << freq << " for proc " << proc << endl;
// #endif
//   int max_item = 0;
//   max_length = 0;
//   F.clear();
//   for (iterator it=freq.begin(); it!=freq.end(); it++) {
//     Pattern &p = *it;
//     {
//       max_length = max(max_length, int(p.itemset.size()));
//       if (p.itemset.size()==1) {
// 	int item = p.itemset[0];
//         vector<int>& procs = dist_vec[item];
// 	if (find(procs.begin(), procs.end(), proc) != procs.end()) {
// 	  F.insert(item);
// 	  max_item = max(max_item, item);
// 	  count[item] = p.count;
// 	}
//       }
//     }
//   }
//   num_items = max_item + 1;
//   num_transactions = freq.num_transactions;

//   // form count vector
//   count_vec.resize(num_items);
//   for (map<int,int>::iterator it=count.begin(); it!=count.end(); it++)
//     count_vec[it->first] = it->second;

//   // partition by pattern size
//   bysize.resize(0); // clear
//   bysize.resize(max_length + 1);
//   for (list<Pattern>::iterator x=freq.begin(); x!=freq.end(); x++) {
//     Pattern & pattern = *x;
//     bool subset = true;
//     for (vector<int>::iterator it=pattern.itemset.begin();
// 	 subset && it!=pattern.itemset.end(); it++) {
//       if (dist_vec[*it] != proc)
// 	subset = false;
//     }
//     if (subset)
//       bysize[x->itemset.size()].push_back(pattern);
//   }

// #ifdef DEBUG
//   nlog << "bysize=" << bysize << endl;
// #endif

// }

void Pattern_Set::read(const string &fname)
{
  ifstream pattern_input(fname.c_str());
  if (pattern_input) 
    read(pattern_input);
  else {
    cerr << "Cannot open " << fname << " for reading" << endl;
    exit(1);
  }
}

void Pattern_Set::read(ifstream &pattern_input)
{
  int max_item = 0;
  max_length = 0;
  while (!pattern_input.eof()) {
    Pattern p;
    if (p.read(pattern_input)) {
      push_back(p);
      max_length = max(max_length, int(p.itemset.size()));
      if (p.itemset.size()==1) {
	int item = p.itemset[0];
	F.insert(item);
	max_item = max(max_item, item);
	count[item] = p.count;
      }
    }
  }
  num_items = max_item + 1;

  count_vec.resize(max_item+1);
  for (map<int,int>::iterator it=count.begin(); it!=count.end(); it++)
    count_vec[it->first] = it->second;

  decompose_by_size();

  if (bysize.size() >= 1)
    num_transactions = bysize[0].front().count;
  else 
    num_transactions = 0;
}

int Pattern_Set::min_length()
{
  int min_length = begin()->itemset.size();
  for (iterator it=begin(); it!=end(); it++) {
    Pattern &p = *it;
    min_length = min(min_length, int(p.itemset.size()));
  }
  return min_length;
}

// compute values and organize patterns
void Pattern_Set::compute()
{
  int max_item = 0;
  max_length = 0;
  F.clear();
  for (iterator it=begin(); it!=end(); it++) {
    Pattern &p = *it;
    {
      max_length = max(max_length, int(p.itemset.size()));
      if (p.itemset.size()==1) {
	int item = p.itemset[0];
	F.insert(item);
	max_item = max(max_item, item);
	count[item] = p.count;
      }
    }
  }
  num_items = max_item + 1;

  count_vec.resize(max_item+1);
  for (map<int,int>::iterator it=count.begin();
       it!=count.end(); it++) {
    count_vec[it->first] = it->second;
  }

  decompose_by_size();

  if (bysize.size() >= 1)
    num_transactions = bysize[0].front().count;
  else 
    num_transactions = 0;
}


// decompose a list of patterns by their size
void Pattern_Set::decompose_by_size()
{
  // find maximum pattern length
  for (list<Pattern>::iterator x=begin();x!=end();x++) {
    max_length = max(max_length, int(x->itemset.size()));
  }

  // partition by pattern size
  //bysize.resize(0); // clear
#ifdef DEBUG
  //nlog << "decompose: bysize=" << bysize << endl;
#endif
  bysize.resize(max_length + 1);
#ifdef DEBUG
  //nlog << "decompose: bysize=" << bysize << endl;
#endif
  for (list<Pattern>::iterator x=begin();x!=end();x++) {
    bysize[x->itemset.size()].push_back(*x);
  }
#ifdef DEBUG
  //nlog << "decompose: bysize=" << bysize << endl;
#endif
}


void Pattern_Set::write(const string &fname, bool bys) const
{
  ofstream pattern_output(fname.c_str());
  write(pattern_output, bys);
}

//TODO: output by size
void Pattern_Set::write(ofstream &out, bool bys) const
{
  assert(bys);
  // for (  vector< list<Pattern> >::const_iterator i=bysize.begin();
  //        i!=bysize.end(); i++) {
  //   for (list<Pattern>::const_iterator j=i->begin(); j!=i->end(); j++) {
  //     out << *j << endl;
  //   }
  // }  
  for (list<Pattern>::const_iterator j=begin(); j!=end(); j++) {
      out << *j << endl;
  }
}
