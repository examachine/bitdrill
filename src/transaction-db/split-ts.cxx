//
//
// C++ Implementation file for application
//
// Description: includes the main(...) function
//
// exa
//
//

#include "generic/General.hxx"

#include "transaction-db/Transaction_Set.hxx"

int main(int argc, char *argv[])
{

  if (argc!=4) {
    cerr << "give me a filename, a prefix and the number of parts" << endl;
    exit(-1);
  }

  string filename(argv[1]);
  istringstream is(argv[3]);
  int num_procs;
  is >> num_procs;

  ifstream db(filename.c_str(), ios::binary);

  cout << "splitting database " << filename.c_str()
       << " into " << num_procs << endl;
  Transaction_Set::split(db, num_procs, argv[2], Transaction::ascii);

  return 0;
}
