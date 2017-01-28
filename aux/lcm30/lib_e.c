

#ifndef _lib_e_c_
#define _lib_e_c_

#include<stdio.h>
#include<string.h>
#define QUEUE_INT int


/*  max, min  */
#define MAX(a,b)      ((a)>(b)?a:b)
#define MIN(a,b)      ((a)<(b)?a:b)

#define   malloc2(f,a,b,c)     if(!(f=(a *)malloc(sizeof(a)*(b)))){printf("memory error %s (%d byte)\n",c,sizeof(a)*(b));exit(1);}
#define   realloc2(f,a,b,c)     if(!(f=(a *)realloc(f,sizeof(a)*(b)))){printf("memory error, %s (%d byte)\n",c,sizeof(a)*(b));exit(1);}
#define  free2(a)      {if(a){free(a);(a)=NULL;}a=0;}

#define   fopen2r(f,a,c)     if(!(f=fopen(a,"r+"))){printf("%s:file open error %s\n",c,a);exit(1);}
#define   fopen2w(f,a,c)     if(!(f=fopen(a,"w+"))){printf("%s:file open error %s\n",c,a);exit(1);}

/* structure for bit-array */
typedef struct {
  unsigned long *a;
  int end;
} BARRAY;

/* macros for bit-array operations */
#define BARRAY_SET(A,x) ((A).a[(x)/32]|=BITMASK_1[(x)%32])
#define BARRAY_RESET(A,x) ((A).a[(x)/32]&=BITMASK_31[(x)%32])
#define BARRAY_BIT(A,x) ((A).a[(x)/32]&BITMASK_1[(x)%32])
#define BARRAY_01(A,x) (((A).a[(x)/32]&BITMASK_1[(x)%32])/BITMASK_1[(x)%32])

/* structure for integer queue */
typedef struct {
  int s;
  int t;
  int end;
  int *q;
} QUEUE;


/* swap's */
int swap_tmp_int;
double swap_tmp_double;
char swap_tmp_char;
void *swap_tmp_pointer;
#define SWAP_INT(a,b)  (swap_tmp_int=a,a=b,b=swap_tmp_int)
#define SWAP_DOUBLE(a,b)  (swap_tmp_double=a,a=b,b=swap_tmp_double)
#define SWAP_CHAR(a,b)  (swap_tmp_char=a,a=b,b=swap_tmp_char)
#define SWAP_PNT(a,b)  (swap_tmp_pointer=(void *)a,a=b,b=swap_tmp_pointer)

/* macros for QUEUE operation */
#define QUEUE_LENGTH(Q) ((Q).t-(Q).s)
#define QUEUE_F_LOOP_(Q,i)  for((i)=(Q).s;(i)<(Q).t;(i)++)
#define QUEUE_FE_LOOP_(Q,i,x)  for((i)=(Q).s,x=(Q).q[i];(i)<(Q).t;(i)++,x=(Q).q[i])
#define QUEUE_B_LOOP_(Q,i)  for((i)=(Q).t-1;(i)>=(Q).s;(i)--)
#define QUEUE_BE_LOOP_(Q,i,x)  for((i)=(Q).t-1,x=(Q).q[i];(i)>=(Q).s;(i)--,x=(Q).q[i])
#define QUEUE_RMALL(Q) ((Q).t=(Q).s)
#define QUEUE_RMALL_(Q) ((Q).t=0)


/* constants for bit mask */
int BITMASK_UPPER1[32] = { 0xffffffff, 0xfffffffe, 0xfffffffc, 0xfffffff8,
                           0xfffffff0, 0xffffffe0, 0xffffffc0, 0xffffff80,
                           0xffffff00, 0xfffffe00, 0xfffffc00, 0xfffff800,
                           0xfffff000, 0xffffe000, 0xffffc000, 0xffff8000,
                           0xffff0000, 0xfffe0000, 0xfffc0000, 0xfff80000,
                           0xfff00000, 0xffe00000, 0xffc00000, 0xff800000,
                           0xff000000, 0xfe000000, 0xfc000000, 0xf8000000,
                           0xf0000000, 0xe0000000, 0xc0000000, 0x80000000 };
int BITMASK_UPPER1_[32] = { 0xfffffffe, 0xfffffffc, 0xfffffff8, 0xfffffff0,
                            0xffffffe0, 0xffffffc0, 0xffffff80, 0xffffff00,
                            0xfffffe00, 0xfffffc00, 0xfffff800, 0xfffff000,
                            0xffffe000, 0xffffc000, 0xffff8000, 0xffff0000,
                            0xfffe0000, 0xfffc0000, 0xfff80000, 0xfff00000,
                            0xffe00000, 0xffc00000, 0xff800000, 0xff000000,
                            0xfe000000, 0xfc000000, 0xf8000000, 0xf0000000,
                            0xe0000000, 0xc0000000, 0x80000000, 0x00000000 };

int BITMASK_LOWER1[32] = { 0x00000000, 0x00000001, 0x00000003, 0x00000007,
                           0x0000000f, 0x0000001f, 0x0000003f, 0x0000007f,
                           0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff,
                           0x00000fff, 0x00001fff, 0x00003fff, 0x00007fff,
                           0x0000ffff, 0x0001ffff, 0x0003ffff, 0x0007ffff,
                           0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff,
                           0x00ffffff, 0x01ffffff, 0x03ffffff, 0x07ffffff,
                           0x0fffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff };
int BITMASK_LOWER1_[32] = { 0x00000001, 0x00000003, 0x00000007, 0x0000000f,
                            0x0000001f, 0x0000003f, 0x0000007f, 0x000000ff,
                            0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff,
                            0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff,
                            0x0001ffff, 0x0003ffff, 0x0007ffff, 0x000fffff,
                            0x001fffff, 0x003fffff, 0x007fffff, 0x00ffffff,
                            0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff,
                            0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff };

int BITMASK_1[32] = { 0x00000001, 0x00000002, 0x00000004, 0x00000008,
                      0x00000010, 0x00000020, 0x00000040, 0x00000080,
                      0x00000100, 0x00000200, 0x00000400, 0x00000800,
                      0x00001000, 0x00002000, 0x00004000, 0x00008000,
                      0x00010000, 0x00020000, 0x00040000, 0x00080000,
                      0x00100000, 0x00200000, 0x00400000, 0x00800000,
                      0x01000000, 0x02000000, 0x04000000, 0x08000000,
                      0x10000000, 0x20000000, 0x40000000, 0x80000000 };
int BITMASK_31[32] = { 0xfffffffe, 0xfffffffd, 0xfffffffb, 0xfffffff7,
                       0xffffffef, 0xffffffdf, 0xffffffbf, 0xffffff7f,
                       0xfffffeff, 0xfffffdff, 0xfffffbff, 0xfffff7ff,
                       0xffffefff, 0xffffdfff, 0xffffbfff, 0xffff7fff,
                       0xfffeffff, 0xfffdffff, 0xfffbffff, 0xfff7ffff,
                       0xffefffff, 0xffdfffff, 0xffbfffff, 0xff7fffff,
                       0xfeffffff, 0xfdffffff, 0xfbffffff, 0xf7ffffff,
                       0xefffffff, 0xdfffffff, 0xbfffffff, 0x7fffffff };


#define LCMINT int


/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/

/* allocate integer array and set value */
int *intarray_malloc_const ( int end, int c ){
  int i, *a;
  malloc2 ( a, int, end, "intarray_malloc_const");
  for ( i=0 ; i<end ; i++ ) a[i]=c;
  return (a);
}
/* quick sort of integers and its relationship function */
int qsort_int_cmp ( const void *x, const void *y ){
  if ( *((int *)x) < *((int *)y) ) return (-1);
  else return ( *((int *)x) > *((int *)y) );
}
int qsort_int_cmp_ ( const void *x, const void *y ){
  if ( *((int *)x) > *((int *)y) ) return (-1);
  else return ( *((int *)x) < *((int *)y) );
}
void qsort_int ( int *a, int siz ){
    qsort ( a, siz, sizeof(int), qsort_int_cmp );
}
void qsort_int_ ( int *a, int siz ){
    qsort ( a, siz, sizeof(int), qsort_int_cmp_ );
}
void qsort_int_struct ( int *a, int siz, int unit, int flag ){  
  if ( flag ) qsort ( a, siz, unit, qsort_int_cmp_ );
  else qsort ( a, siz, unit, qsort_int_cmp );
}
void qsort_int2 ( int *a, int siz ){
    qsort ( a, siz, sizeof(int)*2, qsort_int_cmp );
}
void qsort_int2_ ( int *a, int siz ){
    qsort ( a, siz, sizeof(int)*2, qsort_int_cmp_ );
}


/* give an identical permutation */
int *init_perm ( int end ){
  int i, *p;
  malloc2 ( p, int, end, "init_perm" );
  for ( i=0 ; i<end ; i++ ) p[i] = i;
  return ( p );
}
/* give an inverse of a permutation */
int *inverse_perm ( int *perm, int end ){
  int i, *p;
  malloc2 ( p, int, end, "inverse_perm" );
  for ( i=0 ; i<end ; i++ ) p[i] = -1;
  for ( i=0 ; i<end ; i++ )
      if ( perm[i]>=0 && perm[i]<end ) p[perm[i]] = i; 
  return ( p );
}
/* permute array of struct according to permutation */
void perm_struct ( void *a, int unit, int *perm, int siz ){
  int i;
  char *s, *ss = a;
  malloc2 ( s, char, unit*siz, "perm_struct" );
  memcpy ( s, ss, unit*siz );
  for ( i=0 ; i<siz ; i++ )
    memcpy ( ss + unit*perm[i], s + unit*i, unit );
  free ( s );
}
/* radix sort */
int *radix_sort ( void *a, int siz, int unit, int mm, int m, int *perm, int flag ){ 
  int *ll, *l, k, i, t;
  l = intarray_malloc_const ( m-mm, -1 );
  malloc2 ( ll, int, siz, "radix_sort");
  for ( i=0 ; i<siz ; i++ ){
    k = (*((int *)(((char *)a) + unit*i ))) - mm;
    ll[i] = l[k];
    l[k] = i;
  }
  if ( perm ){
    for ( k=0,i=0 ; k<m-mm ; k++ ){
      while ( l[k] >= 0 ){
        t = l[k];
        l[k] = ll[t];
        ll[t] = perm[i];
        i++;
      }
    }
    memcpy ( perm, ll, sizeof(int)*siz );
    free ( ll );
    free ( l );
    return ( perm );
  } else {
    for ( k=0,i=0 ; k<m-mm ; k++ ){
      while ( l[k] >= 0 ){
        t = l[k];
        l[k] = ll[t];
        ll[t] = i;
        i++;
      }
    }
    free ( l );
    return ( ll );
  }
}




/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/

/* print a QUEUE */
void QUEUE_print ( QUEUE *Q ){
  int i;
  for ( i=Q->s ; i<Q->t ; i++ ){
    printf ("%d,",Q->q[i]);
  }
  printf ("\n");
}
void QUEUE_print_ ( QUEUE *Q ){
  int i;
  for ( i=0 ; i<Q->t ; i++ ){
    printf ("%d,",Q->q[i]);
  }
  printf ("\n");
}
/* initialize QUEUE structure */
void QUEUE_init ( QUEUE *Q, int siz ){
  Q->s = 0;
  Q->t = 0;
  Q->end = siz+1;
  malloc2 ( Q->q, int, siz+1, "QUEUE_init");
}
/* free QUEUE */
void QUEUE_end ( QUEUE *Q ){
  if ( Q->q ){
    free ( Q->q );
    Q->q = 0;
  }
}
/* insert an element to the end of QUEUE */
void QUEUE_ins_ ( QUEUE *Q, int e ){
  Q->q[Q->t] = e;
  Q->t++;
}
QUEUE_INT QUEUE_ext_tail_ ( QUEUE *Q ){
  (Q->t)--;
  return ( Q->q[Q->t] );
}

void QUEUE_sort_ ( QUEUE *Q ){
  qsort_int ( Q->q, Q->t );
}
void QUEUE_sortr_ ( QUEUE *Q ){  /* reverse order */
  qsort_int_ ( Q->q, Q->t );
}

/* expanding array  */
typedef struct {
  void *h;
  int unit;
  int num;
  int end;
  int dellist; 
} ARY;
#define ARY_CELL(a,b,c) ((a *)((b).h+((b).unit*c)))

void ARY_exp ( ARY *A, int num ){
  if ( num >= A->end ){
    A->end = MAX(A->end*2, num+1);
    realloc2 ( A->h, char, A->unit*A->end, "ARY_exp: A->h" );
  }
  if ( A->num <= num ) A->num = num+1;
}

void ARY_inc ( ARY *A ){
  ARY_exp ( A, A->num );
}

void ARY_exp_const ( ARY *A, int num, int c ){
  int end = A->end, *x;
  void *p;
  ARY_exp ( A, num );
  for ( p=A->h ; end<A->end ; end++ ){
    x = p + A->unit*end;
    *x = c;
  }
}

void ARY_init ( ARY *A, int unit ){
  A->num = 0;
  A->end = 0;
  A->unit = unit;
  A->h = NULL;
  A->dellist = -1;
}

void ARY_end ( ARY *A ){
  free2 ( A->h );
}

void ARY_del ( ARY *A, int num ){
  *ARY_CELL(int, *A, num) = A->dellist;
  A->dellist = -num-2;
}

void ARY_rm ( ARY *A, int num ){
  if ( num<0 || num>=A->num ){
    printf ("ARY_rm: num is outside the array %d",num);
    exit(1);
  }
  A->num--;
  memcpy ( ((char *)(A->h)) + num*A->unit,
       ((char *)(A->h)) + A->num*A->unit, A->unit );
}

int ARY_new ( ARY *A ){
  int num;
  if ( A->dellist != -1 ){
    num = -A->dellist-2;
    A->dellist = *ARY_CELL (int, *A, num);
  } else {
    num = A->num;
    ARY_inc ( A );
  }
  return ( num );
}


/******   BUF   **************/

typedef struct {
  int num;
  int end;
  int unit;
  void **h;
  void *l;
  void *c;
  int siz;
  int init_siz;
} BUF;


void BUF_clear (BUF *B){
  while ( B->h[B->num] ){ free2(B->h[B->num]); B->num--; }
  B->num--;
}

void BUF_inc (BUF *B){
  B->num++;
  if ( B->num >= B->end ){
    B->end *= 2;
    realloc2 ( B->h, void*, B->end, "BUF_inc: B->h" );
  }
}

void BUF_set (BUF *B){
  BUF_inc (B);
  malloc2(B->h[B->num], void*, B->unit*B->siz, "BUF_set");
  B->c = B->h[B->num];
  B->l = B->c + B->unit*B->siz;
}

void BUF_reset (BUF *B){
  BUF_inc (B);
  B->h[B->num] = NULL;
  B->siz = B->init_siz;
  BUF_set (B);
}

void BUF_init (BUF *B, int unit, int init_siz){
  B->unit = unit;
  B->init_siz = init_siz;
  B->end = 8;
  B->num = -1;
  malloc2(B->h, void*, 8, "BUF_init");
  BUF_reset (B);
}

void BUF_end (BUF *B){
  while ( B->num>=0 ){ free2(B->h[B->num]); B->num--; }
  free2 ( B->h );
}

void *BUF_get (BUF *B, int siz){
  void *zz, *z = B->c + (siz*B->unit);
  if ( z > B->l ){
    B->siz = MAX (siz, B->siz*2);
    BUF_set (B);
    z = B->c + (siz*B->unit);
  }
  zz = B->c;
  B->c = z;
  return ( zz );
}


#endif
