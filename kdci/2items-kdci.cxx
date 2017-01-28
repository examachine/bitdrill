#include "fim_all.hh"

int main(int argc, char ** argv)
{

  if(argc < 3 || argc > 4) {
    cerr << "Usage: " << argv[0] << 
      " input_file min_count [output_file]" << endl;
    return 1;
  }
  
  unsigned int min_count;
  min_count = atoi(argv[2]);
  if (atoi(argv[2]) <= 0) {
    cerr << "Usage: " << argv[0] << " input_file min_count" << endl;
    cerr << "        Value of min_count not allowed (min_count > 0)\n";
    return 2;
  }

  Transaction_Set ts(argv[1], Transaction::ascii);

//     if (fstat()) {
//     cerr << "input file " << argv[1] 
// 	 << " cannot be opened!" << endl;
//     exit(1);
//   }  

  string out;
  if (argc == 4) {
    out = string(argv[3]);
  }  
  else
    out = "";

  KDCI::kdci_all(ts, min_count, out, 2, true);

}
