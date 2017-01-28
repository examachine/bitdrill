//
//
// C++ Interface for module: Vertex_Separator
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that came with this distribution
//
//

#ifndef Vertex_Separator_Interface
#define Vertex_Separator_Interface

#include "generic/General.hxx"
#include "Metis_Graph.hxx"

class Vertex_Separator
{
public:
  Vertex_Separator();
  ~Vertex_Separator();

  void clear();
  void broadcast(int src);
  void run_onmetis(Metis_Graph& G);
  void run_oemetis(Metis_Graph& G);
  int part(int u) const;
  void project(Vertex_Separator& sep, const vector<int>& vmap);
  ostream& dot_separator(ostream& out);

  set<int> A, B, S;
  unsigned int part_weights[3];

  GraphType graph;
  CtrlType ctrl;
  bool algorithm_run;

};

inline int Vertex_Separator::part(int u) const
{
  return graph.where[u];
}

extern "C" {
  void GKfree(void**,...);
}


#endif
