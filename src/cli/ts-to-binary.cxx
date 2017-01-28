//
//
// C++ Implementation for module: ts-to-binary
//
// Description: Converts a transaction set to binary format
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
    cerr << "ts-to-binary -- convert a ts to binary" << endl;
    cerr << "input format: each transaction on a single line" << endl;
    cerr << "output format: quest datagen binary" << endl;

    if ( argc!=3 ) {
      cerr << "usage: ts-to-binary <input-db> <output-db>" << endl;
      exit(-1);
    }

    string in_db_name(argv[1]);
    string out_db_name(argv[2]);

    ifstream in(in_db_name.c_str());
    ofstream out(out_db_name.c_str(), ios::binary);

    // do a pass over local db.
    int num_trans = 0;
    while (!in.eof()) {
      Transaction t_i;
      if (t_i.read(in, Transaction::ascii)) {
	num_trans++;
	if (t_i.size()>0)
	  t_i.write(out, Transaction::binary);
      }
      else {
	cerr << "last transaction" << endl;
	break;
      }
    }
    cerr << num_trans << " transactions read" << endl;

    return 0; // success
};
