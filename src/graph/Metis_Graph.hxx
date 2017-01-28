//
//
// C++ Interface for module: Metis_Graph
//
// Description:
//
//
// Author: exa
//
// Copyright: See COPYING file that came with this distribution
//
//

#ifndef Metis_Graph_Interface
#define Metis_Graph_Interface

#include "generic/General.hxx"

#include "Graph.hxx"

#include <metis.h>

class Metis_Graph
{
public:

  Metis_Graph();
  Metis_Graph(Graph& G);
  void load(Graph& G, bool remove_zerodeg = true);
  ~Metis_Graph();

//   write(const string& fn)

  int nvtxs;
  int* xadj;
  int* vwgt;
  int nedges;
  int* adjncy;
  int* adjwgt;
};

#endif

