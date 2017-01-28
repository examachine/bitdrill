//
//
// C++ Interface for module: UT_Matrix
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef UT_Matrix_Interface
#define UT_Matrix_Interface

#include "generic/General.hxx"
#include <stdint.h>

// Upper Triangular Square Matrix
class UT_Matrix
{
public:
  class Walk {
  public:
    Walk(UT_Matrix& u) : U(u), i(0), j(0), ptr(u.rep) {}
    int * ref() const { return ptr; }
    void next() {
      ptr++;
      j++;
      if (j==U.get_cols()) {
	i++;
	j = i;
      }
    }
    bool end() const {
      return ptr > U.last;
    }
    UT_Matrix& U;
    uint64_t i;
    uint64_t j;
  private:
    int* ptr;
  };
  class Const_Walk {
  public:
    Const_Walk(const UT_Matrix& u) : U(u), i(0), j(0), ptr(u.rep) {}
    const int* ref() const { return ptr; }
    void next() {
      ptr++;
      j++;
      if (j==U.get_cols()) {
	i++;
	j = i;
      }
    }
    bool end() const {
      return ptr > U.last;
    }
    const UT_Matrix& U;
    uint64_t i;
    uint64_t j;
  private:
    int* ptr;
  };
  friend class Walk;
  friend class Const_Walk;

  UT_Matrix(uint64_t n);
  const unsigned int get_rows() const {
    return rows;
  }
  const unsigned int get_cols() const {
    return rows;
  }
  int get(uint64_t i, uint64_t j) {
    if (i<=j)
      return *elt(i,j);
    else
      return 0;
  }
  void set(uint64_t i, uint64_t j, int val) {
    assert (i<=j);
    *elt(i,j) = val;
  }
  int nonzero() const;
  uint64_t get_size() const { return size;}
  int* data() { return rep; }
private:
  unsigned int rows;
  uint64_t size;
  int* rep;
  int* last;
  int* elt(uint64_t i, uint64_t j) {
    uint64_t n = rows;
    return rep + (n*(n+1)-(n-i)*(n-i+1))/2 + j - i;
  }
};


#endif
