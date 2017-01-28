//
//
// C++ Implementation for module: Graph
//
// Description:
//
//
// Author: exa
//
// Copyright: See COPYING file that came with this distribution
//
//

#include "Graph.hxx"

Graph::Graph(unsigned int nvtx) {
  resize(nvtx);
}

void Graph::reserve_vertex(unsigned int u)
{
  if (u >= adjacency.size()) {
//     resize( max( adjacency.size() * 2, (unsigned int)(u+1) )  );
    resize( (u+1) );
  }
}

void Graph::resize(unsigned int nvtx)
{ 
  adjacency.resize(nvtx);
  // initialize new vertex weights to 1
  unsigned int oldsize = weights.size();
  weights.resize(nvtx);
  if (oldsize < nvtx)
    fill(weights.begin() + oldsize, weights.end(), 0);
}

void Graph::Adjacency::add(Edge x) {
  //check duplicate
  for (vector<Edge>::iterator elt=begin(); elt!=end(); elt++) {
    if (elt->neighbor==x.neighbor) {
      elt->weight += x.weight;
      return;
    }
  }
  push_back(x);
}

void Graph::add(unsigned int u, unsigned int v, int w) {
  reserve_vertex(max(u,v));
  adjacency[u].add(Edge(v, w));
}

void Graph::stats() {
  nlog << "Number of vertices: " << adjacency.size()
       << ", total vertex weight: " << sum_weight() << endl;
  nlog << "Number of (undirected) edges: " << num_uedges()
       << ", total (undirected) edge weight: " << sum_edge_weights()/2 << endl;
}

unsigned int Graph::num_edges() {
  int num = 0;
  for (Adjacency_Vector::iterator i=adjacency.begin();
       i!=adjacency.end(); i++) {
    num += i->size();
  }
  return num;
}

unsigned int Graph::sum_weight() {
  unsigned int s = 0;
  for (unsigned int i=0; i<num_vertices(); i++)
    s += weights[i];
  return s;
}

unsigned int Graph::sum_edge_weights() {
  unsigned int s = 0;
  for (Graph::Adjacency_Vector::iterator i=adjacency.begin();
       i!=adjacency.end(); i++) {
    Graph::Adjacency & adj = *i;
    for (vector<Edge>::iterator e = adj.begin(); e != adj.end(); e++) {
      s+= e->weight;
    }
  }
  return s;
}

void Graph::write_metis(const string& fn)
{
//   nlog << "Graph::write_metis" << endl;
  ofstream out(fn.c_str());

  vector<int> map(adjacency.size());
  int num_vertices = 0;
  fill(map.begin(), map.end(), -1);

  int u=0;
  for (Adjacency_Vector::iterator i=adjacency.begin();
       i!=adjacency.end(); i++) {
    Adjacency & adj = *i;
    if (adj.size()>0) {
      map[u] = num_vertices++;
    }
    u++;
  }

  // make new graph without 0-deg vertices

  out << num_vertices << " " << num_uedges() << " 11" << endl;

  u = 0;
  for (Adjacency_Vector::iterator i=adjacency.begin();
       i!=adjacency.end(); i++) {
   Adjacency & adj = *i;
   if (adj.size()>0) {
      out << weight(u) << " "; // write weight
      for (vector<Edge>::iterator v = adj.begin(); v != adj.end(); v++) {
	out << map[v->neighbor] + 1 << " " << v->weight << " ";
      }
      out << endl;
    }
    u++;
  }
}

void Graph::write(const string& fn)
{
  ofstream out(fn.c_str());

  int u = 0;
  out << "|V| = " << num_vertices() << endl;
  for (Adjacency_Vector::iterator i=adjacency.begin();
       i!=adjacency.end(); i++) {
    Adjacency & adj = *i;
    if (adj.size()>0) {
      out << "adj(" << u << ") = " << adj << endl;
      out << endl;
    }
    u++;
  }
}

void Graph::encode(vector<int>& buf)
{
  nlog << "encoding graph" << endl;
  buf.erase(buf.begin(), buf.end());

  // number of vertices first
  buf.push_back(num_vertices());

  // vertex weights
  for (unsigned int i=0; i<num_vertices(); i++) {
    buf.push_back(weights[i]);
  }

  for (Adjacency_Vector::iterator i=adjacency.begin();
       i!=adjacency.end(); i++) {
    Adjacency & adj = *i;
    buf.push_back(adj.size());
    for (vector<Edge>::iterator e = adj.begin(); e != adj.end(); e++) {
      int u = e->neighbor;
      int w = e->weight;
      buf.push_back(u);
      buf.push_back(w);
    }
  }
}

void Graph::decode(vector<int>& buf)
{
  nlog << "decoding graph" << endl;
  vector<int>::iterator bufit = buf.begin();

  // number of vertices first
  int nvtx = *bufit++;

  // vertex weights
  for (int i=0; i<nvtx; i++) {
    weights[i] = *bufit++;
  }

  int u = 0;
  while ( bufit != buf.end() ) {
    int size = *bufit++;
    for (int i=0; i<size; i++) {
      int v = *bufit++;
      int w = *bufit++;
      add(u, v, w);
    }
    u++;
  }
}

void remove_small_deg_vertices(Graph& G, Graph& G_p, unsigned int min_degree)
{
  vector<int> map(G.adjacency.size());
  int num_vertices = 0;
  fill(map.begin(), map.end(), -1);

  // construct a vertex map : u -> v where u in G, v in G_p
  int u=0;
  for (Graph::Adjacency_Vector::iterator i=G.adjacency.begin();
       i!=G.adjacency.end(); i++) {
    Graph::Adjacency & adj = *i;
    if (adj.size()>min_degree) {
      map[u] = num_vertices++;
      //cout << "adj(" << u << ")="<< adj << endl;
    }
    u++;
  }

  // apply vertex map to vertex maps
  for (u=0; u<num_vertices; u++) {
    if (map[u]!=-1)
      G_p.set_weight(map[u], G.weight(u));
  }

  // apply vertex map to adjacency list
  u = 0;
  for (Graph::Adjacency_Vector::iterator i=G.adjacency.begin();
       i!=G.adjacency.end(); i++) {
    Graph::Adjacency & adj = *i;
    for (vector<Edge>::iterator e = adj.begin(); e != adj.end(); e++) {
//       assert(u!=e->neighbor);
      int m_u = map[u];
      int m_v = map[e->neighbor];
      if (m_u!=-1 && m_v!=-1) {
	G_p.add(m_u, m_v, e->weight);
      }
    }
    u++;
  }

}

void remove_small_edges(Graph& G, int min_degree)
{
  // NOT IMPLEMENTED
  assert(false);
}

ostream& Graph::dot_graph(ostream& out, const string & dot_options)
{
  out << "graph G { " << endl;
  out << dot_options << endl;
  dot_structure(out);
  out << "}" << endl;

  return out;
}

ostream& Graph::dot_structure(ostream& out)
{
  int u = 0;
  for (Adjacency_Vector::iterator i=adjacency.begin();
       i!=adjacency.end(); i++) {
    Adjacency & adj = *i;
    if (adj.size() > 0){
      out << "  " << u << " [ label=" << weights[u] << "];" << endl;
    }
    u++;
  }
  u = 0;
  for (Adjacency_Vector::iterator i=adjacency.begin();
       i!=adjacency.end(); i++) {
    Adjacency & adj = *i;
    for (vector<Edge>::iterator e = adj.begin(); e != adj.end(); e++) {
      int v = e->neighbor;
      //int w = e->weight;
      if (u < v)		// don't print twice
	out << "  " << u << " -- " << v << ";" << endl;
    }
    u++;
  }

  return out;
}
