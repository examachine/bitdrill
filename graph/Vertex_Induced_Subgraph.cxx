//
//
// C++ Implementation for module: Vertex_Induced_Subgraph
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "Vertex_Induced_Subgraph.hxx"

void vertex_induced_subgraph(Graph& G, set<int> A, Graph& Gsub)
{
  nlog << "vertex_induced_subgraph" << endl;
//   nlog << A << endl;
  Gsub.resize(0);		// erase Gsub
  Gsub.resize(G.num_vertices());

  G.stats();

  for (set<int>::iterator ui=A.begin(); ui!=A.end(); ui++) {
    int u = *ui;
    Gsub.set_weight(u,G.weight(u)); // copy vertex weight
    Graph::Adjacency & adj = G.adjacency[u];
    for (Graph::Adjacency::iterator e = adj.begin(); e != adj.end(); e++) {
      int v = e->neighbor;
      if (A.find(v)!=A.end()) { // v in A
	Gsub.add(u,v,e->weight);
// 	nlog << "add " << u << ", " << v << ", " << e->weight << endl;
      }
    }
  }
//   nlog << "subgraph computed" << endl;
 
}


