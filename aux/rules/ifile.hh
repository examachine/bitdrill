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

#ifndef __IFILE_HH__
#define __IFILE_HH__

#include <stdio.h>
#include <vector>

using namespace std;

#define IF_TEMP_FILE "TEMPDATA.IFILE"

class ifile {
public:
  ifile();
  ~ifile();

  // ----------- mandatory loading steps ------------------
  void preLoad();                   // prepare for loading
  void load(int* data, int len);    // loads one data unit
  void endLoad();                   // end of loading phase

  // ----------- querying ----------------------------------------
  int get(int* data, int len, int* result);  // assume result buff
                                             // is large enough
  // ----------- info---------------------------
  int getTerms() { return n_terms; }
  int getDocs()  { return n_docs; }
  int getSize()  { return size; }

private:

  FILE* temp;
  int n_docs;
  int n_terms;
  int size; 
  vector<int> counters;
  int** INDEX;
  int* DATA;

  // debuggin
public:
  void pprint();
};

#endif
