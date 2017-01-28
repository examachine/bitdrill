//
//
// C++ Implementation for module: check-ts
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
    cout << "check-ts -- verify a transaction set" << endl;

    if (!(argc==2 || argc==3)) {
      cout << "usage: check-ts <transaction-db> [<type>]" << endl;
      exit(-1);
    }

    string db_name(argv[1]);
    cout << "checking transaction set " << db_name << endl;
    ifstream in(db_name.c_str(), ios::binary);

    int filetype;
    if (argc==3)
      filetype = atoi(argv[2]);
    else
      filetype = 0;
    cout << "filetype is " << filetype << endl;
    Transaction::File_Format format = (Transaction::File_Format) filetype;

    // do a pass over local db.
    int num_trans = 0;
    int max_item = 0;

    while (!in.eof()) {
      Transaction t_i;
      if (t_i.read(in, format)) {
	num_trans++;
	for (Transaction::iterator i=t_i.begin(); i!=t_i.end(); i++) {
	  if (*i>max_item) max_item=*i;
          if (*i<0) {
            cerr << "negative item: " << *i << endl;
            return 1;
          }
	}
      }
      else {
	cout << "last transaction" << endl;
	break;
      }
    }
    cout << num_trans << " transactions read" << endl;
    cout << "maximum item is " << max_item << endl;
    cout << "transaction set valid" << endl;

    return 0; // success
};
