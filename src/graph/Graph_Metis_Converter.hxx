//
//
// C++ Interface for module: Graph_Metis_Converter
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that came with this distribution
//
//

#ifndef Graph_Metis_Converter_Interface
#define Graph_Metis_Converter_Interface

#include "generic/General.hxx"
#include "Graph.hxx"
#include "Metis_Graph.hxx"

class Graph_Metis_Converter
{
public:
  vector<int> vertex_map;
  vector<int> inverse_vertex_map;
  void convert(Graph& G, Metis_Graph& M, int scale=1);
};

#endif
