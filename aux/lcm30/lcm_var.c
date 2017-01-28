


#ifndef _lcm_var_c_
#define _lcm_var_c_

/**********************************************************************/
/*  Global Variables                                                  */
/**********************************************************************/

time_t LCM_start_time;
int LCM_maximum_time = 1000;

int **LCM_Ot, **LCM_Os, *LCM_Ofrq, *LCM_Op, *LCM_Ofrq_;
int LCM_th;  /* support */
int LCM_frq; /* frequency of the current itemset */
int LCM_Eend;  /* #frequent items */
int LCM_trsact_num;   /* #transactions in original file */
int *LCM_ary; /* array of size (#frequent item), for temporary uses */
int *LCM_prune_ary; /* array to recover marks for pruning */
int LCM_iters = 0;  /* counting #iterations */
long *LCM_shrink_p;   /* working space for shrinking */
QUEUE LCM_shrink_jump;  /* working space for shrinking */
QUEUE LCM_jump, LCM_add, LCM_prune;
QUEUE LCM_itemset, LCM_Qtmp;  // needless?
int LCM_itemsett, LCM_jumpt, LCM_prv, LCM_pprv;
char **LCM_itemsetp, *LCM_itemsetbuf, *LCM_p;
QUEUE_INT *LCM_buf;  /* main buffer for storing items */
ARY LCM_Trsact; /* original transactions */
int *LCM_sc;    /* # of itemsets of size i, and max.size of itemsets */

  /* printflag&1:output to file &2:print messages &4 print solutions */
int LCM_problem = 0, LCM_method=0;
#define LCM_FREQSET 1
#define LCM_CLOSED  2
#define LCM_MAXIMAL 4

#ifdef LCM_MESSAGES
int LCM_print_flag=2;
#else 
int LCM_print_flag=0;
#endif


#endif

