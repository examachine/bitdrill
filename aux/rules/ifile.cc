/*
  Copyright (C) 2009, Claudio Lucchese

  This file is part of FINDRULES.

  FINFRULES is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  FINDRULES is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with FINFRULES.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ifile.hh"

#include <stdlib.h>

#include <iostream>
using namespace std;

ifile::ifile() {
  temp  = NULL;
  INDEX = NULL;
  DATA  = NULL;
}

ifile::~ifile() {
  if (INDEX) delete [] INDEX;
  if (DATA)  delete [] DATA;

  // check that file exists ??!
  remove(IF_TEMP_FILE);
}


void ifile::preLoad() {
  temp    = fopen(IF_TEMP_FILE, "w");
  n_docs  = 0;
  n_terms = 0; 
  size    = 0;
  counters.clear();
  counters.reserve(1024);
}

void ifile::load(int* data, int len){
  n_docs++;
  for (int i=0; i<len; i++) {
    if (data[i]>=n_terms) {
      counters.resize(data[i]+1,0);
      n_terms = counters.size();
    }
    counters[data[i]]++;
  }
  fwrite(&len, sizeof(int), 1,   temp);
  fwrite(data, sizeof(int), len, temp);
}

void ifile::endLoad() {
  // ----------------------------
  // allocate data structure
  size = 0;
  for (int i=0; i<n_terms; i++)
    size += counters[i];

  INDEX = new int* [n_terms];
  DATA  = new int  [size];

  int* addr= &(DATA[0]);
  for (int i=0; i<n_terms; i++) {
    INDEX[i] = addr;
    addr += counters[i];
  }

  // ---------------------------
  // reparse file from beginning
  counters.clear();
  counters.resize(n_terms, 0);

  int len = 0;
  int* data = new int [n_terms];

  fclose(temp);
  temp = fopen(IF_TEMP_FILE, "r");
  int d = 0;
  while (d<n_docs) {
    fread(&len, sizeof(int), 1,   temp);
    fread(data, sizeof(int), len, temp);
    for (int i=0; i<len; i++) {
      int term = data[i];
      INDEX[term][counters[term]] = d;
      counters[term]++;
    }
    d++;
  }

  fclose(temp);
  delete [] data;
}


// -------------------------------------------------
// return pos
// such that a[pos] is the first element >= doc_id
// ---- a contains distinct elements
int find( int doc_id, int* a, int first, int last ) {
  while(first<=last) {
    int mid = (first+last)/2;
    if (doc_id > a[mid])
      first = mid +1;
    else if (doc_id < a[mid])
      last = mid-1;
    else { 
      first = mid; 
      break; 
    }
  }
  return first;
}

int intersect(int* a, int len_a, int* b, int len_b, int* result) {
  int start_a = 0;
  int start_b = 0;
  int end_a   = len_a-1;
  int end_b   = len_b-1;

  int len = 0;
  while (start_a<=end_a && start_b<=end_b) {
    bool a_turn = (end_a-start_a) < (end_b-start_b);
    if (a_turn) {
      // cout << "searching for " << a[start_a] << endl;
      start_b = find(a[start_a], b, start_b, end_b);
      if (a[start_a] == b[start_b])   // found !
	result[len++] = b[start_b++];
      start_a++;
    } else {
      // cout << "searching for " << b[start_b] << endl;
      start_a = find(b[start_b], a, start_a, end_a);
      if (a[start_a] == b[start_b])   // found !
	result[len++] = a[start_a++];
      start_b++;
    }
    // cout << "len: " << len << endl;
    // if (len>0) cout << "added " << result[len-1] << endl;
  }
  return len;
}

int ifile::get(int* data, int len, int* result) {
  if (len==0) return 0;
  if (len==1) {
    int term = data[0];
    for (int i=0; i<counters[term]; i++)
      result[i] = INDEX[term][i];
    return counters[term];
  }

  // first intersection
  int new_len = intersect( INDEX[data[0]], counters[data[0]], 
			   INDEX[data[1]], counters[data[1]], 
			   result);
  // other intersections
  for (int i=2; i<len; i++)
    new_len = intersect( result, new_len, 
			 INDEX[data[i]], counters[data[i]], 
			 result);
  return new_len;
}


  // debuggin
void ifile::pprint() {
  for (int t=0; t<n_terms; t++) {
    cout << t << ":";
    for (int i = 0; i<counters[t]; i++)
      cout << " " << INDEX[t][i];
    cout << endl;
  }
}
