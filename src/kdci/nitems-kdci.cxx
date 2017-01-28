#include "fim_all.hh"

using namespace KDCI;

int main(int argc, char ** argv)
{

  if(argc < 4 || argc > 5) {
    cerr << "Usage: " << argv[0] << 
      " input_file min_count max_level [output_file]" << endl;
    return 1;
  }
  
  unsigned int min_count = atoi(argv[2]);

  int max_level = atoi(argv[3]);

  if (min_count <= 0 || max_level <= 0) {
    cerr << "Usage: " << argv[0] << " input_file min_count max_level " << endl;
    cerr << "        Value of min_count not allowed (min_count > 0)\n";
    return 2;
  }

  ::Transaction_Set ts(argv[1], ::Transaction::ascii);

//     if (fstat()) {
//     cerr << "input file " << argv[1] 
// 	 << " cannot be opened!" << endl;
//     exit(1);
//   }  

  string out;
  if (argc == 5) {
    out = string(argv[4]);
  }  
  else
    out = "";

  cout << "mining " << argv[1] << " up to level " << max_level << endl;

  kdci_all(ts, min_count, out, max_level, true);

}
