//
//
// C++ Implementation for module: KCanonical_Rep
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <gmpxx.h>
#include "KCanonical_Rep.hxx"

KCanonical_Rep::KCanonical_Rep(int n, int k)
{
  init(n, k);
}

mpz_class KCanonical_Rep::choose(mpz_class n, int k)
{
  mpz_class c;
  mpz_bin_ui(c.get_mpz_t(), n.get_mpz_t(), k);
  return c;
}

void KCanonical_Rep::init(mpz_class n, int _k)
{
  k = _k;
  m.erase(m.begin(), m.end());

  int k_i = k;

  mpz_class n_i = n;		// middle term in inequality 

  while (n_i > 0 && k_i >=1) {

    int m_i  = k_i;

    while (! (n_i <  choose(m_i+1, k_i) ) ) {
      m_i++;
    }
    m.push_back(m_i);
    n_i -= choose(m_i, k_i);
    k_i--;
  }
  
}


mpz_class KCanonical_Rep::compute()
{
  mpz_class s = 0;
  int k_i = k;
  for (vector<int>::iterator i=m.begin(); i!=m.end(); i++) {
    s += choose(*i, k_i);
    k_i--;
  }
  return s;
}
