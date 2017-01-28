//
//
// C++ Implementation for module: Vertex_Separator
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that came with this distribution
//
//

#include "Vertex_Separator.hxx"

void Vertex_Separator::clear()
{
  A.clear();
  B.clear();
  S.clear();
}

class convert_vertex
{
public:
  convert_vertex(const vector<int>& _v) : v(_v) {}
  int operator()(int u) {
    return v[u];
  }
  const vector<int>& v;
};

// project from vertex separator through given map
void Vertex_Separator::project(Vertex_Separator& sep,
			       const vector<int>& vmap)
{
  convert_vertex project(vmap);
  transform (sep.A.begin(), sep.A.end(),
	     inserter(A, A.begin()), project); 
  transform (sep.B.begin(), sep.B.end(),
	     inserter(B, B.begin()), project); 
  transform (sep.S.begin(), sep.S.end(),
	     inserter(S, S.begin()), project); 
}

Vertex_Separator::Vertex_Separator()
  : algorithm_run(false)
{
}

Vertex_Separator::~Vertex_Separator()
{
  if (algorithm_run)
    FreeWorkSpace(&ctrl, &graph);
}

int count_part(GraphType* graph, int part) {
  int n = 0;
  int i;
  for (i=0; i<graph->nvtxs; i++) {
    if (graph->where[i]==part)
      n++;
  }
  return n;
}

unsigned int countedges_part(GraphType* graph, int part) {
  unsigned int n = 0;
  int i;
  int a;
  for (i=0; i<graph->nvtxs; i++) {
    if (graph->where[i]==part) {
      for(a = graph->xadj[i]; a < graph->xadj[i+1]; a++) {
        n += graph->adjwgt[a];
      }
    }
  }
  return n;
}

void Vertex_Separator::run_onmetis(Metis_Graph& G)
{
  nlog << "Vertex_Separator::run_onmetis" << endl;

  ctrl.CType   = ONMETIS_CTYPE;
  ctrl.IType   = ONMETIS_ITYPE;
  ctrl.RType   = ONMETIS_RTYPE;
  ctrl.dbglvl  = ONMETIS_DBGLVL;
  ctrl.oflags  = 0; /* no compression  ONMETIS_OFLAGS*/;
  ctrl.pfactor = ONMETIS_PFACTOR;
  ctrl.nseps   = ONMETIS_NSEPS;
  if (ctrl.nseps < 1)
    ctrl.nseps = 1;

  ctrl.optype = OP_ONMETIS;
  ctrl.CoarsenTo = 100;

  int *nvtxs=&G.nvtxs;
  int *xadj=G.xadj;
  int *adjncy=G.adjncy;
  int *vwgt=G.vwgt;

  // Do it random
  InitRandom(time(0));

  // Stable and good enough
//   InitRandom(-1);

  // initialize metis graph data structure
  SetUpGraph(&graph, OP_ONMETIS, *nvtxs, 1, xadj, adjncy, vwgt, NULL, 2);

  ctrl.maxvwgt = lrint(1.5*(idxsum(graph.nvtxs, graph.vwgt)/ctrl.CoarsenTo));
  AllocateWorkSpace(&ctrl, &graph, 2);

  float ubfactor = ORDER_UNBALANCE_FRACTION;
  int tvwgt, tpwgts2[2];

  /* Determine the weights of the partitions */
  tvwgt = idxsum(*nvtxs, graph.vwgt);
  nlog << "GPVS: total weight: " << tvwgt << endl;
  tpwgts2[0] = tvwgt/2;
  tpwgts2[1] = tvwgt-tpwgts2[0];

#ifdef DEBUG
  ctrl.dbglvl = 1;
  if (CheckGraph(&graph))
    nlog << "GPVS: graph ok" << endl;
  else {
    cerr << "graph incorrect" << endl;
    exit(-1);
  }
#endif

  MlevelNodeBisectionMultiple(&ctrl, &graph, tpwgts2, ubfactor);
  copy(graph.pwgts, graph.pwgts+3, part_weights);

  nlog << "GPVS: Nvtxs: " <<  graph.nvtxs
       <<", Parts: [ A:" <<  count_part(&graph,0) << ":" << graph.pwgts[0] 
       << " B:" <<  count_part(&graph,1) << ":" << graph.pwgts[1] 
       << " S:" << count_part(&graph,2) << ":" << graph.pwgts[2]
       << "]" << endl;

  nlog << "GPVS0: Nvtxs: " <<  graph.nvtxs
       <<", PartsVtx: [ A:" <<  count_part(&graph,0)
       << " B:" <<  count_part(&graph,1)
       << " S:" << count_part(&graph,2) << "]" << endl;


  algorithm_run = true;

  // find out sets

  for (int u=0; u<graph.nvtxs; u++) {
    switch(part(u)) {
      case 0:
	A.insert(u); break;
      case 1:
	B.insert(u); break;
      case 2: 
	S.insert(u); break;
    }
  }


}


void Vertex_Separator::run_oemetis(Metis_Graph& G)
{
  nlog << "Vertex_Separator::run_oemetis" << endl;

  ctrl.CType   = OEMETIS_CTYPE;
  ctrl.IType   = OEMETIS_ITYPE; /* initial partitioning type */
  ctrl.RType   = OEMETIS_RTYPE;
  ctrl.dbglvl  = 0 ; /* on */
  ctrl.oflags  = 0; /* no compression was: ONMETIS_OFLAGS*/;
  ctrl.pfactor = -1;
  ctrl.nseps   = 1;

  ctrl.optype = OP_OEMETIS;
  ctrl.CoarsenTo = 20;

  int *nvtxs=&G.nvtxs;
  int *xadj=G.xadj;
  int *adjncy=G.adjncy;
  int *vwgt=G.vwgt;
  int *adjwgt=G.adjwgt;

  // Do it random
  InitRandom(time(0));

  // Stable and good enough
//   InitRandom(-1);

  IFSET(ctrl.dbglvl, DBG_TIME, InitTimers(&ctrl));
  IFSET(ctrl.dbglvl, DBG_TIME, starttimer(ctrl.TotalTmr));

  // initialize metis graph data structure
  SetUpGraph(&graph, OP_OEMETIS, *nvtxs, 1, xadj, adjncy, vwgt, adjwgt, 3);

  ctrl.maxvwgt = lrint(1.5*(idxsum(graph.nvtxs, graph.vwgt)/ctrl.CoarsenTo));
  AllocateWorkSpace(&ctrl, &graph, 2);

  float ubfactor = ORDER_UNBALANCE_FRACTION;
  int tvwgt, tpwgts2[2];

  /* Determine the weights of the partitions */
  tvwgt = idxsum(*nvtxs, graph.vwgt);
  nlog << "GPVS: total weight: " << tvwgt << endl;
  tpwgts2[0] = tvwgt/2;
  tpwgts2[1] = tvwgt-tpwgts2[0];

#ifdef DEBUG
  ctrl.dbglvl = 1;
  if (CheckGraph(&graph))
    nlog << "GPVS: graph ok" << endl;
  else {
    cerr << "GPVS: graph incorrect" << endl;
    exit(-1);
  }
#endif

  nlog << "edge bisection" << endl;
  MlevelEdgeBisection(&ctrl, &graph, tpwgts2, ubfactor);

  nlog << "mincover" << endl;
  IFSET(ctrl.dbglvl, DBG_TIME, starttimer(ctrl.SepTmr));
  ConstructMinCoverSeparator(&ctrl, &graph, ubfactor);

  nlog << "GPVS: Nvtxs: " <<  graph.nvtxs
       <<", Parts: [ A:" <<  count_part(&graph,0) << ":"
       << graph.pwgts[0] << ":" << countedges_part(&graph,0) 
       << " B:" <<  count_part(&graph,1) << ":" << graph.pwgts[1]
       << ":" << countedges_part(&graph,1)
       << " S:" << count_part(&graph,2) << ":" << graph.pwgts[2]
       << ":" << countedges_part(&graph,2) << "]" << endl;

  nlog << "GPVS0: Nvtxs: " <<  graph.nvtxs
       <<", PartsVtx: [ A:" <<  count_part(&graph,0)
       << " B:" <<  count_part(&graph,1)
       << " S:" << count_part(&graph,2) << "]" << endl;

  algorithm_run = true;

  // find out sets

  for (int u=0; u<graph.nvtxs; u++) {
    switch(part(u)) {
      case 0:
	A.insert(u); break;
      case 1:
	B.insert(u); break;
      case 2: 
	S.insert(u); break;
    }
  }


}


void dot_part(ostream& out, set<int>& part, const string& partname)
{
  out << "  subgraph cluster_" << partname << " {" << endl;
  for (set<int>::iterator i=part.begin();i!=part.end();i++) {
    out << "    " << *i << ";" << endl;
  }
  out << "    style= \"dashed\";" << endl;
  out << "    label = \"" << partname << "\";" << endl;
  out << "  }" << endl;
}

ostream& Vertex_Separator::dot_separator(ostream& out)
{

  dot_part(out, A, "A");
  dot_part(out, B, "B");
  dot_part(out, S, "S");

  return out;
}



