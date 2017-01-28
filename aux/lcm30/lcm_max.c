/* Linear time Closed itemset Miner for Frequent Itemset Mining problems */
/* 2004/4/10 Takeaki Uno */
/* This program is available for only academic use.
   Neither commercial use, modification, nor re-distribution is allowed */

#ifndef _lcm_max_c_
#define _lcm_max_c_

#include<time.h>
#include"trsact.c"

QUEUE LCM_itemset2;

/***********************************/
/*   print weighted transactions   */
/***********************************/
void TRSACT_print_weight ( ARY *T ){
  int i, j;
  QUEUE *Q = T->h;
  for ( i=0 ; i<T->num ; i++ ){
    for ( j=0 ; j<Q[i].t ; j++ ){
      printf ("[%d %d],", Q[i].q[j*2], Q[i].q[j*2+1]);
    }
    printf (" (%d)\n", Q[i].end );
  }
}


/*************************************************************************/
/* compute the frequency by occurrence deliver , for LCMmax */
/* INPUT: T:transactions, occ:occurences, max_item.
   OUTPUT: maximum frequent item
   OPERATION:
   1. for each item i<=max_item, 
        set O->h[i] to the list of transactions in occ including i,
   2. set jump to the list of items i s.t. at least one occurrence of occ 
      includes i. 
   3. set occ->end to the frequency of occ
        (sum of multiplicity of transactions in occ).    */
/*************************************************************************/
int LCMmax_freq_calc( ARY *T, QUEUE *occ, ARY *O, QUEUE *jump, QUEUE_INT max_item ){
  QUEUE_INT *x, *xx, *jq=&(jump->q[jump->t]);
  int i, ee, n=0;
  long *t, *tt=((long *)(occ->q))+occ->t;
  QUEUE *Q = T->h, *QQ = O->h;
  for ( t=(long *)(occ->q) ; t<tt ; t++ ){
    x = Q[*t].q;
    for ( ; *x<max_item ; x+=2 ){
      if ( QQ[*x].end == 0 ){ *jq=*x; jq++; }
      QQ[*x].end += *(x+1); n++;
    }
  }
  jump->t = jq-jump->q;
  return ( n );
}


/*******************************************************/
/*       Occurrence Deliver for LCMmax                 */
/* input: T:transactions, max_item  */
/* for each item e<max_item, set O->h[e].q to the transactions including e,
 and set O->h[e].t to the # of such transactions */
/*******************************************************/
void LCMmax_occ_deliver_all( ARY *T, ARY *O, QUEUE_INT max_item){
  int t;
  QUEUE_INT *x;
  QUEUE *Q = T->h, *QQ = O->h;
  for ( t=0 ; t<T->num ; t++ ){
    for ( x=Q[t].q ; *x<=max_item ; x+=2 ){
      ((long *)QQ[*x].q)[QQ[*x].t++] = t;
    }
  }
}


/**************************************************************/
/* shrink T for LCMmax */
/* find the identical transactions from T by radix sort (bucket sort) 
  and merge them into one transaction, and sum up the weights of 
  each item, by occurrence deliver like algorithm */
/* input: T:transactions, jump,q:working space(re-use for omiting initialization) */
/* set T->h[i].end to the multiplicity of transaction i
     (# of transactions merged to i) */
/*************************************************************************/
QUEUE_INT *LCMmax_shrink(ARY *T, QUEUE *jump, long *p, int max_item, int cnt){
  int ii, j, t, tt, v, vv, jj, flag;
  QUEUE_INT *jt, *jtt, *jq=jump->q, *jqq=jump->q+jump->end+1;
  long *pp=&p[jump->end+1],*q=&p[jump->end*2+2],*qq=&p[jump->end*2+2+T->num*2];
  QUEUE *Q = T->h;
  QUEUE_INT *buf, *b, *bbuf, *buf_org=Q->q, *x1, *xx, *x2, *s1, *s2;
  
     /* intialization */
  for ( t=0,jtt=jqq ; t<T->num ; t++ ){
    ii = Q[t].q[0];
    if ( pp[ii] == -1 ){ *jtt = ii; jtt++; }
    qq[t*2] = pp[ii];
    qq[t*2+1] = 0;
    pp[ii] = t;
  }
  malloc2 ( buf, QUEUE_INT, cnt, "LCMmax_shrink");
  b = buf;

    /* radix sort */
  for ( j=2 ; jtt>jqq ; j+=2 ){
    for ( jt=jq ; jtt>jqq ; ){
      jtt--;
      if ( *jtt == max_item ) continue;
      t = pp[*jtt];
      pp[*jtt] = -1;
      v = -1;

      do {
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
      flag = 0;
      s1 = buf;
      s2 = Q[t].q;
      flag = 0;
      while (1){
        tt = qq[tt*2];
        if ( tt < 0 ) break;
        if ( qq[tt*2+1] != v ) break;
        Q[t].end += Q[tt].end;
        Q[tt].end = 0;

          /* union of items>max_item */
        xx = s1;
        x1 = s2;
        x2 = Q[tt].q;
        while (1) {
          while ( *x2<*x1 ){
            *xx = *x2;
            *(xx+1) = *(x2+1);
            xx += 2;
            x2 += 2;
          }
          if ( *x1 >= jump->end ) break;
          *xx = *x1;
          if ( *x2==*x1 ){
            *(xx+1) = *(x1+1) + *(x2+1);
            x2 += 2;
          } else *(xx+1) = *(x1+1);
          xx += 2;
          x1 += 2;
        }
        Q[t].t = (xx-s1)/2;
        *xx = jump->end;
        if ( flag ){
          SWAP_PNT ( s1, s2 );
        } else {
          s2 = buf;
          s1 = LCM_ary;
        }
        flag = 1;
      }
      if ( flag ){
        Q[t].q = buf;
        if ( s1 == buf )
            memcpy ( buf, LCM_ary, (Q[t].t*2+1)*sizeof(QUEUE_INT) );
        buf += (Q[t].t*2+1);
        Q[t].t += jump->end;
      }
      if ( tt < 0 ) break;
    }

    END3:;
    SWAP_PNT ( p, pp );
    SWAP_PNT ( q, qq );
    SWAP_PNT ( jq, jqq );
    SWAP_PNT ( jt, jtt );
  }

    /* remove deleted transactions */
  for ( t=j=0 ; t<T->num ; t++ ){
    if ( Q[t].end > 0 ){
      if ( Q[t].t <= jump->end ){
        for ( x1=Q[t].q,x2=Q[t].q+Q[t].t*2,Q[t].q=buf ; x1<x2 ; x1+=2 ){
          *buf = *x1;
          *(buf+1) = *(x1+1);
          buf+=2;
        }
        *buf = jump->end;
        buf++;
      } else Q[t].t -= jump->end;

      if ( t!= j) Q[j] = Q[t];
      j++;
    }
  }
  T->num = j;
  free ( buf_org );
  return ( b );
}

/*************************************************************************/
/* construct a weighted transaction from a transaction */
/* INPUT: T:transactions */
/* set T to the weighted transactions */
/* the original transactions remain in memory!! has to free them */
/*************************************************************************/
void LCMmax_mk_weighted_trsact( ARY *T, int cnt ){
  QUEUE_INT *buf, *b, *q;
  int i, j;
  QUEUE *Q=T->h;
  
  malloc2 ( buf, QUEUE_INT, cnt, "LCMmax_mk_weighted_trsact");
  q = Q->q;
  for ( i=0 ; i<T->num ; i++ ){
    b = buf;
    for ( j=0 ; j<Q[i].t ; j++ ){
      *buf = Q[i].q[j];
      buf++;
      *buf = Q[i].end;
      buf++;
    }
    *buf = Q[i].q[j];
    buf++;
    Q[i].q = b;
  }
}

/*************************************************************************/
/* set TT to the transactions of occ with frequency in the given
  upper and lower bounds, for LCMmax*/
/* return #all freq items */
/*************************************************************************/
int LCMmax_mk_freq_trsact( ARY *TT, ARY *T, ARY *O, QUEUE *occ, int cnt,
                          QUEUE_INT max_item, int th, int frq ){
  QUEUE_INT *x, *buf, *bbuf, *b;
  long *t, *tt=((long *)(occ->q))+occ->t;
  int i, n=0;
  QUEUE *Q=T->h, *QQ, *OQ=O->h;

  ARY_init ( TT, sizeof(QUEUE) );
  ARY_exp ( TT, occ->t-1 );
  QQ = TT->h;
  malloc2 ( buf, QUEUE_INT, cnt, "LCMmax_mk_freq_trsact" );
  bbuf = buf;
  for ( t=(long *)(occ->q) ; t<tt ; t++ ){
    QQ[n].q = b = buf;
    if ( Q[*t].q[0] > max_item ) continue;
    for ( x=Q[*t].q ; *x<LCM_Eend ; x+=2 ){
      if ( OQ[*x].end>0 ){
        *buf = *x;
        buf++;
        *buf = *(x+1);
        buf++;
      }
    }
    if ( buf != b ){
      QQ[n].t = (buf-b)/2;
      QQ[n].end  = Q[*t].end;   /* multiplicity of transaction t */
      *buf = LCM_Eend;
      buf++;
      n++;
    }
  }
  TT->num = n;
  return ( buf-bbuf );
}

/*************************************************************************/
/* set TT to the transactions of O with frequency in the given
  upper and lower bounds */
/* return #all freq items */
/*************************************************************************/
int LCMmax_mk_freq_weighted_trsact( ARY *TT, ARY *T, ARY *O, QUEUE *occ,
               int cnt, QUEUE_INT max_item, int th, int frq ){
  QUEUE_INT *x, *buf, *bbuf, *b;
  long *t, *tt=((long *)(occ->q))+occ->t;
  int i, n=0, e;
  QUEUE *Q=T->h, *QQ, *OQ=O->h;

  ARY_init ( TT, sizeof(QUEUE) );
  ARY_exp ( TT, occ->t-1 );
  QQ = TT->h;
  malloc2 ( buf, QUEUE_INT, cnt, "LCMmax_mk_freq_trsact" );
  bbuf = buf;
  for ( t=(long *)(occ->q) ; t<tt ; t++ ){
    QQ[n].q = b = buf;
    if ( Q[*t].q[0] > max_item ) continue;
    e = Q[*t].end;
    for ( x=Q[*t].q ; *x<LCM_Eend ; x++ ){
      if ( OQ[*x].end>0 ){
        *buf = *x;
        buf++;
        *buf = e;
        buf++;
      }
    }
    if ( buf != b ){
      QQ[n].t = (buf-b)/2;
      QQ[n].end  = e;   /* multiplicity of transaction t */
      *buf = LCM_Eend;
      buf++;
      n++;
    }
  }
  TT->num = n;
  return ( buf-bbuf );
}

/*************************************************************************/
/* re-allocate memory for denotations when the indices of items are re-ordered */
/*************************************************************************/
int *LCMmax_alloc_occ(ARY *T, ARY *O, ARY *OO, int cnt, QUEUE *jump, int *pm, int item){
  int i, ii=0, *perm;
  QUEUE_INT e;
  QUEUE *Q, *QQ;
  ARY_init ( OO, sizeof(QUEUE) );
  ARY_exp ( OO, jump->t-jump->s-1 );
  malloc2 ( perm, int, (jump->t-jump->s)*2, "LCMmax_alloc_occ");
  QQ = OO->h; Q = O->h;
  QUEUE_BE_LOOP_ ( *jump, i, e ){
    QQ[ii] = Q[e];
    LCM_ary[e] = ii;
    perm[ii] = pm[e];
    perm[(jump->t-jump->s)+ii] = e;
    ii++;
  }
  return ( perm );
}

/*************************************************************************/
/* replace the indices of items in the transactios, and occurence deliver */
/*************************************************************************/
void LCMmax_replace_index_occurrence_deliver(ARY *T,ARY *O,QUEUE_INT max_item){
  int t;
  QUEUE_INT *x;
  QUEUE *Q = T->h, *QQ = O->h;
  for ( t=0 ; t<T->num ; t++ ){
    for ( x=Q[t].q ; *x<LCM_Eend; x+=2 ){
      *x = LCM_ary[*x];
      if ( *x<max_item ) ((long *)QQ[*x].q)[QQ[*x].t++] = t;
    }
    qsort_int2 ( Q[t].q, Q[t].t );
  }
}

/*************************************************************************/
/* remove large items from jump and sort in increasing order
     INPUT: jump, max_item
     OPERATION: 1. for each item i>max_item in jump, remove i from jump
     2. set occ[e].s = occ[e].end for each i<max_item (preserve frequency)
     3. set occ[i].end = 0 for each i in jump
     4. sort items in jump (from jump.s to jump.t) in the increasing order */
/*************************************************************************/
void LCMmax_remove_large_items(QUEUE *jump, int max_item, QUEUE *o,QUEUE *occ){
  QUEUE_INT e, ee;
  int i, ii=jump->s;
  QUEUE_FE_LOOP_ ( *jump, i, e ){
    ee = LCM_ary[e];
    if ( e<max_item ){
      occ[ee].s = occ[ee].end;
      if ( ii != i ) jump->q[ii] = ee;
      ii++;
    }
    o[e].end = occ[ee].end = 0;
  }
  jump->t = ii;
}

void LCMmax_remove_large_items_(QUEUE *jump, int max_item, QUEUE *occ){
  QUEUE_INT e, ee;
  int i, ii=jump->s;
  QUEUE_FE_LOOP_ ( *jump, i, e ){
    if ( e<max_item ){
      occ[e].s = occ[e].end;
      if ( ii != i ) jump->q[ii] = e;
      ii++;
    }
    occ[e].end = 0;
  }
  jump->t = ii;
}



/*************************************************************************/
/* lcm_max main iteration */
/*************************************************************************/
int LCMmax_iter(ARY *T,ARY *O,int item, int *pm, int *pm2, int prv, QUEUE *jump){
  ARY TT, OO;
  QUEUE *occ = O->h, *oocc, jump2;
  int i, ii, im, e, ee, n, nn, addtail=LCM_add.t, js=LCM_jump.s, *perm;
  int flag=-1, flag2, cc=0, itemt, itemt2, itemt22, ff=0;
  QUEUE_INT *q;

  LCM_jump.s = LCM_jump.t;
  LCM_iters++;
  QUEUE_ins_ ( &LCM_itemset, pm[item] );
  QUEUE_ins_ ( &LCM_itemset2, pm2? pm2[item]: item );
  LCM_frq = occ[item].s;
  
  n = LCMmax_freq_calc( T, &occ[item], O, jump, LCM_Eend );
  if ( prv >= 0 ) occ[prv].end = 0;
  occ[item].end = 0;
  ii = LCM_jump_rm_infreq ( O, jump, LCM_th, LCM_frq, &LCM_add);
  for ( i=addtail ; i<LCM_add.t ; i++ ){
    e = LCM_add.q[i];
    if ( e<item ){
      QUEUE_ins_ ( &LCM_itemset2, pm2? pm2[e]: e );
    }
    LCM_add.q[i] = pm[e];
  }
  itemt = LCM_itemset2.t;
  if ( ii > item ){ flag=ii; goto END2; }  /* itemset is not closed */

  if ( QUEUE_LENGTH(*jump)==0 ){   /* no freq. item => maximal  */
    LCM_print_solution_ ( &LCM_add );
    goto END2;
  }
  ii=0; ee=-1; QUEUE_FE_LOOP_(*jump, i, e){ /* find largest freq.item < item */
    if ( e < item ){
      if ( ee < e ) ee=e;
      ii++;
    }
  }
  if ( ii==0 ) goto END2; /* no rec.call */

/******************************************************/

  int s, t, tt, l = QUEUE_LENGTH(*jump);
  QUEUE_INT *x, *itemset2;
  
  QUEUE_init ( &jump2, (jump->t-jump->s)*2+2 );
  occ[item].end = LCM_th;
  nn = LCMmax_mk_freq_trsact(&TT, T, O, &occ[item], n*2+occ[item].t, ee, LCM_th, LCM_frq);
  occ[item].end = 0;
  q = ((QUEUE *)(TT.h))->q;
  if ( ii >= 2 && TT.num>7 )
    q = LCMmax_shrink ( &TT, &LCM_shrink_jump, LCM_shrink_p, item, nn );

  if ( ii<4 ){
    LCMmax_occ_deliver_all( &TT, O, item-1 );
    LCMmax_remove_large_items_ ( jump, item, occ );
    QUEUE_sortr_ ( jump );
    do {
      i = QUEUE_ext_tail_ ( jump );
      LCM_itemset2.t = itemt;
      LCMmax_iter ( &TT, O, i, pm, NULL, item, &jump2 );
      occ[i].t = occ[i].end = 0;
    } while ( jump->t > jump->s );
    goto END1;
  }

  occ[ee].s = occ[ee].end;
  for ( i=0 ; i<TT.num ; i++ ){
    for ( x=((QUEUE *)(TT.h))[i].q ; *x<ee ; x+=2 );
    if ( *x == ee ) occ[ee].q[occ[ee].t++] = i;
  }
  QUEUE_FE_LOOP_ ( *jump, i, e ){
    jump->q[i+jump->t] = occ[e].end;
    occ[e].end = 0;
  }
  LCMmax_iter ( &TT, O, ee, pm, NULL, item, &jump2 );
  occ[ee].t = 0;

  QUEUE_FE_LOOP_ ( *jump, i, e ) occ[e].end = jump->q[i+jump->t];
  itemt22 = itemt2 = LCM_itemset2.t;
  malloc2(itemset2, QUEUE_INT, ii+1,"LCMmax_iter","itemset2__");
  memcpy(itemset2,LCM_itemset2.q+itemt,(itemt2-itemt)*sizeof(QUEUE_INT));
  qsort_int_ ( itemset2, itemt2-itemt );
  itemset2[itemt2-itemt] = -1;
  jump->q[jump->t++] = item;

  if ( itemt22-itemt-1 > 0 ){
    t = jump->s; tt=0;
    QUEUE_FE_LOOP_ ( *jump, i, e ){
      if ( e<item ) LCM_ary[tt++] = e;
      else jump->q[t++] = e;
    }
    ii = t+1;
    qsort_int_ ( LCM_ary, tt );
    for ( s=1,i=0 ; i<tt ; i++ ){
      while ( itemset2[s] > LCM_ary[i] ) s++; 
      if ( itemset2[s] != LCM_ary[i] ) jump->q[t++]=LCM_ary[i];
    }
    memcpy(jump->q+t, itemset2+1, (itemt2-itemt-1)*sizeof(QUEUE_INT) );
    for ( i=0; i+ii<t ; i++ ){
      e = jump->q[i+ii];
      LCM_ary[i*2] = occ[e].end;
      LCM_ary[i*2+1] = e;
    }
    qsort_int2 ( LCM_ary, i );
    for ( i=0 ; i+ii<t ; i++ ) jump->q[i+ii] = LCM_ary[i*2+1];
  } else {
    QUEUE_sortr_ ( jump );
    QUEUE_FE_LOOP_ ( *jump, i, e ) if ( e<ee ) break;
    if ( i<jump->t ){
      for ( ii=i ; ii<jump->t ; ii++ ){
        e = jump->q[ii];
        LCM_ary[ii*2] = occ[e].end;
        LCM_ary[ii*2+1] = e;
      }
      qsort_int2 ( LCM_ary+(i*2), ii-i );
      for ( ii=i ; ii<jump->t ; ii++ ) jump->q[ii] = LCM_ary[ii*2+1];
    }
  }

  tt = jump->t - jump->s;
  perm = LCMmax_alloc_occ ( &TT, O, &OO, n, jump, pm, item );
  oocc = (QUEUE *)(OO.h);
  for ( i=0 ; i<OO.num ; i++ ) oocc[i].t = 0;
  LCMmax_replace_index_occurrence_deliver( &TT, &OO, LCM_ary[item] );
  LCMmax_remove_large_items ( jump, item, occ, oocc );
  prv = LCM_ary[item];

  ff = 0;
  for ( i=0 ; i<prv-1 ; i++ ){
    LCM_itemset2.t = itemt;
    if ( i>=itemt2-itemt-1 ){
      LCMmax_iter(&TT, &OO, i, perm, &perm[tt], prv, &jump2);
      if ( LCM_itemset2.t>itemt22 || (LCM_itemset2.t==itemt2 && ff==0) ){
        ff = 1;
        itemt2 = LCM_itemset2.t;
        memcpy(itemset2,LCM_itemset2.q+itemt,(itemt2-itemt)*sizeof(QUEUE_INT));
        qsort_int_ ( itemset2, itemt2-itemt );
      }
    }
    oocc[i].t = oocc[i].end = 0;
  }

  free2 ( perm );
  ARY_end ( &OO );
  LCM_itemset2.t = itemt2;
  if ( pm2 ){
    for ( i=0 ; i+itemt<itemt2 ; i++ ){
      LCM_itemset2.q[i+itemt] = pm2[itemset2[i]];
    }
  } else {
    memcpy ( LCM_itemset2.q+itemt, itemset2, (itemt2-itemt)*sizeof(QUEUE_INT));
  }
  free2 ( itemset2 );
  goto END11;
  END1:;
  if ( pm2 ){
    for ( i=0 ; i+itemt<LCM_itemset2.t ; i++ ){
      LCM_itemset2.q[i+itemt] = pm2[LCM_itemset2.q[i+itemt]];
    }
  }
  END11:;
  QUEUE_end ( &jump2 );
  free2 ( q );
  ARY_end ( &TT );
  goto END3;
  END2:;
  while ( jump->t>jump->s ){
    ii = jump->q[--jump->t];
    occ[ii].end = 0;
  }
  END3:;
  LCM_itemset.t--;
  LCM_add.t = addtail;
  LCM_jump.s = js;
  jump->s = jump->t = 0;
  return ( flag );
}

/*************************************************************************/
/* lcm_max main iteration (without weight -> second level recursions ) */
/* this subroutine is for reducing the memory space by not constructing 
 unnecessary weighted transactions for iterations of the upper level */
/*************************************************************************/
int LCMmax_first_iter( int item ){
  ARY TT, OO, *T = &LCM_Trsact;
  QUEUE *occ = LCM_occ, *oocc, jump2;
  int i, ii, im, e, ee, n, addtail=LCM_add.t, js=LCM_jump.s, *perm;
  int flag=-1, flag2, cc=0, itemt, itemt2, itemt22, ff=0;
  QUEUE_INT *q, *qq;

  LCM_jump.s = LCM_jump.t;
  LCM_iters++;
  QUEUE_ins_ ( &LCM_itemset, LCM_perm[item] );
  QUEUE_ins_ ( &LCM_itemset2, item );
  LCM_frq = occ[item].s;

  n=LCM_freq_calc( T, &LCM_occ[item], &LCM_Occ, &LCM_jump, LCM_Eend-1 );
  occ[item].end = 0;
  ii = LCM_jump_rm_infreq ( &LCM_Occ, &LCM_jump, LCM_th, LCM_frq, &LCM_add);
  for ( i=addtail ; i<LCM_add.t ; i++ ){
    e = LCM_add.q[i];
    if ( e<item ) QUEUE_ins_ ( &LCM_itemset2, e );
    LCM_add.q[i] = LCM_perm[e];
  }
  itemt = LCM_itemset2.t;
  if ( ii > item ){ flag=ii; goto END2; }  /* itemset is not closed */

  if ( QUEUE_LENGTH(LCM_jump)==0 ){   /* no freq. item => maximal  */
    LCM_print_solution_ ( &LCM_add );
    goto END2;
  }
  ii=0; ee=-1; QUEUE_FE_LOOP_(LCM_jump, i, e){ /* find largest freq.item < item */
    if ( e < item ){
      if ( ee < e ) ee=e;
      ii++;
    }
  }
  if ( ii==0 ) goto END2; /* no rec.call */

/******************************************************/

  int s, t, tt, l = QUEUE_LENGTH(LCM_jump), prv;
  QUEUE_INT *x, *itemset2;
  
  QUEUE_init ( &jump2, (LCM_jump.t-LCM_jump.s)*2+2 );
  occ[item].end = LCM_th;
  n = LCMmax_mk_freq_weighted_trsact(&TT, T, &LCM_Occ, &occ[item], n*2+occ[item].t, ee, LCM_th, LCM_frq);
  occ[item].end = 0;
  q = ((QUEUE *)(TT.h))->q;
  if ( ii >= 2 && TT.num>7 )
    q = LCMmax_shrink ( &TT, &LCM_shrink_jump, LCM_shrink_p, item, n );

  if ( ii<4 ){
    LCMmax_occ_deliver_all( &TT, &LCM_Occ, item-1 );
    LCMmax_remove_large_items_ ( &LCM_jump, item, occ );
    QUEUE_sortr_ ( &LCM_jump );
    do {
      i = QUEUE_ext_tail_ ( &LCM_jump );
      LCM_itemset2.t = itemt;
      LCMmax_iter ( &TT, &LCM_Occ, i, LCM_perm, NULL, item, &jump2 );
      occ[i].t = occ[i].end = 0;
    } while ( LCM_jump.t > LCM_jump.s );
    goto END1;
  }

  occ[ee].s = occ[ee].end;
  for ( i=0 ; i<TT.num ; i++ ){
    for ( x=((QUEUE *)(TT.h))[i].q ; *x<ee ; x+=2 );
    if ( *x == ee ) occ[ee].q[occ[ee].t++] = i;
  }
  QUEUE_FE_LOOP_ ( LCM_jump, i, e ){
    LCM_jump.q[i+LCM_jump.t] = occ[e].end;
    occ[e].end = 0;
  }
  LCMmax_iter ( &TT, &LCM_Occ, ee, LCM_perm, NULL, item, &jump2 );
  occ[ee].t = 0;

  QUEUE_FE_LOOP_ ( LCM_jump, i, e ) occ[e].end = LCM_jump.q[i+LCM_jump.t];
  itemt22 = itemt2 = LCM_itemset2.t;
  malloc2(itemset2, QUEUE_INT, ii+1,"LCMmax_iter","itemset2__");
  memcpy(itemset2,LCM_itemset2.q+itemt,(itemt2-itemt)*sizeof(QUEUE_INT));
  qsort_int_ ( itemset2, itemt2-itemt );
  itemset2[itemt2-itemt] = -1;
  LCM_jump.q[LCM_jump.t++] = item;

  if ( itemt22-itemt-1 > 0 ){
    t = LCM_jump.s; tt=0;
    QUEUE_FE_LOOP_ ( LCM_jump, i, e ){
      if ( e<item ) LCM_ary[tt++] = e;
      else LCM_jump.q[t++] = e;
    }
    ii = t+1;
    qsort_int_ ( LCM_ary, tt );
    for ( s=1,i=0 ; i<tt ; i++ ){
      while ( itemset2[s] > LCM_ary[i] ) s++; 
      if ( itemset2[s] != LCM_ary[i] ) LCM_jump.q[t++]=LCM_ary[i];
    }
    memcpy(LCM_jump.q+t, itemset2+1, (itemt2-itemt-1)*sizeof(QUEUE_INT) );
    for ( i=0; i+ii<t ; i++ ){
      e = LCM_jump.q[i+ii];
      LCM_ary[i*2] = occ[e].end;
      LCM_ary[i*2+1] = e;
    }
    qsort_int2 ( LCM_ary, i );
    for ( i=0 ; i+ii<t ; i++ ) LCM_jump.q[i+ii] = LCM_ary[i*2+1];
  } else {
    QUEUE_sortr_ ( &LCM_jump );
    QUEUE_FE_LOOP_ ( LCM_jump, i, e ) if ( e<ee ) break;
    if ( i<LCM_jump.t ){
      for ( ii=i ; ii<LCM_jump.t ; ii++ ){
        e = LCM_jump.q[ii];
        LCM_ary[ii*2] = occ[e].end;
        LCM_ary[ii*2+1] = e;
      }
      qsort_int2 ( LCM_ary+(i*2), ii-i );
      for ( ii=i ; ii<LCM_jump.t ; ii++ ) LCM_jump.q[ii] = LCM_ary[ii*2+1];
    }
  }

  tt = LCM_jump.t - LCM_jump.s;
  perm = LCMmax_alloc_occ ( &TT, &LCM_Occ, &OO, n, &LCM_jump, LCM_perm, item );
  oocc = (QUEUE *)(OO.h);
  for ( i=0 ; i<OO.num ; i++ ) oocc[i].t = 0;
  LCMmax_replace_index_occurrence_deliver( &TT, &OO, LCM_ary[item] );
  LCMmax_remove_large_items ( &LCM_jump, item, occ, oocc );
  prv = LCM_ary[item];

  ff = 0;
  for ( i=0 ; i<prv-1 ; i++ ){
    LCM_itemset2.t = itemt;
    if ( i>=itemt2-itemt-1 ){
      LCMmax_iter(&TT, &OO, i, perm, &perm[tt], prv, &jump2);
      if ( LCM_itemset2.t>itemt22 || (LCM_itemset2.t==itemt2 && ff==0) ){
        ff = 1;
        itemt2 = LCM_itemset2.t;
        memcpy(itemset2,LCM_itemset2.q+itemt,(itemt2-itemt)*sizeof(QUEUE_INT));
        qsort_int_ ( itemset2, itemt2-itemt );
      }
    }
    oocc[i].t = oocc[i].end = 0;
  }

  free2 ( perm );
  ARY_end ( &OO );
  LCM_itemset2.t = itemt2;
  free2 ( itemset2 );
  END1:;
  QUEUE_end ( &jump2 );
  free2 ( q );
  ARY_end ( &TT );
  goto END3;
  END2:;
  while ( LCM_jump.t>LCM_jump.s ){
    ii = LCM_jump.q[--LCM_jump.t];
    occ[ii].end = 0;
  }
  END3:;
  LCM_itemset.t--;
  LCM_add.t = addtail;
  LCM_jump.s = js;
  LCM_jump.s = LCM_jump.t = 0;
  return ( flag );
}

/*************************************************************************/
/* anytime database reduction + occurrence deliver closure               */
/*************************************************************************/
void LCMmax ( int n ){
  int i;
  QUEUE_init ( &LCM_itemset2, LCM_Eend );

  for ( i=0 ; i<LCM_Eend ; i++ ){
    LCM_occ[i].s = LCM_occ[i].end;
    LCM_occ[i].end = 0;
  }
  for ( i=0 ; i<LCM_Eend ; i++ ){
    LCM_itemset2.t = 0;
    LCMmax_first_iter ( i );
    LCM_occ[i].t = LCM_occ[i].end = 0;
  }
  LCM_iters++;
  QUEUE_end ( &LCM_itemset2 );
}

/*************************************************************************/
/*************************************************************************/
main ( int argc, char *argv[] ){
  int i, n;
  n = LCM_init ( argc, argv );
  LCMmax ( n );
  LCM_output ( 0 );
  LCM_end ();
  ARY_end ( &LCM_Trsact );
  exit (0);
}


#endif


