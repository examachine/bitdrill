#ifndef _lcm_io_c_
#define _lcm_io_c_

#include"fastio.c"
#include"lcm_bm.c"
#include"lcm_var.c"

/**************************************************************/
/* load transaction database from file                        */
/* INPUT: filename:filename, th:minimum support               */
/* RETURN: sum of #items over all transaction                 */
/* set LCM_Trsact to the transactions loaded from file        */
/*     FASTO_perm to permutation of items ( sorted by frq )   */
/* set LCM_Eend to #frq items (maximum index of item +1 )     */
/**************************************************************/
int LCM_BM_load ( char *filename ){
  int e, i, bnum, item, transaction_num, m, zz; /* max item, sum of #items */
  QUEUE_INT *ibuf, *b;  /* buffer for storing all items */
  char ch;
    /* first pass of input file. compute the frequency of each item, 
       and compute the re-numbers of items in the file.
       the re-numbered number of infrequent items are set to -1 */
  ARY E = FASTI_count ( filename, &LCM_trsact_num, &bnum, &m );
  if ( LCM_print_flag &2 )
     printf ("database: #transactions=%d, #item=%d #elements=%d\n", LCM_trsact_num, E.num, bnum );
  int *Eq = E.h;
  FASTO_perm = TRSACT_sort_item_by_frq ( &E, LCM_th, &bnum, &LCM_Eend );

    /* compute adequite value for LCM_BM_MAXITEM
       (working space is not larger than transaction database */
  for (item=0,bnum=0 ; item<LCM_Eend ; item++)
       bnum += Eq[FASTO_perm[item]];
//  if ( LCM_print_flag &2 ) printf ("Memory(#int): Original %d ", bnum*2 );
  long long z=-1, mm = LCM_PROBLEM==LCM_FREQSET? 2: 5;
  for ( item=0 ; item<(MIN(LCM_Eend,32)) ; item++ ){
    if ( mm+bnum*2<z || z==-1 ){
      z = mm + bnum*2;
      zz=item+1;
    }
    bnum -= Eq[FASTO_perm[item]];
    mm *= 2;
  }
  LCM_BM_MAXITEM = MAX(zz,12);
  if ( LCM_print_flag &2 )
     printf ("Reduced %d, #items in BitMap= %d %d\n", z*2, LCM_BM_MAXITEM);
  for ( bnum=item=0 ; item<LCM_Eend ; item++ ){
    if ( Eq[FASTO_perm[item]] < LCM_th ) Eq[FASTO_perm[item]] = -1;
      /* if item FASTO_perm[item] is infrequent, set Eq[FASTO_perm[item]]=-1 */
    else if ( item >= LCM_BM_MAXITEM ) bnum += Eq[FASTO_perm[item]]; /* count up the size of transaction database */
    Eq[FASTO_perm[item]] = item;     /* if item FASTO_perm[item] is frequnet set Eq[FASTO_perm[item]] to the re-numbered item */
  }
  
    /* initialization of variables */
  FASTI_init(filename);
  ARY_init ( &LCM_Trsact, sizeof(QUEUE) );
  ARY_exp ( &LCM_Trsact, LCM_trsact_num+2 );
  QUEUE *Q = LCM_Trsact.h, *QQ = Q;
  malloc2 ( ibuf, QUEUE_INT, bnum+LCM_trsact_num+4, "TRSACT_load: ibuf");

    /* read file and store frequent items in memory */
  LCM_Trsact.num = 0;

  while (1){
    b = Q->q = ibuf;
    Q->s = 0;      /* bitmap for header items */
    Q->end = 1;    /* multiplicity */
    START:;
    while (1){
    
    
       /* read an integer to variable "item" */
      do {
    if ( FASTI_p >= FASTI_bufend_ ){
      if ( FASTI_bufend - FASTI_buf < FASTI_BUFSIZ-1 ){
        if ( FASTI_p >= FASTI_bufend ){
          goto END;
        }
      } else {
        i = FASTI_bufend - FASTI_p;
        memcpy ( FASTI_buf, FASTI_p, i);
        FASTI_p = FASTI_buf;
       FASTI_bufend=FASTI_buf+i+fread(FASTI_buf+i,1,FASTI_BUFSIZ-i-1,FASTI_fp);
        FASTI_bufend_ = FASTI_bufend - FASTI_REST;
      }
      *FASTI_bufend = 0;
    }
    ch = FASTI_flag[*FASTI_p++];
    } while ( ch==127 );
    if ( ch == 126 ){ goto END1; }

  for ( item=(int)ch; 1 ; item=item*10 +(int)ch ){
    ch = FASTI_flag[*FASTI_p++];
    if ( ch&64 ) break;
  }
    /* end of "read an item" */

      if ( (e=Eq[item]) >= 0 ){
        if ( e<LCM_BM_MAXITEM ){
          Q->s |= BITMASK_1[e];
        } else {
          *ibuf = e;
          ibuf++;
        }
      }
      if ( ch == 126 ) goto END1;
    }
    END1:;
    if ( (e = ibuf-b) == 0 ){
      if ( Q->s == 0 ) goto START;    /* no item is inserted */
      Q->t = 0;
    } else {
      Q->t = e; /* #items in this transaction */
      if ( Q->t > 1 ){
        qsort_int ( b, Q->t );
        for ( item=0,i=1 ; i<Q->t ; i++ ){
          if ( b[item] != b[i] ){
            item++;
            if ( item < i ) b[item]=b[i];
          }
#ifdef LCM_MESSAGES
 else printf ("duplicated item %d in %dth row\n", item, LCM_Trsact.num);
#endif
        }
        Q->t = item+1;
      }
    }
    *ibuf = LCM_Eend;   /* loop stopper */
    LCM_Trsact.num++;
    ibuf++;
    Q++;
  }
  END:;
  FASTI_end ();
  ARY_end ( &E );
  return ( bnum );
}


/*****************************************************************************/
/* Output routines, for given queue of items, output "permutated items"      */
/* In the computation, items are re-numbered so that computation become fast */
/* When output an itemset, the re-numbered items are numbered according to   */
/* "FASTO_perm" array to get the original number                             */
/*****************************************************************************/

void LCM_additem (int item){
  if ( LCM_print_flag&1 ){
    LCM_itemsetp[LCM_itemsett] = LCM_p;
    char *p = &FASTO_digit[item*FASTO_mmag];
    FASTO_cpy (LCM_p, p, p+FASTO_mag[item]);
    LCM_p += FASTO_mag[item];
  }
  LCM_itemsett++;
}
void LCM_delitem (){
  LCM_itemsett--;
  if ( LCM_print_flag&1 ) LCM_p = LCM_itemsetp[LCM_itemsett];
}

void LCM_output_itemset (){
  int *t;
  FASTO_cpy ( FASTO_p, LCM_itemsetbuf, LCM_p );
  FASTO_p += LCM_p-LCM_itemsetbuf;
  FASTO_char ('(');
  FASTO_int_large ( LCM_frq );
  FASTO_p--;
  FASTO_char (')');
  FASTO_char ('\n');
  FASTO_flush_ ();
}

/* process when find a solution */
void LCM_solution (){
#ifdef LCM_OVERTIME_END
  if ( time(NULL)-LCM_start_time > LCM_maximum_time ){
    printf ("time over\n"); exit(1); }
#endif
  LCM_sc[LCM_itemsett]++;
  if ( LCM_print_flag&1 ) LCM_output_itemset ();
}


/* iteration of output solutions (for LCM_freq) */
void LCMFREQ_output_itemset ( int *q ){
  LCM_output_itemset ();
  for ( q-- ; q>=LCM_add.q ; q-- ){
    LCM_additem ( *q );
    LCMFREQ_output_itemset ( q );
    LCM_delitem ();
  }
}

/* process when find a solution (for LCM_freq) */
void LCMFREQ_solution (){
  if ( LCM_print_flag&1 ) LCMFREQ_output_itemset ( LCM_add.q+LCM_add.t );
//  int i, cc=1, s = LCM_itemset.t;
  int i, cc=1, s = LCM_itemsett;
  for ( i=0 ; i<=LCM_add.t ; i++ ){
    LCM_sc[s] += cc;
    cc = cc*(LCM_add.t - i) / (i+1);
    s++;
  }
}

/*******************************************************************/
/* add an item to the cullent solution and print it (for LCM_freq) */
/* input: item:item to be added, frq:frequency of obtained itemset */
/*******************************************************************/
void LCM_print_last ( int item, int frq ){
  LCM_iters++;
  LCM_frq = frq;
  LCM_additem ( item );
  LCMFREQ_solution ();
  LCM_delitem ();
}

/*******************************************************************/
/* output at the termination of the algorithm */
/* print #of itemsets of size k, for each k */
/* flag=1:all,closed,   0:maximal */
/*******************************************************************/
void LCM_output (){
  int i, n, ii;
  LCM_sc[0] = LCM_problem==LCM_MAXIMAL? 0: 1;
  for ( n=i=0 ; i<=LCM_Eend ; i++ ){
    n += LCM_sc[i];
    if ( LCM_sc[i] != 0 ) ii = i;
  }
  printf ("%d\n", n);
  for ( i=0 ; i<=ii ; i++ ) printf ("%d\n", LCM_sc[i] );
  if ( LCM_PROBLEM != LCM_MAXIMAL ){
    if (LCM_print_flag&1 ){
      FASTO_char ('(');
      FASTO_int_large ( LCM_trsact_num );
      FASTO_p--;
      FASTO_char (')');
      FASTO_char ('\n');
    }
  }
  if ( LCM_print_flag&2 ){
    printf ("iters=%d\n", LCM_iters);
    if ( LCM_PROBLEM == LCM_FREQSET )
        printf ("compress ratio = %f\n", ((double)n)/LCM_iters);
  }
}


#endif
