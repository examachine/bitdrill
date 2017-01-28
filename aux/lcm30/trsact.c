/*  routines used FIM algorithms */
/* 2003/9/1 Takeaki Uno */
/* This program is available for only academic use.
   Neither commercial use, modification, nor re-distribution is allowed */

#ifndef _trsact_c_
#define _trsact_c_

#define QUEUE_INT int

#include<stdio.h>
#include"lib_e.c"
#include"lcm_bm.c"

/***********************************/
/*   print transactions            */
/***********************************/
void TRSACT_print ( ARY *T ){
  int i, j, e;
  QUEUE *Q = T->h;
  for ( i=0 ; i<T->num ; i++ ){
    for ( j=0 ; j<Q[i].t ; j++ ){
      e = Q[i].q[j];
      printf ("%d,", e);
    }
    printf (" (%d)\n", Q[i].end );
  }
}


/********************************************************************/
/* sort indices of frequent items by their frequency                */
/* INPUT: E:counters of items, th:support                           */
/* RETURN: permutation, #freq items                                 */
/* CHANGE: counters (-1 if infrequent )                             */
/* set *num to the size of database composed only of frequent items */
/* set *Enum to #frequent items                                     */
/********************************************************************/
int *TRSACT_sort_item_by_frq ( ARY *E, int th, int *num, int *Enum ){
  int *Eq = E->h, *perm, i;
  *num = 0;
  *Enum = 0;
  malloc2 ( perm, int, E->num*2, "TRSACT_perm_freq_item: perm");
  for ( i=0 ; i<E->num ; i++ ){
    if ( Eq[i] >= th ){
      perm[(*Enum)*2] = Eq[i];
      perm[(*Enum)*2+1] = i;
      (*Enum)++;
      *num += Eq[i];
    } else Eq[i] = -1;
  }
  qsort ( perm, *Enum, sizeof(int)*2, qsort_int_cmp_ );
  for ( i=0 ; i<*Enum ; i++ ) perm[i] = perm[i*2+1];
  realloc2 ( perm, int, *Enum, "TRSACT_sort_item_by_frq: perm" );
  return ( perm );
}

/*****************************/
/* sort transactions by their sizes */
/* input: T:transactions, jump:list of frequnet items, p:working space */
/*****************************/
void TRSACT_sort_by_size ( ARY *T, QUEUE *jump, long *p ){
  int *q, t, tt, ii;
  QUEUE *Q = T->h, *Qtmp;
  QUEUE_INT *jq, *jqq;

  malloc2 ( Qtmp, QUEUE, T->num, "TRSACT_sort_size: Qtmp"); 
  malloc2 ( q, int, T->num, "TRSACT_sort_size: q"); 
  
  for ( t=0,jq=jump->q ; t<T->num ; t++ ){
    ii = Q[t].t;
    if ( p[ii] == -1 ){ *jq = ii; jq++; }  /* insert each tr */
    q[t] = p[ii];
    p[ii] = t;
  }

  jump->t = jq-jump->q;
  QUEUE_sort_ ( jump );  /* sort "sizes" */
  jump->t = 0;

  for ( tt=0,jqq=jq-1 ; jqq>=jump->q ; jqq-- ){
    for ( t=p[*jqq] ; t>=0 ; t=q[t] ){
      Qtmp[tt] = Q[t];
      tt++;
    }
    p[*jqq] = -1;
  }
  memcpy ( Q, Qtmp, sizeof(QUEUE)*T->num );

  free ( Qtmp );
  free ( q );
}


/************************************************************************/
/*       Occurrence Deliver                                             */
/* input: T:transactions, max_item                                      */
/* CAHNGE: for each item e<max_item, set O->h[e].q to the transactions  */
/*     including e, and set O->h[e].t to the # of such transactions     */
/************************************************************************/
void LCM_occ_deliver( ARY *T, QUEUE_INT max_item){
  int t;
  QUEUE_INT *x;
  QUEUE *Q = T->h;
  for ( t=0 ; t<T->num ; t++ ){
    for ( x=Q[t].q ; *x<=max_item ; x++ ){ *LCM_Ot[*x] = t; *LCM_Ot[*x]++; }
  }
}

/**************************************************************/
/* initialization of shrink operation */
/* input: T:transactions, Eend:# of items */
/* output: working space (array of long integers) */
/* alloc the working space to *jump (QUEUE) */
/**************************************************************/
void LCM_shrink_init (ARY *T, QUEUE_INT Eend){
  int i;
  malloc2 ( LCM_shrink_p, long, Eend*2+T->num*4+4, "TRSACT_shrink_init");
  for ( i=0 ; i<Eend*2+2 ; i++ ) LCM_shrink_p[i] = -1;
  QUEUE_init ( &LCM_shrink_jump, Eend*2+2 );
  LCM_shrink_jump.end = Eend;
}


void LCM_trsact_and (QUEUE_INT *q, QUEUE_INT *qq, int *t){
  int *xx, *x1, *x2;
  for ( xx=x1=q,x2=qq ; *x1<LCM_Eend ; x1++ ){
    while ( *x2<*x1 ) x2++;
    if ( *x2 == *x1 ){
      if ( x1!=xx ) *xx=*x1;
      xx++;
      x2++;
    }
  }
  *xx = LCM_Eend;
  *t = xx - q;
}


/**************************************************************/
/* shrink T */
/* input: T:transactions, jump,q:working space(re-use for omiting initialization) */
/* find the identical transactions from T by radix sort (bucket sort) 
  and merge them into one transaction */
/* set T->h[i].end to the multiplicity of transaction i
     (# of transactions merged to i) */
/* flag&1 => BitMap version */
/* flag&2 => for closed sets */
/*************************************************************************/
void LCM_shrink ( ARY *T, int max_item, int flag ){
  int ii, j, t, tt=-1, v, vv, end=LCM_shrink_jump.end;
  QUEUE_INT *jt, *jtt, *jq=LCM_shrink_jump.q, *jqq=jq+end+1;
  long *p = LCM_shrink_p;
  long *pp=&p[end+1],*q=&p[end*2+2],*qq=&p[end*2+2+T->num*2];
  QUEUE *Q = T->h;
  QUEUE_INT *x1, *xx, *x2;

  if ( flag&1 ){  /*  BitMap version */
    LCM_BM_weight[0] = 0;  // it is not cleared in other routines
    for ( t=0 ; t<T->num ; t++ ){
      j = LCM_BM_weight[Q[t].s];
      if ( LCM_BM_weight[Q[t].s] == 0 ){
        qq[t*2] = tt;
        qq[t*2+1] = t;
        LCM_BM_weight[Q[t].s] = t+1;
        tt = t;
      } else {
        j--;
        qq[t*2] = qq[j*2];
        qq[j*2] = t;
        qq[t*2+1] = j;
      }
    }
    jtt = jqq+1; *jqq = 0; pp[0] = tt;
    for ( t=0 ; t<T->num ; t++ ) LCM_BM_weight[Q[t].s] = 0;
    j=0;
  } else {   /* normal version */
    exit(1);
    for ( t=0,jtt=jqq ; t<T->num ; t++ ){
      ii = Q[t].q[0];
      if ( pp[ii] == -1 ){ *jtt = ii; jtt++; }
      qq[t*2] = pp[ii];
      qq[t*2+1] = 0;
      pp[ii] = t;
    }
    j=1;
  }

  for ( ; jtt>jqq ; j++ ){
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
      while (1){   /* shrink the same transactions into one */
        if ( (tt=qq[tt*2]) < 0 ) goto END3;
        if ( qq[tt*2+1] != v ) break;
        Q[t].end += Q[tt].end; /* add multiplicity */
        Q[tt].end = 0;
        if ( LCM_PROBLEM == LCM_CLOSED ){    /* for closed sets */
          if ( max_item < LCM_Eend ){
            LCM_trsact_and (Q[t].q+j, Q[tt].q+j, &(Q[t].t));
            Q[t].t += j;
          }
        }
      }
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




































#endif
