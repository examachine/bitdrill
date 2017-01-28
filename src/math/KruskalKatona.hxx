//
//
// C++ Interface for module: KruskalKatona
//
// Description: Compute the KruskalKatona bound used in the context
// of frequency mining
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef KruskalKatona_Interface
#define KruskalKatona_Interface

#include "KCanonical_Rep.hxx"

class KruskalKatona : public KCanonical_Rep {
public:
  KruskalKatona(int n, int k) : KCanonical_Rep(n, k) {}
  mpz_class compute(int p);
  int last_level();
};

inline mpz_class kruskalkatona(int n, int k, int p) {
  KruskalKatona kk(n, k);
  return kk.compute(p);
}

#endif
