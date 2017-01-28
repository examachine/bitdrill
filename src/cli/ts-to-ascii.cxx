//
//
// C++ Implementation for module: ts-to-ascii
//
// Description: Converts a transaction set to ascii format
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
    cerr << "ts-to-ascii -- convert a ts to ascii" << endl;
    cerr << "format: each transaction on a single line" << endl;

    if ( argc!=2 ) {
      cerr << "usage: ts-to-ascii <transaction-db>" << endl;
      exit(-1);
    }

    string db_name(argv[1]);

    ifstream in(db_name.c_str(), ios::binary);

    // do a pass over local db.
    int num_trans = 0;
    while (!in.eof()) {
      Transaction t_i;
      if (t_i.read(in, Transaction::binary)) {
	num_trans++;
	for (Transaction::iterator i=t_i.begin(); i!=t_i.end(); i++) {
	  cout << *i << " ";
	}
	cout << endl;
      }
      else {
	cerr << "last transaction" << endl;
	break;
      }
    }
    cerr << num_trans << " transactions read" << endl;

    return 0; // success
};
