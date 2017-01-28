//
//
// C++ Implementation file for parfreq CLI
//
// Description: Perform parallel frequent itemset mining
// on input database with a given threshold
// includes the main(...) function
//
// exa
//
//

#include "generic/General.hxx"

#include "transaction-db/Transaction_Set.hxx"
#include "2-items/Count.hxx"
#include "graph/Graph.hxx"
#include "bitdrill/Miner.hxx"
#include "bitdrill/Pattern.hxx" 

int main(int argc, char *argv[])

{
  initialize(argv[0], true);

//   if (absolute_root_processor()) {
  cout << "gen-candidates -- generate candidates from a set of freq. itemsets" << endl;
  cout << "(C) Eray Ozkural" << endl;
//   }

  //nlog << node_str << argv[0] << endl;
    
  if ( argc!=3 && argc!=4 ) {
    cout << "usage: " << argv[0]
	 << " freq-itemsets candidates [maxlevel]" 
	 << endl;
    exit(-1);
  }
  
  string freqpat_fn(argv[1]);

  string cand_fn(argv[2]);
  
  int maxlevel = 0;
  if (argc==4) {
    istringstream is(argv[3]);
    is >> maxlevel;
  }


  try {
    Wall_Time t_start;
    
    // read frequent patterns
    Pattern_Set freq;
    nlog << "Reading patterns from " << freqpat_fn << endl;
    ifstream freqpat(freqpat_fn.c_str());
    if (freqpat.good())
      freq.read(freqpat);
    else {
      nlog << "Error opening file " << freqpat_fn << endl;
      return 2;
    }
    
#ifdef DEBUG
    nlog << "freq: " << freq << endl;
#endif

    Pattern_Set cand;
    Miner::gen_candidates(freq, cand, maxlevel);
    cand.decompose_by_size();
    cand.write(cand_fn);
  }
  catch (MPI::Exception e) {
    cerr << "MPI Error caught" << endl;
  }

  finalize();
  nlog << "END RUN" << endl;
  return 0; // success
}
