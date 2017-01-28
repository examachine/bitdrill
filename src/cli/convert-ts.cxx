//
//
// C++ Implementation for module: convert-ts
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
  cerr << argv[0] << "convert a transaction set" << endl;

    if ( argc!=5 ) {
      cerr << "usage: " << argv[0]
	   << " <ts1> <format1> <ts2> <format2>" << endl;
      exit(-1);
    }

    string in_db_name(argv[1]);
    ifstream in(in_db_name.c_str(), ios::binary);
    int infiletype = atoi(argv[2]);
    Transaction::File_Format informat =
      (Transaction::File_Format) infiletype;

    string out_db_name(argv[3]);
    ofstream out(out_db_name.c_str(), ios::binary);
    int outfiletype = atoi(argv[4]);
    Transaction::File_Format outformat =
      (Transaction::File_Format) outfiletype;

    // do a pass over local db.
    int num_trans = 0;
    while (!in.eof()) {
      Transaction t_i;
      if (t_i.read(in, informat)) {
	num_trans++;
	t_i.write(out, outformat);
      }
      else {
	cerr << "last transaction" << endl;
	break;
      }
    }
    cerr << num_trans << " transactions read" << endl;

    return 0; // success
};
