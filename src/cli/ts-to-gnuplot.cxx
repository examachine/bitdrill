//
//
// C++ Implementation for module: read-ts
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "generic/General.hxx"
#include "transaction-db/Transaction_Set.hxx"

int main(int argc, char *argv[])
{
    cerr << "conv-ts-to-ascii -- convert a ts to gnuplot data" << endl;
    cerr << "format: size of row, followed by row indices" << endl;

    if ( ! argc==3 ) {
      cerr << "usage: ts-to-ascii <transaction-db> <num-items>" << endl;
      exit(-1);
    }

//     if ( !(argc==2 || argc==3)) {
//       cout << "usage: read-ts <transaction-db> [<num-items>]" << endl;
//       exit(-1);
//     }

    string db_name(argv[1]);
    istringstream is(argv[2]);
    int num_items;
    is >> num_items;

    ifstream in(db_name.c_str(), ios::binary);

    vector<int> count(num_items);

    fill(count.begin(), count.end(), 0);
    // do a pass over local db.
    int num_trans = 0;
    while (!in.eof()) {
      Transaction t_i;
      if (t_i.read(in, Transaction::ascii)) {
	for (Transaction::iterator i=t_i.begin(); i!=t_i.end(); i++) {
	  assert ( *i < count.size()-1 );
	  count[*i]++;
	  cout << *i << " " << num_trans << endl;
	}
	num_trans++;
// 	cout << endl;		// leave a newline between rows
      }
      else {
	cerr << "last transaction" << endl;
	break;
      }
    }
    cerr << num_trans << "transactions read" << endl;

    return 0; // success
};
