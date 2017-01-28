//
//
// C++ Implementation file for application
// split a transaction db transaction-wise with cyclic distribution
//
// Description: includes the main(...) function
//
// exa
//
//

#include "generic/General.hxx"
#include "parallel/Parallel.hxx"
#include "transaction-db/Transaction_Set.hxx"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>




int main(int argc, char *argv[])
{
  initialize(argv[0]);			// parallel program

  if (argc!=3) {
    cerr << "give me a filename and a prefix" << endl;
    exit(-1);
  }

#ifdef DEBUG
  // for using gdb to debug parallel app
  {
    int i = 0;
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    cerr << "PID " << getpid() << " on " << hostname 
	 << " ready for attach" << endl << flush << flush;
    nlog << "PID " << getpid() << " on " << hostname 
	 << " ready for attach" << endl << flush;
    while (0 == i)
      sleep(1);
  }
#endif

  string filename(argv[1]);

  ifstream db(filename.c_str()); // not ios::binary

  if (absolute_root_processor()) {
    struct stat buf;
    if (stat(filename.c_str(), &buf)!=0) {
      cerr << "file " << filename << " does not exist" << endl;
      exit(-1);
    }

    cout << "splitting database " << filename.c_str()
         << " into " << num_processors << endl;

  };
    
  Transaction_Set::par_split(db, argv[2], Transaction::ascii);

  finalize();
  return 0;
  
}

