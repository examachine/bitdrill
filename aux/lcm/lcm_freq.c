/* Linear time Closed itemset Miner for Frequent Itemset Mining problems */
/* 2004/4/10 Takeaki Uno */
/* This program is available for only academic use.
   Neither commercial use, modification, nor re-distribution is allowed */

#ifndef _lcmfreq_c_
#define _lcmfreq_c_

#include<time.h>
#include"trsact.c"

/**********************************************************************/
/* sub-routine when #items == 3 */
/* input: T:transactions, occ:denotations, e1,e2,e3:items */
/**********************************************************************/
void LCM_freq_iter_few3 ( ARY *T, QUEUE *occ, int e1, int e2, int e3 ){
  int i, f12,f13,f23,f123;
  QUEUE *Q = T->h;
  QUEUE_INT *x;
  long *t, *tt=((long *)(occ->q))+occ->t;
  f12 = f13 = f23 = f123 = 0;
  for ( t=(long *)(occ->q) ; t<tt ; t++ ){
    x=Q[*t].q;
    while ( *x<e1 ) x++;
    if ( *x==e1 ){
      x++;
      while ( *x<e2 ) x++;
      if ( *x==e2 ){
        x++;
        while ( *x<e3 ) x++;
        if ( *x==e3 ) f123 += Q[*t].end;
        else f12 += Q[*t].end;
      } else {
        while ( *x<e3 ) x++;
        if ( *x==e3 ) f13 += Q[*t].end;
      }
    } else {
      while ( *x<e2 ) x++;
      if ( *x==e2 ){
        x++;
        while ( *x<e3 ) x++;
        if ( *x==e3 ) f23 += Q[*t].end;
      }
    }
  }
  if ( f123>=LCM_th ){
    QUEUE_ins_ ( &LCM_itemset, LCM_perm[e3] );
    QUEUE_ins_ ( &LCM_itemset, LCM_perm[e2] );
    LCM_print_last ( e1, f123);
    LCM_itemset.t-=2;
  }
  if ( f12+f123>=LCM_th ){
    QUEUE_ins_ ( &LCM_itemset, LCM_perm[e2] );
    LCM_print_last ( e1, f12+f123 );
    LCM_itemset.t--;
  }
  if ( f13+f123>=LCM_th ){
    QUEUE_ins_ ( &LCM_itemset, LCM_perm[e3] );
    LCM_print_last ( e1, f13+f123 );
    LCM_itemset.t--;
  }
  if ( f23+f123>=LCM_th ){
    QUEUE_ins_ ( &LCM_itemset, LCM_perm[e3] );
    LCM_print_last ( e2, f23+f123 );
    LCM_itemset.t--;
  }
}

/**********************************************************************/
/* sub-routine when #items == 2 */
/* input: T:transactions, occ:denotations, e1,e2,e3:items */
/**********************************************************************/
void LCM_freq_iter_few2 ( ARY *T, QUEUE *occ, int e1, int e2 ){
  int i, f=0;
  QUEUE *Q = T->h;
  long *t, *tt=((long *)(occ->q))+occ->t;
  QUEUE_INT *x;
  for ( t=(long *)(occ->q) ; t<tt ; t++ ){
    x=Q[*t].q;
    while ( *x<e2 ) x++;
    if ( *x==e2 ){
      x++;
      while ( *x<e1 ) x++;
      if ( *x==e1 ){
        f += Q[*t].end;
      }
    }
  }

  if ( f>=LCM_th ){
    QUEUE_ins_ ( &LCM_itemset, LCM_perm[e1] );
    LCM_print_last ( e2, f );
    LCM_itemset.t --;
  }
}


/*************************************************************************/
/* LCMfreq iteration (lcm_closed-base type ) */
/* input: T:transactions(database), item:tail(current solution) */
/*************************************************************************/
void LCMfreq_iter( ARY *T, int item ){
  ARY TT;
  int i, ii, im, e, ee, n;
  int js = LCM_jump.s, it = LCM_itemset.t, qt = LCM_Qtmp.t;
  QUEUE_INT *q;

  LCM_jump.s = LCM_jump.t;
  LCM_frq = LCM_occ[item].end;
  LCM_iters++;
  QUEUE_ins_ ( &LCM_itemset, LCM_perm[item] );
  n = LCM_freq_calc( T, &LCM_occ[item], &LCM_Occ, &LCM_jump, item-1 );
  LCM_jump_rm_infreq ( &LCM_Occ, &LCM_jump, LCM_th, LCM_frq, &LCM_Qtmp );
  for ( i=qt ; i<LCM_Qtmp.t ; i++ )
      LCM_Qtmp.q[i] = LCM_perm[LCM_Qtmp.q[i]];
  LCM_print_solution2 ();

  if ( QUEUE_LENGTH(LCM_jump) == 0 ) goto END2;
  QUEUE_sortr_ ( &LCM_jump );


  i = QUEUE_ext_tail_ ( &LCM_jump );
  LCM_print_last ( i, LCM_occ[i].end );
  if ( QUEUE_LENGTH(LCM_jump)<=2 ){
    LCM_occ[i].t = LCM_occ[i].end = 0;
    if ( QUEUE_LENGTH(LCM_jump)>0 ){
      ii = QUEUE_ext_tail_ ( &LCM_jump );
      LCM_print_last ( ii, LCM_occ[ii].end );
      LCM_occ[ii].t = LCM_occ[ii].end = 0;
      if ( QUEUE_LENGTH(LCM_jump)>0 ){
        e = QUEUE_ext_tail_ ( &LCM_jump );
        LCM_print_last ( e, LCM_occ[e].end );
        LCM_occ[e].t = LCM_occ[e].end = 0;
        LCM_freq_iter_few3 ( T, &LCM_occ[item], i, ii, e );
      } else LCM_freq_iter_few2 ( T, &LCM_occ[item], ii, i );
    }
    goto END2;
  }

/******************************************************/

  LCM_mk_freq_trsact(&TT, T, &LCM_Occ, &LCM_occ[item], n+LCM_occ[item].t, LCM_jump.q[LCM_jump.s], LCM_jump.q[LCM_jump.s], LCM_th, LCM_frq);
  q = ((QUEUE *)(TT.h))->q;
  if ( QUEUE_LENGTH(LCM_jump) >= 2 && TT.num>7 )
    TRSACT_shrink ( &TT, &LCM_shrink_jump, LCM_shrink_p );
  TRSACT_occ_deliver_all( &TT, &LCM_Occ, item-1 );
  LCM_occ[i].t = LCM_occ[i].end = 0;

  while ( QUEUE_LENGTH(LCM_jump) > 0 ){
    i = QUEUE_ext_tail_ ( &LCM_jump );
    LCMfreq_iter ( &TT, i);
    LCM_occ[i].t = LCM_occ[i].end = 0;
  }

  free2 ( q );
  ARY_end ( &TT );
  END2:;
  LCM_jump.t = LCM_jump.s;
  LCM_jump.s = js;
  LCM_itemset.t = it;
  LCM_Qtmp.t = qt;
}

/*************************************************************************/
/* LCM_freq main (lcm_closed-base type ) */
/*************************************************************************/
void LCMfreq (){
  int i;
  for ( i=0 ; i<LCM_Eend ; i++ ){
    LCMfreq_iter ( &LCM_Trsact, i );
    LCM_occ[i].t = LCM_occ[i].end = 0;
  }
  LCM_iters++;
}

/******************************************************************/
/* main */
/******************************************************************/
main ( int argc, char *argv[] ){
  int i, n;

  LCM_problem = 1;
  LCM_init ( argc, argv );
  LCMfreq ();
  LCM_output ( 1 );
  LCM_end ();
  ARY_end ( &LCM_Trsact );
  exit (0);
}

/******************************************************************/
/******************************************************************/

#endif


