//
//
// C++ Implementation for module: Mtx_Graph
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "Mtx_Graph.hxx"

void Undir_Mtx_Graph::stats() {
  nlog << "|V|= " << num_vertices()
       << ", |E|= " << num_edges()
       << ", storage= " << get_size() << endl;
}


void Undir_Mtx_Graph::print() const
{
  for (Undir_Mtx_Graph::Const_Walk w(*this);
       !w.end(); w.next()) {
    int weight = *w.ref();
    nlog << w.u() << ", " << w.v() << " : " << weight << endl;
  }
}
