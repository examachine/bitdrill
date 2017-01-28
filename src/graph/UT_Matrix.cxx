//
//
// C++ Implementation for module: UT_Matrix
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "UT_Matrix.hxx"

UT_Matrix::UT_Matrix(uint64_t n)
  : rows(n)
{
  size = n*(n+1)/2;
  rep = new int[size];
  last = rep + size - 1;
  fill(rep, last+1, 0);
};

int UT_Matrix::nonzero() const
{
  int nz = 0;
  for (Const_Walk w(*this); !w.end(); w.next()) {
    if (*w.ref()!=0) nz++;
  }
//   for (int* e=rep; e<=last; e++) {
//     if (*e!=0)
//       nz++;
//   }
  return nz;
}
