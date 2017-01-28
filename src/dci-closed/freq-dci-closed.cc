#include "fim_closed.hh"

using namespace DCIClosed;

int main(int argc, char ** argv)
{

  if (argc < 3 || argc > 4) {
    cerr << "Usage: " << argv[0] 
	 << " input_file min_count [output_file]" << endl;
    return 1;
  }
  
  unsigned int min_count;
  min_count = atoi(argv[2]);
  if (min_count <= 0) {
    cerr << "Usage: " << argv[0] << " input_file min_count" << endl;
    cerr << "        Value of min_count not allowed (min_count > 0)" << endl;
    return 2;
  }

  initialize(false);		// serial exec

  ::Transaction_Set ts(argv[1], ::Transaction::ascii);

  string out;
  if (argc == 4) {
    out = string(argv[3]);
  }  
  else
    out = "";

  mine_closed(ts, min_count, out);

  finalize();
}
