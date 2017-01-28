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
    cout << "read-ts -- read a transaction set" << endl;

    if ( ! argc==3 ) {
      cout << "usage: read-ts <transaction-db> <num-items>" << endl;
      exit(-1);
    }

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
	cout << "read " << t_i;
	num_trans++;
	for (Transaction::iterator i=t_i.begin(); i!=t_i.end(); i++) {
	  assert ( *i < count.size() );
	  count[*i]++;
	}
	cout << "." << endl;
      }
      else {
	cout << "last transaction" << endl;
	break;
      }
    }
    cout << num_trans << "transactions read" << endl;

    return 0; // success
};
