//
//
// C++ Implementation for module: Metis_Graph
//
// Description:
//
//
// Author: exa
//
// Copyright: See COPYING file that came with this distribution
//
//

#include "Metis_Graph.hxx"
#include "Graph_Metis_Converter.hxx"

Metis_Graph::Metis_Graph()
  : xadj(0), adjncy(0), adjwgt(0)
{
}

Metis_Graph::Metis_Graph(Graph& G)
{
  load(G);
}

void Metis_Graph::load(Graph& G, bool remove_zero_deg)
{

  if (remove_zero_deg) {
    Graph_Metis_Converter c;
    c.convert(G, *this);
  }
  else {
    nvtxs=G.adjacency.size();
    xadj = new int[nvtxs+1];
    vwgt = new int[nvtxs+1];
    nedges = G.num_edges();
    adjncy = new int[G.num_edges()];
    adjwgt = new int[G.num_edges()];
    int u=0;
    int metisv=0;
    int metise=0;
    xadj[0]=0;
    for (Graph::Adjacency_Vector::iterator i=G.adjacency.begin();
	 i!=G.adjacency.end(); i++,u++) {
      Graph::Adjacency & adj = *i;
      for (Graph::Adjacency::iterator v = adj.begin(); v != adj.end(); v++) {
	assert(metise<nedges);
	adjncy[metise]=v->neighbor;
	adjwgt[metise]=v->weight;
	metise++;
      }
      metisv++;
      xadj[metisv]=metise;
      vwgt[metisv]=G.weight(u);
    }
  }
}

// Metis_Graph::Metis_Graph(Graph& G, Metis_Graph::mode m)

Metis_Graph::~Metis_Graph()
{
  if (xadj)
    delete xadj;
  if (adjncy)
    delete adjncy;
}
