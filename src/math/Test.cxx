//
//
// C++ Implementation for module: Test
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "Choose.hxx"
#include "KCanonical_Rep.hxx"
#include "KruskalKatona.hxx"
#include <gmpxx.h>

int main()
{

  {
    KCanonical_Rep kc(13,2);
    cout << "rep(13,2): " << kc.m << endl;
    cout << "n=" << kc.compute() << endl;
  }
  {
    KCanonical_Rep kc(4000,2);
    cout << "rep(4000,2): " << kc.m << endl;
    cout << "n=" << kc.compute() << endl;
  }

  cout << "kk(1000,2,3) = " << kruskalkatona(1000,2,3) << endl;
  
  cout << "fac(4)=" << factorial(4) << endl;

  cout << "fac(10)=" << factorial(10) << endl;

  cout << "generate all combinations of a,b,c,d" << endl;

  vector<char> x;
  x.push_back('a');
  x.push_back('b');
  x.push_back('c');
  x.push_back('d');

  list< vector<char> > *C = gen_all_combs(x);

  typedef list< vector<char> >::iterator it;
  for (it i=C->begin(); i!=C->end(); i++) {
    cout << *i << endl;
  }

  return 0;

}
