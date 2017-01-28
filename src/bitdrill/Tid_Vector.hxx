#ifndef Tid_List_Vector_Interface
#define Tid_List_Vector_Interface

#include "generic/General.hxx"

class Tid_Vector : public vector<int>
{
public:
  
  // intersect with another vector destructively
  void intersect(const Tid_Vector& bv) {
    Tid_Vector tmp;
    intersect(bv, tmp);
    *this = tmp;
  }

  // intersect with another vector
  void intersect(const Tid_Vector& bv, Tid_Vector& result) const {
    result.resize(0);
    set_intersection(begin(), end(), bv.begin(), bv.end(), 
		     back_inserter< vector<int> >((vector<int>&)result)); 
  }

  // intersect with a bit vector
  void intersect(const Bit_Vector& bv, Tid_Vector& result) const {
#ifdef DEBUG
    nlog << "intersecting " << *this << " and ";
    bv.print(nlog);
#endif
    result.resize(0);
    for (const_iterator it=begin(); it!=end(); it++) {
      int txn_id = *it;
      if (bv.get(txn_id)) {
	result.push_back(txn_id);
      }
    }
#ifdef DEBUG
    nlog << " result = " << result << endl;
#endif
  }

  // count number of 1's
  int count() const {
    return size();
  }


  // intersect with another bit vector destructively and count number of 1's
  int intersect_and_count(const Tid_Vector& bv) {
    intersect(bv);
    return count();
  }

  // Count  transactions in intersection, but don't write the result
  int count_intersection(const Tid_Vector& tv, int support) const {
    const_iterator lhs_it = begin();
//     int lhs_remaining = size();
    const_iterator rhs_it = tv.begin();
//     int rhs_remaining = tv.size();
    int count = 0;
    while (lhs_it!=end() && rhs_it!=tv.end()) {
      if (*lhs_it==*rhs_it) {
	count++;
	// advance both iterators
	lhs_it++;
// 	lhs_remaining--;
	rhs_it++;
// 	rhs_remaining--;
      }
      else if (*lhs_it < *rhs_it) {
	lhs_it++;
// 	lhs_remaining--;
      }
      else {
	rhs_it++;
// 	rhs_remaining--;
      }
    }
    return count;
  }

  // count intersection with a bit vector
  int count_intersection(const Bit_Vector& bv) const {
    int count = 0;
    for (const_iterator it=begin(); it!=end(); it++) {
      int txn_id = *it;
      if (bv.get(txn_id))
	count++;
    }
    return count;
  }

  // for debugging
  void print(ostream& out = cout) const
  {
    out << "tid vector: " << *this;
  }


private:
};

#endif
