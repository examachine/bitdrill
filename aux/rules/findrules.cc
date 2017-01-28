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

#include <stdlib.h>
#include <iostream>
#include <cstring>

#include "ifile.hh"
#include "utils.hh"

using namespace std;

int readItemset(FILE* in, int* data) {
  int c;
  int len = 0;
  do {
    c = fgetc(in);
    if ((c >= '0') && (c <= '9')) {
      unsigned int item = c-'0';
      c = fgetc(in);
      while((c >= '0') && (c <= '9')) {
	item *=10;
	item += c-'0';
	c = fgetc(in);
      }
      data[len++] = item;
    }
  } while(c != '\n' && !feof(in));
  return len;
}

void loadItemsets( ifile* index, char* input_file, 
		   vector<int> *supports, vector<int*> *itemsets) {
  FILE* in = fopen(input_file, "r");

  int len;
  int* data = new int[1024];
  while (!feof(in)) {
    len = readItemset(in, data);
    if (len>=2) {  // at least one item and one support
      len--;
      index->load(data, len);
      supports->push_back(data[len]);
      int* set = new int[len+1];
      set[0] = len;
      memcpy(set+1, data, len*sizeof(int));
      itemsets->push_back(set);
    }
  }

  delete [] data;
  fclose(in);
}


int computeRules( ifile* index, 
		  char* input_file, char* output_file, 
		  vector<int> *supports, vector<int*> *itemsets,
		  float min_conf) {
  FILE* in  = fopen(input_file, "r");
  FILE* out = fopen(output_file, "w");

  bool* in_head = new bool[index->getTerms()];
  for (int i=0; i<index->getTerms(); i++) in_head[i] = false;

  int n_rules = 0;
  int len;
  int* data   = new int[index->getTerms()];
  int* result = new int[index->getDocs()];
  while (!feof(in)) {
    len = readItemset(in, data);
    if (len>=2) {  // at least one item and one support
      float support = (float) data[len-1];
      // get all supersets
      int r_len = index->get(data, len-1, result);
      // for each superset
      for (int s=0; s<r_len; s++) {
	int* set = itemsets->at(result[s]);
	if (set[0]>len-1) { // skip if they're the same
	  float conf = supports->at(result[s]) / support;
	  if (conf >= min_conf) {
	    n_rules++;
	    // Print rule
	    for(int i=0; i<len-1; i++) {
	      fprintf(out, "%d ", data[i]);
	      in_head[data[i]] = true;
	    }
	    fprintf(out, "=> ");
	    for(int i=0; i<set[0]; i++) 
	      if (!in_head[set[i+1]])
		fprintf(out, "%d ", set[i+1]);
	    fprintf(out, "(%d, %f)\n", supports->at(result[s]), conf);
	    for(int i=0; i<len-1; i++)
	      in_head[data[i]] = false;
	  }
	}
      }
    }
  }

  delete [] data;
  delete [] result;
  delete [] in_head;
  fclose(in);
  fclose(out);
  return n_rules;
}

int main (int argc, char* argv[]) {
  // presentation message
  cout << "FindRules Copyright (C) 2009 Claudio Lucchese" << endl
       << "Rule discovery from frequent sets." << endl
       << endl;

  // usage message
  if (argc < 4) {
    cout << "usage: " << argv[0] << " datafile minconf outfile" << endl;
    return 1;
  }

  // read parameters
  char* input_file  = argv[1];
  float min_conf    = atof(argv[2]);
  char* output_file = argv[3];

  // timers
  Chronos time_all;
  time_all.StartChronos();

  // load itemsets
  ifile index;
  vector<int> supports;
  vector<int*> itemsets;
  supports.reserve(1024*1024);      // reserve 1MB
  itemsets.reserve(1024*1024*100);  // reserve 100MB
  index.preLoad();
  loadItemsets(&index, input_file, &supports, &itemsets);
  index.endLoad();

  cout << "items : " << index.getTerms() << endl;
  cout << "sets  : " << index.getDocs() << endl;
  cout << "index size: " << index.getSize() << endl;

  // compute rules and confidence
  int no_rules = computeRules( &index, input_file, output_file, 
			       &supports, &itemsets,
			       min_conf);

  cout << "rules : " << no_rules << endl;
  cout << "elapsed time : " << time_all.ReadChronos() << endl;

  // free some memory
  for (unsigned int i=0; i<itemsets.size(); i++)
    delete [] itemsets[i];

  return 0;
}
