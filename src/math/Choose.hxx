//
//
// C++ Interface for module: Choose
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef Choose_Interface
#define Choose_Interface

#include "generic/General.hxx"

inline int factorial(int k) {
  int p = 1;
  for (int i=1; i<=k; i++)
    p *= i;
  return p;
}

inline int choose(int n, int k) {
  return factorial(n)/(factorial(k)*factorial(n-k));
}

template <typename T>
list< vector<T> > *gen_all_combs( const vector<T>& a)
{
  list< vector<T> > *c = new list< vector<T> >;
  gen_all_combs_aux(a,c);
  return c;
}

template <typename T>
void gen_all_combs_aux( const vector<T>& a, list< vector<T> > *c )
{
  typedef typename list< vector<T> >::iterator it;

  assert(a.size()>0);

  if (a.size()>1) {
    vector<T> ap = a;
    ap.pop_back();
    gen_all_combs_aux(ap, c);
  }
  vector<T> al;
  al.push_back(a.back());
  c->push_back(al);

  // now duplicate 
  it marker = c->end();
  marker--; // one before the last element
  for (it i=c->begin(); i!=marker; i++) {
    vector<T> x = *i;
    x.push_back(a.back());
    c->push_back(x);
  }
}

inline void gen_2comb(const vector<int>& A, vector< pair<int,int> >& C)
{
  if (A.size()>1) {
    vector<int>::const_iterator i=A.begin();
    vector<int>::const_iterator j=A.end();
    j--;
    int u = *j;
    for (; i!=j; i++) {
      int v = *i;
      C.push_back( pair<int,int>(u,v) );
    }
    vector<int> A_p = A;
    A_p.pop_back();
    //recursive step
    vector< pair<int,int> > C_p;
    gen_2comb(A_p, C_p);
    C.insert(C.end(), C_p.begin(), C_p.end());
  }
}


#endif
