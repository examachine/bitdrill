/*************************************************************************/
/* INPUT: O:occurrence, jump:items, th:support, frq:frequency, add:itemset
   OUTPUT: maximum item i s.t. frq(i)=frq
   OPERATION: remove infrequent items from jump, and 
    insert items i to "add" s.t. frq(i)=frq                              */
/*************************************************************************/
int LCM_jump_rm_infreq (int item){
  QUEUE_INT *x, *xx, *e=LCM_jump.q+LCM_jump.t;
  int flag=-1;
  for ( x=xx=(LCM_jump.q+LCM_jump.s) ; x!=e ; x++ ){
    if ( LCM_Ofrq[*x] >= LCM_th ){
      if ( LCM_Ofrq[*x] < LCM_frq ){
        if ( x != xx ) *xx = *x;
        xx++;
        if ( LCM_PROBLEM != LCM_FREQSET ){
          if ( *x<item ) LCM_Ofrq_[*x] = LCM_Ofrq[*x];
        }
        continue;
      } else {
        if ( LCM_PROBLEM == LCM_FREQSET ) QUEUE_ins_ ( &LCM_add, *x );
        else {
          if ( *x < item ) LCM_additem ( *x );
          else if ( *x > flag ) flag = *x;
        }
      }
    }
    LCM_Ofrq[*x] = 0;
  }
  LCM_jump.t = xx-LCM_jump.q;
  return ( flag );
}

/*************************************************************************/
/* compute the frequency by occurrence deliver , for LCMmax */
/* INPUT: T:transactions, occ:occurences, max_item.
   OPERATION:
   1. for each item i<=max_item, 
        set O->h[i] to the list of transactions in occ including i,
   2. set jump to the list of items i s.t. at least one occurrence of occ 
      includes i.   */
/*************************************************************************/
int LCM_freq_calc (ARY *T, int item, int max_item){
  QUEUE_INT *x, *jq=&(LCM_jump.q[LCM_jump.t]);
  int ee, n=0, *t, *t_end;
  QUEUE *Q = T->h;
  if ( T->num < LCM_Eend ){
    for ( t=LCM_Os[item],t_end=LCM_Ot[item] ; t<t_end ; t++ ){
      ee=Q[*t].end;    /* multiplicity of transaction t */
      for ( x=Q[*t].q ; *x<=max_item ; x++ ){
        if ( LCM_Ofrq[*x] == 0 ){ *jq=*x; jq++; }
        LCM_Ofrq[*x] += ee; n++;
      }
    }
  } else {
    for ( t=LCM_Os[item],t_end=LCM_Ot[item] ; t<t_end ; t++ ){
      ee=Q[*t].end;    /* multiplicity of transaction t */
      for ( x=Q[*t].q ; *x<=max_item ; x++ ){
        LCM_Ofrq[*x] += ee; n++;
      }
    }
    for ( ee=max_item ; ee>=0 ; ee-- ){
      if ( LCM_Ofrq[ee] > 0 ) { *jq = ee; jq++; }
    }
  }
  LCM_jump.t = jq-LCM_jump.q;
  return ( n );
}


/*************************************************************************/
/* remove large items from jump and sort in increasing order
     INPUT: jump, max_item
     OPERATION: 1. for each item i>max_item in jump, remove i from jump
     2. set occ[e].s = occ[e].end for each i<max_item (preserve frequency)
     3. set occ[i].end = 0 for each i in jump
     4. sort items in jump (from jump.s to jump.t) in the increasing order */
/*************************************************************************/
void LCM_partition_prefix (int item){
  int i, e;
  for ( i=LCM_jump.s ; i<LCM_jump.t ; i++ ){
    e = LCM_jump.q[i];
    if ( e>item ){
      LCM_jump.t--;
      LCM_jump.q[i] = LCM_jump.q[LCM_jump.t];
      LCM_jump.q[LCM_jump.t] = e;
      i--;
    }
  }
  QUEUE_sortr_ ( &LCM_jump );
  qsort_int_ ( LCM_jump.q+LCM_jump.t, LCM_jumpt - LCM_jump.t );
}


/*************************************************************************/
/* delete marks for pruning of non-closed itemsets 
   (items not making recursive call) */
/*************************************************************************/
void LCMclosed_pruning_del ( int tail, int upper_th ){
  while ( tail < LCM_Qtmp.t ){
    if ( LCM_Qtmp.q[LCM_Eend+LCM_Qtmp.q[LCM_Qtmp.t-1]] > upper_th ) break;
    LCM_Qtmp.t--;
    LCM_Qtmp.q[LCM_Eend+LCM_Qtmp.q[LCM_Qtmp.t]] = -1;
  }
}

/*************************************************************************/
/* set mark for pruning  of non-closed itemsets  
   (items not making recursive call) */
/*************************************************************************/
void LCMclosed_pruning_set ( int item, int mark_item, int qt ){
  int j = LCM_Qtmp.t;
  if ( mark_item < 0 ) return;
  LCM_Qtmp.t++;
  while ( qt < j ){
    if ( LCM_Qtmp.q[LCM_Eend+LCM_Qtmp.q[j-1]] >= mark_item ) break;
    LCM_Qtmp.q[j] = LCM_Qtmp.q[j-1];
    j--;
  }
  LCM_Qtmp.q[LCM_Eend+item] = mark_item;
  LCM_Qtmp.q[j] = item;
}

