//
//
// C++ Interface for module: Mtx_Graph
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef Mtx_Graph_Interface
#define Mtx_Graph_Interface

#include "generic/General.hxx"
#include "UT_Matrix.hxx"

class Undir_Mtx_Graph : private UT_Matrix
{
public:
  class Walk {
  public:
    Walk(Undir_Mtx_Graph& g) : w(g) {}
    int* ref() { return w.ref();}
    void next() { w.next();}
    bool end() { return w.end();}
  private:
    UT_Matrix::Walk w;
  };
  class Const_Walk {
  public:
    Const_Walk(const Undir_Mtx_Graph& g) : w(g) {}
    const int* ref() const { return w.ref();}
    void next() { w.next();}
    bool end() const { return w.end();}
    uint64_t u() { return w.i;}
    uint64_t v() { return w.j;}
  private:
    UT_Matrix::Const_Walk w;
  };
  friend class Walk;
  friend class Const_Walk;

  Undir_Mtx_Graph(uint64_t V) : UT_Matrix(V) {}
  int num_vertices() const {
    return get_rows();
  }
  int num_edges() const {
    return nonzero();
  }
  bool edge(uint64_t u, uint64_t v) {
    if (u>v) swap(u,v);
    return get(u,v) > 0;
  }
  int weight(uint64_t u,uint64_t v) {
    if (u>v) swap(u,v);
    return get(u,v);
  }
  // add undirected edge
  void add(uint64_t u, uint64_t v, int w = 1) {   // add edge u, v with weight v
    if (u>v) swap(u,v);
    set(u, v, get(u,v)+w);
  }
  void addu(uint64_t u, uint64_t v, int w = 1) {   // add edge u, v with weight v
    add(u,v,w);
  }
  uint64_t size() { return UT_Matrix::get_size(); }
  int* data() { return UT_Matrix::data();}
  void stats();
  void print() const;
};

#endif

