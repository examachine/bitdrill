#ifndef FASTIO_C_
#define FASTIO_C_

#include"lib_e.c"

#define FASTO_SEPARATOR ' '
#define FASTI_BUFSIZ 20000
#define FASTO_BUFSIZ 20000
#define FASTI_REST 256
FILE *FASTI_fp=NULL, *FASTO_fp=NULL;
char *FASTI_buf=NULL;
char *FASTI_flag=NULL;
char *FASTO_buf=NULL, *FASTO_digit=NULL, *FASTO_mag=NULL;
char FASTO_buf2[64], *FASTO_buf2end = FASTO_buf2+32;
char *FASTO_p, *FASTO_th, *FASTI_p, *FASTI_bufend, *FASTI_bufend_;
int FASTI_bufsiz, FASTO_mmag, *FASTO_perm;
int FASTI_err;    /* 0:normal, 1:newline, 2:end of file */

/*****************************************************************/
/* initialization of FAST INPUT                                  */
/* open the file "filename" and set FASTI_fp to its file pointer */
/* initialize FASTI_p, FASTI_buf, FASTI_bufsiz                   */
/*****************************************************************/
void FASTI_init(char *filename){
  malloc2 ( FASTI_buf, char, FASTI_BUFSIZ, "FASTI_init: FASTI_buf" );
  FASTI_bufend = FASTI_buf + FASTI_BUFSIZ;
  FASTI_bufend_ = FASTI_buf + FASTI_BUFSIZ - FASTI_REST;
  FASTI_p = FASTI_bufend;
  fopen2r ( FASTI_fp, filename, "FASTI_init");
  malloc2 ( FASTI_flag, char, 256, "FASTI_init: FASTI_flag" );
  int i; for ( i=0 ; i<256 ; i++ ) FASTI_flag[i] = 127;
  for ( i='0' ; i<='9' ; i++ ) FASTI_flag[i] = i-'0';
  FASTI_flag['\n'] = 126;
}

/*****************************************************************/
/* post-processing of FAST INPUT                                 */
/* close the file pointer and free buffer                        */
/*****************************************************************/
void FASTI_end (){
  fclose ( FASTI_fp );
  free2 ( FASTI_buf );
  free2 ( FASTI_flag );
}

/*********************************************************************/
/* read a charactor from input-buffer                                */
/* RETURN: an integer                                                */
/* if EOF, set FASTI_err := 2, else not change FASTI_err             */
/* CHANGES: FASTI_p, FASTI_buf, FASTI_bufsiz                         */
/*********************************************************************/
char FASTI_char (){
  if ( FASTI_p >= FASTI_bufend ){
    FASTI_p = FASTI_buf;
    FASTI_bufend = FASTI_buf + fread (FASTI_buf, 1, FASTI_BUFSIZ, FASTI_fp);
    if ( FASTI_bufend <= FASTI_buf ){
      FASTI_err = 2;
      return (0);
    }
  }
  return (*FASTI_p++);
}


char FASTI_char_ (){
  int i = FASTI_bufend - FASTI_p;
  if ( FASTI_p >= FASTI_bufend_ ){
    if ( FASTI_bufend - FASTI_buf < FASTI_BUFSIZ-1 ){
      if ( FASTI_p >= FASTI_bufend ){
        FASTI_err = 2;
        return (0);
      }
    } else {
      memcpy ( FASTI_buf, FASTI_p, i);
      FASTI_p = FASTI_buf;
      FASTI_bufend=FASTI_buf+i+fread(FASTI_buf+i,1,FASTI_BUFSIZ-i-1,FASTI_fp);
      FASTI_bufend_ = FASTI_bufend - FASTI_REST;
    }
    *FASTI_bufend = 0;
  }
  return (*FASTI_p++);
}


/*********************************************************************/
/* read an integer from input-buffer                                 */
/*   - can't read real numbers                                       */
/*   - can't read negative number                                    */
/*   - ignore preceeding non-number charactors                       */
/*   - if the following charactor is '\n', backtrack FASTI_p (pointer)
       by one char (at the next execusion, set FASTI_err := 1        */
/* RETURN: an integer                                                */
/* FAST_err := 1: newline following, 2:eof(read no integer),
               3: newline without reading integer
               not change: otherwise                                 */
/* CHANGES: FASTI_p, FASTI_buf, FASTI_bufsiz                         */
/*********************************************************************/
int FASTI_int (){
  int item, flag =1, i;
  char ch;
  do {
    ch = FASTI_char_ ();
    if ( ch == '\n' ){ FASTI_err = 3; return (0); }
    if ( FASTI_err == 2 ) return (0);

/*
    i = FASTI_bufend - FASTI_p;
    if ( i < FASTI_REST ){
      if ( FASTI_bufend - FASTI_buf < FASTI_BUFSIZ-1 ){
        if ( FASTI_p >= FASTI_bufend ){
          FASTI_err = 2;
          return (0);
        }
      } else {
        memcpy ( FASTI_buf, FASTI_p, i);
        FASTI_p = FASTI_buf;
       FASTI_bufend=FASTI_buf+i+fread(FASTI_buf+i,1,FASTI_BUFSIZ-i-1,FASTI_fp);
      }
      *FASTI_bufend = 0;
    }
    ch = *FASTI_p++;
    if ( ch == '\n' ){ FASTI_err = 3; return (0); }
*/
  } while ( FASTI_flag[ch]==127 );

  for ( item=(int)(ch-'0') ; 1 ; item=item*10 +(int)(ch-'0') ){
    ch = FASTI_char ();
    if ( FASTI_err == 2 ) return (0);
    if ( FASTI_flag[ch]==127 ) break;
  }
  if ( ch == '\n' ) FASTI_p--;
  return ( item );
}

int FASTI_int_ (){
  int item, flag =1, i;
  char ch;
  
  do {
    if ( FASTI_p >= FASTI_bufend_ ){
      if ( FASTI_bufend - FASTI_buf < FASTI_BUFSIZ-1 ){
        if ( FASTI_p >= FASTI_bufend ){
          FASTI_err = 2;
          return (0);
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
  if ( ch == 126 ){ FASTI_err = 3; return (0); }

  for ( item=(int)ch; 1 ; item=item*10 +(int)ch ){
    ch = FASTI_flag[*FASTI_p++];
    if ( ch&64 ) break;
  }
  if ( ch == 126 ) FASTI_err = 1;
  return ( item );
}



/*****************************************************************/
/* count items and transactions in the given file                */
/* fname: filename */ 
/* RETURN: counters for items, #transactions                     */
/* set *rows to #rows (!= # of newlines)
       *deg to maximum size of rows
       *eles to #items                                           */
/*****************************************************************/
ARY FASTI_count ( char *filename, int *rows, int *eles, int *deg ){
  ARY E;
  int item, i;
char ch;

  *rows = 0;           /* # rows */
  *deg = 0;            /* max. size */
  *eles = 0;           /* # elements (items) */
  ARY_init ( &E, sizeof(int) );
  FASTI_init ( filename );

  while (1){
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
      (*eles)++;
      ARY_exp_const ( &E, item, 0 );
      ((int *)E.h)[item]++;
      if ( ch == 126 ) goto END1;
    }
    END1:;
    (*rows)++;  /* increase #transaction */
  }
  END:;
//  (*deg)++;
  FASTI_end ();
  return ( E );
}




/*****************************************************************/
/* write an integer to FASTO_buf2                                */
/* INPUT: i:integer to be written                                */
/* RETURN: # digits written                                      */
/* CHANGE: FASTO_buf2                                            */
/*****************************************************************/
char *FASTO_int_buf2 (int i){
  char *p = FASTO_buf2end, k=0;
  if ( i == 0 ){
    p--;
    *p = '0';
  } else {
    do {
      p--;
      *p = '0'+(i%10);
      i /= 10;
    } while ( i>0 );
  }
  return ( p );
}

/*****************************************************************/
/* copy an integer from FASTO_buf2 to *dest                      */
/* INPUT: dest:pointer to the position to be writen, 
          p:pointer to the beginning of the digit                */
/* RETURN: # digits written                                      */
/* CHANGE: FASTO_buf2                                            */
/*****************************************************************/
int FASTO_cpy (char *dest, char *p, char *end){
  while ( 1 ){
    *((int *)dest) = *((int *)p);
    p += sizeof(int);
    if ( p >= end ) break;
    dest += sizeof(int);
  }
}


/*****************************************************************/
/* initialization of FAST OUTPUT                                 */
/* open the file "filename" and set FASTO_fp to its file pointer */
/* initialize FASTO_p, FASTO_buf                                 */
/* set the initialize FASTO_p, FASTO_buf                         */
/*****************************************************************/
void FASTO_init (char *filename, int end){
  malloc2 ( FASTO_buf, char, FASTO_BUFSIZ, "FASTO_init: FASTO_buf" );
  fopen2w ( FASTO_fp, filename, "FASTO_init");
  FASTO_p = FASTO_buf;
  FASTO_th = FASTO_buf + (FASTO_BUFSIZ/2);

  int i, k;
  for ( FASTO_mmag=1,i=end ; i>0 ; FASTO_mmag++,i/=10 );
  malloc2 (FASTO_digit, char, FASTO_mmag*end+sizeof(int)*4, "FASTO_init: FASTO_digit");
  malloc2 (FASTO_mag, char, end, "FASTO_init: FASTO_mag");
  char *p, *pnt = FASTO_digit;

  for ( i=0 ; i<end ; i++ ){
    p = FASTO_int_buf2 ( i );
    k = FASTO_buf2end - p;
    FASTO_mag[i] = k + 1;
    FASTO_cpy (pnt, p, FASTO_buf2end);
    *(pnt+k) = FASTO_SEPARATOR;
    pnt += FASTO_mmag;
  }
}


/*****************************************************************/
/* initialization of FAST OUTPUT (permutation version)           */
/* write perm[i] when i is given                                 */
/* open the file "filename" and set FASTO_fp to its file pointer */
/* initialize FASTO_p, FASTO_buf                                 */
/* set the initialize FASTO_p, FASTO_buf                         */
/*****************************************************************/
void FASTO_init_perm (char *filename, int end){
  malloc2 ( FASTO_buf, char, FASTO_BUFSIZ, "FASTO_init_perm: FASTO_buf" );
  fopen2w ( FASTO_fp, filename, "FASTO_init");
  FASTO_p = FASTO_buf;
  FASTO_th = FASTO_buf + (FASTO_BUFSIZ/2);

  int i, k;
  for ( k=i=0 ; i<end ; i++ ) if ( k<FASTO_perm[i] ) k=FASTO_perm[i];
  for ( FASTO_mmag=1 ; k>0 ; FASTO_mmag++,k/=10 );
  malloc2 (FASTO_digit, char, FASTO_mmag*end+sizeof(int)*4, "FASTO_init_perm: FASTO_digit");
  malloc2 (FASTO_mag, char, end, "FASTO_init_perm: FASTO_mag");
  char *p, *pnt = FASTO_digit;

  for ( i=0 ; i<end ; i++ ){  /* write digits to memory */
    p = FASTO_int_buf2 ( FASTO_perm[i] );
    k = FASTO_buf2end - p;
    FASTO_mag[i] = k + 1;
    FASTO_cpy (pnt, p, FASTO_buf2end);
    *(pnt+k) = FASTO_SEPARATOR;
    pnt += FASTO_mmag;
  }
}

/*********************************************************************/
/* flush buffer to output file                                       */
/* CHANGE: FASTO_p := FASTO_buf                                      */
/* FASTO_flush_ flushes if wrote buffer size is larger than FASTO_th */
/*********************************************************************/
void FASTO_flush (){
  fwrite ( FASTO_buf, 1, ((int)(FASTO_p-FASTO_buf)), FASTO_fp );
  FASTO_p = FASTO_buf;
}
void FASTO_flush_ (){
  if ( FASTO_p > FASTO_th ) FASTO_flush ();
}

/*****************************************************************/
/* post-processing of FAST OUTPUT                                */
/* close the file pointer and free buffer                        */
/*****************************************************************/
void FASTO_end (){
  FASTO_flush ();
  fclose ( FASTO_fp );
  free2 ( FASTO_buf );
  free2 ( FASTO_mag );
  free2 ( FASTO_digit );
}

/*****************************************************************/
/* write a charactor to output-file buffer                       */
/* CHANGE: FASTO_p += 1                                          */
/*****************************************************************/
void FASTO_char ( char ch ){
  *FASTO_p = ch;
  FASTO_p++;
}

/*****************************************************************/
/* write an integer to output buffer                             */
/* CHANGE: FASTO_p += FASTO_mag[i]                               */
/*****************************************************************/
void FASTO_int ( int i ){
  char *p = &FASTO_digit[i*FASTO_mmag];
  FASTO_cpy (FASTO_p, p, p+FASTO_mag[i]);
  FASTO_p += FASTO_mag[i];
}
void FASTO_int_ ( int i ){
  long long *j = (long long *)(&FASTO_digit[i*FASTO_mmag]);
  *((long long *)FASTO_p) = *j;
  FASTO_p += FASTO_mag[i];
}
void FASTO_int_large ( int i ){
  char *p = FASTO_int_buf2 ( i );
  int k = FASTO_buf2end - p;
  FASTO_cpy (FASTO_p, p, FASTO_buf2end);
  FASTO_p += k;
  *(FASTO_p) = FASTO_SEPARATOR;
  FASTO_p++;
}

/*****************************************************************/
/* write integers in a QUEUE to output buffer, and flush         */
/* CHANGE: FASTO_p, FASTO_buf                                    */
/* FASTO_QUEUEn:without newline, FASTO_QUEUE:with newline        */
/*****************************************************************/
void FASTO_QUEUEn (QUEUE *Q){
  int i, e;
  QUEUE_FE_LOOP_ (*Q, i, e) FASTO_int ( e );
}

void FASTO_QUEUE (QUEUE *Q){
  int i, e;
  QUEUE_FE_LOOP_ (*Q, i, e) FASTO_int ( e );
  FASTO_p--;
  FASTO_char ('\n');
}

void FASTO_QUEUE_perm (QUEUE *Q, int *perm){
  int i, e;
  QUEUE_FE_LOOP_ (*Q, i, e) FASTO_int ( perm[e] );
}

#endif
