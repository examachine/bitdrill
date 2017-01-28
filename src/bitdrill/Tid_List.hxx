// Implemented with if switches because there are only two 
// implementations. For 3 or more implementations, an abstract
// base class might be more appropriate.

#ifndef Tid_List_Interface
#define Tid_List_Interface

#include "Bit_Vector.hxx"
#include "Tid_Vector.hxx"

class Tid_List
{
public:

  enum Implementation { Bitvector, Tidvector };

  Tid_List(Implementation impl=Bitvector) 
    : implementation(impl) {    
  }
  
  static Implementation choose_implementation(int num_txn, 
					      int expected_count) {
// #ifdef DEBUG
//     if (expected_count % 2) // test if two impls work together well
// #else
    // consider a number >32, because tidvector is slower
    if (num_txn / expected_count < 32) 
// #endif
      return Bitvector;
    else
      return Tidvector;
  }

  void reset(int num_txn, int expected_count);

  void make_bitvector(int num_txn) {
    implementation = Bitvector;
    bitvector.resize(num_txn);
    tidvector.resize(0);
  }
  
  void make_tidvector() {
    implementation = Tidvector;
    bitvector.resize(0);
    tidvector.resize(0);
  }
  
  void add_transaction(int txn_id) {
    if (implementation==Bitvector)
      bitvector.set(txn_id);
    else
      tidvector.push_back(txn_id);
  }

  // intersect with another tidlist destructively
  void intersect(const Tid_List& tl) {
    if (implementation==Bitvector)
      if (tl.implementation==Bitvector)
	bitvector.intersect(tl.bitvector);
      else
	error();
    else
      if (tl.implementation==Bitvector)
	tidvector.intersect(tl.tidvector);
      else
	error();
  }

  // intersect with another tidlist and write result
  void intersect(const Tid_List& tl, Tid_List& result) const {
    
    if (implementation==Bitvector) {
      if (tl.implementation==Bitvector) {
	result.make_bitvector(bitvector.size());
	bitvector.intersect(tl.bitvector, result.bitvector);
      }
      else 
	if (tl.implementation==Tidvector) {
	  result.make_tidvector();
	  tl.tidvector.intersect(bitvector, result.tidvector);
	}
	else
	  error();
    }
    else
      if (tl.implementation==Tidvector) {
	result.make_tidvector();
	tidvector.intersect(tl.tidvector, result.tidvector);
      }
      else {
	result.make_tidvector();
	tidvector.intersect(tl.bitvector, result.tidvector);
      }
  }

  // count transactions in tidlist
  int count() const {
    if (implementation==Bitvector)
      return bitvector.count();
    else
      return tidvector.size();
  }
  
  // intersect with another bit vector destructively and count transactions
  int intersect_and_count(const Tid_List& tl) {
    if (implementation==Bitvector)
      if (tl.implementation==Bitvector)
	return bitvector.intersect_and_count(tl.bitvector);
      else
	error();
    else
      if (tl.implementation==Tidvector)
	return tidvector.intersect_and_count(tl.tidvector);
      else
	error();
  }
  
  // count the number of transactions in intersection, but don't write the result
  int count_intersection(const Tid_List& tl, int support) const {
#ifdef DEBUG
    nlog << "count intersection of "; print(nlog);
    nlog << " and "; tl.print(nlog); nlog << endl;
#endif
    if (implementation==Bitvector)
      if (tl.implementation==Bitvector)
	return bitvector.count_intersection(tl.bitvector);
      else
	return tl.tidvector.count_intersection(bitvector);
    else
      if (tl.implementation==Tidvector)
	return tidvector.count_intersection(tl.tidvector, support);
      else
	return tidvector.count_intersection(tl.bitvector);
  }

  void error() const {
    cerr << "Tid List Error" << endl;
    exit(1);
  }

  void print(ostream& out=cout) const {
    if (implementation==Bitvector)
      out << "bitvector " << bitvector;
    else
      if (implementation==Bitvector)
	out << "tidvector " << tidvector;
      else {
	out << "INVALID TYPE" << endl;
	error();
      }
  }

private:
  Implementation implementation;
  Bit_Vector bitvector;
  Tid_Vector tidvector;
};


inline ostream& operator<<(ostream& out, const Tid_List &tl) {
  tl.print(out);
  return out;
}


#endif

