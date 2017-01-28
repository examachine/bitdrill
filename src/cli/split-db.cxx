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

#include <sys/stat.h>

int main(int argc, char *argv[])
{

  cout << "split-db -- split a transaction database" << endl;
  
  if (!(argc==3 || argc==4)) {
    cout << "usage: split-db <transaction-db> <k> [<type>]" << endl;
    exit(-1);
  }

  string filename(argv[1]);

  ifstream db(filename.c_str()); // not ios::binary

  int k = atoi(argv[2]);

  int filetype;
  if (argc==4)
    filetype = atoi(argv[3]);
  else
    filetype = 0;


  struct stat buf;
  if (stat(filename.c_str(), &buf)!=0) {
    cerr << "file " << filename << " does not exist" << endl;
    exit(-1);
  }

  cout << "splitting database " << filename.c_str()
       << " into " << k << endl;

  Transaction_Set::split(db, k, filename, (Transaction::File_Format) filetype);

  return 0;
}
