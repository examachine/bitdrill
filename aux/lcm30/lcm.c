/* Linear time Closed itemset Miner for Frequent Itemset Mining problems */
/* 2004/4/10 Takeaki Uno */
/* This program is available for only academic use.
   Neither commercial use, modification, nor re-distribution is allowed */

#ifndef _lcm_c_
#define _lcm_c_

#include<time.h>
#include"lib_e.c"
#include"lcm_var.c"
#define LCM_PROBLEM LCM_CLOSED
#include"trsact.c"
#include"lcm_io.c"
#include"lcm_init.c"
#include"lcm_lib.c"

void LCMclosed_BM_iter (int item, int m, int pmask);

void LCMclosed_BM_recursive (int item, int mask, int pmask){
  int i;
  if ( (i=LCM_Ofrq[0]) >= LCM_th ){
    if ( (LCM_BM_pp[1]&pmask)==1 && LCM_BM_pt[1]==0 )
        LCM_print_last ( LCM_Op[0], i );
//     else printf ("11clo item%d %d: %d : %x & %x = %x,   %d:  prv%d pprv%d \n", LCM_Op[0], LCM_itemsett, LCM_frq, pmask, LCM_BM_pp[1], LCM_BM_pp[1] & pmask, LCM_BM_pt[1], LCM_prv, LCM_pprv);
   // pruning has to be here
  }
  LCM_BM_weight[1] = LCM_Ofrq[0] = 0;
  LCM_Ot[0] = LCM_Os[0];
  for ( i=1 ; i<item ; i++ )
    if ( LCM_Ofrq[i] >= LCM_th ) LCMclosed_BM_iter (i, mask, pmask);
}



/*************************************************************************/
/* LCMclosed iteration (bitmap version ) */
/* input: T:transactions(database), item:tail(current solution) */
/*************************************************************************/
void LCMclosed_BM_iter (int item, int m, int pmask){
  int mask, it = LCM_itemsett, ttt;

  LCM_frq = LCM_Ofrq[item];
  pmask &= BITMASK_31[item];
  if ( (ttt = LCM_BM_closure(item, pmask)) >0 ){
   // pruning has to be here
//    printf ("BMclo %d item%d it%d frq%d,  prv%d pprv%d::  ttt=%d,%d pmask%x\n", item, LCM_Op[item], LCM_itemsett, LCM_frq, LCM_prv, LCM_pprv, ttt,LCM_Op[ttt], pmask );
    LCM_BM_occurrence_delete (item);
    return;
  }
  LCM_iters++;
  BUF_reset ( &LCM_B );
  LCMclosed_BM_occurrence_deliver_ (item, m);
  LCM_additem ( LCM_Op[item] );
  mask = LCM_BM_rm_infreq (item, &pmask);

  LCM_solution ();
  LCMclosed_BM_recursive (item, mask, pmask);
  while ( LCM_itemsett>it ) LCM_delitem ();
  BUF_clear ( &LCM_B );
}




/***************************************************************/
/* iteration of LCM ver. 2 */
/* INPUT: T:transactions(database), item:tail of the current solution */
/*************************************************************************/
int LCMclosed_iter ( ARY *T, int item, int prv ){
  ARY TT;
  int i, ii, e, ee, n, js=LCM_jump.s, it=LCM_itemsett, mask;
  int flag=-1, perm[LCM_BM_MAXITEM], pmask = 0xffffffff;
  QUEUE_INT *q;

  LCM_jump.s = LCM_jump.t;
  LCM_iters++;
  LCM_additem ( item );
  LCM_frq = LCM_Ofrq_[item];
  LCM_prv = item;
  LCM_pprv = prv;
  
//  printf ( " Ot %d %d (%d,%d)\n", LCM_Ot[item]-LCM_Os[item], LCM_frq, item,prv);
  n = LCM_freq_calc ( T, item, LCM_Eend-1 );
  if ( prv >= 0 ) LCM_Ofrq[prv] = 0;
  LCM_Ofrq[item] = 0;
  ii = LCM_jump_rm_infreq (item);
  LCM_jumpt = LCM_jump.t;
  if ( ii > item ){
    flag = ii;
//    printf ("###clo item%d %d %d: %d\n", item, ii, LCM_Ofrq[ii], LCM_Ofrq_[ii]);
    goto END2;
  }  /* itemset is not closed */

  BUF_reset ( &LCM_B );
  LCM_partition_prefix (item);

  if ( QUEUE_LENGTH(LCM_jump)==0 ){
    LCM_Ofrq[item] = 0;
    LCMclosed_BM_occurrence_deliver_first (item, T);
    for ( i=LCM_jump.s ; i<LCM_jumpt ; i++ ) LCM_Ofrq[LCM_jump.q[i]] = 0;
    mask = LCM_BM_rm_infreq(LCM_BM_MAXITEM, &pmask);
    LCM_solution ();
    LCMclosed_BM_recursive (LCM_BM_MAXITEM, mask, pmask);
    BUF_clear ( &LCM_B );
    goto END2;
  }

  LCM_BM_occurrence_deliver_first (item, T);
  QUEUE_FE_LOOP_ ( LCM_jump, i, ii ) LCM_Ofrq[ii] = 0;
  mask = LCM_BM_rm_infreq(LCM_BM_MAXITEM, &pmask);
  LCM_solution ();
  BUF_clear ( &LCM_B );

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  
  for ( i=0 ; i<LCM_BM_MAXITEM ; i++ ) perm[i] = LCM_Op[i];
  QUEUE_FE_LOOP_ ( LCM_jump, i, ii ) LCM_Ofrq[ii] = LCM_th;
  LCM_Ofrq[item] = LCM_th;
  LCM_mk_freq_trsact(&TT, T,item,LCM_Eend-1,n+(LCM_Ot[item]-LCM_Os[item]),mask);  LCM_Ofrq[item] = 0;

  BUF_reset ( &LCM_B );
  for ( i=0 ; i<LCM_BM_MAXITEM ; i++ ) LCM_BM_occurrence_delete (i);
  LCMclosed_BM_occurrence_deliver_first (-1, &TT);
  for ( i=LCM_jump.s ; i<LCM_jumpt ; i++ ) LCM_Ofrq[LCM_jump.q[i]] = 0;
  LCMclosed_BM_recursive (LCM_BM_MAXITEM, 0xffffffff, 0xffffffff);
  BUF_clear ( &LCM_B );

  if ( QUEUE_LENGTH(LCM_jump) == 0 ) goto END0;
  q = ((QUEUE *)(TT.h))->q;
  if ( ii >= 2 && TT.num>5 ) LCM_shrink ( &TT, item, 1 );
  LCM_occ_deliver (&TT, item-1 );
  
  do {
    i = QUEUE_ext_tail_ ( &LCM_jump );
//    printf ("i=%d(%d) %d %d  :%d,%d\n", i, item, LCM_Ot[item]-LCM_Os[item], LCM_Ot[i]-LCM_Os[i], LCM_Ofrq[i], LCM_Ofrq_[i]);
    ii = LCMclosed_iter ( &TT, i, item);
    LCM_Ot[i] = LCM_Os[i];
    LCM_Ofrq_[i] = 0;
  } while ( LCM_jump.t > LCM_jump.s );

  free2 ( q );
  ARY_end ( &TT );
  END0:;
  for ( i=0 ; i<LCM_BM_MAXITEM ; i++ ) LCM_Op[i] = perm[i];
  goto END3;
  END2:;
  for ( i=LCM_jump.s ; i<LCM_jumpt ; i++ ) LCM_Ofrq[LCM_jump.q[i]] = 0;
  LCM_jump.t = LCM_jump.s;
  END3:;
  LCM_jump.s = js;
  while ( it<LCM_itemsett ) LCM_delitem ();
  return ( flag );
}

/***************************************************************/
/* main of LCM ver. 3 */
/*************************************************************************/
void LCMclosed (){
  int i;

  BUF_reset ( &LCM_B );
  LCMclosed_BM_occurrence_deliver_first (-1, &LCM_Trsact);
  for ( i=LCM_BM_MAXITEM ; i<LCM_Eend ; i++ ){
    LCM_Ofrq_[i] = LCM_Ofrq[i];
    LCM_Ofrq[i] = 0;
  }
  LCMclosed_BM_recursive (LCM_BM_MAXITEM, 0xffffffff, 0xffffffff);
  BUF_clear ( &LCM_B );

  for ( i=LCM_BM_MAXITEM ; i<LCM_Eend ; i++ ){
    LCMclosed_iter ( &LCM_Trsact, i, -1 );
    LCM_Ot[i] = LCM_Os[i];
    LCM_Ofrq_[i] = LCM_Ofrq[i] = 0;
  }
  LCM_iters++;
}

/*************************************************************************/
/*************************************************************************/
main ( int argc, char *argv[] ){
  int i;
  LCM_problem = LCM_CLOSED;
  LCM_init ( argc, argv );

  LCMclosed ();

  LCM_output ( 1 );
  LCM_end ();
  ARY_end ( &LCM_Trsact );
  exit (0);
}


#endif


