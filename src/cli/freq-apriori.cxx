/*----------------------------------------------------------------------
  File     : aprioritest.cpp
  Contents : apriori algorithm for finding frequent sets
  Author   : Bart Goethals
  Update   : 1/6/2003
----------------------------------------------------------------------*/

#include "apriori/AprioriSets.hxx"
#include <iostream>
#include <time.h>

int main(int argc, char *argv[])
{
  cout << "Apriori frequent itemset mining implementation" << endl;
  cout << "by Bart Goethals, 2000-2003" << endl;
  cout << "http://www.cs.helsinki.fi/u/goethals/" << endl << endl;
  
  if (argc < 3 || argc > 4) {
    cerr << "usage: " << argv[0] << " datafile minsup [output]" << endl;
  }
  else {
    AprioriSets a;
    a.setVerbose(); // print information on nr of candidate itemsets etc
    Transaction_Set ts(argv[1], Transaction::ascii);

    a.setCountType(2); 
    // 1: to check k-subsets of transaction in set of candidates
    // 2: to check all candidates in transaction (default - best performance)

    a.setMinSup(atoi(argv[2]));
    if(argc==4) a.setOutputSets(argv[3]);
    
    clock_t start = clock();
    int sets = a.generateSets();
    cout << sets << "\t[" << (clock()-start)/double(CLOCKS_PER_SEC) << "s]" << endl;
    if(argc==4) cout << "Frequent sets written to " << argv[3] << endl;
  }

  return 0;
}

