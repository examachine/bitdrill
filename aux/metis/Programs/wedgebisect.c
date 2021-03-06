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
 * $Id: wedgebisect.c 1907 2005-02-18 16:45:00Z exa $
 *
 */

#include <metis.h>

void METIS_WEdgeBS(int *nvtxs, idxtype *xadj, idxtype *adjncy, idxtype *vwgt,
		   int *numflag, int *options, idxtype *perm, idxtype *iperm)
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
    ctrl.dbglvl  = 1; /* on */
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

  SetUpGraph(&graph, OP_OEMETIS, *nvtxs, 1, xadj, adjncy, vwgt, NULL, 2);

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

    printf("Nvtxs: %6d, [%6d %6d]\n", graph.nvtxs, graph.pwgts[0], graph.pwgts[1]);
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
  if (wgtflag == 0) {
    printf("Weighted graph required\n");
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
  METIS_WEdgeBS(&graph.nvtxs, graph.xadj, graph.adjncy, graph.vwgt,
		&numflag, options, perm, iperm);
  stoptimer(METISTmr);

  stoptimer(TOTALTmr);

  printf("\nTiming Information --------------------------------------------------\n");
  printf("  I/O:                     \t %7.3f\n", gettimer(IOTmr));
  printf("  Node Separator:          \t %7.3f\n", gettimer(METISTmr));
  printf("  Total:                   \t %7.3f\n", gettimer(TOTALTmr));
  printf("**********************************************************************\n");


  GKfree(&graph.xadj, &graph.adjncy, &graph.vwgt, &perm, &iperm, LTERM);
}


