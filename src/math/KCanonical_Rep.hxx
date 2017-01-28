//
//
// C++ Interface for module: KCanonicalRep
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef KCanonical_Rep_Interface
#define KCanonical_Rep_Interface

#include "generic/General.hxx"
#include <gmpxx.h>

class KCanonical_Rep
{
public:
  KCanonical_Rep(int n, int k);
  void init(mpz_class n, int k);
  mpz_class compute();

  int k;
  vector<int> m;
  static mpz_class choose(mpz_class n, int k);
};

#endif
