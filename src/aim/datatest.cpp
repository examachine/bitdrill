/*---------------------------------------
  File:		datatest.cpp
  Contents:	test dataset management
---------------------------------------*/

#include "StdAfx.h"
#include <iostream>
#include <set>
using namespace std;
#include "data.h"

int main(int argc, char *argv[])
{
  if(argc != 2) {
    cerr << "Usage: datatest filename" << endl;
    return 1;
  }

  Data d(argv[1]);

  if(!d.isOpen()) {
    cerr << argv[1] << " could not be opened!" << endl;
    return 2;
  }

  int nr_of_transactions=0, totalsize=0;
  set<int> items;
  while(Transaction *t = d.getNextTransaction()) {
    for(int i=0; i<t->length; i++) items.insert(t->t[i]);
    nr_of_transactions++;
    totalsize += t->length;
    delete t;
  }

  cout << argv[1] << " contains " << nr_of_transactions << " transactions and "
       << items.size() << " different items." << endl
       << "The smallest item being " << *items.begin()
       << " and the largest " << *items.rbegin() << endl
       << "The total number of items in the database is " << totalsize << endl;

  
  return 0;
}
