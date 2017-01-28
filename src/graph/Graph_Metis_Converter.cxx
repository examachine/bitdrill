//
//
// C++ Implementation for module: Graph_Metis_Converter
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that came with this distribution
//
//

#include "Graph_Metis_Converter.hxx"

// converts a digraph G to a metis graph M
void Graph_Metis_Converter::convert(Graph& G, Metis_Graph& M, int scale)
{
  vertex_map.resize(G.adjacency.size());
  inverse_vertex_map.resize(G.adjacency.size());
  int num_vertices = 0;
  fill(vertex_map.begin(), vertex_map.end(), -1);

  int u=0;
  for (Graph::Adjacency_Vector::iterator i=G.adjacency.begin();
       i!=G.adjacency.end(); i++) {
    Graph::Adjacency & adj = *i;
    if (adj.size()>0) {
      vertex_map[u] = num_vertices;
      inverse_vertex_map[num_vertices] = u;
      num_vertices++;
    }
    u++;
  }

  inverse_vertex_map.resize(num_vertices);
  M.nvtxs=num_vertices;
  M.vwgt=new int[M.nvtxs+1];
  M.xadj = new int[M.nvtxs+1];
  M.nedges = G.num_edges();
  M.adjncy = new int[G.num_edges()];
  M.adjwgt = new int[G.num_edges()];
  for (u=0; u<M.nvtxs; u++) {
    M.vwgt[u]=G.weight(inverse_vertex_map[u])/scale;
  }
  u=0;
  int metisv=0;
  int metise=0;
  M.xadj[0]=0;
  for (Graph::Adjacency_Vector::iterator j=G.adjacency.begin();
       j!=G.adjacency.end(); j++) {
    Graph::Adjacency & adj = *j;
    if (adj.size()!=0) { // process vertices with degree > 0
      for (Graph::Adjacency::iterator e = adj.begin(); e != adj.end(); e++) {
	int v = e->neighbor;
 	//nlog << "convert (" << u << ',' << v << ") " << endl << flush;
	assert(metise<M.nedges);
	assert(vertex_map[v]>=0);
	assert(vertex_map[v]<M.nvtxs);
	M.adjncy[metise]=vertex_map[v];
	M.adjwgt[metise]=e->weight/scale;
	metise++;
      }
      metisv++;
      M.xadj[metisv]=metise;
    }
    u++;
  }

}

