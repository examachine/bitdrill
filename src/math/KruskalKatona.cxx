//
//
// C++ Implementation for module: KruskalKatona
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "KCanonical_Rep.hxx"
#include "KruskalKatona.hxx"

// compute bound for p levels ahead of current level k
mpz_class KruskalKatona::compute(int p)
{
  mpz_class s = 0;
  int k_i = k + p;
  for (vector<int>::iterator i=m.begin(); i!=m.end(); i++) {
    if (*i<k_i)
      break;
    s += choose(*i, k_i);
    k_i--;
  }
  return s;
}
