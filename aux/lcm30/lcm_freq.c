/* Linear time Closed itemset Miner for Frequent Itemset Mining problems */
/* 2004/4/10 Takeaki Uno */
/* This program is available for only academic use.
   Neither commercial use, modification, nor re-distribution is allowed */

#ifndef _lcmfreq_c_
#define _lcmfreq_c_

// #define LCM_MESSAGES   // remove!!!!

#include<time.h>
#include"lib_e.c"
#include"lcm_var.c"
#define LCM_PROBLEM LCM_FREQSET
#include"trsact.c"
#include"lcm_io.c"
#include"lcm_init.c"
#include"lcm_lib.c"

/*  Transaction: each Q[i].s ( ((QUEUE *)(LCM_Trsact->h))[i] ) is
        for bitmap representation for frequent items
    Occurrence:  each Q[i].s ( ((QUEUE *)(LCM_Occ->h))[i] ) is
        original number of item i if i is in "bitmap items"
*/

void LCMfreq_BM_iter (int item, int m);

void LCMfreq_BM_recursive (int item, int mask){
  int i;
  if ( (i=LCM_Ofrq[0]) >= LCM_th ) LCM_print_last ( LCM_Op[0], i );
  LCM_BM_weight[1] = LCM_Ofrq[0] = 0;
  LCM_Ot[0] = LCM_Os[0];

  if ( (i=LCM_Ofrq[1]) >= LCM_th ){
    LCM_frq = i;
    LCM_additem ( LCM_Op[1] );
    LCMFREQ_solution ();
    if ((i=LCM_BM_weight[3])>=LCM_th && (mask&1)) LCM_print_last (LCM_Op[0],i);
    LCM_delitem ();
  }
  LCM_BM_weight[2] = LCM_BM_weight[3] = LCM_Ofrq[1] = 0;
  LCM_Ot[1] = LCM_Os[1];

  for ( i=2 ; i<item ; i++ )
      if ( LCM_Ofrq[i] >= LCM_th ) LCMfreq_BM_iter (i, mask);
}

/*************************************************************************/
/* LCMfreq iteration (bitmap version ) */
/* input: T:transactions(database), item:tail(current solution) */
/*************************************************************************/
void LCMfreq_BM_iter (int item, int m){
  int qt=LCM_add.t, mask;

  LCM_iters++;
  LCM_frq = LCM_Ofrq[item];
  LCM_additem ( LCM_Op[item] );
  LCM_BM_occurrence_deliver_ (item, m);
  mask = LCM_BM_rm_infreq (item, NULL);

  LCMFREQ_solution ();
  LCMfreq_BM_recursive (item, mask);

  LCM_delitem ();
  LCM_add.t = qt;
}




/*************************************************************************/
/* LCM_freq main (bitmap version ) */
/*************************************************************************/
int LCMfreq_BM (ARY *T){
  LCM_BM_occurrence_deliver_first (-1, T);
/*
int i;
for ( i=LCM_BM_MAXITEM ; i<LCM_Eend ; i++ ){
printf ("%d %d\n", i, LCM_Ofrq[i]);
}
*/
  LCMfreq_BM_recursive (LCM_BM_MAXITEM, 0xffffffff);
}


/*************************************************************************/
/* LCMfreq iteration (lcm_closed-base type ) */
/* input: T:transactions(database), item:tail(current solution) */
/*************************************************************************/
void LCMfreq_iter (ARY *T, int item){
  ARY TT;
  int i, ii, mask, n;
  int perm[LCM_BM_MAXITEM];
  int js = LCM_jump.s, qt = LCM_add.t;
  QUEUE_INT *q;
  LCM_jump.s = LCM_jump.t;

  LCM_iters++;
  LCM_frq = LCM_Ofrq[item];
  LCM_additem ( item );
  n = LCM_freq_calc (T, item, item-1);
  LCM_jump_rm_infreq (item);
  QUEUE_sortr_ ( &LCM_jump );
  LCM_BM_occurrence_deliver_first (item, T);
  mask = LCM_BM_rm_infreq(LCM_BM_MAXITEM, NULL);

  LCMFREQ_solution ();
  if ( QUEUE_LENGTH(LCM_jump) == 0 ){
    LCMfreq_BM_recursive (LCM_BM_MAXITEM, mask);
    goto END2;
  }
/******************************************************/
  for ( i=0 ; i<LCM_BM_MAXITEM ; i++ ) perm[i] = LCM_Op[i];
  LCM_mk_freq_trsact (&TT, T, item, LCM_jump.q[LCM_jump.s], n+(LCM_Ot[item]-LCM_Os[item]), mask);
  for ( i=0 ; i<LCM_BM_MAXITEM ; i++ ) LCM_BM_occurrence_delete (i);

  LCMfreq_BM (&TT); /* OCC ‚Å BM ‚ðÄ‹AŒÄ‚Ño‚µ */

  if ( QUEUE_LENGTH(LCM_jump) == 0 ) goto END1;
  q = ((QUEUE *)(TT.h))->q;
  if (QUEUE_LENGTH(LCM_jump)>=3 && TT.num>=20) LCM_shrink (&TT, LCM_Eend, 1);
  LCM_occ_deliver (&TT, item-1);

  while ( QUEUE_LENGTH(LCM_jump) > 0 ){
    i = QUEUE_ext_tail_ ( &LCM_jump );
    LCMfreq_iter ( &TT, i );
    LCM_Ot[i] = LCM_Os[i];
    LCM_Ofrq[i] = 0;
  }

  free2 ( q );
  ARY_end ( &TT );
  END1:;
  for ( i=0 ; i<LCM_BM_MAXITEM ; i++ ) LCM_Op[i] = perm[i];

  END2:;
  LCM_jump.t = LCM_jump.s;
  LCM_jump.s = js;
  LCM_delitem ();
  LCM_add.t = qt;
}

/*************************************************************************/
/* LCM_freq main (lcm_closed-base type ) */
/*************************************************************************/
void LCMfreq (){
  int i;
  LCMfreq_BM (&LCM_Trsact);
  for ( i=LCM_BM_MAXITEM ; i<LCM_Eend ; i++ ){
    LCMfreq_iter ( &LCM_Trsact, i );
    LCM_Ot[i] = LCM_Os[i];
    LCM_Ofrq[i] = 0;
  }
  LCM_iters++;
}

/******************************************************************/
/* main */
/******************************************************************/
main ( int argc, char *argv[] ){
  LCM_problem = LCM_FREQSET;
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


