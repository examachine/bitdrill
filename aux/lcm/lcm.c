/* Linear time Closed itemset Miner for Frequent Itemset Mining problems */
/* 2004/4/10 Takeaki Uno */
/* This program is available for only academic use.
   Neither commercial use, modification, nor re-distribution is allowed */

#ifndef _lcm_c_
#define _lcm_c_

#include<time.h>
#include"trsact.c"

/**************************************************************/
/* shrink T for LCM Ver.1 */
/* find the identical transactions from T by radix sort (bucket sort) 
  and merge them into one transaction, and make a list of consisting
   transactions of original database */
/* input: T:transactions, jump,q:working space(re-use for omiting initialization) */
/* set T->h[i].end to the multiplicity of transaction i
     (# of transactions merged to i) */
/*************************************************************************/
void LCM_shrink ( ARY *T, ARY *T_org, QUEUE *jump, long *p ){
  int ii, j, t, tt, v, vv, ttmax=T->num;
  QUEUE_INT *jt, *jtt, *jq=jump->q, *jqq=jump->q+jump->end+1;
  long *pp=&p[jump->end+1],*q=&p[jump->end*2+2],*qq=&p[jump->end*2+2+T->num*2];
  QUEUE *Q = T->h, *Q_org, *Qtmp;
  
  Q_org = T_org->h;
  malloc2 ( Qtmp, QUEUE, T->num, "LCM_shrink", "Qtmp"); 
  for ( t=0,jtt=jqq ; t<T->num ; t++ ){
    ii = Q[t].q[0];
    if ( pp[ii] == -1 ){ *jtt = ii; jtt++; }
    qq[t*2] = pp[ii];
    qq[t*2+1] = 0;
    pp[ii] = t;
  }
  
  for ( j=1 ; jtt>jqq ; j++ ){
    for ( jt=jq ; jtt>jqq ; ){
      jtt--;
      if ( *jtt == jump->end ) goto END2;
      t = pp[*jtt];
      pp[*jtt] = -1;
      v = -1;
      do{
        tt = qq[t*2];
        if ( v != qq[t*2+1] ){
          v = qq[t*2+1];
          vv = t;
          if ( tt<0 ) goto END2;
          if ( qq[tt*2+1] != v ) goto END1;
        }
        ii = Q[t].q[j];
        if ( p[ii] == -1 ){ *jt = ii; jt++; }
        q[t*2] = p[ii];
        p[ii] = t;
        q[t*2+1] = vv;
        END1:;
        t = tt;
      } while ( tt>=0 );
      END2:;
    }

    if ( (tt=pp[jump->end]) < 0 ) goto END3;
    pp[jump->end] = -1;
    while (1){
      t = tt;
      v = qq[t*2+1];
      while (1){
        tt = qq[tt*2];
        if ( tt < 0 ) goto END3;
        if ( qq[tt*2+1] != v ) break;
        Q[t].end += Q[tt].end;
        Q[tt].end = 0;
        ttmax--;
        Qtmp[ttmax].s = Q[tt].s;   /* remember original pointer */
        Qtmp[ttmax].end = Q[t].s;   /* remember original pointer of t */
               /* change pointer to concatinate */
        SWAP_INT ( Q_org[Q[tt].s].s, Q_org[Q[t].s].s );
        if ( Q_org[Q[t].s].t > Q_org[Q[tt].s].t ) Q[t].s = Q[tt].s;
      }
      END4:;
    }

    END3:;
    SWAP_PNT ( p, pp );
    SWAP_PNT ( q, qq );
    SWAP_PNT ( jq, jqq );
    SWAP_PNT ( jt, jtt );
  }

  for ( t=j=0 ; t<T->num ; t++ ){
    if ( Q[t].end > 0 ){
      if ( t!= j) Q[j] = Q[t];
      j++;
    }
  }
    /* remember shrinked transaction-lists in later-area */ 
  for ( ; ttmax<T->num ; ttmax++ ){
    Q[ttmax].s = Qtmp[ttmax].s;
    Q[ttmax].end = Qtmp[ttmax].end;
  }
  free ( Qtmp );
  T->num = j;
}


/**************************************************************/
/* shrink T for LCM ver 2 */
/* find the identical transactions from T by radix sort (bucket sort) 
  and merge them into one transaction, and compute the interior intersection */
/* input: T:transactions, jump,q:working space(re-use for omiting initialization) */
/* set T->h[i].end to the multiplicity of transaction i
     (# of transactions merged to i) */
/*************************************************************************/
void LCM_shrink_ ( ARY *T, QUEUE *jump, long *p, int max_item ){
  int ii, j, t, tt, v, vv;
  QUEUE_INT *jt, *jtt, *jq=jump->q, *jqq=jump->q+jump->end+1;
  long *pp=&p[jump->end+1],*q=&p[jump->end*2+2],*qq=&p[jump->end*2+2+T->num*2];
  QUEUE *Q = T->h;
  QUEUE_INT *x1, *xx, *x2;
  
  for ( t=0,jtt=jqq ; t<T->num ; t++ ){
    ii = Q[t].q[0];
    if ( pp[ii] == -1 ){ *jtt = ii; jtt++; }
    qq[t*2] = pp[ii];
    qq[t*2+1] = 0;
    pp[ii] = t;
  }

  for ( j=1 ; jtt>jqq ; j++ ){
    for ( jt=jq ; jtt>jqq ; ){
      jtt--;
      if ( *jtt == max_item ) goto END2;
      t = pp[*jtt];
      pp[*jtt] = -1;
      v = -1;
      do{
        tt = qq[t*2];
        if ( v != qq[t*2+1] ){
          v = qq[t*2+1];
          vv = t;
          if ( tt<0 ) goto END2;
          if ( qq[tt*2+1] != v ) goto END1;
        }
        ii = Q[t].q[j];
        if ( p[ii] == -1 ){ *jt = ii; jt++; }
        q[t*2] = p[ii];
        p[ii] = t;
        q[t*2+1] = vv;
        END1:;
        t = tt;
      } while ( tt>=0 );
      END2:;
    }

    if ( (tt=pp[max_item]) < 0 ) goto END3;
    pp[max_item] = -1;
    while (1){
      t = tt;
      v = qq[t*2+1];
      while (1){
        tt = qq[tt*2];
        if ( tt < 0 ) goto END3;
        if ( qq[tt*2+1] != v ) break;
        Q[t].end += Q[tt].end;
        Q[tt].end = 0;

          /* intersection of items>max_item */
        for ( xx=x1=(Q[t].q+j),x2=Q[tt].q+j ; *x1<jump->end ; x1++ ){
          while ( *x2<*x1 ) x2++;
          if ( *x2 == *x1 ){
            if ( x1!=xx ) *xx=*x1;
            xx++;
            x2++;
          }
        }
        *xx = jump->end;
        Q[t].t = xx - Q[t].q;
      }
      END4:;
    }

    END3:;
    SWAP_PNT ( p, pp );
    SWAP_PNT ( q, qq );
    SWAP_PNT ( jq, jqq );
    SWAP_PNT ( jt, jtt );
  }

  for ( t=j=0 ; t<T->num ; t++ ){
    if ( Q[t].end > 0 ){
      if ( t!= j) Q[j] = Q[t];
      j++;
    }
  }
  T->num = j;
}

/*************************************************************************/
/* recover concatinated transaction-lists when they were shrinked */
/* input T:merged transaction, T_org, original transaction, ttmax, #of deleted transactions */
/*************************************************************************/
void LCM_shrink_recov ( ARY *T, ARY *T_org, int ttmax ){
  int i;
  QUEUE *Q=T->h, *Q_org = T_org->h;
  for ( i=T->num ; i<ttmax ; i++ ){
    SWAP_INT ( Q_org[Q[i].s].s, Q_org[Q[i].end].s );
               /* change pointer to recover */
  }
}


/*************************************************************************/
/* set TT to the transactions of O with frequency in the given
  upper and lower bounds for LCM ver. 1 */
/* return #all freq items */
/*************************************************************************/
int LCM_mk_freq_trsact_( ARY *TT, ARY *T, ARY *O, QUEUE *occ, int cnt,
                          QUEUE_INT max_item, int th, int frq ){
  QUEUE_INT *x, *buf, *bbuf, *b;
  long *t, *tt=((long *)(occ->q))+occ->t;
  int i, n=0, mm=O->num+1;
  QUEUE *Q=T->h, *QQ, *OQ=O->h;

  ARY_init ( TT, sizeof(QUEUE) );
  ARY_exp (TT, occ->t-1 );
  QQ = TT->h;
  malloc2 ( buf, QUEUE_INT, cnt, "TRSACT_mk_freq_trsact", "buf");
  bbuf = buf;
  for ( t=(long *)(occ->q) ; t<tt ; t++ ){
    QQ[n].q = b = buf;
    for ( x=Q[*t].q ; *x<=max_item ; x++ ){
      if ( OQ[*x].end > 0 ){ *buf = *x; buf++; }
    }
    if ( buf != b ){
      QQ[n].t = buf-b;
      QQ[n].end = Q[*t].end;   /* multiplicity of transaction t */
      QQ[n].s = Q[*t].s;   /* top of shrinked transaction list */
      *buf = O->num;
      buf++;
      n++;
    }
  }
  TT->num = n;
  return ( buf-bbuf );
}

/***************************************************************/
/* Check ppc condition */
/* return 1 if (closure(itemset) = itemset) respect to item+1,...,Eend */
/***************************************************************/
int LCM_closure ( ARY *T, QUEUE *occ, int min_item ){
  int i, ii, t, flag=-1, y, y0, c=0, *z=(int *)LCM_Tjump.q, *tx, *txx;
  QUEUE_INT *x;
  QUEUE *Q = T->h, *QQ=LCM_Trsact.h, *tQ;
  unsigned int cmax=0, cc, smin = LCM_Eend;

  y0 = Q[((int *)(occ->q))[occ->t-1]].s;
  if ( QQ[y0].t > QQ[QQ[y0].s].t ) y0 = QQ[y0].s;
  y = Q[((int *)(occ->q))[0]].s;
  if ( QQ[y0].t > QQ[y].t ) y0 = y;
  if ( QQ[y0].t > QQ[QQ[y].s].t ) y0 = QQ[y].s;

  QUEUE_RMALL_ ( LCM_Tjump );
  for ( x=QQ[y0].q+QQ[y0].t-1 ; *x>=min_item ; x-- ){
    if ( LCM_ary[*x] < 0 ) continue;

    for ( t=0 ; t<occ->t ; t++ ){
      i = Q[((int *)(occ->q))[t]].s;
      c1++;
      if ( LCM_adjmat[i].a ){
        if ( !BARRAY_BIT(LCM_adjmat[i],*x) ) goto END1;
      } else {
        if ( LCM_Tmark[i] == QQ[i].q+QQ[i].t-1 && *(LCM_Tmark[i]) > *x ){
          LCM_Tmark[i]--;
          z[LCM_Tjump.t++] = i;
        }
        while ( *(LCM_Tmark[i]) > *x ) LCM_Tmark[i]--;
        if ( *(LCM_Tmark[i]) < *x ) goto END1;
      }
    }
    if ( occ->t == occ->end ){ flag = *x; c1++; goto END2; }
    for ( t=0 ; t<occ->t ; t++ ){
      ii = Q[((int *)(occ->q))[t]].s;
      for ( i=QQ[ii].s ; i!=ii ; i=QQ[i].s ){
        if ( LCM_adjmat[i].a ){
          if ( !BARRAY_BIT(LCM_adjmat[i],*x) ) goto END1;
        } else {
          if ( LCM_Tmark[i] == QQ[i].q+QQ[i].t-1 && *(LCM_Tmark[i]) > *x ){
            LCM_Tmark[i]--;
            z[LCM_Tjump.t++] = i;
          }
          while ( *(LCM_Tmark[i]) > *x ) LCM_Tmark[i]--;
          if ( *(LCM_Tmark[i]) < *x ) goto END1;
        }
      }
    }
    flag = *x;
    goto END2;
    END1:;
  }
  END2:;
  QUEUE_F_LOOP_ ( LCM_Tjump, i ) LCM_Tmark[z[i]] = QQ[z[i]].q+QQ[z[i]].t-1;
  return ( flag );
}

/***************************************************************/
/* iteration of LCM ver. 1 + anytime database reduction for frequency counting
 (without interior intersection) */
/* INPUT: T:transactions(database), item:tail of the current solution */
/*************************************************************************/
int LCMclosed_iter( ARY *T, int item ){
  ARY TT;
  int i, n, js=LCM_jump.s, itemt=LCM_itemset.t;
  int qt = LCM_Qtmp.t, ii, tt;
  QUEUE_INT *q;
  
  LCM_jump.s = LCM_jump.t;
  LCM_iters++;
  QUEUE_ins_ ( &LCM_itemset, item );
  LCM_ary[item] = -1;

  n = LCM_freq_calc( T, &LCM_occ[item], &LCM_Occ, &LCM_jump, item-1 );
  LCM_frq = LCM_occ[item].end;
  LCM_jump_rm_infreq( &LCM_Occ,&LCM_jump,LCM_th, LCM_frq, &LCM_itemset);
  LCM_print_solution ();
  if ( QUEUE_LENGTH(LCM_jump) == 0 ) goto END2;

  QUEUE_sortr_ ( &LCM_jump );
  for ( i=itemt+1 ; i<LCM_itemset.t ; i++ ) LCM_ary[LCM_itemset.q[i]] = -1;
  LCM_mk_freq_trsact_(&TT,T,&LCM_Occ,&LCM_occ[item],n+LCM_occ[item].t,LCM_jump.q[LCM_jump.s],LCM_th,LCM_frq);
//  LCM_mk_freq_trsact_( &TT, T, &LCM_Occ, &LCM_occ[item], n+LCM_occ[item].t,item-1, LCM_th, LCM_frq );
  q = ((QUEUE *)(TT.h))->q;
  TT.end = TT.num;
  if ( QUEUE_LENGTH(LCM_jump) >= 2 && TT.num>7 )
      LCM_shrink ( &TT, &LCM_Trsact, &LCM_shrink_jump, LCM_shrink_p );
  TRSACT_occ_deliver_all( &TT, &LCM_Occ, item-1 );

  do {
    i = QUEUE_ext_tail_ ( &LCM_jump );
    LCMclosed_pruning_del ( qt, i );
    if ( LCM_Qtmp.q[LCM_Eend+i] == -1){
      if ( (ii=LCM_closure(&TT, &LCM_occ[i], i+1)) == -1){
        LCMclosed_iter ( &TT, i);
      } else {
        LCMclosed_pruning_set ( i, ii, qt );
      }
    }
    LCM_occ[i].t = LCM_occ[i].end = 0;
  } while ( QUEUE_LENGTH(LCM_jump) > 0 );
  LCMclosed_pruning_del ( qt, LCM_Eend );

  LCM_shrink_recov ( &TT, &LCM_Trsact, TT.end );
  free2 ( q );
  ARY_end ( &TT );
  END2:;
  for ( i=itemt ; i<LCM_itemset.t ; i++ ) LCM_ary[LCM_itemset.q[i]] = 0;
  LCM_itemset.t = itemt;
  LCM_jump.s = js;
  return ( -1 );
}

/***************************************************************/
/* main of LCM ver. 1 + anytime database reduction for frequency counting
 (without interior intersection) */
/*************************************************************************/
void LCMclosed (){
  int i;
  QUEUE *QQ = LCM_Trsact.h;
  malloc2 ( LCM_Tmark, QUEUE_INT*, LCM_Trsact.num, "LCM_init", "LCM_Tmark" );
  for ( i=0 ; i<LCM_Trsact.num ; i++ ) LCM_Tmark[i] = QQ[i].q+QQ[i].t-1;
  QUEUE_init ( &LCM_Tjump, LCM_Trsact.num*2 );
  LCM_adjmat = TRSACT_sparse_adjmat (
         &LCM_Trsact, LCM_Eend, LCM_ratio, &LCM_adjmat_full );

  for ( i=0 ; i<LCM_Eend ; i++ ){
    if ( i==LCM_Eend-1 || LCM_occ[i].end>LCM_occ[i+1].end || 
      LCM_closure ( &LCM_Trsact, &LCM_occ[i], i+1 )==-1 )
        LCMclosed_iter ( &LCM_Trsact, i );
    LCM_occ[i].t = LCM_occ[i].end = 0;
  }
  LCM_iters++;
  for ( i=0 ; i<LCM_Trsact.num ; i++ ) BARRAY_end ( &LCM_adjmat[i] );

  QUEUE_end ( &LCM_Tjump );
  free ( LCM_Tmark );
}



/***************************************************************/
/* iteration of LCM ver. 2 */
/* INPUT: T:transactions(database), item:tail of the current solution */
/*************************************************************************/
int LCMclosed_iter_( ARY *T, int item, int prv ){
  ARY TT;
  QUEUE *oc=&LCM_occ[item];
  int i, ii, e, ee, n, js=LCM_jump.s, itemt=LCM_itemset.t;
  int flag=-1, tt, qt=LCM_Qtmp.t;
  QUEUE_INT *q;

  LCM_jump.s = LCM_jump.t;
  LCM_iters++;
  QUEUE_ins_ ( &LCM_itemset, item );
  LCM_frq = oc->s;

  n = LCM_freq_calc ( T, oc, &LCM_Occ, &LCM_jump, LCM_Eend-1 );
  if ( LCM_itemset.t >= 2 ) LCM_occ[prv].end = 0;
  oc->end = 0;
  ii = LCM_jump_rm_infreq ( &LCM_Occ, &LCM_jump, LCM_th, LCM_frq, &LCM_itemset );
  if ( ii > item ){ flag = ii; goto END2; }  /* itemset is not closed */
  LCM_print_solution ();

  if ( QUEUE_LENGTH(LCM_jump)==0 ) goto END2;   /* no rec.call */
  ii=0,ee=-1; QUEUE_FE_LOOP_ ( LCM_jump, i, e ){
    if ( e<item ){
      ii++;
      if ( e>ee ) ee = e;
    }
  }
  if ( ii==0 ) goto END2; /* no rec.call */

  oc->end = LCM_th;
  LCM_mk_freq_trsact(&TT, T, &LCM_Occ, oc, n+LCM_occ[item].t, ee, LCM_Eend-1, LCM_th, LCM_frq);
  q = ((QUEUE *)(TT.h))->q;
  if ( ii >= 2 && TT.num>5 )
      LCM_shrink_ ( &TT, &LCM_shrink_jump, LCM_shrink_p, item );
  TRSACT_occ_deliver_all( &TT, &LCM_Occ, item-1 );
  LCM_remove_large_items ( &LCM_jump, item, LCM_occ );
  QUEUE_sortr_ ( &LCM_jump );

  do {
    i = QUEUE_ext_tail_ ( &LCM_jump );
    LCMclosed_pruning_del ( qt, i );
    if ( LCM_Qtmp.q[LCM_Eend+i] == -1 ){
      ii = LCMclosed_iter_ ( &TT, i, item);
      LCMclosed_pruning_set ( i, ii, qt );
    }
    LCM_occ[i].t = LCM_occ[i].end = 0;
  } while ( LCM_jump.t > LCM_jump.s );
  LCMclosed_pruning_del ( qt, LCM_Eend );

  free2 ( q );
  ARY_end ( &TT );
  goto END3;
  END2:;
  while ( QUEUE_LENGTH(LCM_jump)>0 ){
    LCM_jump.t--;
    LCM_occ[LCM_jump.q[LCM_jump.t]].end = 0;
  }
  END3:;
  LCM_itemset.t = itemt;
  LCM_jump.s = js;
  return ( flag );
}

/***************************************************************/
/* main of LCM ver. 2 */
/*************************************************************************/
void LCMclosed_ (){
  int i;
  for ( i=0 ; i<LCM_Eend ; i++ ){
    LCM_occ[i].s = LCM_occ[i].end;
    LCM_occ[i].end = 0;
  }
  for ( i=0 ; i<LCM_Eend ; i++ ){
    LCMclosed_iter_ ( &LCM_Trsact, i, -1 );
    LCM_occ[i].t = LCM_occ[i].end = 0;
  }
  LCM_iters++;
}

/*************************************************************************/
/*************************************************************************/
main ( int argc, char *argv[] ){
  int i;
  LCM_init ( argc, argv );
  if ( LCM_th >= 20 ) LCMclosed_ (); else LCMclosed ();
  LCM_output ( 1 );
  LCM_end ();
  ARY_end ( &LCM_Trsact );
  exit (0);
}


#endif


