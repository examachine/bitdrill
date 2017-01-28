


#ifndef _lcm_bm_c_ 
#define _lcm_bm_c_ 

#include"lcm_var.c"
void LCM_trsact_and (QUEUE_INT *q, QUEUE_INT *qq, int *t);
void LCM_additem ( int item );

int LCM_BM_space;
int *LCM_BM_weight;
char *LCM_BM_highbit;
int LCM_BM_MAXITEM, LCM_BM_prefix_max;
int *LCM_BM_pt;
int *LCM_BM_pp;
QUEUE_INT **LCM_BM_pq;
BUF LCM_B;


void LCM_BM_init (){
  int i, ii, m, mm;
  for ( ii=0,LCM_BM_space=1 ; ii<LCM_BM_MAXITEM ; ii++,LCM_BM_space*=2 );
  malloc2 (LCM_BM_weight, int, LCM_BM_space, "LCM_BM_init: LCM_BM_weight");
  malloc2 (LCM_BM_highbit, char, LCM_BM_space, "LCM_BM_init: LCM_BM_highbit");
  if ( LCM_PROBLEM == LCM_CLOSED ){
    malloc2 (LCM_BM_pt, int, LCM_BM_space, "LCM_BM_init: LCM_BM_pt");
    malloc2 (LCM_BM_pp, int, LCM_BM_space, "LCM_BM_init: LCM_BM_pt");
    malloc2 (LCM_BM_pq, QUEUE_INT*, LCM_BM_space, "LCM_BM_init: LCM_BM_pq");
    BUF_init ( &LCM_B, sizeof(int), 10000 );
  }
  for ( m=ii=0,mm=2 ; ii<LCM_BM_MAXITEM ; ii++,mm*=2 ){
    for ( ; m<mm ; m++ ){
      LCM_BM_weight[m] = 0;
      LCM_BM_highbit[m] = ii;
    }
  }
}

void LCM_BM_end (){
  free2 (LCM_BM_weight);
  free2 (LCM_BM_highbit);
  if ( LCM_PROBLEM == LCM_CLOSED ){
    free2 (LCM_BM_pt);
    free2 (LCM_BM_pp);
    free2 (LCM_BM_pq);
    BUF_end ( &LCM_B );
  }
}

/****************************************************************************/
/*       Occurrence Deliver for Bitmap                                      */
/* input: occ:occurrences, max_item                                         */
/* CAHNGE: for each item i<max_item, set O->h[i] to the transactions of occ */
/*    including i (items larger than i are removed from the transactions)   */
/****************************************************************************/
void LCM_BM_occurrence_deliver_proc (int p, int frq){
  int *ww = &LCM_BM_weight[p];
  if ( *ww==0 ){
    int h=LCM_BM_highbit[p];
    *LCM_Ot[h] = p;
    LCM_Ot[h]++;
  }
  *ww += frq;
}


void LCM_BM_occurrence_deliver_iter (int item, int mask){
  int w=0, *t, *t_end, ww;
  mask &= BITMASK_LOWER1[item];
  for ( t=LCM_Os[item],t_end=LCM_Ot[item] ; t<t_end ; t++ ){
    ww = LCM_BM_weight[*t];
    w += ww;
    LCM_BM_occurrence_deliver_proc (*t&mask, ww);
  }
  LCM_Ofrq[item] = w;
}

void LCM_BM_occurrence_deliver_iter_delete (int item, int mask){
  int i, *t, *t_end, *ww;
  mask &= BITMASK_LOWER1[item];
  for ( t=LCM_Os[item],t_end=LCM_Ot[item] ; t<t_end ; t++ ){
    ww = &(LCM_BM_weight[*t]);
    LCM_BM_occurrence_deliver_proc (*t&mask, *ww);
    *ww = 0;
  }
  LCM_Ot[item] = LCM_Os[item];
  LCM_Ofrq[item] = 0;
}

void LCM_BM_occurrence_deliver (int item, int mask){
  for ( ; item>1 ; item--) LCM_BM_occurrence_deliver_iter(item, mask);
  LCM_Ofrq[0] = LCM_BM_weight[1] + LCM_BM_weight[3];
  LCM_Ofrq[1] = LCM_BM_weight[2] + LCM_BM_weight[3];
}

void LCM_BM_occurrence_deliver_ (int item, int mask){
  LCM_BM_occurrence_deliver_iter_delete (item, mask);
  LCM_BM_occurrence_deliver (item-1, mask);
}


void LCM_BM_occurrence_deliver_first (int item, ARY *T){
  int p, i, m = BITMASK_LOWER1[LCM_BM_MAXITEM];
  QUEUE *TQ=T->h;
  if ( item <0 ){  /* then, occ is considered top be "all transactions" */
    long t;
    for ( t=0 ; t<T->num ; t++ )
        LCM_BM_occurrence_deliver_proc ( TQ[t].s&m, TQ[t].end );
  } else {
    int *t, *t_end;
    for ( t=LCM_Os[item],t_end=LCM_Ot[item] ; t<t_end ; t++ )
        LCM_BM_occurrence_deliver_proc ( TQ[*t].s&m, TQ[*t].end );
  }
  LCM_BM_occurrence_deliver (LCM_BM_MAXITEM-1, 0xffffffff);
}


void LCM_BM_occurrence_delete (int item){
  if ( item == 0 ){
    LCM_BM_weight[1] = 0;
  } else {
    int *t, *t_end;
    for (t=LCM_Os[item],t_end=LCM_Ot[item] ; t<t_end ; t++){
      LCM_BM_weight[*t] =0;
    }
  }
  LCM_Ot[item] = LCM_Os[item];
  LCM_Ofrq[item] = 0;
}


/****************************************************************************/
/*       Occurrence Deliver for Bitmap                                      */
/* input: occ:occurrences, max_item                                         */
/* CAHNGE: for each item i<max_item, set O->h[i] to the transactions of occ */
/*    including i (items larger than i are removed from the transactions)   */
/****************************************************************************/
void LCMclosed_BM_occ_deliver_proc (int p, int pp){
  if ( p==0 ) return;
  if ( LCM_BM_weight[p]==0 ){
    *LCM_Ot[LCM_BM_highbit[p]]++ = p;
    int *z = BUF_get ( &LCM_B, LCM_BM_pt[pp]+1 );
    memcpy ( z, LCM_BM_pq[pp], (LCM_BM_pt[pp]+1)*sizeof(int));
    LCM_BM_pp[p] = LCM_BM_pp[pp];
    LCM_BM_pq[p] = z;
    LCM_BM_pt[p] = LCM_BM_pt[pp];
  } else {
    LCM_trsact_and (LCM_BM_pq[p], LCM_BM_pq[pp], &LCM_BM_pt[p]);
    LCM_BM_pp[p] &= LCM_BM_pp[pp];
  }
  LCM_BM_weight[p] += LCM_BM_weight[pp];
}
void LCMclosed_BM_occ_deliver_first_proc (QUEUE *Q, int mask, int imax){
  int p=Q->s&mask, pp=p, *x;
  if ( p==0 ) return;
  for ( x=Q->q ; *x<=imax ; x++ )
      if ( LCM_Ofrq[*x] >= LCM_th ) pp |= BITMASK_1[LCM_Op[*x]];
  if ( LCM_BM_weight[p]==0 ){
    *LCM_Ot[LCM_BM_highbit[p]]++ = p;
    int *z = BUF_get ( &LCM_B, Q->t+1 ), *zz;
    for ( zz=z ; *x<LCM_Eend ; x++ )
        if ( LCM_Ofrq[*x] >= LCM_th ) { *zz = *x; zz++; }
    *zz = LCM_Eend;
    LCM_BM_pp[p] = pp;
    LCM_BM_pq[p] = z;
    LCM_BM_pt[p] = zz-z;
  } else {
    LCM_trsact_and (LCM_BM_pq[p], x, &(LCM_BM_pt[p]));
    LCM_BM_pp[p] &= pp;
  }
  LCM_BM_weight[p] += Q->end;
}

void LCMclosed_BM_occurrence_deliver_iter (int item, int mask){
  int w=0, *t, *t_end;
  mask &= BITMASK_LOWER1[item];
  for ( t=LCM_Os[item],t_end=LCM_Ot[item] ; t<t_end ; t++ ){
    w += LCM_BM_weight[*t];
    LCMclosed_BM_occ_deliver_proc (*t&mask, *t);
  }
  LCM_Ofrq[item] = w;
}

void LCMclosed_BM_occurrence_deliver_iter_delete (int item, int mask){
  int *t, *t_end;
  mask &= BITMASK_LOWER1[item];
  for ( t=LCM_Os[item],t_end=LCM_Ot[item] ; t<t_end ; t++ ){
    LCMclosed_BM_occ_deliver_proc (*t&mask, *t);
    LCM_BM_weight[*t] = 0;
  }
  LCM_Ot[item] = LCM_Os[item];
  LCM_Ofrq[item] = 0;
}

void LCMclosed_BM_occurrence_deliver (int item, int mask){
  for ( ; item>1 ; item--) LCMclosed_BM_occurrence_deliver_iter(item, mask);
  LCM_Ofrq[0] = LCM_BM_weight[1] + LCM_BM_weight[3];
  LCM_Ofrq[1] = LCM_BM_weight[2] + LCM_BM_weight[3];
  if ( LCM_BM_weight[3]>0 ) LCMclosed_BM_occ_deliver_proc (1, 3);
}

void LCMclosed_BM_occurrence_deliver_ (int item, int mask){
  LCMclosed_BM_occurrence_deliver_iter_delete (item, mask);
  LCMclosed_BM_occurrence_deliver (item-1, mask);
}


void LCMclosed_BM_occurrence_deliver_first (int item, ARY *T){
  int p, i, m = BITMASK_LOWER1[LCM_BM_MAXITEM], jt=LCM_jump.t, imax=-1, flag=0;
  QUEUE *TQ=T->h;
   //  shift prefix items to bitmap of upper parts
  for ( i=LCM_BM_MAXITEM ; i<sizeof(int)*8 ; i++ ){
    START:;
    if ( flag==1 && jt==LCM_jump.t ) break;
    if ( jt == LCM_jump.s ){ jt = LCM_jumpt; flag = 1; goto START; }
    jt--;
    imax = LCM_jump.q[jt];
    LCM_Op[imax] = i;
// printf ("(%d), %d %d: %d -> %x  (%dth,%d:%d)\n", item, LCM_pprv, LCM_prv, imax, BITMASK_1[i], jt-LCM_jump.s, LCM_jump.t-LCM_jump.s, LCM_jumpt-LCM_jump.s);
  }
  LCM_BM_prefix_max = i;

  if ( item <0 ){  /* then, occ is considered top be "all transactions" */
    long t;
    for ( t=0 ; t<T->num ; t++ )
        LCMclosed_BM_occ_deliver_first_proc (&TQ[t], m, imax);
  } else {
    int *t, *t_end;
    for ( t=LCM_Os[item],t_end=LCM_Ot[item] ; t<t_end ; t++ )
        LCMclosed_BM_occ_deliver_first_proc (&TQ[*t], m, imax);
  }
  LCMclosed_BM_occurrence_deliver (LCM_BM_MAXITEM-1, 0xffffffff);
}




/********************************************************************/
/********************************************************************/
/********************************************************************/


int LCM_BM_rm_infreq (int item, int *pmask){
  int i, m=0, flag=-1;
  for ( i=0 ; i<item ; i++ ){
    if ( LCM_Ofrq[i] >= LCM_th ){
      if ( LCM_Ofrq[i] == LCM_frq ){
        if (LCM_PROBLEM == LCM_FREQSET) QUEUE_ins_ (&LCM_add, LCM_Op[i]);
        else {
          LCM_additem (LCM_Op[i]);
          *pmask &= BITMASK_31[i];
        }
        LCM_BM_occurrence_delete (i);
      } else m |= BITMASK_1[i];
    } else {
      LCM_BM_occurrence_delete (i);
    }
  }
  return ( m );
}

/*************************************************************************/
/* set TT to the transactions of occ with frequency in the given
  upper and lower bounds */
/* return #all freq items */
/*************************************************************************/
int LCM_mk_freq_trsact( ARY *TT, ARY *T, QUEUE_INT item, QUEUE_INT max_item, int cnt, int mask ){
  QUEUE_INT *x, *buf, *bbuf, *b;
  QUEUE *Q=T->h, *QQ;
  int i, ii, n=0, imax=-1, flag, jt=LCM_jump.t, *t, *t_end;

  for ( i=0 ; i<LCM_BM_MAXITEM ; i++ ){
    if ( LCM_Ofrq[i] < LCM_th ){
      LCM_jump.t--;
      imax = LCM_jump.q[LCM_jump.t];
      LCM_Op[imax] = i;
      LCM_Op[i] = imax;
      if ( LCM_jump.t == LCM_jump.s ) break;
    }
  }

  ARY_init ( TT, sizeof(QUEUE) );
  ARY_exp (TT, (LCM_Ot[item]-LCM_Os[item])-1 );
  QQ = TT->h;
  malloc2 ( buf, QUEUE_INT, cnt, "TRSACT_mk_freq_trsact: buf");
  bbuf = buf;
  for (t=LCM_Os[item],t_end=LCM_Ot[item] ; t<t_end ; t++){
    QQ[n].q = b = buf;
    QQ[n].s = Q[*t].s & mask;
    x=Q[*t].q;
    for ( ; *x<=imax ; x++ )
        if ( LCM_Ofrq[*x] > 0 ) QQ[n].s |= BITMASK_1[LCM_Op[*x]];
    for ( ; *x<=max_item ; x++ )
        if ( LCM_Ofrq[*x] > 0 ){ *buf = *x; buf++; }
    if ( buf != b || QQ[n].s > 0){
      QQ[n].t = buf-b;
      QQ[n].end  = Q[*t].end;   /* multiplicity of transaction t */
      *buf = LCM_Eend;
      buf++;
      n++;
    }
  }
  for (i=LCM_jump.t ; i<jt ; i++){
    LCM_Ofrq[LCM_jump.q[i]] = 0;
    if ( LCM_PROBLEM != LCM_FREQSET ) LCM_Ofrq_[LCM_jump.q[i]] = 0;
  }
  if ( LCM_PROBLEM != LCM_FREQSET ){
    for ( imax=LCM_jump.t ; i<LCM_jumpt ; i++,imax++ )
        LCM_jump.q[imax] = LCM_jump.q[i];
    LCM_jumpt = imax;
  }
  TT->num = n;
  return ( buf-bbuf );
}

///////////////////////////////////////////
/*
int LCM_BM_closure (int item, int pmask){
  int i, *t, *t_end, p=pmask&BITMASK_UPPER1[item], ee, z=-1;
  QUEUE_INT *x, *xx;
  int fi=*LCM_Os[item], tt=LCM_BM_pt[fi], qq[tt], *qq_=qq;
  for ( i=0 ; i<=tt ; i++ ) qq_[i] = LCM_BM_pq[fi][i];

  for (t=LCM_Os[item]+1,t_end=LCM_Ot[item] ; t<t_end ; t++){
    LCM_trsact_and ( qq_, LCM_BM_pq[*t], &tt);
    if ( tt==0 ) goto CONT;
  }
  return ( qq_[tt-1] );

  CONT:;
  for (t=LCM_Os[item],t_end=LCM_Ot[item] ; t<t_end ; t++) p &= LCM_BM_pp[*t];
  if (p){
    for ( i=item+1 ; i<LCM_BM_MAXITEM ; i++ )
        if ( (p&BITMASK_1[i]) && LCM_Op[i]>z ) z = LCM_Op[i];
    for (  ; i<LCM_BM_prefix_max ; i++ )
        if ( (p&BITMASK_1[i]) ) z = LCM_prv;
  }
  return ( z );
}
*/

int LCM_BM_closure (int item, int pmask){
  int i, *t, *t_end, p=pmask&BITMASK_UPPER1[item], ee, z=-1;
  QUEUE_INT *x, *jq=&(LCM_jump.q[LCM_jump.t]), *jqq=jq, *jjq, *xx;

  for (t=LCM_Os[item],t_end=LCM_Ot[item] ; t<t_end ; t++){
    p &= LCM_BM_pp[*t];
    ee = LCM_BM_weight[*t];
    for ( x=LCM_BM_pq[*t] ; *x<LCM_Eend ; x++ ){
      if ( LCM_Ofrq[*x] == 0 ){ *jq=*x; jq++; }
      LCM_Ofrq[*x] += ee;
    }
  }
  if (p){
    for ( i=item+1 ; i<LCM_BM_MAXITEM ; i++ )
        if ( (p&BITMASK_1[i]) && LCM_Op[i]>z ) z = LCM_Op[i];
    for (  ; i<LCM_BM_prefix_max ; i++ )
        if ( (p&BITMASK_1[i]) ) z = LCM_prv;
  }
  for ( jjq=jq-1; jjq>=jqq ; jjq-- )
    if ( LCM_Ofrq[*jjq]==LCM_frq && *jjq>z ) z = *jjq;

  if ( z < 0 && jq>jqq ){
    for (t=LCM_Os[item],t_end=LCM_Ot[item] ; t<t_end ; t++){
      for ( xx=x=LCM_BM_pq[*t] ; *x<LCM_Eend ; x++ ){
        if ( LCM_Ofrq[*x] >= LCM_th ){
          if ( x>xx ) *xx = *x;
          xx++;
        }
      }
      if ( x!=xx ){ *xx = LCM_Eend; LCM_BM_pt[*t] = xx-LCM_BM_pq[*t]; }
    }
  }

  for ( jjq=jq-1; jjq>=jqq ; jjq-- ) LCM_Ofrq[*jjq]=0;
  return ( z );
}

#endif

