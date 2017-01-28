//
//
// C++ Interface for module: Vertex_Induced_Subgraph
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef Vertex_Induced_Subgraph_Interface
#define Vertex_Induced_Subgraph_Interface

#include "generic/General.hxx"
#include "Graph.hxx"

// find a subgraph Gs of G induced by set of vertices A
// template <typename Set>
// void vertex_induced_subgraph(Graph& G, Set A, Graph& Gs);

void vertex_induced_subgraph(Graph& G, set<int> A, Graph& Gs);

#endif
