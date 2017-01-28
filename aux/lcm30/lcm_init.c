

#ifndef _lcm_init_c_
#define _lcm_init_c_

#include"lcm_io.c"

/*************************************************************************/
/* Common initialization for LCM, LCMfreq, LCMmax */
/*************************************************************************/
int LCM_init ( int argc, char *argv[] ){
  int i, m, n, nn, *bbuf;
  QUEUE *Q;
  QUEUE_INT *x;

  if ( argc<2 ){
    printf ("LCM: input-filename support [output-filename]\n");
    exit (1);
  }

  LCM_start_time = time(NULL);
  LCM_th = atoi(argv[2]);
  LCM_print_flag |= (argc>3? 1: 0);
  n = LCM_BM_load ( argv[1] );   /* loding file of bitmap version */

  if ( LCM_Trsact.num == 0 ){
    if ( LCM_print_flag &2 ) printf ("there is no frequent itemset\n");
    ARY_end ( &LCM_Trsact );
    exit (0);
  }

  if ( LCM_print_flag &2 ){
    printf ("shrinked database: #transactions=%d, #item=%d #elements=%d\n", LCM_Trsact.num, LCM_Eend, n );
  }
  
  bbuf = ((QUEUE *)(LCM_Trsact.h))->q;
  if ( LCM_BM_MAXITEM > LCM_Eend ) LCM_BM_MAXITEM = LCM_Eend;
  LCM_BM_init ();

  if ( LCM_BM_MAXITEM < LCM_Eend ){
    LCM_shrink_init (&LCM_Trsact, LCM_Eend);
    LCM_shrink (&LCM_Trsact, LCM_Eend, 1);
  }
//  LCM_buf = (int *)realloc ( bbuf, sizeof(int)*(nn+LCM_Trsact.num));
//  for ( i=0 ; i<LCM_Trsact.num ; i++ )
// ((QUEUE*)(LCM_Trsact.h))[i].q =((QUEUE*)(LCM_Trsact.h))[i].q+(LCM_buf-bbuf);

/***********************************************************************/
//  TRSACT_sort_by_size ( &LCM_Trsact, &LCM_shrink_jump, LCM_shrink_p );
//    realloc2 ( A->h, char, A->unit*A->end, "ARY_exp: A->h" );
/***********************************************************************/

  if ( LCM_print_flag &2 ){
    for (n=i=0 ; i<LCM_Trsact.num ; i++) n += ((QUEUE*)(LCM_Trsact.h))[i].t;
    printf ("shrinked database2: #transactions=%d, #item=%d #elements=%d\n", LCM_Trsact.num, LCM_Eend, n );
    printf ("LCM_BM_MAXITEM = %d\n", LCM_BM_MAXITEM );
  }

  malloc2 ( LCM_sc, int, LCM_Eend+1, "LCM_init: LCM_sc" );
  QUEUE_init ( &LCM_jump, LCM_Eend*2+2 );
  LCM_jump.end = LCM_Eend;
  LCM_itemsett = 0;
  malloc2 ( LCM_itemsetp, char*, LCM_Eend, "LCM_init: LCM_itemsetp" );

  QUEUE_init ( &LCM_add, LCM_Eend );
  if ( LCM_PROBLEM != LCM_FREQSET ){
    malloc2 ( LCM_ary, int, LCM_Eend*2, "LCM_init: LCM_ary" );
    QUEUE_init ( &LCM_Qtmp, LCM_Eend*2+2 );
    malloc2 ( LCM_Ofrq_, int, LCM_Eend, "LCM_init: LCM_Ofrq_" );
    for ( i=0 ; i<LCM_Eend ; i++ ){
      LCM_ary[i] = 0;
      LCM_Qtmp.q[i+LCM_Eend] = -1;
    }
  }
  malloc2 ( LCM_Os, int*, LCM_Eend, "LCM_init: LCM_Os" );
  malloc2 ( LCM_Ot, int*, LCM_Eend, "LCM_init: LCM_Ot" );
  malloc2 ( LCM_Ofrq, int, LCM_Eend, "LCM_init: LCM_Ofrq" );
  malloc2 ( LCM_Op, int, LCM_Eend, "LCM_init: LCM_Op" );
  for ( i=0 ; i<LCM_Eend ; i++ ){
    LCM_sc[i] = LCM_Ofrq[i] = 0;
    LCM_Ot[i]=NULL;
  }
  LCM_sc[LCM_Eend] = 0;
  for ( Q=LCM_Trsact.h,i=0 ; i<LCM_Trsact.num ; i++ ){
    m = Q[i].end;
    for ( x=Q[i].q ; *x<LCM_Eend ; x++ ){
      LCM_Ot[*x]++;
      LCM_Ofrq[*x] += m;
    }
  }
  int *ttt=0;
  for ( i=0,m=2 ; i<LCM_Eend ; i++ ){
    if ( i < LCM_BM_MAXITEM ){
      malloc2 (LCM_Os[i], int, m+2, "LCM_init: LCM_Os[i]");
      m *= 2;
    } else malloc2(LCM_Os[i], int, (LCM_Ot[i]-ttt)+2, "LCM_init: LCM_Os[i]");
    if ( LCM_Os[i] == NULL ){
      printf ("LCM_init: memory short for LCM_Os[i]");
      exit(1);
    }
    LCM_Ot[i] = LCM_Os[i];
  }

  for ( i=0 ; i<LCM_BM_MAXITEM ; i++ ) LCM_Op[i] = i;  /* item permutation for bitmap items */
  LCM_occ_deliver( &LCM_Trsact, LCM_Eend-1 );
  if ( LCM_print_flag&1 ){
    FASTO_init_perm ( argv[3], LCM_Eend );
    malloc2 (LCM_itemsetbuf, char, FASTO_mmag*LCM_Eend+sizeof(int)*4, "FASTO_init: FASTO_digit");
    LCM_p = LCM_itemsetbuf;
  }
}

/*************************************************************************/
/* Common end processing for LCM, LCMfreq, LCMmax */
/*************************************************************************/
void LCM_end (){
  int j, i, *x, n=0, m;
  QUEUE *Q;

  if ( LCM_print_flag&1 ){ FASTO_end (); free2( LCM_itemsetbuf ); }
  QUEUE_end ( &LCM_jump );
  QUEUE_end ( &LCM_add );
  if ( LCM_PROBLEM != LCM_FREQSET ){
    QUEUE_end ( &LCM_Qtmp );
    free2 ( LCM_Ofrq_ );
    free2 ( LCM_ary );
  }
  for ( i=0 ; i<LCM_Eend ; i++ ) free2 ( LCM_Os[i] );
  free2 ( LCM_Os );
  free2 ( LCM_Ot );
  free2 ( LCM_Ofrq );
  free2 ( LCM_Op );
  free2 ( LCM_sc );
  free2 ( LCM_buf );
}

#endif

