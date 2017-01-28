//
//
// C++ Implementation file for application
//
// Description: includes the main(...) function
//
// exa
//
//

#include "generic/General.hxx"
#include "Graph.hxx"
#include "Metis_Graph.hxx"
#include "Graph_Metis_Converter.hxx"
#include "Vertex_Separator.hxx"
#include "Mtx_Graph.hxx"

int main(int argc, char *argv[])
{
  Graph G;
  G.addu(3,7,1);
  G.addu(5,2,2);
  G.addu(9,8,3);
  G.addu(5,7,2);
  cout << "Number of vertices: " << G.adjacency.size()
       << ", total (directed) vertex weight: " << G.sum_weight() << endl;
  cout << "Number of (directed) edges: " << G.num_edges() << endl;
  cout << "G.edge(3,7)=" << G.edge(3,7) << endl;
  cout << "G.edge(5,2)=" << G.edge(5,2) << endl;
  cout << "G.edge(2,5)=" << G.edge(2,5) << endl;
  cout << "G.edge(3,6)=" << G.edge(3,3) << endl;
  cout << "G.edge(9,8)=" << G.edge(9,8) << endl;

  Graph_Metis_Converter graph_converter;
  Metis_Graph graph;

  graph_converter.convert(G, graph); // convert to metis fmt
  cout << "V_new = " << graph_converter.inverse_vertex_map << endl;

  // write graph for debugging
  G.write_metis("G.metis");
  int nvtxs=graph.nvtxs;
  int *xadj=graph.xadj;
  int *adjncy=graph.adjncy;
  int *vwgt=graph.vwgt;

  // initialize metis graph data structure
  WriteGraph("graph.metis", nvtxs, xadj, adjncy);

  Undir_Mtx_Graph A(10);
  //Vertex_Separator vsep;
  //vsep.run(M);

  return 0; // success
}
