#include "Tid_List.hxx"

void Tid_List::reset(int num_txn, int expected_count) {
  assert(expected_count > 0);
#ifdef DEBUG
   nlog << "num_txn=" << num_txn << ", expected_count=" << expected_count
	<< ", sparsity=" << num_txn/expected_count << endl;
#endif
   if (choose_implementation(num_txn,expected_count)==Bitvector) {
#ifdef DEBUG
    nlog << "using bitvector" << endl;
#endif
    implementation = Bitvector;
    bitvector.resize(num_txn);
    tidvector.resize(0);
  }
  else {
#ifdef DEBUG
    nlog << "using tidvector" << endl;
#endif
    implementation = Tidvector;
    bitvector.resize(0);
    tidvector.resize(0);
  }
}
