//
//
// C++ Implementation file for application gen-2-items
//
// Description: includes the main(...) function
//
// exa
//
//

#include "generic/General.hxx"

#ifdef Utility_Interface
using namespace Utility;
#endif

#include <metis.h>

#include "transaction-db/Transaction_Set.hxx"
#include "Count.hxx"
#include "graph/Graph.hxx"

int main(int argc, char *argv[])
{
  cout << "gen-2-items" << endl;

  if (!(argc==3 || argc==4)) {
    cerr << "usage: gen-2-items db-prefix threshold [only-1-items]" << endl;
    exit(-1);
  }

  string db_prefix(argv[1]);

  istringstream is2(argv[2]);
  float sup;
  is2 >> sup;

  try {

    Wall_Time t_start;
  
    int max_level = 2;
    if (argc==4)
      max_level = atoi(argv[3]);

    TS_Counter counter(db_prefix);
    if (sup < 1.0)
      counter.count_items_relative(sup,max_level);      // count 2-items
    else
      counter.count_items_absolute(int(sup),max_level);

    {

      ofstream out( "2-items" );
      out << "(" << counter.total_num_transactions << ")" << endl;
      for (set<int>::iterator it=counter.F.begin();
	   it!=counter.F.end(); it++) {
	out << *it << " (" << counter.count[*it] << ")" << endl;
      }
      
      Wall_Time t_trav;
      if (max_level >=2) {
	// traverse the F2 graph
	for (unsigned int u=0; u<counter.F.size(); u++) {
	  for (Graph::Adjacency::iterator
		 it=counter.F2.adjacency[u].begin();
	       it!=counter.F2.adjacency[u].end(); it++) {
	    Edge& e = *it;
	    unsigned int v = e.neighbor;
	    if (u<v) {
	      out << counter.f_item[u] << ' ' <<
		counter.f_item[v] << " (" << e.weight << ')' << endl;
	    }
	  }
	}
	//nlog << "f_item = " << counter.f_item << endl;
	//nlog << "item_f = " << counter.item_f << endl;
      }
      nlog << "output time " << Wall_Time() - t_trav << endl;      
    }

    Wall_Time t_end;
    cout << "total time " << t_end - t_start << endl;

  }
  catch (...) {
    cerr << "Error caught" << endl;
  }

  return 0; // success

}
