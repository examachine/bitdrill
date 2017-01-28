/*
 * Copyright 1997, Regents of the University of Minnesota
 *
 * ndissect.c
 *
 * This file contains the driving routine for multilevel method
 *
 * Started 8/28/94
 * George
 *
 * $Id: ndissect.c 1135 2003-09-22 18:08:32Z exa $
 *
 */

#include <metis.h>

void MlevelNestedDissection(CtrlType *ctrl, GraphType *graph, idxtype *order, float ubfactor, int lastvtx);

void NodeND(int *nvtxs, idxtype *xadj, idxtype *adjncy, int *numflag,
	    int *options, idxtype *perm, idxtype *iperm) 
{
  int i, ii, j, l, wflag, nflag;
  GraphType graph;
  CtrlType ctrl;
  idxtype *cptr, *cind, *piperm;

  if (*numflag == 1)
    Change2CNumbering(*nvtxs, xadj, adjncy);

  if (options[0] == 0) {  /* Use the default parameters */
    ctrl.CType   = ONMETIS_CTYPE;
    ctrl.IType   = ONMETIS_ITYPE;
    ctrl.RType   = ONMETIS_RTYPE;
    ctrl.dbglvl  = ONMETIS_DBGLVL;
    ctrl.oflags  = 0; /* no compression */ ONMETIS_OFLAGS;
    ctrl.pfactor = ONMETIS_PFACTOR;
    ctrl.nseps   = ONMETIS_NSEPS;
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

  ctrl.optype = OP_ONMETIS;
  ctrl.CoarsenTo = 100;

  IFSET(ctrl.dbglvl, DBG_TIME, InitTimers(&ctrl));
  IFSET(ctrl.dbglvl, DBG_TIME, starttimer(ctrl.TotalTmr));

  InitRandom(-1);

  if (ctrl.pfactor > 0) { 
    /*============================================================
    * Prune the dense columns
    ==============================================================*/
    piperm = idxmalloc(*nvtxs, "ONMETIS: piperm");

    PruneGraph(&ctrl, &graph, *nvtxs, xadj, adjncy, piperm, (float)(0.1*ctrl.pfactor));
  }
  else if (ctrl.oflags&OFLAG_COMPRESS) {
    /*============================================================
    * Compress the graph 
    ==============================================================*/
    cptr = idxmalloc(*nvtxs+1, "ONMETIS: cptr");
    cind = idxmalloc(*nvtxs, "ONMETIS: cind");

    CompressGraph(&ctrl, &graph, *nvtxs, xadj, adjncy, cptr, cind);

    if (graph.nvtxs >= COMPRESSION_FRACTION*(*nvtxs)) {
      ctrl.oflags--; /* We actually performed no compression */
      GKfree(&cptr, &cind, LTERM);
    }
    else if (2*graph.nvtxs < *nvtxs && ctrl.nseps == 1)
      ctrl.nseps = 2;
  }
  else {
    SetUpGraph(&graph, OP_ONMETIS, *nvtxs, 1, xadj, adjncy, NULL, NULL, 0);
  }


  /*=============================================================
  * Do the nested dissection ordering 
  --=============================================================*/
  ctrl.maxvwgt = 1.5*(idxsum(graph.nvtxs, graph.vwgt)/ctrl.CoarsenTo);
  AllocateWorkSpace(&ctrl, &graph, 2);

  if (ctrl.oflags&OFLAG_CCMP) 
    ;/*MlevelNestedDissectionCC(&ctrl, &graph, iperm, ORDER_UNBALANCE_FRACTION, graph.nvtxs);*/
  else {
    float ubfactor = ORDER_UNBALANCE_FRACTION;
    int lastvtx = graph.nvtxs;
    int i, j, nvtxs, nbnd, tvwgt, tpwgts2[2];
    idxtype *label, *bndind;
    GraphType lgraph, rgraph;

    MlevelNestedDissection(&ctrl, &graph, iperm, ubfactor, graph.nvtxs);
    nvtxs = graph.nvtxs;

    /* Determine the weights of the partitions */
/*     tvwgt = idxsum(nvtxs, graph.vwgt); */
/*     tpwgts2[0] = tvwgt/2; */
/*     tpwgts2[1] = tvwgt-tpwgts2[0]; */

/*     MlevelNodeBisectionMultiple(&ctrl, &graph, tpwgts2, ubfactor); */

/*     printf("Nvtxs: %6d, [%6d %6d %6d]\n", graph.nvtxs, graph.pwgts[0], graph.pwgts[1], graph.pwgts[2]); */
  }

  FreeWorkSpace(&ctrl, &graph);

  if (ctrl.pfactor > 0) { /* Order any prunned vertices */
    if (graph.nvtxs < *nvtxs) { 
      idxcopy(graph.nvtxs, iperm, perm);  /* Use perm as an auxiliary array */
      for (i=0; i<graph.nvtxs; i++)
        iperm[piperm[i]] = perm[i];
      for (i=graph.nvtxs; i<*nvtxs; i++)
        iperm[piperm[i]] = i;
    }

    GKfree(&piperm, LTERM);
  }
  else if (ctrl.oflags&OFLAG_COMPRESS) { /* Uncompress the ordering */
    if (graph.nvtxs < COMPRESSION_FRACTION*(*nvtxs)) { 
      /* construct perm from iperm */
      for (i=0; i<graph.nvtxs; i++)
        perm[iperm[i]] = i; 
      for (l=ii=0; ii<graph.nvtxs; ii++) {
        i = perm[ii];
        for (j=cptr[i]; j<cptr[i+1]; j++)
          iperm[cind[j]] = l++;
      }
    }

    GKfree(&cptr, &cind, LTERM);
  }


  for (i=0; i<*nvtxs; i++)
    perm[iperm[i]] = i;

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
    printf("Nested dissection can only be applied to graphs with one constraint.\n");
    exit(0);
  }
  stoptimer(IOTmr);

  /* Ordering does not use weights! */
  GKfree(&graph.vwgt, &graph.adjwgt, LTERM);

  printf("**********************************************************************\n");
  printf("%s", METISTITLE);
  printf("Graph Information ---------------------------------------------------\n");
  printf("  Name: %s, #Vertices: %d, #Edges: %d\n\n", filename, graph.nvtxs, graph.nedges/2);
  printf("Node Separator... ----------------------------------------------\n");

  perm = idxmalloc(graph.nvtxs, "main: perm");
  iperm = idxmalloc(graph.nvtxs, "main: iperm");
  options[0] = 0;

  starttimer(METISTmr);
  NodeND(&graph.nvtxs, graph.xadj, graph.adjncy, &numflag, options, perm, iperm);
  stoptimer(METISTmr);

  stoptimer(TOTALTmr);

  printf("\nTiming Information --------------------------------------------------\n");
  printf("  I/O:                     \t %7.3f\n", gettimer(IOTmr));
  printf("  Node Separator:          \t %7.3f\n", gettimer(METISTmr));
  printf("  Total:                   \t %7.3f\n", gettimer(TOTALTmr));
  printf("**********************************************************************\n");


  GKfree(&graph.xadj, &graph.adjncy, &perm, &iperm, LTERM);
}  


