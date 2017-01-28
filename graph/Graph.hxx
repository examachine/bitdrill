//
//
// C++ Interface for module: Graph
//
// Description:
//
//
// Author: exa
//
// Copyright: See COPYING file that came with this distribution
//
//

#ifndef Graph_Interface
#define Graph_Interface

#include "generic/General.hxx"

class Weighted_Edge {
public:
  Weighted_Edge() {}
  Weighted_Edge(unsigned int n, int w)
    : neighbor(n), weight(w) {}
  unsigned int neighbor;
  unsigned int weight;
};

typedef Weighted_Edge Edge;

inline ostream& operator <<(ostream& out, const Edge& e) {
  out << e.neighbor << ":" << e.weight;
  return out;
}

// Adjacency List Representation
class Graph
{
public:

  class Adjacency : public vector<Edge> {
  public:
    static const int default_capacity = 3;

    Adjacency() { reserve(default_capacity); }
    void add(Edge x);
    bool edge(unsigned int neighbor) {
      for (vector<Edge>::iterator u = begin(); u!=end(); u++) {
	if (u->neighbor==neighbor)
	  return true;
      }
      return false;
    }
    int weight(unsigned int neighbor) {
      for (vector<Edge>::iterator u = begin(); u!=end(); u++) {
	if (u->neighbor==neighbor)
	  return u->weight;
      }
      return 0;
    }
  };

  vector<int> weights;
  typedef vector<Adjacency> Adjacency_Vector;
  Adjacency_Vector adjacency;

  Graph() {}
  Graph(unsigned int nvtx);
  void reserve_vertex(unsigned int u);
  void resize(unsigned int nvtx);
  void addu(unsigned int u, unsigned int v, int w = 1) {
    // add undirected edge u, v with weight w
    add(u,v,w);
    add(v,u,w);
  }
// add edge u, v with weight w
  void add(unsigned int u, unsigned int v, int w = 1);
  bool vertex(unsigned int v) {
    return adjacency[v].size() != 0;
  }
  int degree(unsigned int v) {
    assert(v < adjacency.size()); // ensure adjacency is big enough
    return adjacency[v].size();
  }

  // vertex weight
  void set_weight(unsigned int u, int w) {
    weights[u] = w;
  }
  unsigned int weight(unsigned int u) {
    return weights[u];
  }

  // detect edge
  bool edge(unsigned int u, unsigned int v) {
    return adjacency[u].edge(v);
  }

  // edge weight
  bool weight(unsigned int u, int v) {
    return adjacency[u].weight(v);
  }

  void stats();
  unsigned int num_vertices() const { return adjacency.size(); }
  unsigned int real_num_vertices() {
    int nv = 0;
    for (unsigned int u=0; u<adjacency.size(); u++) {
      if (degree(u)>0) {
	nv++;
      }
    }
    return nv;
  }
  unsigned int num_edges();		// return number of DIRECTED edges
  unsigned int num_uedges() {		// return number of UNDIRECTED edges
    return num_edges()/2;
  }
  unsigned int sum_weight();
  unsigned int sum_edge_weights();
  void write_metis(const string& fn);
  void write(const string& fn);
  void encode(vector<int> &buffer);
  void decode(vector<int> &buffer);
  void broadcast();
  ostream& dot_structure(ostream& out);
  ostream& dot_graph(ostream& out, const string& dot_options="");

};

inline ostream& operator <<(ostream& out, const Graph::Adjacency& adj){
    out << "<" << adj.size() << " : ";
    if (adj.size()>0) {
      vector<Edge>::const_iterator i = adj.begin();
      vector<Edge>::const_iterator i_end = adj.end();
      out << *(i++);
      for (; i!=i_end; i++)
	out << "," << *i;
    }
    out << ">";
  return out;
}

void remove_small_deg_vertices(Graph& G, Graph& Gp, int min_degree);
void remove_small_edges(Graph& G, int min_degree);
#endif

