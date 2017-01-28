//
//
// C++ Implementation for module: Load_Estimate
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "Count.hxx"
//#include "math/KruskalKatona.hxx"

// Load Estimation functions

//my invention:

double TS_Counter::estimate_load_all(Load_Estimate_Func fxn)
{
  return estimate_load(fxn,F);
}

double TS_Counter::estimate_load(Load_Estimate_Func fxn, set<int>& A)
{
  double w = 0;			// work estimate for proj. T over A
  switch (fxn) {
  case fpgrowthload: {
    w = estimate_fpgrowth_load(A);
    break;
  }
  case datumsize: {
    w = size_items(A);
    break;
  }
  case approx3patterns: {
    w = approx_3patterns(A);
    break;
  }
  case kkstar: {
    w = kkstar_load(A);
    break;
  }
  case linearloadgraph: {
    w = linear_load_graph(A);
    break;
  }
  case f2weight: {
    w = f2_weight(A);
    break;
  }
  }
  return w;
}

// datum_size

int TS_Counter::size_items(set<int>& A)
{
  int size = 0;
  for (set<int>::iterator i=A.begin(); i!=A.end(); i++) {
    size += count[*i];
  }
  return size;
}

// approx_3items

int TS_Counter::approx_3patterns(set<int>& A)
{
  int t = *max_element(count.begin(), count.end());
  double f = 0;
  for (set<int>::iterator i=A.begin(); i!=A.end(); i++) {
    int deg = F2.degree(item_f[*i]);
    int degchoose2 = deg * (deg-1) / 2;
    f += degchoose2;
  }
  f /= A.size();
  double estimate = f * t;

  return lrint(estimate);
}

// my invention

double TS_Counter::estimate_fpgrowth_load(set<int>& A)
{
  int t = *max_element(count.begin(), count.end());
  double f = 0;
  for (set<int>::iterator i=A.begin(); i!=A.end(); i++) {
    double deg = (double)F2.degree(item_f[*i]);
    if (deg > 0)
      f += log(deg) * deg;
  }
  f /= A.size();
  f = max(f,1.0);
  double estimate = f * t;
  return estimate;
}

// KK* by Bart Goethals

double TS_Counter::kkstar_load(set<int>& A)
{
  return 0;
}

double TS_Counter::linear_load_graph(set<int>& A)
{
  double f = 0;
  for (set<int>::iterator i=A.begin(); i!=A.end(); i++) {
    double deg = (double)F2_load.weight(item_f[*i]);
      f += deg;
  }
  return f;
}

double TS_Counter::f2_weight(set<int>& A)
{
  double f = 0;
  for (set<int>::iterator i=A.begin(); i!=A.end(); i++) {
    f += F2.weight(item_f[*i]);
  }
  return f;
}
