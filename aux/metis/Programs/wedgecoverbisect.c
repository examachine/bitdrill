/*
 * Copyright 1997, Regents of the University of Minnesota
 *
 * onmetis.c
 *
 * This file contains the driving routine for multilevel method
 *
 * Started 8/28/94
 * George
 *
 * $Id: wedgecoverbisect.c 2667 2007-09-13 20:08:28Z exa $
 *
 */

#include <metis.h>

int count_part(GraphType* graph, int part) {
  int n = 0;
  int i;
  for (i=0; i<graph->nvtxs; i++) {
    if (graph->where[i]==part)
      n++;
  }
  return n;
}

void write_partvector(GraphType* graph) {
  int i;
  FILE *f = fopen("partvector", "w");

  for (i=0; i<graph->nvtxs; i++) {
    fprintf(f, "%d ", graph->where[i]);
  }
  fprintf(f, "\n");
}


int countedges_part(GraphType* graph, int part) {
  int n = 0;
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


void METIS_WEdgeCoverBS(int *nvtxs, idxtype *xadj, idxtype *adjncy,
			idxtype *vwgt, idxtype *adjwgt,	int *numflag,
			int *options, idxtype *perm, idxtype *iperm)
{
  int i, ii, j, l, wflag, nflag;
  GraphType graph;
  CtrlType ctrl;
  idxtype *cptr, *cind, *piperm;

  /* Randomize thy thing */
  InitRandom(time(0));

  /* Stable and good enough */
  /*InitRandom(-1);*/

  if (*numflag == 1)
    Change2CNumbering(*nvtxs, xadj, adjncy);

  /* set up bisection parameters */

  if (options[0] == 0) {  /* Use the default parameters */
    ctrl.CType   = OEMETIS_CTYPE;
    ctrl.IType   = OEMETIS_ITYPE; /* initial partitioning type */
    ctrl.RType   = OEMETIS_RTYPE;
    ctrl.dbglvl  = DBG_SEPINFO ; /* on */
    ctrl.oflags  = 0; /* no compression was: ONMETIS_OFLAGS*/;
    ctrl.pfactor = -1;
    ctrl.nseps   = 1;
  }
  else {
    ctrl.CType   = options[OPTION_CTYPE];
    ctrl.IType   = options[OPTION_ITYPE];
    ctrl.RType   = options[OPTION_RTYPE];
    ctrl.dbglvl  = options[OPTION_DBGLVL];
    ctrl.oflags  = options[OPTION_OFLAGS];
    ctrl.pfactor = options[OPTION_PFACTOR];
    ctrl.nseps   = options[OPTION_NSEPS];
  }
  if (ctrl.nseps < 1)
    ctrl.nseps = 1;

  ctrl.optype = OP_OEMETIS;
  ctrl.CoarsenTo = 20;

  /* end parameters */

  IFSET(ctrl.dbglvl, DBG_TIME, InitTimers(&ctrl));
  IFSET(ctrl.dbglvl, DBG_TIME, starttimer(ctrl.TotalTmr));

  SetUpGraph(&graph, OP_OEMETIS, *nvtxs, 1, xadj, adjncy, vwgt, adjwgt, 3);

  ctrl.maxvwgt = 1.5*(idxsum(graph.nvtxs, graph.vwgt)/ctrl.CoarsenTo);
  AllocateWorkSpace(&ctrl, &graph, 2);

  {
    float ubfactor = ORDER_UNBALANCE_FRACTION;
    int tvwgt, tpwgts2[2];
    idxtype *label, *bndind;
    GraphType lgraph, rgraph;

    /* Determine the weights of the partitions */
    tvwgt = idxsum(graph.nvtxs, graph.vwgt);
    printf("total weight: %d\n", tvwgt);
    tpwgts2[0] = tvwgt/2;
    tpwgts2[1] = tvwgt-tpwgts2[0];

#ifdef DEBUG
  if (CheckGraph(&graph))
    cerr << "graph ok" << endl;
  else {
    cerr << "graph incorrect" << endl;
    exit(-1);
  }
#endif

    MlevelEdgeBisection(&ctrl, &graph, tpwgts2, ubfactor);

    //IFSET(ctrl->dbglvl, DBG_TIME, starttimer(ctrl->SepTmr));
    ConstructMinCoverSeparator(&ctrl, &graph, ubfactor);
    //IFSET(ctrl->dbglvl, DBG_TIME, stoptimer(ctrl->SepTmr));

    printf("Nvtxs: %6d, [A %d:%6d:%6d, B %d:%6d:%6d, S %d:%6d:%6d]\n",
	   graph.nvtxs,
	   count_part(&graph,0), graph.pwgts[0], countedges_part(&graph,0), 
	   count_part(&graph,1), graph.pwgts[1], countedges_part(&graph,1),
	   count_part(&graph,2), graph.pwgts[2], countedges_part(&graph,2));

    write_partvector(&graph);
  }

  FreeWorkSpace(&ctrl, &graph);

  IFSET(ctrl.dbglvl, DBG_TIME, stoptimer(ctrl.TotalTmr));
  IFSET(ctrl.dbglvl, DBG_TIME, PrintTimers(&ctrl));

  if (*numflag == 1)
    Change2FNumberingOrder(*nvtxs, xadj, adjncy, perm, iperm);

}


/*************************************************************************
* Let the game begin
**************************************************************************/
main(int argc, char *argv[])
{
  int i, options[10];
  idxtype *perm, *iperm;
  GraphType graph;
  char filename[256];
  int numflag = 0, wgtflag;
  timer TOTALTmr, METISTmr, IOTmr, SMBTmr;


  if (argc != 2) {
    printf("Usage: %s <GraphFile>\n",argv[0]);
    exit(0);
  }

  strcpy(filename, argv[1]);

  cleartimer(TOTALTmr);
  cleartimer(METISTmr);
  cleartimer(IOTmr);
  cleartimer(SMBTmr);

  starttimer(TOTALTmr);
  starttimer(IOTmr);
  ReadGraph(&graph, filename, &wgtflag);
  if (graph.nvtxs <= 0) {
    printf("Empty graph. Nothing to do.\n");
    exit(0);
  }
  if (graph.ncon != 1) {
    printf("This program runs on graphs with one constraint.\n");
    exit(0);
  }
  if (wgtflag != 3) {
    printf("Weights on both vertices and edges required\n");
    exit(0);
  }
  stoptimer(IOTmr);

  printf("**********************************************************************\n");
  printf("%s", METISTITLE);
  printf("Graph Information ---------------------------------------------------\n");
  printf("  Name: %s, #Vertices: %d, #Edges: %d\n\n", filename, graph.nvtxs, graph.nedges/2);
  printf("Node Separator... ----------------------------------------------\n");

  perm = idxmalloc(graph.nvtxs, "main: perm");
  iperm = idxmalloc(graph.nvtxs, "main: iperm");
  options[0] = 0;

  starttimer(METISTmr);
  METIS_WEdgeCoverBS(&graph.nvtxs, graph.xadj, graph.adjncy, graph.vwgt,
		     graph.adjwgt, &numflag, options, perm, iperm);
  stoptimer(METISTmr);

  stoptimer(TOTALTmr);

  printf("\nTiming Information --------------------------------------------------\n");
  printf("  I/O:                     \t %7.3f\n", gettimer(IOTmr));
  printf("  Node Separator:          \t %7.3f\n", gettimer(METISTmr));
  printf("  Total:                   \t %7.3f\n", gettimer(TOTALTmr));
  printf("**********************************************************************\n");


  GKfree(&graph.xadj, &graph.adjncy, &graph.vwgt, &perm, &iperm, LTERM);
}


