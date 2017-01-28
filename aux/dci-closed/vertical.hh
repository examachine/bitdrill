// Copyright (C) 2003 salvatore orlando <salvatore.orlando@unive.it>
// University of Venice, Ca' Foscari, Dipartimento di Informatica, (VE) Italy
// Istituto di Scienza e Tecnologia dell'Informazione, ISTI - CNR, (PI) Italy
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software 
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef __VERTICAL_H
#define __VERTICAL_H

#include "transaction.hh"
#include "tid_list_macros.hh"
#include <iomanip>
#include <pthread.h>

using namespace std;

#include "my_defs.hh"
#include "print.hh"
#include "memory.hh"
#include "utils.hh"

extern Chronos GLOBAL_CLOCK;


inline unsigned char mask_byte(int k) {
  return (unsigned char) 1 << k;
}

template <class T, class T1> class DCI_VD;
template <class T, class T1> class Master;
template <class T, class T1> class Worker;
template <class T, class T1> class Policy;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~ Vertical representation
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <class T, class T1>
class DCI_VD {
public:
  DCI_VD( T1 min,                  // min supp
	  T1* s,                   // items supports
	  T m,                     // no. of items
	  T1 n_trans,              // no. of transactions
	  T conditional_ub,        // largest prefix item
	  T* conditional_unmap);   // from partition to initial mapping

  ~DCI_VD();

  inline void write_t_in_VD (T1 n_tr, dci_transaction<T>& t);
  inline void write_t_in_VD (T1 n_tr, Transaction<T>& t);

  void find_section_equal   ();
  void reorder_bits_compact (tid_list *equal);

  inline bool belongs_to_SE(T elem) {
    if (elem >=  start_sect_eq) 
      return true;
    else
      return false;	
  }


  DCI_VD<T,T1>* CreateNewDataset(T item);

  int masked_shrink (	unsigned char* dest,
			unsigned char* src,
			unsigned char* mask,
			int* shifted,
			unsigned int n0,
			unsigned int n1);

  T1             min_count;
  T1*            singletons_support;
  void*          aux_VD;
  tid_list*      VD;
  unsigned int   VD_size;
  T1             tid_list_size;
  T              VD_rows;
  T1             VD_trans;
  T1             ub;
  FILE*          out_file;
  T*             unmap;

  // stores boundaries of the EQ section: used only for dense
  T start_sect_eq;
  T end_sect_eq;
  T1 sz_sect_eq;
  T1 sz_tail;
  // store the count for the EQ section: used only for dense
  T1 n_bit_set_eq;
  // store the current count for the candidate: used only for dense
  T1 num_bit_set_init; 

};

template <class T, class T1>
DCI_VD<T,T1>::DCI_VD( 
		     T1 min,                  // min supp
		     T1* s,                   // items supports
		     T m,                     // no. of items
		     T1 n_trans,              // no. of transactions
		     T conditional_ub,        // largest prefix item
		     T* conditional_unmap)    // from partition to initial mapping
{
  if (m<2)
    fatal_error("There are less than 2 items!");

  singletons_support = s;
  aux_VD             = NULL;
  min_count          = min;
  ub                 = conditional_ub;
  unmap              = conditional_unmap;
  
  VD_trans = n_trans;
  tid_list_size = n_trans / (sizeof(tid_list) * 8);  
  if ((n_trans % (sizeof(tid_list) * 8)) != 0)
    tid_list_size++;
  VD_rows = m;
  VD_size = VD_rows * tid_list_size * sizeof(tid_list);
  
  aligned16_malloc (VD, aux_VD, tid_list_size*VD_rows*sizeof(tid_list));
  if (!aux_VD) fatal_error("Could not allocate memory for storing the vertical bitmap.");

  //bzero(VD,VD_rows*tid_list_size*sizeof(tid_list));
  memset(VD,0x0,VD_rows*tid_list_size*sizeof(tid_list));
}

template <class T, class T1>
DCI_VD<T,T1>::~DCI_VD() {
  if (aux_VD != NULL)
    free( aux_VD );
}

template <class T,class T1>  
inline void DCI_VD<T,T1>::write_t_in_VD ( T1 n_tr, 
					  dci_transaction<T>& t) {
#ifdef MY_SET_BIT
  for (int i=0; i<(int) t.t_len; i++) 
    set_bit((void*)&VD[tid_list_size * t.elements[i]],n_tr);
#else
  T1 byte_sel;
  T1 displ_sel;
  unsigned char *p;

  byte_sel = n_tr / 8;
  displ_sel = n_tr % 8;

  for (int i=0; i<(int) t.t_len; i++) {
    p = (unsigned char *) &VD[tid_list_size * t.elements[i]];
    p[byte_sel] = (p[byte_sel] | (mask_byte(displ_sel)));
  }
#endif
}
  
template <class T,class T1>  
inline void DCI_VD<T,T1>::write_t_in_VD ( T1 n_tr, 
					  Transaction<T>& t) {
#ifdef MY_SET_BIT
  for (int i=0; i<(int) t.length; i++) 
    my_set_bit(n_tr, (void*)&VD[tid_list_size * t.t[i]]);
#else
  T1 byte_sel;
  T1 displ_sel;
  unsigned char *p;

  byte_sel = n_tr / 8;
  displ_sel = n_tr % 8;

  for (int i=0; i<(int) t.length; i++) {
    p = (unsigned char *) &VD[tid_list_size * t.t[i]];
    p[byte_sel] = (p[byte_sel] | mask_byte(displ_sel));
  }
#endif
}

template <class T, class T1>
void DCI_VD<T, T1>::find_section_equal()
{
  T start, end;
  int cc=0, perc=0;

  void* aux_m, *aux_r;
  tid_list *mask, *last_row, *result;

  float perc_eq_limit[] = { 95.0, 90.0, 85.0, 80.0, 75.0};
  int max_perc_ind = sizeof(perc_eq_limit) / sizeof(float);
  int curr_perc = 0;

  int curr_perc_OK = 0;
  
  T start_OK, perc_OK;

  aligned16_malloc (mask, aux_m, tid_list_size*sizeof(tid_list));
  if (!aux_m) fatal_error("Could not allocate memory in chk_compact_vertical.");

  aligned16_malloc (result, aux_r, tid_list_size*sizeof(tid_list));
  if (!aux_r) fatal_error("Could not allocate memory in chk_compact_vertical.");

  for (T1 i=0; i<tid_list_size; i++)
    mask[i].f[0] = mask[i].f[1] = mask[i].f[2] = mask[i].f[3]= 0xffffffff;
  
  end = VD_rows;
  
  last_row = &VD[(end-1) * tid_list_size];

  start_OK = end-1;
  perc_OK = 0;
  start = end-2;

  while(start < end) {
    // ----------------------------------------------------
    // find the percentage of identical elements in 
    // the last start-end tidlists (most frequent items).
    float perc_tmp;
    int cc_tmp;
    //bzero(result, tid_list_size*sizeof(tid_list)); 
    memset(result, 0x0, tid_list_size*sizeof(tid_list)); 

    tid_list* tmp_vd = &(VD[start * tid_list_size]);	
    for (T1 i=0; i<tid_list_size; i++) {
      for (unsigned int jj=0; jj<4; jj++) {
	result[i].f[jj] = ~( tmp_vd[i].f[jj] ^ last_row[i].f[jj]);
	result[i].f[jj] = (result[i].f[jj] & mask[i].f[jj]);
      }
    }

    cc_tmp = count_1_bits( result, tid_list_size );

    perc_tmp =  100.0 * (float) cc_tmp / 
      (float) (tid_list_size*sizeof(tid_list)*8);
    // ----------------------------------------------------
    
    bool found = false;

    // ----------------------------------------------------
    // now test all percentage range limits 
    // given in perc_eq_limit
    // ----------------------------------------------------
    while (curr_perc < max_perc_ind) {

      if (perc_tmp >= perc_eq_limit[curr_perc]) {
	// we found the percentage range
	// we store the partial result in mask
	// and go on with another item
	memcpy((void *) mask, (void *) result, tid_list_size*sizeof(tid_list));
	cc=cc_tmp;

	perc_OK = (int) perc_tmp;
	start_OK = start;

	found = true;
	curr_perc_OK = curr_perc;

	start--;
	break;
      } else if ((end-start) < VD_rows/6)
	// percentage is lower
	// we consider another range if the number of 
	// tidlists considered so far is not too big
	curr_perc++;
      else
	// percentage is low and there are no more enough 
	// items to consider. 
	break;
    }
    
    // we know which is the percentage range of superposition
    // among tidlists. if there are other items to consider 
    // we go on, otherwise we break the loop
    if (!found)
      break;
    

  }

  start = start_OK;
  perc = perc_OK;

  int limit_num_freq_eq;
  if (perc > 80)
    limit_num_freq_eq = VD_rows/7; 
  else if (perc > 75)
    limit_num_freq_eq = VD_rows/6;
  else if (perc > 70)
    limit_num_freq_eq = VD_rows/5;
  else if (perc > 65)
    limit_num_freq_eq = VD_rows/4;
  else if (perc > 60)
    limit_num_freq_eq = VD_rows/3;
  else 
    limit_num_freq_eq = VD_rows/2;

  start_sect_eq=start;
  end_sect_eq=end;
  sz_sect_eq = cc / (sizeof(tid_list)*8);

#ifdef MY_STATS
  DW = tid_list_size;
  DH = end_sect_eq;
  SEW = sz_sect_eq;
  SEH = start_sect_eq;
#endif

  if (end_sect_eq-start_sect_eq>1)
    reorder_bits_compact(mask);
  else {
    n_bit_set_eq=0;
    sz_sect_eq = 0; 
    start_sect_eq = end_sect_eq;

#ifdef MY_STATS
    SEW = sz_sect_eq;
    SEH = start_sect_eq;
#endif
  }

  sz_tail       = tid_list_size-sz_sect_eq;
  
  free(aux_m);
  free(aux_r);
}

template <class T,class T1>
void DCI_VD<T,T1>::reorder_bits_compact(tid_list *equal)	 
{
  tid_list    *new_list;
  void* aux_n;

  aligned16_malloc (new_list, aux_n, tid_list_size*end_sect_eq*sizeof(tid_list));
  if (!aux_n) fatal_error(NULL);

  unsigned char   *eq;
  eq = (unsigned char *) equal;

  for (T i = 0; i < VD_rows; i++) {
    unsigned char   *o;
    unsigned char   *n;
    o = (unsigned char *) &VD[i * tid_list_size];
    n = (unsigned char *) new_list;
    //bzero(n, tid_list_size * sizeof(tid_list));
    memset(n, 0x0, tid_list_size * sizeof(tid_list));

    int tid=0;

    for (T1 j = 0; j < tid_list_size * sizeof(tid_list); j++)
      for (unsigned int k = 0; k < 8; k++) {
	if ((eq[j] & mask_byte(k)) != 0) {
	  if ((o[j] & mask_byte(k)) != 0) {
	    int i_b, i_B;
				    
	    i_B = tid / 8;
	    i_b = tid % 8;
	    n[i_B] = (n[i_B] | (0x1 << i_b));
	  }
	  tid++;
	}
      }


    for (T1 j = 0; j < tid_list_size * sizeof(tid_list); j++)
      for (unsigned int k = 0; k < 8; k++) {
	if ((eq[j] & mask_byte(k)) == 0) {
	  if ((o[j] & mask_byte(k)) != 0) {
	    int i_b, i_B;
	    i_B = tid / 8;
	    i_b = tid % 8;
	    n[i_B] = (n[i_B] | (0x1 << i_b));
	  }
	  tid++;
	}
      }

    memcpy(o, n, tid_list_size * sizeof(tid_list));
  }

  tid_list *pp = VD + start_sect_eq * tid_list_size;

  n_bit_set_eq = count_1_bits(pp, sz_sect_eq);
  free(aux_n);
}

template <class T,class T1> 
DCI_VD<T,T1>* DCI_VD<T,T1>::CreateNewDataset(T item){
  // INIT
  DCI_VD<T,T1>* new_Data = new DCI_VD<T,T1>(*this);
  if (!new_Data) fatal_error("Could not allocate projected dataset object.");

  new_Data->tid_list_size = singletons_support[item]/sizeof(tid_list)/8+1;
  aligned16_malloc (new_Data->VD, new_Data->aux_VD, new_Data->tid_list_size * end_sect_eq*sizeof(tid_list));
  if (!new_Data->aux_VD) fatal_error("Could not allocate projected dataset bitmap.");

  int* shifted;
  unsigned int shifted_i, shifted_val;
  unsigned char* mask;
  tid_list* vd_pointer;
  unsigned char *dest, *src;

  // --------------------------------------
  // PROJECTION
  // --------------------------------------
  shifted     = new int [tid_list_size*sizeof(tid_list)*8];
  if (!shifted) fatal_error(NULL);
  shifted_i   = 0;
  shifted_val = 0;
  mask = (unsigned char*) (VD+item*tid_list_size);
  // build shiftings
  // for each byte transaction
  for (T1 i=0; i<tid_list_size*sizeof(tid_list); i++) {
    if (i==sz_sect_eq*sizeof(tid_list)) {
      // store the new shifted position
      new_Data->sz_sect_eq = (shifted_val)/sizeof(tid_list)/8;
      new_Data->sz_tail    = new_Data->tid_list_size - new_Data->sz_sect_eq;
    }
    // if the mask_byte != 0
    if (mask[i] != 0) {
      // for each bit
      for (unsigned int b=0; b<8; b++) {
	if ((mask[i] & mask_byte(b)) != 0) {
	  // if the bit is 1 update store the new position
	  shifted[shifted_i++] = shifted_val++;
	} else {
	  // if the bit is 0 than it has to be skipped
	  shifted[shifted_i++] = -1;
	}
      }
    } else {
      //if the mask byte is 0 than we have 8 bit to skip
      for (unsigned int b=0; b<8; b++) {
	shifted[shifted_i++] = -1;
      }
    }
  }


  if (tid_list_size == sz_sect_eq) {
      // store the new shifted position
      new_Data->sz_sect_eq    = new_Data->tid_list_size;
      new_Data->sz_tail       = new_Data->tid_list_size - new_Data->sz_sect_eq;
  }

  // PROJECT
  vd_pointer = new_Data->VD;
  dest       = (unsigned char*) vd_pointer;

  for (T1 i=0; i<end_sect_eq; i++) {
    src	= (unsigned char*) (VD+i*tid_list_size);

    masked_shrink(dest, src, mask, shifted, tid_list_size*sizeof(tid_list), new_Data->tid_list_size*sizeof(tid_list));

    vd_pointer += new_Data->tid_list_size;
    dest = (unsigned char*) vd_pointer;
  }

  if (start_sect_eq != end_sect_eq)
    new_Data->n_bit_set_eq = 
      count_1_bits(new_Data->VD+new_Data->start_sect_eq*new_Data->tid_list_size, new_Data->sz_sect_eq);
  else 
    new_Data->n_bit_set_eq = 0;

  delete [] shifted;
  return new_Data;
}

template <class T, class T1> 
int DCI_VD<T,T1>::masked_shrink ( unsigned char* dest,
				  unsigned char* src,
				  unsigned char* mask,
				  int* shifted,
				  unsigned int n0,
				  unsigned int n1){
  unsigned int shifted_i = 0;
  int i_b, i_B;
  //bzero(dest, n1);
  memset(dest,0x0, n1);
  int support = 0;
  for (unsigned int j=0; j<n0; j++) {
    unsigned char inter = mask[j] & src[j];
    if (inter!=0) {
      for (unsigned b=0; b<8; b++) {
	if ( (inter & mask_byte(b))!=0 ) {
	  support++;
	  i_B = shifted[shifted_i] / 8;
	  i_b = shifted[shifted_i] % 8;
	  dest[i_B] = (dest[i_B] | (0x1 << i_b));
	}
	shifted_i++;
      }
    } else {
      shifted_i+=8;
    }
  }
  return support;
}



// ------------------------------------------------------------------------



template <class T, class T1>
class Worker {
public:
  Worker(Master<T,T1>* m, unsigned int thid, Policy<T,T1>* p, FILE* o) {
    master = m;  // Master, i.e. job-dispenser
    thID = thid; // Thread ID
    Data = NULL;
    policy = p;
    current_level = 0;
    initial_level = 0;
    outfile = o;

    THREAD_STATUS = NOT_STARTED;

#ifdef WORK_STEALING
    pthread_mutex_init(&job_lock, NULL);
#endif

#ifdef THREADS_VERBOSE
    get_job_time    = 0.0;
    projection_time = 0.0;
    lock_time       = 0.0;
    no_of_jobs      = 0;
#endif
  }

  ~Worker() {
    // ------------------------ 
    // Free memory 
    // ------------------------ 
    free(aux_STATIC_TID_LISTS); 
    delete [] STATIC_closure;
    delete [] STATIC_post_list; 
    delete [] STATIC_pre_list; 
    delete [] STATIC_pre_list_included; 

    delete [] LEVEL_closedSets;
    delete [] LEVEL_ub;
    delete [] LEVEL_pre_list_lung;
    delete [] LEVEL_pre_list_included;
    delete [] LEVEL_post_list;

#ifdef WORK_STEALING
    delete [] LEVEL_post_i;
    pthread_mutex_destroy(&job_lock);
#endif
  }


  void setVD(DCI_VD<T,T1>* d) {
    Data = d;
  }

  static void* start_thread(void* arg) {
    Worker<T,T1>* myself = (Worker<T,T1>*) arg;

    myself->start();

    return NULL;
  }

  void start() {
#ifdef THREADS_VERBOSE
    double thread_started = GLOBAL_CLOCK.ReadChronos();
#endif

    // allocate memory
    allocate_memory();

#ifdef THREADS_VERBOSE
    Chronos get_job_timer;
    get_job_timer.RestartChronos();
#endif

    // get job initilization and mine
    while (policy->get_job(thID)) {

#ifdef MINING_AT_HOME_STATS
      // take stats
      mat_num_closed = 0;
      mat_closed_size = 0;
      // take time
      Chronos JobTime;
      JobTime.StartChronos();
      int stat_pre = LEVEL_pre_list_lung[initial_level];
      int stat_post = (int) (LEVEL_post_list[initial_level+1]-LEVEL_post_list[initial_level]);
#endif

#ifdef THREADS_VERBOSE
      get_job_time += get_job_timer.ReadChronos();
#endif
#ifdef JOB_DURATION
      double job_started = GLOBAL_CLOCK.ReadChronos();
#endif

      // --------------------------
      // Actual mining 
      Mine(initial_level);
      // --------------------------

#ifdef JOB_DURATION
    double job_ended = GLOBAL_CLOCK.ReadChronos();
    synch_printf("# ## ## ## Job Duration %f\n", job_ended - job_started );
#endif
#ifdef THREADS_VERBOSE
    get_job_timer.RestartChronos();
    no_of_jobs++;
#endif


#ifdef MINING_AT_HOME_STATS
      JobTime.StopChronos();
      { // print job description & elapsed time
	char buf[1024];
	char* msg = &buf[0];
	msg += sprintf(msg, "# ## ## MAT # set:"); 
	for (unsigned int ii =0; ii<LEVEL_closedSets[initial_level].lung; ii++)
	  msg += sprintf(msg, " %d", LEVEL_closedSets[initial_level].items[ii]); 

	msg += sprintf(msg, ", data size: %dx%d, items: %d + %d, num: %d, sz: %d, time: %.3e \n", 
			Data->end_sect_eq, Data->tid_list_size*sizeof(tid_list),
			stat_pre, stat_post,
			mat_num_closed, 
			mat_closed_size,
			JobTime.ReadChronos() );
	synch_printf(buf);
      }
#endif
    }

#ifdef THREADS_VERBOSE
    get_job_time += get_job_timer.ReadChronos();

    double thread_ended = GLOBAL_CLOCK.ReadChronos();
#ifdef WORK_STEALING
    lock_time = lock_chron.ReadChronos();
#endif
    synch_printf("# ## ## ## Thread %d started and ended at: %f - %f\n", 
		 thID, thread_started, thread_ended);
    synch_printf("# ## ## ## Thread %d time spent waiting for a job: %f\n", 
		 thID, get_job_time);
    synch_printf("# ## ## ## Thread %d time spent projecting dataset: %f\n", 
		 thID, projection_time);
    synch_printf("# ## ## ## Thread %d blocked time: %f\n", 
		 thID, lock_time);
    synch_printf("# ## ## ## Thread %d idle/wasted time: %f\n", 
		 thID, lock_time+get_job_time-projection_time);
    synch_printf("# ## ## ## Thread %d Number of jobs: %d\n", 
		 thID, no_of_jobs);
#else
    synch_printf( "# ## ## ## Thread %d has finished.\n", thID);
#endif
  }

  void allocate_memory(){
    if (!Data) 
      fatal_error("Impossible to allocate memory without vertical dataset.");

    // ----------------------------------------------------
    // Initialize static memory
    // ----------------------------------------------------
    aligned16_malloc (STATIC_TID_LISTS, aux_STATIC_TID_LISTS, Data->tid_list_size*(Data->end_sect_eq+1)*sizeof(tid_list));
    if (!aux_STATIC_TID_LISTS) fatal_error("Could not allocate memory for itemsets tid-lists.");
    STATIC_closure = new T [Data->end_sect_eq];	 
    if (!STATIC_closure) fatal_error("Could not allocate memory for closed itemset.");     
    STATIC_pre_list = new T [Data->end_sect_eq]; 
    if (!STATIC_pre_list) fatal_error("Could not allocate memory for pre_list");
    STATIC_pre_list_included = new T1 [Data->end_sect_eq*Data->end_sect_eq]; 
    if (!STATIC_pre_list_included) fatal_error("Could not allocate memory for pre_list_included");
    STATIC_post_list = new POST_item<T,T1> [(Data->end_sect_eq*(Data->end_sect_eq+1))/2]; 
    if (!STATIC_post_list) fatal_error("Could not allocate memory for post_list");

    // ----------------------------------------------------
    // Initialize dinamic pointers
    // ----------------------------------------------------
    LEVEL_closedSets = new close_itemset<T,T1> [Data->end_sect_eq +2]; //
    if (!LEVEL_closedSets) fatal_error("Could not allocate memory for closed itemset properties.");     
    for (T i=0; i<Data->end_sect_eq+2; i++)
      LEVEL_closedSets[i].items = STATIC_closure;   // items are accumulated in the same shared memory  
    LEVEL_ub = new T [Data->end_sect_eq+2];
    if (!LEVEL_ub) fatal_error("Could not allocate memory for prefix upper bounds");
    LEVEL_pre_list_lung = new T [Data->end_sect_eq+2];
    if (!LEVEL_pre_list_lung) fatal_error("Could not allocate memory for pre list lengths");
    LEVEL_pre_list_included = new T1* [Data->end_sect_eq+3];
    if (!LEVEL_pre_list_included) fatal_error("Could not allocate memory for pre list included pointers");
    LEVEL_post_list = new POST_item<T,T1>* [Data->end_sect_eq+3];
    if (!LEVEL_post_list) fatal_error("Could not allocate memory for post list included");

#ifdef WORK_STEALING
    LEVEL_post_i = new T [Data->end_sect_eq+2];
    if (!LEVEL_post_i) fatal_error("Could not allocate memory for current post_i");
#endif
  }


  bool Check_Inclusion( close_itemset<T,T1> &i,
			T j,
			T1 &eq,
			T1 &new_eq){
    T1 kk = 0;
    
    // initialize tid_list pointers
    tid_list* tid_list_i = i.head_list; 
    tid_list* tid_list_j = Data->VD+j*Data->tid_list_size; 
 
 
    // ------------------------- 
    // section equal 
    // ------------------------- 
    bool match = true; 
    if (Data->sz_sect_eq>0 && Data->belongs_to_SE(j)) { 
      if (! i.isSectionIncluded) { 
	// use additional info about sect_eq 
	if (i.bytes_included_sect_eq > eq) 
	  kk = i.bytes_included_sect_eq; 
	else 
	  kk = eq; 
#ifdef MY_STATS 
	EXT_NO_INCLUSION+= kk;
#endif 
	if (kk < Data->sz_sect_eq) { 
	  if (match)
	    match = tid_list_inclusion<T1>(tid_list_i,tid_list_j,kk,Data->sz_sect_eq);
	  i.bytes_included_sect_eq = kk-1; 
	} 
	else  
	  i.bytes_included_sect_eq = kk; 
      }  
      // else match stays true for section equal 
#ifdef MY_STATS 
      HEAD_NO_INCLUSION+=sz_sect_eq;
#endif 
    } else { 
      // if j not in sect_eq 
      // no section equal optimization 
#ifdef MY_STATS 
      EXT_NO_INCLUSION+= eq;
#endif 
      
      kk = eq;
      if (match)
	match = tid_list_inclusion<T1>(tid_list_i,tid_list_j,kk, Data->sz_sect_eq);
    } 
    
    // -------------------------------
    // tail (outside section equal)
    // -------------------------------
    if (match) {
      
      tid_list_i = i.tail_list; 
      tid_list_j += Data->sz_sect_eq; 
      
      if (eq<=Data->sz_sect_eq) 
	kk = 0; 
      else 
	kk = eq - Data->sz_sect_eq; 
#ifdef MY_STATS 
      EXT_NO_INCLUSION+= kk;
#endif 
      
      match = tid_list_inclusion<T1>(tid_list_i,tid_list_j,kk,Data->sz_tail);
      
      new_eq = Data->sz_sect_eq + kk -1; 
    } 
    else  // if no match in section equal 
      new_eq = kk-1; 
 
    return match; 
  }
  
  void Intersect_and_Count( close_itemset<T,T1> &dest,
			    close_itemset<T,T1> &cs,
			    T item,
			    POST_item<T,T1>* post_lists
			    ){
    T1 max;
    T1 start;
    
    T1 old_inc;
    T1 new_inc;
    
    T id = post_lists[item].id;

    if (!cs.isSectionIncluded && dest.isSectionIncluded && Data->belongs_to_SE(id)) {
      // we have already performed intersection operations on sect_eq
      //   during this iteration
      // DO NOTHING !!!!!!!!!
#ifdef MY_STATS 
      HEAD_NO_INTERSECTION+=Data->sz_sect_eq; 
#endif
    } else if (cs.isSectionIncluded && Data->belongs_to_SE(id)) {
      // since cs <= id, no intersection
      dest.sect_eq_num_bit	= cs.sect_eq_num_bit;
      dest.head_list		= cs.head_list;
#ifdef MY_STATS 
      HEAD_NO_INTERSECTION+=Data->sz_sect_eq; 
#endif
    } else {
    // copy included bytes without intersection
      max = post_lists[item].included;
      if (max>Data->sz_sect_eq)
	max = Data->sz_sect_eq;

      if (item > 0) 
	start = post_lists[item-1].included;
      else 
	start = 0;
      
      if (start<max)
	memcpy( dest.head_list	+ start,
		cs.head_list	+ start,
		(max-start) *sizeof(tid_list));
    
      new_inc = cs.min_included;
      old_inc = cs.old_min_included;
      if (new_inc > Data->sz_sect_eq)
	new_inc = Data->sz_sect_eq;
      if (old_inc > Data->sz_sect_eq)
	old_inc = Data->sz_sect_eq;
    
    
      // tid-list 
      // |--- old ---- new ---- max ---------------------
      if (new_inc != old_inc) {
	// update number of included bits
	cs.bits_included = cs.old_bits_included + 
	  count_1_bits(dest.head_list+old_inc, new_inc-old_inc);

#ifdef MY_STATS 
	COUNTS += new_inc-old_inc;
#endif
	cs.old_min_included = cs.min_included;
      } 
#ifdef MY_STATS 
      else {
	MIN_COUNTS_SAVED += new_inc;
      }
#endif
      
      dest.sect_eq_num_bit =  cs.bits_included +
	count_1_bits(dest.head_list+new_inc, max-new_inc);
      
#ifdef MY_STATS 
      COUNTS += max-new_inc;
#endif
      
      dest.sect_eq_num_bit += and_tid_list_count_1(	
						   dest.head_list	+ max,
						   cs.head_list	+ max,
						   Data->VD + id*Data->tid_list_size	+ max,
						   Data->sz_sect_eq - max);

#ifdef MY_STATS 
      COUNTS += Data->sz_sect_eq-max;
#endif
      
#ifdef MY_STATS 
      EXT_NO_INTERSECTION+=max; 
      BYTE_INTERSECTION +=(Data->sz_sect_eq-max);
#endif
    }
	
    if (post_lists[item].included>Data->sz_sect_eq)
      max = post_lists[item].included - Data->sz_sect_eq;
    else 
      max = 0;
    
    if (item > 0) 
      start = post_lists[item-1].included;
    else 
      start = 0;
    
    if (start > Data->sz_sect_eq)
      start -= Data->sz_sect_eq;
    else 
      start = 0;
    
    if (start<max)
      memcpy( dest.tail_list + start,
	      cs.tail_list   + start,
	      (max-start)*sizeof(tid_list));
    
  // TODO ??!??
//     new_inc = cs.min_included;
//     old_inc = cs.old_min_included;
//     if (new_inc >sz_sect_eq)
//       new_inc -= sz_sect_eq;
//     else
//       new_inc = 0;
//     if (old_inc >sz_sect_eq)
//       old_inc -= sz_sect_eq;
//     else
//       old_inc = 0;

//     // tid-list 
//     // |--- old ---- new ---- max ---------------------
//     if (new_inc != old_inc) {
//       // update number of included bits
//       cs.bits_included = cs.old_bits_included + 
// 	count_1_bits(dest.head_list+old_inc, new_inc-old_inc);
//       cs.old_min_included = cs.min_included;
//     }

//     dest.sect_eq_num_bit =  cs.bits_included +
//       count_1_bits(dest.head_list+new_inc, max-new_inc);
    
//     dest.sect_eq_num_bit += and_tid_list_count_1(	
//  						 dest.head_list	+ max,
//  						 cs.head_list	+ max,
//  						 VD + id*tid_list_size	+ max,
//  						 sz_sect_eq - max);



    dest.count = dest.sect_eq_num_bit +
      count_1_bits(dest.tail_list, max);


    dest.count += and_tid_list_count_1(	
				       dest.tail_list			+ max,
				       cs.tail_list				+ max,
				       Data->VD+id*Data->tid_list_size+Data->sz_sect_eq	+ max,
				       Data->sz_tail - max);
    

#ifdef MY_STATS 
    COUNTS += sz_tail;
#endif
    
    
    dest.isSectionIncluded |= Data->belongs_to_SE(id); 

#ifdef MY_STATS 
    EXT_NO_INTERSECTION+=max;
    BYTE_INTERSECTION +=(Data->sz_tail-max);
#endif
  }

  void Mine( T level) {	 
    // ------------------------------------------------
    // get iteration parameters from shared data structure
    // ------------------------------------------------
#ifdef WORK_STEALING
#ifdef THREADS_VERBOSE
    lock_chron.RestartChronos();
#endif
    pthread_mutex_lock( &job_lock );
#ifdef THREADS_VERBOSE
    lock_chron.StopChronos();
#endif
#endif
    current_level = level;

    close_itemset<T,T1>* closedSet = &(LEVEL_closedSets[current_level]);
    tid_list* new_tid_list = STATIC_TID_LISTS + current_level*Data->tid_list_size;

    POST_item<T,T1>* post_list         = LEVEL_post_list[current_level];
    POST_item<T,T1>* new_post_list     = LEVEL_post_list[current_level+1];
    T  post_list_lung               = new_post_list - post_list;

    T1* pre_list_included           = LEVEL_pre_list_included[current_level];
    T1* new_pre_list_included       = LEVEL_pre_list_included[current_level+1];

    // set for the next iteration
    LEVEL_pre_list_included[current_level+2] = LEVEL_pre_list_included[current_level+1]
      + post_list_lung + LEVEL_pre_list_lung[current_level];


#ifdef WORK_STEALING
    LEVEL_post_i[current_level] = 0;
    pthread_mutex_unlock( &job_lock );
#endif

    // ------------------------------------------------
    // init closure variables
    // ------------------------------------------------
#ifdef MY_STATS 
    SUPPORT_TIME.StartChronos(); 
#endif

    // initialize new_closure
    close_itemset<T,T1>* new_closure = &(LEVEL_closedSets[current_level+1]); 
    new_closure->head_list = new_tid_list; 
    new_closure->tail_list = new_tid_list + Data->sz_sect_eq; 
    new_closure->isSectionIncluded = closedSet->isSectionIncluded; 
    // isSectionIncluded is set here, and it is used dinamically 
    //   in the following to detect were operations involving SECT_EQ 
 
#ifdef MY_STATS 
    SUPPORT_TIME.StopChronos(); 
#endif

    // aux variables
    T post_i = 0;
    T post_j;
    T1 included;

    // for each post_i 
    //    for (post_i=0; post_i < LEVEL_ub[current_level]; post_i++) { // critic section !??!?
    while (true) {
      // ------------------------------------------------
      // Create a new Generator
      // ------------------------------------------------
#ifdef MY_STATS 
      SUPPORT_TIME.StartChronos();
#endif
      // reset previous closures and add current item to the current closure 
      new_closure->lung = closedSet->lung; 
      new_closure->items[new_closure->lung++] = post_list[post_i].id; 

      // Intersect tid-list
      Intersect_and_Count(*new_closure, *closedSet, post_i, post_list); 
 
      new_closure->old_bits_included = new_closure->bits_included = closedSet->bits_included;
      new_closure->old_min_included  = closedSet->min_included;
      new_closure->min_included      = Data->tid_list_size;

#ifdef MY_STATS 
      SUPPORT_TIME.StopChronos(); 
#endif 
 
      // is it frequent ?? 
      if ( Data->min_count > new_closure->count  ) { 
#ifdef MY_STATS
	NUM_FAILED++;
#endif 
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// try with the next generator 
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifdef WORK_STEALING
#ifdef THREADS_VERBOSE
	lock_chron.RestartChronos();
#endif
	pthread_mutex_lock( &job_lock );
#ifdef THREADS_VERBOSE
	lock_chron.StopChronos();
#endif
#endif
	post_i++;
#ifdef WORK_STEALING
	LEVEL_post_i[current_level] = post_i;
#endif
	if (post_i >= LEVEL_ub[current_level]) {
#ifdef WORK_STEALING
	  pthread_mutex_unlock( &job_lock );
#endif
	  break;
	}
#ifdef WORK_STEALING
	pthread_mutex_unlock( &job_lock );
#endif
	continue; 
      } 
 
      // ------------------------------------------------
      // Discard Duplicates
      // ------------------------------------------------
#ifdef MY_STATS 
      DUPLICATE_TIME.StartChronos();
      AUX_DELTA = BYTE_INCL;
      DUPLICATE_CHECKS++;
#endif

      int pre_list_lung     = LEVEL_pre_list_lung[current_level];
      new_closure->bytes_included_sect_eq = closedSet->bytes_included_sect_eq;
      int pre_i;
      for( 
	  pre_i = pre_list_lung-1;   
	  pre_i>=0  && 
	    !Check_Inclusion(  *new_closure, 
			       STATIC_pre_list[pre_i],  
			       pre_list_included[pre_i],  
			       new_pre_list_included[pre_i]);	 
	  pre_i--);

#ifdef MY_STATS 
      DUPLICATES_BYTE_INCL += (BYTE_INCL-AUX_DELTA);
      DUPLICATE_TIME.StopChronos(); 
#endif

      // if an inclusion was found go with the next post Item 
      if (pre_i>=0){ 
#ifdef MY_STATS
	DUPLICATED++;
	NUM_FAILED++; 
#endif 
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// try with the nest generator 
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifdef WORK_STEALING
#ifdef THREADS_VERBOSE
	lock_chron.RestartChronos();
#endif
	pthread_mutex_lock( &job_lock );
#ifdef THREADS_VERBOSE
	lock_chron.StopChronos();
#endif
#endif
	post_i++;
#ifdef WORK_STEALING
	LEVEL_post_i[current_level] = post_i;
#endif
	if (post_i >= LEVEL_ub[current_level]) {
#ifdef WORK_STEALING
	  pthread_mutex_unlock( &job_lock );
#endif
	  break;
	}
#ifdef WORK_STEALING
	pthread_mutex_unlock( &job_lock );
#endif
	continue; 
      }		
 

      // ------------------------------------------------
      // Calculate Closure
      // ------------------------------------------------
#ifdef MY_STATS 
      CLOSURE_TIME.StartChronos(); 
      AUX_DELTA = BYTE_INCL;
#endif 


      POST_item<T,T1>* new_post_list_last = new_post_list; 

      for (post_j=post_i+1; post_j<post_list_lung; post_j++) { 
	// if an inclusion is found 
	if ( Check_Inclusion( *new_closure,	 
			      post_list[post_j].id, 
			      post_list[post_j].included, 
			      included)	) { 
	  
	  // add current item to the current closure 
	  new_closure->items[new_closure->lung++] = post_list[post_j].id; 
	} else { 
	  new_post_list_last->id        = post_list[post_j].id; 
	  new_post_list_last->included	= included; 
	  new_post_list_last++;
	  // memorizzo il min nel closed itemset
	  if (new_closure->min_included > included) {
	    new_closure->min_included = included;
	  }
	} 
      } 

      LEVEL_post_list[current_level+2] = new_post_list_last; 

#ifdef MY_STATS 
      CLOSURE_BYTE_INCL += (BYTE_INCL-AUX_DELTA);
      CLOSURE_TIME.StopChronos(); 
#endif 
 
      // ------------------------------------------------
      // write output
      // ------------------------------------------------
#ifdef MY_STATS 
      OUTPUT_TIME.StartChronos(); 
#endif 
      // output 
      if (new_closure->lung > 1) {
	new_closure->printTofile(outfile, Data->unmap);
#ifdef MINING_AT_HOME_STATS
	mat_num_closed++;
	mat_closed_size+= new_closure->lung*sizeof(T) + sizeof(T1) + sizeof(T);
#endif
      }

#ifdef MY_STATS 
      OUTPUT_TIME.StopChronos(); 
#endif 
   
      // ------------------------------------------------
      // Recursion
      // ------------------------------------------------
      if ( new_post_list_last != new_post_list ) {
#ifdef MY_STATS 
	PRE+= pre_list_lung; 
#endif 
 	// set up and run next iteration
	LEVEL_pre_list_lung[current_level+1] = pre_list_lung; 
	LEVEL_ub[current_level +1] = new_post_list_last - new_post_list;

	Mine( current_level +1);

      } 
 
      // --------------------------------------- 
      // Updates for the next level  Pre List 
      // --------------------------------------- 
#ifdef WORK_STEALING
#ifdef THREADS_VERBOSE
      lock_chron.RestartChronos();
#endif
      pthread_mutex_lock( &job_lock );
#ifdef THREADS_VERBOSE
      lock_chron.StopChronos();
#endif
#endif
      
      STATIC_pre_list[pre_list_lung]	= post_list[post_i].id; 
      pre_list_included[pre_list_lung]	= 0; 
      LEVEL_pre_list_lung[current_level]++; 
      post_i++;

#ifdef WORK_STEALING
      LEVEL_post_i[current_level] = post_i;
#endif

      if (post_i >= LEVEL_ub[current_level]) {
#ifdef WORK_STEALING
	pthread_mutex_unlock( &job_lock );
#endif
	break;
      }
#ifdef WORK_STEALING
      pthread_mutex_unlock( &job_lock );
#endif
    }

    // decrease level
#ifdef WORK_STEALING
#ifdef THREADS_VERBOSE
    lock_chron.RestartChronos();
#endif
    pthread_mutex_lock( &job_lock );
#ifdef THREADS_VERBOSE
    lock_chron.StopChronos();
#endif
#endif
    current_level--;
#ifdef WORK_STEALING
    pthread_mutex_unlock( &job_lock );
#endif
  }

public:

  unsigned int thID;
  Master<T,T1>* master;
  FILE* outfile;

  DCI_VD<T,T1>* Data;

  // STATIC means pre-allocated and indirectly used
  // (tid-lists to be changed in a levelwise fashion)
  void*      aux_STATIC_TID_LISTS;          // this is for the un-aligned allocation
  tid_list*      STATIC_TID_LISTS;          // this is the alinged starting point
  T*             STATIC_pre_list;           // pre_set
  T1*            STATIC_pre_list_included;  // included bytes by pre-set (T1 ??)
  POST_item<T,T1>*  STATIC_post_list;          // post_set
  T*             STATIC_closure;            // closed itemset

  // level means per-level directly usable info
  T current_level;                             //!! sezione critica
  T initial_level;
  close_itemset<T,T1>* LEVEL_closedSets;         //
  T*              LEVEL_ub;                 //!! sezione critica
  T*              LEVEL_pre_list_lung;      // pre_lungs
  T1**            LEVEL_pre_list_included;  // included bytes by pre-set (T1 ??)
  POST_item<T,T1>**  LEVEL_post_list;          // post_set

  Policy<T,T1>* policy;

  unsigned int    THREAD_STATUS;  
  static const unsigned int NOT_STARTED = 0;
  static const unsigned int RUNNING     = 1;
  static const unsigned int FINISHED    = 2;

#ifdef WORK_STEALING
  pthread_mutex_t job_lock;
  T*              LEVEL_post_i;
#endif

#ifdef MINING_AT_HOME_STATS
  int mat_num_closed;
  int mat_closed_size;
#endif

#ifdef THREADS_VERBOSE
  double get_job_time;
  double projection_time;
  double lock_time;
  Chronos lock_chron;
  unsigned int no_of_jobs;
#endif
};



/*
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * One thread no Projection
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
template <class T, class T1>
class Simplest_Policy : public Policy<T,T1> {
public:
  Simplest_Policy(DCI_VD<T,T1>* d, unsigned int n, Worker<T,T1>** w) {
    Data = d;
    no_of_threads = n;
    threads = w;
    pthread_mutex_init(&master_lock, NULL);
    status = ZERO;
  }
  
  ~Simplest_Policy() {
    pthread_mutex_destroy(&master_lock);
  }
  
  bool get_job(unsigned int th_ID) {
    // enter critic section
    pthread_mutex_lock(&master_lock);

    if (status == WORK_DONE) {
      // leave critic section
      pthread_mutex_unlock(&master_lock);
      return false;
    }

    status = WORK_DONE;

    // one job only !
    (threads[0])->initial_level = 1;
    (threads[0])->LEVEL_closedSets[1].lung                     = 0; 
    (threads[0])->LEVEL_closedSets[1].count	               = 0;
    (threads[0])->LEVEL_closedSets[1].isSectionIncluded	       = false;
    (threads[0])->LEVEL_closedSets[1].head_list		       = (threads[0])->STATIC_TID_LISTS;
    (threads[0])->LEVEL_closedSets[1].tail_list		       = (threads[0])->STATIC_TID_LISTS + Data->sz_sect_eq; 
    memset((threads[0])->STATIC_TID_LISTS, 0xFFFFFFFF, Data->tid_list_size*sizeof(tid_list));
    (threads[0])->LEVEL_closedSets[1].sect_eq_num_bit	       = 0; 
    (threads[0])->LEVEL_closedSets[1].bytes_included_sect_eq   = 0; 
    (threads[0])->LEVEL_closedSets[1].old_bits_included        = 0;
    (threads[0])->LEVEL_closedSets[1].bits_included            = 0;
    (threads[0])->LEVEL_closedSets[1].old_min_included         = 0;
    (threads[0])->LEVEL_closedSets[1].min_included             = 0;
    if (Data->ub > Data->end_sect_eq)
      (threads[0])->LEVEL_ub[1] = Data->end_sect_eq;
    else
      (threads[0])->LEVEL_ub[1] = Data->ub;
    (threads[0])->LEVEL_pre_list_lung[1]                       = 0;
    (threads[0])->LEVEL_pre_list_included[1]                   = (threads[0])->STATIC_pre_list_included;
    (threads[0])->LEVEL_pre_list_included[2]                    = (threads[0])->STATIC_pre_list_included + Data->end_sect_eq;
    memset((threads[0])->LEVEL_pre_list_included[1], 0, sizeof(T1)*Data->end_sect_eq);
    for (T i=0; i<Data->end_sect_eq; i++) {
      (threads[0])->STATIC_post_list[i].id       = i;
      (threads[0])->STATIC_post_list[i].included = 0;
    }
    (threads[0])->LEVEL_post_list[1] = (threads[0])->STATIC_post_list;
    (threads[0])->LEVEL_post_list[2] = (threads[0])->STATIC_post_list + Data->end_sect_eq;

    // leave critic section
    pthread_mutex_unlock(&master_lock);

    return true;
  }


private:
  DCI_VD<T,T1>* Data;
  unsigned int no_of_threads;
  Worker<T,T1>** threads;

  pthread_mutex_t master_lock;
  unsigned int status;

  const static unsigned int ZERO      = 1;
  const static unsigned int WORK_DONE = 2;
};


/*
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * One thread plus Projection
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
template <class T, class T1>
class One_Thread_Policy : public Policy<T,T1> {
public:
  One_Thread_Policy(DCI_VD<T,T1>* d, unsigned int n, Worker<T,T1>** w) {
    Data = d;
    no_of_threads = n;
    threads = w;
    pthread_mutex_init(&master_lock, NULL);
    current_item = 0;
    max_item = Data->ub;
    if (max_item > Data->end_sect_eq-1)
      max_item = Data->end_sect_eq-1;

  }
  
  ~One_Thread_Policy() {
    pthread_mutex_destroy(&master_lock);
  }
  
  bool get_job(unsigned int th_ID) {
    // enter critic section
    pthread_mutex_lock(&master_lock);
    
    if (current_item >= max_item) {
      // leave critic section
      pthread_mutex_unlock(&master_lock);
      // end of work
      return false;
    }
    
    // set up for current item
    (threads[th_ID])->LEVEL_closedSets[0].lung                = 1; 
    (threads[th_ID])->LEVEL_closedSets[0].items[0]            = current_item; 
    (threads[th_ID])->LEVEL_closedSets[0].count	              = Data->singletons_support[current_item];
    (threads[th_ID])->LEVEL_closedSets[0].isSectionIncluded   = Data->belongs_to_SE(current_item);
    (threads[th_ID])->LEVEL_closedSets[0].head_list	      = Data->VD + current_item*Data->tid_list_size;
    (threads[th_ID])->LEVEL_closedSets[0].tail_list	      = Data->VD + current_item*Data->tid_list_size + Data->sz_sect_eq; 

    if (Data->belongs_to_SE(current_item))
      (threads[th_ID])->LEVEL_closedSets[0].sect_eq_num_bit	   = Data->n_bit_set_eq; 
    else
      (threads[th_ID])->LEVEL_closedSets[0].sect_eq_num_bit	   = 0; 
    (threads[th_ID])->LEVEL_closedSets[0].bytes_included_sect_eq   = 0; 
    (threads[th_ID])->LEVEL_closedSets[0].old_bits_included        = 0;
    (threads[th_ID])->LEVEL_closedSets[0].bits_included            = 0;
    (threads[th_ID])->LEVEL_closedSets[0].old_min_included         = 0;
    (threads[th_ID])->LEVEL_closedSets[0].min_included             = 0;

    (threads[th_ID])->LEVEL_pre_list_lung[0]       = current_item;
    for (T i=0; i<current_item; i++)
      (threads[th_ID])->STATIC_pre_list[i]         = i;
    (threads[th_ID])->LEVEL_pre_list_included[0]   = (threads[th_ID])->STATIC_pre_list_included;
    (threads[th_ID])->LEVEL_pre_list_included[1]   = (threads[th_ID])->STATIC_pre_list_included + Data->end_sect_eq;
    memset((threads[th_ID])->LEVEL_pre_list_included[0], 0, sizeof(T1)*Data->end_sect_eq);

    T l = 0;
    for (T i=current_item+1; i<Data->end_sect_eq; i++) {
      (threads[th_ID])->STATIC_post_list[l].id       = i;
      (threads[th_ID])->STATIC_post_list[l].included = 0;
      l++;
    }

    (threads[th_ID])->LEVEL_post_list[0] = (threads[th_ID])->STATIC_post_list;
    (threads[th_ID])->LEVEL_post_list[1] = (threads[th_ID])->STATIC_post_list + l;

    (threads[th_ID])->LEVEL_ub[0]     = l; 
    (threads[th_ID])->initial_level   = 0;

    current_item++;

    // leave critic section
    pthread_mutex_unlock(&master_lock);

    return true;
  }


private:
  DCI_VD<T,T1>* Data;
  unsigned int no_of_threads;
  Worker<T,T1>** threads;

  pthread_mutex_t master_lock;
  T current_item;
  T max_item;
};



/*
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Multi-thread, no projection, 2-nd level
 * static paritioning, dinamic assignment
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
template <class T, class T1>
class SP_DA_2 : public Policy<T,T1> {
public:
  SP_DA_2(DCI_VD<T,T1>* d, unsigned int n, Worker<T,T1>** w) {
    Data = d;
    no_of_threads = n;
    threads = w;
    pthread_mutex_init(&master_lock, NULL);
    current_first_item  = 0;
    current_second_item = 1;
    max_item = Data->ub;
    if (max_item > Data->end_sect_eq-1)
      max_item = Data->end_sect_eq-1;

  }
  
  ~SP_DA_2() {
    pthread_mutex_destroy(&master_lock);
  }
  
  bool get_job(unsigned int th_ID) {
    // enter critic section
    pthread_mutex_lock(&master_lock);
    
    if ( current_second_item >= Data->end_sect_eq) {
      current_first_item++;
      current_second_item = current_first_item+1;
    }

    if (current_first_item >= max_item) {
      // leave critic section
      pthread_mutex_unlock(&master_lock);
      // end of work
      return false;
    }

    
    // set up for current item
    (threads[th_ID])->LEVEL_closedSets[0].lung                = 1; 
    (threads[th_ID])->LEVEL_closedSets[0].items[0]            = current_first_item; 
    (threads[th_ID])->LEVEL_closedSets[0].count	              = Data->singletons_support[current_first_item];
    (threads[th_ID])->LEVEL_closedSets[0].isSectionIncluded   = Data->belongs_to_SE(current_first_item);
    (threads[th_ID])->LEVEL_closedSets[0].head_list	      = Data->VD + current_first_item*Data->tid_list_size;
    (threads[th_ID])->LEVEL_closedSets[0].tail_list	      = Data->VD + current_first_item*Data->tid_list_size + Data->sz_sect_eq; 

    if (Data->belongs_to_SE(current_first_item))
      (threads[th_ID])->LEVEL_closedSets[0].sect_eq_num_bit	   = Data->n_bit_set_eq; 
    else
      (threads[th_ID])->LEVEL_closedSets[0].sect_eq_num_bit	   = 0; 
    (threads[th_ID])->LEVEL_closedSets[0].bytes_included_sect_eq   = 0; 
    (threads[th_ID])->LEVEL_closedSets[0].old_bits_included        = 0;
    (threads[th_ID])->LEVEL_closedSets[0].bits_included            = 0;
    (threads[th_ID])->LEVEL_closedSets[0].old_min_included         = 0;
    (threads[th_ID])->LEVEL_closedSets[0].min_included             = 0;

    (threads[th_ID])->LEVEL_pre_list_lung[0]       = current_second_item-1;
    T l = 0;
    for (T i=0; i<current_second_item; i++) 
      if (i!=current_first_item)
	(threads[th_ID])->STATIC_pre_list[l++]         = i;
    (threads[th_ID])->LEVEL_pre_list_included[0]   = (threads[th_ID])->STATIC_pre_list_included;
    (threads[th_ID])->LEVEL_pre_list_included[1]   = (threads[th_ID])->STATIC_pre_list_included + Data->end_sect_eq;
    memset((threads[th_ID])->LEVEL_pre_list_included[0], 0, sizeof(T1)*Data->end_sect_eq);

    l = 0;
    for (T i=current_second_item; i<Data->end_sect_eq; i++) {
      (threads[th_ID])->STATIC_post_list[l].id       = i;
      (threads[th_ID])->STATIC_post_list[l].included = 0;
      l++;
    }

    (threads[th_ID])->LEVEL_post_list[0] = (threads[th_ID])->STATIC_post_list;
    (threads[th_ID])->LEVEL_post_list[1] = (threads[th_ID])->STATIC_post_list + l;

    (threads[th_ID])->LEVEL_ub[0]     = 1;  // mine only one item of the second level
    (threads[th_ID])->initial_level   = 0;

//     cout << "thread " << th_ID << " has taken " << (unsigned int) current_first_item << "," << (unsigned int)current_second_item 
//  	 << " (" << (unsigned int)max_item << ")" << endl;

    current_second_item++;

    // leave critic section
    pthread_mutex_unlock(&master_lock);

    return true;
  }


private:
  DCI_VD<T,T1>* Data;
  unsigned int no_of_threads;
  Worker<T,T1>** threads;

  pthread_mutex_t master_lock;
  T current_first_item;
  T current_second_item;
  T max_item;
};

/*
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Multi-thread, no projection, 2-nd level
 * static paritioning, static assignment
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
template <class T, class T1>
class SP_SA_2 : public Policy<T,T1> {
public:
  SP_SA_2(DCI_VD<T,T1>* d, unsigned int n, Worker<T,T1>** w) {
    Data = d;
    no_of_threads = n;
    threads = w;
    pthread_mutex_init(&master_lock, NULL);
    max_item = Data->ub;
    if (max_item > Data->end_sect_eq-1)
      max_item = Data->end_sect_eq-1;

    current_first_item = new T[no_of_threads];
    if ( !current_first_item) fatal_error("Could not allocate memory.");
    for (unsigned int i=0; i<no_of_threads; i++) current_first_item[i] = 0;
    current_second_item = new T[no_of_threads];
    if ( !current_second_item) fatal_error("Could not allocate memory.");
    for (unsigned int i=0; i<no_of_threads; i++) current_second_item[i] = i+1;
  }
  
  ~SP_SA_2() {
    pthread_mutex_destroy(&master_lock);
    if (current_first_item)
      delete [] current_first_item;
    if (current_second_item)
      delete [] current_second_item;
  }
  
  bool get_job(unsigned int th_ID) {
    // enter critic section
    pthread_mutex_lock(&master_lock);
    
    while( current_second_item[th_ID] >= Data->end_sect_eq) {
      current_first_item[th_ID]++;

      if (current_first_item[th_ID] >= max_item) {
	// leave critic section
	pthread_mutex_unlock(&master_lock);
	// end of work
	return false;
      }

      current_second_item[th_ID] = current_first_item[th_ID]+1+(current_second_item[th_ID] % no_of_threads);
    }

    // set up for current item
    (threads[th_ID])->LEVEL_closedSets[0].lung                = 1; 
    (threads[th_ID])->LEVEL_closedSets[0].items[0]            = current_first_item[th_ID]; 
    (threads[th_ID])->LEVEL_closedSets[0].count	              = Data->singletons_support[current_first_item[th_ID]];
    (threads[th_ID])->LEVEL_closedSets[0].isSectionIncluded   = Data->belongs_to_SE(current_first_item[th_ID]);
    (threads[th_ID])->LEVEL_closedSets[0].head_list	      = Data->VD + current_first_item[th_ID]*Data->tid_list_size;
    (threads[th_ID])->LEVEL_closedSets[0].tail_list	      = Data->VD + current_first_item[th_ID]*Data->tid_list_size + Data->sz_sect_eq; 

    if (Data->belongs_to_SE(current_first_item[th_ID]))
      (threads[th_ID])->LEVEL_closedSets[0].sect_eq_num_bit	   = Data->n_bit_set_eq; 
    else
      (threads[th_ID])->LEVEL_closedSets[0].sect_eq_num_bit	   = 0; 
    (threads[th_ID])->LEVEL_closedSets[0].bytes_included_sect_eq   = 0; 
    (threads[th_ID])->LEVEL_closedSets[0].old_bits_included        = 0;
    (threads[th_ID])->LEVEL_closedSets[0].bits_included            = 0;
    (threads[th_ID])->LEVEL_closedSets[0].old_min_included         = 0;
    (threads[th_ID])->LEVEL_closedSets[0].min_included             = 0;

    (threads[th_ID])->LEVEL_pre_list_lung[0]       = current_second_item[th_ID]-1;
    T l = 0;
    for (T i=0; i<current_second_item[th_ID]; i++) 
      if (i!=current_first_item[th_ID])
	(threads[th_ID])->STATIC_pre_list[l++]         = i;
    (threads[th_ID])->LEVEL_pre_list_included[0]   = (threads[th_ID])->STATIC_pre_list_included;
    (threads[th_ID])->LEVEL_pre_list_included[1]   = (threads[th_ID])->STATIC_pre_list_included + Data->end_sect_eq;
    memset((threads[th_ID])->LEVEL_pre_list_included[0], 0, sizeof(T1)*Data->end_sect_eq);

    l = 0;
    for (T i=current_second_item[th_ID]; i<Data->end_sect_eq; i++) {
      (threads[th_ID])->STATIC_post_list[l].id       = i;
      (threads[th_ID])->STATIC_post_list[l].included = 0;
      l++;
    }

    (threads[th_ID])->LEVEL_post_list[0] = (threads[th_ID])->STATIC_post_list;
    (threads[th_ID])->LEVEL_post_list[1] = (threads[th_ID])->STATIC_post_list + l;

    (threads[th_ID])->LEVEL_ub[0]     = 1;  // mine only one item of the second level
    (threads[th_ID])->initial_level   = 0;

//     cout << "thread " << th_ID 
// 	 << " has taken " << (unsigned int) current_first_item[th_ID] 
// 	 << ","           << (unsigned int)current_second_item[th_ID] 
// 	 << " (" << (unsigned int)max_item << ")" << endl;

    current_second_item[th_ID]+= no_of_threads;

    // leave critic section
    pthread_mutex_unlock(&master_lock);

    return true;
  }


private:
  DCI_VD<T,T1>* Data;
  unsigned int no_of_threads;
  Worker<T,T1>** threads;

  pthread_mutex_t master_lock;
  T* current_first_item;
  T* current_second_item;
  T max_item;
};


/*
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Multi-thread, no projection, 2-nd level
 * dinamic paritioning, dinamic assignment
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
#ifdef WORK_STEALING

template <class T, class T1>
class DP_DA_2 : public Policy<T,T1> {
public:
  DP_DA_2(DCI_VD<T,T1>* d, unsigned int n, Worker<T,T1>** w) {
    Data = d;
    no_of_threads = n;
    threads = w;
    pthread_mutex_init(&master_lock, NULL);
    current_first_item  = 0;
    current_second_item = 1;
    max_item = Data->ub;
    if (max_item > Data->end_sect_eq-1)
      max_item = Data->end_sect_eq-1;

    last_stolen = 0;
    first = true;
  }
  
  ~DP_DA_2() {
    pthread_mutex_destroy(&master_lock);
  }
  
  bool get_job(unsigned int th_ID) {
    // enter critic section
    pthread_mutex_lock(&master_lock);

    // if it is the first job
    if (first) {
      first = false;
      // set up for current item
      (threads[th_ID])->initial_level = 1;
      (threads[th_ID])->current_level = 1;
      (threads[th_ID])->LEVEL_post_i[1]  = 0;

      (threads[th_ID])->LEVEL_closedSets[1].lung                     = 0; 
      (threads[th_ID])->LEVEL_closedSets[1].count	               = 0;
      (threads[th_ID])->LEVEL_closedSets[1].isSectionIncluded	       = false;
      (threads[th_ID])->LEVEL_closedSets[1].head_list		       = (threads[th_ID])->STATIC_TID_LISTS;
      (threads[th_ID])->LEVEL_closedSets[1].tail_list		       = (threads[th_ID])->STATIC_TID_LISTS + Data->sz_sect_eq; 
      memset((threads[th_ID])->STATIC_TID_LISTS, 0xFFFFFFFF, Data->tid_list_size*sizeof(tid_list));
      (threads[th_ID])->LEVEL_closedSets[1].sect_eq_num_bit	       = 0; 
      (threads[th_ID])->LEVEL_closedSets[1].bytes_included_sect_eq   = 0; 
      (threads[th_ID])->LEVEL_closedSets[1].old_bits_included        = 0;
      (threads[th_ID])->LEVEL_closedSets[1].bits_included            = 0;
      (threads[th_ID])->LEVEL_closedSets[1].old_min_included         = 0;
      (threads[th_ID])->LEVEL_closedSets[1].min_included             = 0;
      if (Data->ub > Data->end_sect_eq)
	(threads[th_ID])->LEVEL_ub[1] = Data->end_sect_eq;
      else
	(threads[th_ID])->LEVEL_ub[1] = Data->ub;
      (threads[th_ID])->LEVEL_pre_list_lung[1]                       = 0;
      (threads[th_ID])->LEVEL_pre_list_included[1]                   = (threads[th_ID])->STATIC_pre_list_included;
      (threads[th_ID])->LEVEL_pre_list_included[2]                    = (threads[th_ID])->STATIC_pre_list_included + Data->end_sect_eq;
      memset((threads[th_ID])->LEVEL_pre_list_included[1], 0, sizeof(T1)*Data->end_sect_eq);
      for (T i=0; i<Data->end_sect_eq; i++) {
	(threads[th_ID])->STATIC_post_list[i].id       = i;
	(threads[th_ID])->STATIC_post_list[i].included = 0;
      }
      (threads[th_ID])->LEVEL_post_list[1] = (threads[th_ID])->STATIC_post_list;
      (threads[th_ID])->LEVEL_post_list[2] = (threads[th_ID])->STATIC_post_list + Data->end_sect_eq;

      // leave critic section
      (threads[th_ID])->THREAD_STATUS = Worker<T,T1>::RUNNING;

      //      cout << "thread " << th_ID << " has taken everything." << endl;

      pthread_mutex_unlock(&master_lock);
      return true;
    }
    else {
      // Look for a job to be stolen
      unsigned int t;
      for (t=0; t<no_of_threads; t++) {
	unsigned int victim = (last_stolen + t +1) % no_of_threads;
	//	cout << (int) th_ID << " tries to take over " << (int) victim << endl;

	if ( victim == th_ID )
	  continue;  // cannot steal from my self
	
	// "freeze" victim !
	pthread_mutex_lock( &((threads[victim])->job_lock) );

	// is it still running ?
	if ( (threads[victim])->THREAD_STATUS != Worker<T,T1>::RUNNING ) {
	  // cannot steal from a lazy guy
	  pthread_mutex_unlock( &((threads[victim])->job_lock) );
	  continue;
	}
	

	// is there a level from which to steal some homework
	T victim_level;
	//	cout << (int) victim << " started at " << (int) (threads[victim])->initial_level << " now at " << (int) (threads[victim])->current_level << endl;
	for ( victim_level = (threads[victim])->initial_level; 
	      victim_level <= (threads[victim])->current_level; 
	      victim_level++) {
	  // is there anything to mine at victim_level ??
	  if ( (threads[victim])->LEVEL_post_i [victim_level]+LEAST_STEALING < // +1 means at least one more!
	       (threads[victim])->LEVEL_ub     [victim_level]   )
	    break;
	  // put here some contraint maybe
	}
	
	if ( victim_level > (threads[victim])->current_level ) {
	  // no level has been found, try with someone else
	  pthread_mutex_unlock( &((threads[victim])->job_lock) );
	  continue;
	}

	// !!!!!!!!!!!!!!!   STEAL !!!!!!!!!!!!
	// ** set up all the stuff **
	last_stolen = victim;
        T skip = ( (threads[victim])->LEVEL_ub     [victim_level] - 
                   (threads[victim])->LEVEL_post_i [victim_level] - 1 ) /2; // steal only half	

	(threads[th_ID])->initial_level = 1;
	(threads[th_ID])->current_level = 1;
	(threads[th_ID])->LEVEL_post_i[1]  = 0;
	
	// closed itemset
	(threads[th_ID])->LEVEL_closedSets[1].lung                = (threads[victim])->LEVEL_closedSets[victim_level].lung; 
	T i;
	for (i=0; i<(threads[victim])->LEVEL_closedSets[victim_level].lung; i++)
	  (threads[th_ID])->LEVEL_closedSets[1].items[i] = (threads[victim])->LEVEL_closedSets[victim_level].items[i]; 
	(threads[th_ID])->LEVEL_closedSets[1].count	        =  (threads[victim])->LEVEL_closedSets[victim_level].count; 
	(threads[th_ID])->LEVEL_closedSets[1].isSectionIncluded   = (threads[victim])->LEVEL_closedSets[victim_level].isSectionIncluded;
	(threads[th_ID])->LEVEL_closedSets[1].sect_eq_num_bit	     = (threads[victim])->LEVEL_closedSets[victim_level].sect_eq_num_bit; 
	(threads[th_ID])->LEVEL_closedSets[1].bytes_included_sect_eq   = (threads[victim])->LEVEL_closedSets[victim_level].bytes_included_sect_eq; 
	(threads[th_ID])->LEVEL_closedSets[1].old_bits_included        = (threads[victim])->LEVEL_closedSets[victim_level].old_bits_included;
	(threads[th_ID])->LEVEL_closedSets[1].bits_included            = (threads[victim])->LEVEL_closedSets[victim_level].bits_included;
	(threads[th_ID])->LEVEL_closedSets[1].old_min_included         = (threads[victim])->LEVEL_closedSets[victim_level].old_min_included;
	(threads[th_ID])->LEVEL_closedSets[1].min_included             = (threads[victim])->LEVEL_closedSets[victim_level].min_included;
	
	// tid_lists
	(threads[th_ID])->LEVEL_closedSets[1].head_list		= (threads[th_ID])->STATIC_TID_LISTS;
	(threads[th_ID])->LEVEL_closedSets[1].tail_list		= (threads[th_ID])->STATIC_TID_LISTS + Data->sz_sect_eq; 
	memcpy( (threads[th_ID])->LEVEL_closedSets[1].head_list, 
		(threads[victim])->LEVEL_closedSets[victim_level].head_list,
		(threads[victim])->Data->sz_sect_eq*sizeof(tid_list));
	memcpy( (threads[th_ID])->LEVEL_closedSets[1].tail_list, 
		(threads[victim])->LEVEL_closedSets[victim_level].tail_list,
		(threads[victim])->Data->sz_tail*sizeof(tid_list));
	
	// pre lists
	(threads[th_ID])->LEVEL_pre_list_included[1]  = (threads[th_ID])->STATIC_pre_list_included;
	(threads[th_ID])->LEVEL_pre_list_included[2]  = (threads[th_ID])->STATIC_pre_list_included + Data->end_sect_eq;
	memcpy( (threads[th_ID])->LEVEL_pre_list_included[1], 
		(threads[victim])->LEVEL_pre_list_included[victim_level],
		(threads[victim])->LEVEL_pre_list_lung[victim_level]*sizeof(T1));
	
	(threads[th_ID])->LEVEL_pre_list_lung[1]      = (threads[victim])->LEVEL_pre_list_lung[victim_level]
                                                        + 1            // current post_i
                                                        + skip; // additional items not stolen
        // old pre list
	for (i=0; i<(threads[victim])->LEVEL_pre_list_lung[victim_level]; i++) 
	  (threads[th_ID])->STATIC_pre_list[i]         = (threads[victim])->STATIC_pre_list[i];
        // corrent post_i
	(threads[th_ID])->STATIC_pre_list[i]           = (threads[victim])->LEVEL_post_list[victim_level][(threads[victim])->LEVEL_post_i[victim_level]].id;
	(threads[th_ID])->LEVEL_pre_list_included[1][i]  = 0;
        // other skipped
        for (T j=0; j<skip; j++) { 
           i++;
           (threads[th_ID])->STATIC_pre_list[i]           = (threads[victim])->LEVEL_post_list[victim_level][(threads[victim])->LEVEL_post_i[victim_level]+1+j].id;
           (threads[th_ID])->LEVEL_pre_list_included[1][i]  = 0;
        }
	
	// post list
	POST_item<T,T1>* last_post_i  = (threads[victim])->LEVEL_post_list[victim_level] 
	                              + (threads[victim])->LEVEL_post_i[victim_level] 
                                      + 1
                                      + skip;
	T  post_list_lung             = (threads[victim])->LEVEL_post_list[victim_level+1] - last_post_i;

	(threads[th_ID])->LEVEL_post_list[1] = (threads[th_ID])->STATIC_post_list;
	(threads[th_ID])->LEVEL_post_list[2] = (threads[th_ID])->STATIC_post_list + post_list_lung;
	for ( i=0; i<post_list_lung; i++) {
	  (threads[th_ID])->LEVEL_post_list[1][i].id       = last_post_i[i].id;
	  (threads[th_ID])->LEVEL_post_list[1][i].included = last_post_i[i].included;
	}
	
//	if ( (threads[victim])->LEVEL_ub[victim_level] - (threads[victim])->LEVEL_post_i[victim_level] -1 -skip
//	     < post_list_lung )
//	  (threads[th_ID])->LEVEL_ub[1]  = (threads[victim])->LEVEL_ub[victim_level] - (threads[victim])->LEVEL_post_i[victim_level] -1-skip;  
//	else	
//	  (threads[th_ID])->LEVEL_ub[1] = post_list_lung;

        (threads[th_ID])->LEVEL_ub[1]  = (threads[victim])->LEVEL_ub[victim_level] 
                                       - (threads[victim])->LEVEL_post_i[victim_level] 
                                       - 1
                                       - skip;  

	(threads[victim])->LEVEL_ub[victim_level]  = (threads[victim])->LEVEL_post_i[victim_level] 
                                                     + 1
                                                     + skip; // stop with this level
	                                                       // other levels should be already useless !?!??
	
	(threads[th_ID])->THREAD_STATUS = Worker<T,T1>::RUNNING;
	

// 	cout << "thread " << (int) th_ID << " has taken :";
// 	for (i=0; i<(threads[th_ID])->LEVEL_closedSets[1].lung; i++)
// 	  cout << " " << (int) (threads[th_ID])->LEVEL_closedSets[1].items[i];
// 	cout << endl;
// 	cout << "with post lists ";
// 	for (POST_item<T,T1>* ii=(threads[th_ID])->LEVEL_post_list[1];
// 	     ii != (threads[th_ID])->LEVEL_post_list[2];
// 	     ii++) 
// 	  cout << " " << (int) ii->id;
// 	cout << endl;
//       cout << "level ub " << (int) (threads[th_ID])->LEVEL_ub[1] << endl;

	// unlock victim
	pthread_mutex_unlock( &((threads[victim])->job_lock) );
	break;
      }

      if (t == no_of_threads) {
	// no jobs left
	// leave critic section
	(threads[th_ID])->THREAD_STATUS = Worker<T,T1>::FINISHED;
	pthread_mutex_unlock(&master_lock);
	return false;
      } else {
	(threads[th_ID])->THREAD_STATUS = Worker<T,T1>::RUNNING;
	pthread_mutex_unlock(&master_lock);
	return true;
      }
    }
  }


private:
  DCI_VD<T,T1>* Data;
  unsigned int no_of_threads;
  Worker<T,T1>** threads;

  pthread_mutex_t master_lock;
  T current_first_item;
  T current_second_item;
  T max_item;
  unsigned int last_stolen;
  bool first;
};



template <class T, class T1>
class DP_DA_2P : public Policy<T,T1> {
public:
  DP_DA_2P(DCI_VD<T,T1>* d, unsigned int n, Worker<T,T1>** w) {
    Data = d;
    datasets = new DCI_VD<T,T1>* [Data->end_sect_eq];
    if (!datasets) fatal_error("Could not allocate memory.");
    for (unsigned int i=0; i<Data->end_sect_eq; i++)
      datasets[i] = NULL;
    no_of_threads = n;
    threads = w;
    pthread_mutex_init(&master_lock, NULL);
    current_first_item  = 0;
    max_item = Data->ub;
    if (max_item > Data->end_sect_eq-1)
      max_item = Data->end_sect_eq-1;

    in_use = new int [Data->end_sect_eq];
    for (unsigned int i=0; i<Data->end_sect_eq; i++)
      in_use[i] = 0;

    last_stolen = 0;
  }
  
  ~DP_DA_2P() {
    pthread_mutex_destroy(&master_lock);
    if (datasets) {
      for (unsigned int i=0; i<Data->end_sect_eq; i++)
	if (datasets[i] && datasets[i]!= Data)
	  delete datasets[i];
      delete [] datasets;
    }
    if (in_use)
      delete [] in_use;
  }

  void assign_single(Worker<T,T1>* dest) {
    T proj_item = dest->LEVEL_closedSets[1].items[0];


    // Which dataset to work with ??
    DCI_VD<T,T1>* current_vd = datasets[proj_item];
    if (current_vd == NULL) {
      if ( // false &&
	   proj_item > Data->end_sect_eq/6 &&
	   proj_item < Data->end_sect_eq/6*5) {
	// project
	//	cout << dest->thID << " will build the projection on " << (int)proj_item << endl;

#ifdef THREADS_VERBOSE
	Chronos projTime;
	projTime.StartChronos();
#endif

	current_vd = datasets[proj_item] = Data->CreateNewDataset(proj_item); 

#ifdef THREADS_VERBOSE
	dest->projection_time += projTime.ReadChronos();
#endif
      } else {
	// don't project
	//	cout << dest->thID << " will NOT build any projection on " << (int)proj_item << endl;
	current_vd = Data;
      }
    }

    T current_second_item = proj_item+1;
    // set up for current item
    dest->setVD(current_vd);

    dest->LEVEL_closedSets[1].lung                = 1; 
    dest->LEVEL_closedSets[1].items[0]            = proj_item; 
    dest->LEVEL_closedSets[1].count               = current_vd->singletons_support[proj_item];
    dest->LEVEL_closedSets[1].isSectionIncluded   = current_vd->belongs_to_SE(proj_item);
    dest->LEVEL_closedSets[1].head_list	      = current_vd->VD + proj_item*current_vd->tid_list_size;
    dest->LEVEL_closedSets[1].tail_list	      = current_vd->VD + proj_item*current_vd->tid_list_size + current_vd->sz_sect_eq; 

    if (current_vd->belongs_to_SE(proj_item))
      dest->LEVEL_closedSets[1].sect_eq_num_bit	   = current_vd->n_bit_set_eq; 
    else
      dest->LEVEL_closedSets[1].sect_eq_num_bit	   = 0; 
    dest->LEVEL_closedSets[1].bytes_included_sect_eq   = 0; 
    dest->LEVEL_closedSets[1].old_bits_included        = 0;
    dest->LEVEL_closedSets[1].bits_included            = 0;
    dest->LEVEL_closedSets[1].old_min_included         = 0;
    dest->LEVEL_closedSets[1].min_included             = 0;

    dest->LEVEL_pre_list_lung[1]       = current_second_item-1;
    T l = 0;
    for (T i=0; i<current_second_item; i++) 
      if (i!=proj_item)
	dest->STATIC_pre_list[l++]         = i;
    dest->LEVEL_pre_list_included[1]   = dest->STATIC_pre_list_included;
    dest->LEVEL_pre_list_included[2]   = dest->STATIC_pre_list_included + current_vd->end_sect_eq;
    memset(dest->LEVEL_pre_list_included[1], 0, sizeof(T1)*current_vd->end_sect_eq);

    l = 0;
    for (T i=current_second_item; i<current_vd->end_sect_eq; i++) {
      dest->STATIC_post_list[l].id       = i;
      dest->STATIC_post_list[l].included = 0;
      l++;
    }

    dest->LEVEL_post_list[1] = dest->STATIC_post_list;
    dest->LEVEL_post_list[2] = dest->STATIC_post_list + l;

    dest->LEVEL_ub[1]     = 0;
    dest->LEVEL_ub[1]     = l;  // mine only one item of the second level
    dest->initial_level   = 1;
    dest->LEVEL_post_i[1] = 0;
    dest->current_level   = 1;

    //    current_first_item++;
  }
  
  bool get_job(unsigned int th_ID) {
    // enter critic section
    pthread_mutex_lock(&master_lock);

    // Look for a job to be stolen
    unsigned int t;
    for (t=0; t<no_of_threads; t++) {
      unsigned int victim = (last_stolen + t +1) % no_of_threads;
      if ( victim == th_ID )
	continue;  // cannot steal from my self

      // "freeze" victim !
      pthread_mutex_lock( &((threads[victim])->job_lock) );

      if ( (threads[victim])->THREAD_STATUS != Worker<T,T1>::RUNNING ) {
	pthread_mutex_unlock( &((threads[victim])->job_lock) );
	// cannot steal from a lazy guy
	continue;
      }
	

      // is there a level from which to steal some homework
      T victim_level;
      for ( victim_level =  (threads[victim])->initial_level; 
	    victim_level <= (threads[victim])->current_level; 
	    victim_level++) {
	// is there anything to mine at victim_level ??
// 	cout << (int) th_ID << " is checking " << (int) victim << " at level " << (int) victim_level 
// 	     << " current level is " << (int)  (threads[victim])->current_level << endl;

	if ( (threads[victim])->LEVEL_post_i [victim_level]+LEAST_STEALING < // +1 means at least one more!
	     (threads[victim])->LEVEL_ub     [victim_level]   )
	  break;
      }
	
      if ( victim_level > (threads[victim])->current_level ) {
	// no level has been found, try with someone else
	pthread_mutex_unlock( &((threads[victim])->job_lock) );
	continue;
      }

      // !!!!!!!!!!!!!!!   STEAL !!!!!!!!!!!!
      if ( (threads[th_ID])->THREAD_STATUS == Worker<T,T1>::RUNNING ) {
	// old dataset
	if ( (--in_use[ (threads[th_ID])->LEVEL_closedSets[1].items[0] ]) == 0) {
	  if ( (threads[victim])->Data != (threads[th_ID])->Data ) { // did the dataset change ???	
	    if ( datasets[ (threads[th_ID])->LEVEL_closedSets[1].items[0] ] != NULL &&
		 datasets[ (threads[th_ID])->LEVEL_closedSets[1].items[0] ] != Data ) {
	      delete datasets [ (threads[th_ID])->LEVEL_closedSets[1].items[0] ];
	      datasets [ (threads[th_ID])->LEVEL_closedSets[1].items[0] ] = NULL;
	    }
	  }
	}
      }
      // new dataset
      in_use[ (threads[victim])->LEVEL_closedSets[victim_level].items[0] ]++;


      // ** set up all the stuff **
      last_stolen = victim;

      T skip = ( (threads[victim])->LEVEL_ub     [victim_level] - 
		 (threads[victim])->LEVEL_post_i [victim_level] - 1 ) /2; // steal only half	

      (threads[th_ID])->Data =  (threads[victim])->Data;
      (threads[th_ID])->initial_level = 1;
      (threads[th_ID])->current_level = 1;
      (threads[th_ID])->LEVEL_post_i[1]  = 0;
	
      // closed itemset
      (threads[th_ID])->LEVEL_closedSets[1].lung                = (threads[victim])->LEVEL_closedSets[victim_level].lung; 
      T i;
      for (i=0; i<(threads[victim])->LEVEL_closedSets[victim_level].lung; i++)
	(threads[th_ID])->LEVEL_closedSets[1].items[i] = (threads[victim])->LEVEL_closedSets[victim_level].items[i]; 
      (threads[th_ID])->LEVEL_closedSets[1].count	        =  (threads[victim])->LEVEL_closedSets[victim_level].count; 
      (threads[th_ID])->LEVEL_closedSets[1].isSectionIncluded   = (threads[victim])->LEVEL_closedSets[victim_level].isSectionIncluded;
      (threads[th_ID])->LEVEL_closedSets[1].sect_eq_num_bit	     = (threads[victim])->LEVEL_closedSets[victim_level].sect_eq_num_bit; 
      (threads[th_ID])->LEVEL_closedSets[1].bytes_included_sect_eq   = (threads[victim])->LEVEL_closedSets[victim_level].bytes_included_sect_eq; 
      (threads[th_ID])->LEVEL_closedSets[1].old_bits_included        = (threads[victim])->LEVEL_closedSets[victim_level].old_bits_included;
      (threads[th_ID])->LEVEL_closedSets[1].bits_included            = (threads[victim])->LEVEL_closedSets[victim_level].bits_included;
      (threads[th_ID])->LEVEL_closedSets[1].old_min_included         = (threads[victim])->LEVEL_closedSets[victim_level].old_min_included;
      (threads[th_ID])->LEVEL_closedSets[1].min_included             = (threads[victim])->LEVEL_closedSets[victim_level].min_included;
      
      // tid_lists
      (threads[th_ID])->LEVEL_closedSets[1].head_list		= (threads[th_ID])->STATIC_TID_LISTS;
      (threads[th_ID])->LEVEL_closedSets[1].tail_list		= (threads[th_ID])->STATIC_TID_LISTS + (threads[th_ID])->Data->sz_sect_eq; 
      memcpy( (threads[th_ID])->LEVEL_closedSets[1].head_list, 
	      (threads[victim])->LEVEL_closedSets[victim_level].head_list,
	      (threads[victim])->Data->sz_sect_eq*sizeof(tid_list));
      memcpy( (threads[th_ID])->LEVEL_closedSets[1].tail_list, 
	      (threads[victim])->LEVEL_closedSets[victim_level].tail_list,
	      (threads[victim])->Data->sz_tail*sizeof(tid_list));
      
      // pre lists
      (threads[th_ID])->LEVEL_pre_list_included[1]  = (threads[th_ID])->STATIC_pre_list_included;
      (threads[th_ID])->LEVEL_pre_list_included[2]  = (threads[th_ID])->STATIC_pre_list_included + (threads[th_ID])->Data->end_sect_eq;
      memcpy( (threads[th_ID])->LEVEL_pre_list_included[1], 
	      (threads[victim])->LEVEL_pre_list_included[victim_level],
	      (threads[victim])->LEVEL_pre_list_lung[victim_level]*sizeof(T1));
	
      (threads[th_ID])->LEVEL_pre_list_lung[1]      = (threads[victim])->LEVEL_pre_list_lung[victim_level]+1
	                                              + skip; // +1 is the current post_i
      for (i=0; i<(threads[victim])->LEVEL_pre_list_lung[victim_level]; i++) 
	(threads[th_ID])->STATIC_pre_list[i]         = (threads[victim])->STATIC_pre_list[i];
      (threads[th_ID])->STATIC_pre_list[i]           = (threads[victim])->LEVEL_post_list[victim_level][(threads[victim])->LEVEL_post_i[victim_level]].id;
      (threads[th_ID])->LEVEL_pre_list_included[1][i]  = 0;
      // other skipped
      for (T j=0; j<skip; j++) { 
	i++;
	(threads[th_ID])->STATIC_pre_list[i]           = (threads[victim])->LEVEL_post_list[victim_level][(threads[victim])->LEVEL_post_i[victim_level]+1+j].id;
	(threads[th_ID])->LEVEL_pre_list_included[1][i]  = 0;
      }
	
      // post list
//       POST_item<T,T1>* last_post_i  = (threads[victim])->LEVEL_post_list[victim_level] 
// 	+ (threads[victim])->LEVEL_post_i[victim_level] +1;
	POST_item<T,T1>* last_post_i  = (threads[victim])->LEVEL_post_list[victim_level] 
	                              + (threads[victim])->LEVEL_post_i[victim_level] 
                                      + 1
                                      + skip;
      T  post_list_lung             = (threads[victim])->LEVEL_post_list[victim_level+1] - last_post_i;
      
      (threads[th_ID])->LEVEL_post_list[1] = (threads[th_ID])->STATIC_post_list;
      (threads[th_ID])->LEVEL_post_list[2] = (threads[th_ID])->STATIC_post_list + post_list_lung;
      for ( i=0; i<post_list_lung; i++) {
	(threads[th_ID])->LEVEL_post_list[1][i].id       = last_post_i[i].id;
	(threads[th_ID])->LEVEL_post_list[1][i].included = last_post_i[i].included;
      }
      
//       if ( (threads[victim])->LEVEL_ub[victim_level] - (threads[victim])->LEVEL_post_i[victim_level] -1
// 	   < post_list_lung )
// 	(threads[th_ID])->LEVEL_ub[1]  = (threads[victim])->LEVEL_ub[victim_level] - (threads[victim])->LEVEL_post_i[victim_level] -1;  
//       else	
// 	(threads[th_ID])->LEVEL_ub[1] = post_list_lung;
        (threads[th_ID])->LEVEL_ub[1]  = (threads[victim])->LEVEL_ub[victim_level] 
                                       - (threads[victim])->LEVEL_post_i[victim_level] 
                                       - 1
                                       - skip;  
      
//       (threads[victim])->LEVEL_ub[victim_level]  = 0; // stop with this level
//                                                       // other levels should be already useless
	(threads[victim])->LEVEL_ub[victim_level]  = (threads[victim])->LEVEL_post_i[victim_level] 
                                                     + 1
                                                     + skip; // stop with this level
	                                                       // other levels should be already useless !?!??
	
    
      pthread_mutex_unlock( &((threads[victim])->job_lock) );
      (threads[th_ID])->THREAD_STATUS = Worker<T,T1>::RUNNING;


//       cout << "thread " << (int) th_ID << " has taken :";
//       for (i=0; i<(threads[th_ID])->LEVEL_closedSets[1].lung; i++)
// 	cout << " " << (int) (threads[th_ID])->LEVEL_closedSets[1].items[i];
//       cout << endl;
//       cout << "with post lists ";
//       for (POST_item<T,T1>* ii=(threads[th_ID])->LEVEL_post_list[1];
// 	   ii != (threads[th_ID])->LEVEL_post_list[2];
// 	   ii++) 
// 	cout << " " << (int) ii->id;
//       cout << endl;
      
      pthread_mutex_unlock(&master_lock);
      return true;
    }

    // none to stole from has been found
    // assign the first item
    if (current_first_item<max_item) {

      if ( (threads[th_ID])->THREAD_STATUS == Worker<T,T1>::RUNNING ) {
	// old dataset
	if ( (--in_use[ (threads[th_ID])->LEVEL_closedSets[1].items[0] ]) == 0) {
	  if ( datasets[ (threads[th_ID])->LEVEL_closedSets[1].items[0] ] != NULL &&
	       datasets[ (threads[th_ID])->LEVEL_closedSets[1].items[0] ] != Data ) {
	    delete datasets [ (threads[th_ID])->LEVEL_closedSets[1].items[0] ];
	    datasets [ (threads[th_ID])->LEVEL_closedSets[1].items[0] ] = NULL;
	  }
	}
      }
      // new dataset
      in_use[ current_first_item ] ++;

      // assign projection item
      (threads[th_ID])->LEVEL_closedSets[1].items[0] = current_first_item;
      current_first_item++;

      (threads[th_ID])->THREAD_STATUS = Worker<T,T1>::NOT_STARTED; // do not steal from this !!!

      pthread_mutex_unlock(&master_lock); // allow others to continue

      assign_single(threads[th_ID]);

      // leave critic section
      (threads[th_ID])->THREAD_STATUS = Worker<T,T1>::RUNNING;
      
      return true;
    } else {
      // no jobs left
      // leave critic section
      (threads[th_ID])->THREAD_STATUS = Worker<T,T1>::FINISHED;
      pthread_mutex_unlock(&master_lock);
      return false;
    }
  }


private:
  DCI_VD<T,T1>* Data;
  DCI_VD<T,T1>** datasets; // one for each frequent item
  unsigned int no_of_threads;
  Worker<T,T1>** threads;

  pthread_mutex_t master_lock;
  T current_first_item;
  T max_item;
  unsigned int last_stolen;

  int* in_use;
};

#endif


/*
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Multi-thread, no projection, 2-nd level
 * static paritioning, dinamic assignment
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
template <class T, class T1>
class SP_DA_2P : public Policy<T,T1> {
public:
  SP_DA_2P(DCI_VD<T,T1>* d, unsigned int n, Worker<T,T1>** w) {
    Data = d;
    datasets = new DCI_VD<T,T1>* [Data->end_sect_eq];
    if (!datasets) fatal_error("Could not allocate memory.");
    for (unsigned int i=0; i<Data->end_sect_eq; i++)
      datasets[i] = NULL;
    no_of_threads = n;
    threads = w;
    pthread_mutex_init(&master_lock, NULL);
    current_first_item  = 0;
    current_second_item = 1;
    max_item = Data->ub;
    if (max_item > Data->end_sect_eq-1)
      max_item = Data->end_sect_eq-1;

    in_use = new int [Data->end_sect_eq];
    for (unsigned int i=0; i<Data->end_sect_eq; i++)
      in_use[i] = 0;
  }
  
  ~SP_DA_2P() {
    pthread_mutex_destroy(&master_lock);
    if (datasets) {
      for (unsigned int i=0; i<Data->end_sect_eq; i++)
	if (datasets[i] && datasets[i]!= Data)
	  delete datasets[i];
      delete [] datasets;
    }
    if (in_use)
      delete [] in_use;
  }
  
  bool get_job(unsigned int th_ID) {
    // enter critic section
    pthread_mutex_lock(&master_lock);

    if (current_second_item >= Data->end_sect_eq) {
      // go to the next item
      current_first_item++;
      current_second_item = current_first_item+1;
    }
      
    if ( current_first_item  >= max_item) {
      // leave critic section
      (threads[th_ID])->THREAD_STATUS = Worker<T,T1>::FINISHED;
      pthread_mutex_unlock(&master_lock);
      // end of work
      return false;
    }

    // Which dataset to work with ??
    DCI_VD<T,T1>* current_vd = datasets[current_first_item];
    if (current_vd == NULL) {
      if ( current_first_item > Data->end_sect_eq/6 &&
	   current_first_item < Data->end_sect_eq/6*5) {
	// project
	//	    cout << th_ID << " will build the projection on " << (int)current_first_item+1 << endl;

#ifdef THREADS_VERBOSE
	Chronos projTime;
	projTime.StartChronos();
#endif
	current_vd = datasets[current_first_item] = Data->CreateNewDataset(current_first_item); 

#ifdef THREADS_VERBOSE
	(threads[th_ID])->projection_time += projTime.ReadChronos();
#endif

      } else {
	// don't project
	//	    cout << th_ID << " will NOT build any projection on " << (int)current_first_item+1 << endl;
	current_vd = Data;
      }
    }



    if ( (threads[th_ID])->THREAD_STATUS == Worker<T,T1>::RUNNING ) {
      // old dataset
      if ( (--in_use[ (threads[th_ID])->LEVEL_closedSets[0].items[0] ]) == 0) {
	if (current_vd != (threads[th_ID])->Data ) { // did the dataset change ???	
	  if ( datasets[ (threads[th_ID])->LEVEL_closedSets[0].items[0] ] != NULL &&
	       datasets[ (threads[th_ID])->LEVEL_closedSets[0].items[0] ] != Data ) {
	    delete datasets [ (threads[th_ID])->LEVEL_closedSets[0].items[0] ];
	    datasets [ (threads[th_ID])->LEVEL_closedSets[0].items[0] ] = NULL;
	  }
	}
      }
    }

    // new dataset
    in_use[ current_first_item ]++;


    // set up for current item
    (threads[th_ID])->setVD(current_vd);

    (threads[th_ID])->LEVEL_closedSets[0].lung                = 1; 
    (threads[th_ID])->LEVEL_closedSets[0].items[0]            = current_first_item; 
    (threads[th_ID])->LEVEL_closedSets[0].count	              = current_vd->singletons_support[current_first_item];
    (threads[th_ID])->LEVEL_closedSets[0].isSectionIncluded   = current_vd->belongs_to_SE(current_first_item);
    (threads[th_ID])->LEVEL_closedSets[0].head_list	      = current_vd->VD + current_first_item*current_vd->tid_list_size;
    (threads[th_ID])->LEVEL_closedSets[0].tail_list	      = current_vd->VD + current_first_item*current_vd->tid_list_size + current_vd->sz_sect_eq; 

    if (current_vd->belongs_to_SE(current_first_item))
      (threads[th_ID])->LEVEL_closedSets[0].sect_eq_num_bit	   = current_vd->n_bit_set_eq; 
    else
      (threads[th_ID])->LEVEL_closedSets[0].sect_eq_num_bit	   = 0; 
    (threads[th_ID])->LEVEL_closedSets[0].bytes_included_sect_eq   = 0; 
    (threads[th_ID])->LEVEL_closedSets[0].old_bits_included        = 0;
    (threads[th_ID])->LEVEL_closedSets[0].bits_included            = 0;
    (threads[th_ID])->LEVEL_closedSets[0].old_min_included         = 0;
    (threads[th_ID])->LEVEL_closedSets[0].min_included             = 0;

    (threads[th_ID])->LEVEL_pre_list_lung[0]       = current_second_item-1;
    T l = 0;
    for (T i=0; i<current_second_item; i++) 
      if (i!=current_first_item)
	(threads[th_ID])->STATIC_pre_list[l++]         = i;
    (threads[th_ID])->LEVEL_pre_list_included[0]   = (threads[th_ID])->STATIC_pre_list_included;
    (threads[th_ID])->LEVEL_pre_list_included[1]   = (threads[th_ID])->STATIC_pre_list_included + current_vd->end_sect_eq;
    memset((threads[th_ID])->LEVEL_pre_list_included[0], 0, sizeof(T1)*current_vd->end_sect_eq);

    l = 0;
    for (T i=current_second_item; i<current_vd->end_sect_eq; i++) {
      (threads[th_ID])->STATIC_post_list[l].id       = i;
      (threads[th_ID])->STATIC_post_list[l].included = 0;
      l++;
    }

    (threads[th_ID])->LEVEL_post_list[0] = (threads[th_ID])->STATIC_post_list;
    (threads[th_ID])->LEVEL_post_list[1] = (threads[th_ID])->STATIC_post_list + l;

    (threads[th_ID])->LEVEL_ub[0]     = 1;  // mine only one item of the second level
    (threads[th_ID])->initial_level   = 0;

//     cout << "thread " << th_ID << " has taken " << (unsigned int) current_first_item << "," << (unsigned int)current_second_item 
//  	 << " (" << (unsigned int)max_item << ")" << endl;

    current_second_item++;

    (threads[th_ID])->THREAD_STATUS = Worker<T,T1>::RUNNING;
    // leave critic section
    pthread_mutex_unlock(&master_lock);

    return true;
  }


private:
  DCI_VD<T,T1>* Data;
  DCI_VD<T,T1>** datasets; // one for each frequent item
  unsigned int no_of_threads;
  Worker<T,T1>** threads;

  pthread_mutex_t master_lock;
  T current_first_item;
  T current_second_item;
  T max_item;

  int* in_use;
};



/*
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Multi-thread, no projection, 2-nd level
 * static paritioning, static assignment
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
template <class T, class T1>
class SP_SA_2P : public Policy<T,T1> {
public:
  SP_SA_2P(DCI_VD<T,T1>* d, unsigned int n, Worker<T,T1>** w) {
    Data = d;
    datasets = new DCI_VD<T,T1>* [Data->end_sect_eq];
    if (!datasets) fatal_error("Could not allocate memory.");
    for (unsigned int i=0; i<Data->end_sect_eq; i++)
      datasets[i] = NULL;
    no_of_threads = n;
    threads = w;
    pthread_mutex_init(&master_lock, NULL);
    max_item = Data->ub;
    if (max_item > Data->end_sect_eq-1)
      max_item = Data->end_sect_eq-1;

    current_first_item = new T[no_of_threads];
    if ( !current_first_item) fatal_error("Could not allocate memory.");
    for (unsigned int i=0; i<no_of_threads; i++) current_first_item[i] = 0;
    current_second_item = new T[no_of_threads];
    if ( !current_second_item) fatal_error("Could not allocate memory.");
    for (unsigned int i=0; i<no_of_threads; i++) current_second_item[i] = i+1;

    in_use = new int [Data->end_sect_eq];
    for (unsigned int i=0; i<Data->end_sect_eq; i++)
      in_use[i] = 0;
  }
  
  ~SP_SA_2P() {
    pthread_mutex_destroy(&master_lock);
    if (datasets) {
      for (unsigned int i=0; i<Data->end_sect_eq; i++)
	if (datasets[i] && datasets[i]!= Data)
	  delete datasets[i];
      delete [] datasets;
    }
    if (current_first_item)
      delete [] current_first_item;
    if (current_second_item)
      delete [] current_second_item;
    if (in_use)
      delete [] in_use;
  }
  
  bool get_job(unsigned int th_ID) {
    // enter critic section
    pthread_mutex_lock(&master_lock);
    
    while( current_second_item[th_ID] >= Data->end_sect_eq) {
      current_first_item[th_ID]++;

      if (current_first_item[th_ID] >= max_item) {
	// do not delete for now ....

	// leave critic section
	(threads[th_ID])->THREAD_STATUS = Worker<T,T1>::FINISHED;
	pthread_mutex_unlock(&master_lock);
	// end of work
	return false;
      }

      current_second_item[th_ID] = current_first_item[th_ID]+1+(current_second_item[th_ID] % no_of_threads);
    }


    // Which dataset to work with ??
    DCI_VD<T,T1>* current_vd = datasets[current_first_item[th_ID]];
    if (current_vd == NULL) {
      if ( current_first_item[th_ID] > Data->end_sect_eq/6 &&
	   current_first_item[th_ID] < Data->end_sect_eq/6*5) {
	// project
#ifdef THREADS_VERBOSE
	Chronos projTime;
	projTime.StartChronos();
#endif
	current_vd = datasets[current_first_item[th_ID]] = Data->CreateNewDataset(current_first_item[th_ID]); 

#ifdef THREADS_VERBOSE
	(threads[th_ID])->projection_time += projTime.ReadChronos();
#endif
      } else {
	// don't project
	current_vd = Data;
      }
    }

    if ( (threads[th_ID])->THREAD_STATUS == Worker<T,T1>::RUNNING ) {
      // old dataset
      if ( (--in_use[ (threads[th_ID])->LEVEL_closedSets[0].items[0] ]) == 0) {
	if (current_vd != (threads[th_ID])->Data ) { // did the dataset change ???	
	  if ( datasets[ (threads[th_ID])->LEVEL_closedSets[0].items[0] ] != NULL &&
	       datasets[ (threads[th_ID])->LEVEL_closedSets[0].items[0] ] != Data ) {
	    delete datasets [ (threads[th_ID])->LEVEL_closedSets[0].items[0] ];
	    datasets [ (threads[th_ID])->LEVEL_closedSets[0].items[0] ] = NULL;
	  }
	}
      }
    }

    // new dataset
    in_use[ current_first_item[th_ID] ]++;


    // set up for current item
    (threads[th_ID])->Data = current_vd;

    (threads[th_ID])->LEVEL_closedSets[0].lung                = 1; 
    (threads[th_ID])->LEVEL_closedSets[0].items[0]            = current_first_item[th_ID]; 
    (threads[th_ID])->LEVEL_closedSets[0].count	              = current_vd->singletons_support[current_first_item[th_ID]];
    (threads[th_ID])->LEVEL_closedSets[0].isSectionIncluded   = current_vd->belongs_to_SE(current_first_item[th_ID]);
    (threads[th_ID])->LEVEL_closedSets[0].head_list	      = current_vd->VD + current_first_item[th_ID]*current_vd->tid_list_size;
    (threads[th_ID])->LEVEL_closedSets[0].tail_list	      = current_vd->VD + current_first_item[th_ID]*current_vd->tid_list_size + current_vd->sz_sect_eq; 

    if (current_vd->belongs_to_SE(current_first_item[th_ID]))
      (threads[th_ID])->LEVEL_closedSets[0].sect_eq_num_bit	   = current_vd->n_bit_set_eq; 
    else
      (threads[th_ID])->LEVEL_closedSets[0].sect_eq_num_bit	   = 0; 
    (threads[th_ID])->LEVEL_closedSets[0].bytes_included_sect_eq   = 0; 
    (threads[th_ID])->LEVEL_closedSets[0].old_bits_included        = 0;
    (threads[th_ID])->LEVEL_closedSets[0].bits_included            = 0;
    (threads[th_ID])->LEVEL_closedSets[0].old_min_included         = 0;
    (threads[th_ID])->LEVEL_closedSets[0].min_included             = 0;

    (threads[th_ID])->LEVEL_pre_list_lung[0]       = current_second_item[th_ID]-1;
    T l = 0;
    for (T i=0; i<current_second_item[th_ID]; i++) 
      if (i!=current_first_item[th_ID])
	(threads[th_ID])->STATIC_pre_list[l++]         = i;
    (threads[th_ID])->LEVEL_pre_list_included[0]   = (threads[th_ID])->STATIC_pre_list_included;
    (threads[th_ID])->LEVEL_pre_list_included[1]   = (threads[th_ID])->STATIC_pre_list_included + current_vd->end_sect_eq;
    memset((threads[th_ID])->LEVEL_pre_list_included[0], 0, sizeof(T1)*current_vd->end_sect_eq);

    l = 0;
    for (T i=current_second_item[th_ID]; i<current_vd->end_sect_eq; i++) {
      (threads[th_ID])->STATIC_post_list[l].id       = i;
      (threads[th_ID])->STATIC_post_list[l].included = 0;
      l++;
    }

    (threads[th_ID])->LEVEL_post_list[0] = (threads[th_ID])->STATIC_post_list;
    (threads[th_ID])->LEVEL_post_list[1] = (threads[th_ID])->STATIC_post_list + l;

    (threads[th_ID])->LEVEL_ub[0]     = 1;  // mine only one item of the second level
    (threads[th_ID])->initial_level   = 0;

//     cout << "thread " << th_ID 
// 	 << " has taken " << (unsigned int) current_first_item[th_ID] 
// 	 << ","           << (unsigned int)current_second_item[th_ID] 
// 	 << " (" << (unsigned int)max_item << ")" << endl;

    current_second_item[th_ID]+= no_of_threads;

    (threads[th_ID])->THREAD_STATUS = Worker<T,T1>::RUNNING;

    // leave critic section
    pthread_mutex_unlock(&master_lock);

    return true;
  }


private:
  DCI_VD<T,T1>* Data;
  unsigned int no_of_threads;
  Worker<T,T1>** threads;
  DCI_VD<T,T1>** datasets; // one for each frequent item

  pthread_mutex_t master_lock;
  T* current_first_item;
  T* current_second_item;
  T max_item;

  int* in_use;
};








template <class T, class T1>
class Master {
public:
  Master(DCI_VD<T,T1>* d, unsigned int threads, FILE** of) {
    no_of_threads = threads;
    Data = d;
    output_files = of;
  }
  
  ~Master() {
  }
  
  void start() {
    // allocate workers pointers
    workers = new Worker<T,T1>* [no_of_threads];

    // allocate policy
    // switch different policies
    switch (PARTITIONING_POLICY) {
    case SIMPLEST_POLICY:
	policy = new Simplest_Policy<T,T1>(Data, no_of_threads, workers);
      break;
    case ONE_THREAD_POLICY:
	policy = new One_Thread_Policy<T,T1>(Data, no_of_threads, workers);
	break;
    case STATIC_DINAMIC:
	policy = new SP_DA_2<T,T1>(Data, no_of_threads, workers);
      break;
    case STATIC_STATIC:
	policy = new SP_SA_2<T,T1>(Data, no_of_threads, workers);
      break;
#ifdef WORK_STEALING
    case DINAMIC_DINAMIC:
	policy = new DP_DA_2<T,T1>(Data, no_of_threads, workers);
      break;
#endif
    case STATIC_STATIC_P:
	policy = new SP_SA_2P<T,T1>(Data, no_of_threads, workers);
      break;
    case STATIC_DINAMIC_P:
	policy = new SP_DA_2P<T,T1>(Data, no_of_threads, workers);
      break;
#ifdef WORK_STEALING
    case DINAMIC_DINAMIC_P:
	policy = new DP_DA_2P<T,T1>(Data, no_of_threads, workers);
      break;
#endif
    default:
      fatal_error("Wrong partitioning policy definition.");
    }

    if (!policy) fatal_error("Could not allocate policy");
    policy->init_mx(no_of_threads);

    // create workers
    pthread_t* thread_ID = new pthread_t [no_of_threads];
    if (!thread_ID) fatal_error("Could not allocate memory for threads ids");

    // allocate workers instances
    for (unsigned int i = 0; i<no_of_threads; i++) {
      workers[i] = new Worker<T,T1>(this, i, policy, output_files[i]);
      (workers[i])->setVD(Data);
    }
    // launch thread
    for (unsigned int i = 0; i<no_of_threads; i++) {
      if ( pthread_create( thread_ID+i, NULL, (workers[i])->start_thread, (void*) workers[i] ) )
	fatal_error("Could not create thread");
    }

    // join threads
    for (unsigned int i = 0; i<no_of_threads; i++) {
      void* status;
      pthread_join(thread_ID[i], &status);
    }

#ifdef THREADS_VERBOSE
    double total_waiting_time = 0.0;
    double total_proj_time = 0.0;
    double total_job_time = 0.0;
    unsigned int total_n_jobs = 0;
    for (unsigned int i = 0; i<no_of_threads; i++) {
      total_waiting_time += (workers[i])->lock_time;
      total_proj_time    += (workers[i])->projection_time;
      total_job_time     += (workers[i])->get_job_time;
      total_n_jobs       += (workers[i])->no_of_jobs;
    }
    synch_printf("# ## ## ## No. of jobs      time: %d\n", total_n_jobs);
    synch_printf("# ## ## ## Total blocked    time: %f\n", total_waiting_time);
    synch_printf("# ## ## ## Total projection time: %f\n", total_proj_time);
    synch_printf("# ## ## ## Total job req    time: %f\n", total_job_time);
    synch_printf("# ## ## ## Total wasted     time: %f\n", 
		 total_job_time+total_waiting_time-total_proj_time);
#endif

    // delete workers and threads
    for (unsigned int i = 0; i<no_of_threads; i++) 
      delete workers[i];
    delete [] workers;
    delete [] thread_ID;
    delete policy;
  }


private:
  DCI_VD<T,T1>* Data;
  unsigned int no_of_threads;
  Worker<T,T1>** workers;
  Policy<T,T1>* policy;
  FILE** output_files;
};



template <class T, class T1>
class Policy {
public:
  virtual bool get_job(unsigned int th_ID) = 0;

  pthread_mutex_t mm_lock;
  Chronos* timers;
  double waiting_time;

  Policy() {
    timers = NULL;
  };

  virtual ~Policy() {
    if (timers != NULL) {
      // delete chronos
      delete [] timers;
      // delete semaphore
      pthread_mutex_destroy(&mm_lock);
    }
  };

  virtual void init_mx(unsigned int n_threads) {
    // allocate single semaphore
    pthread_mutex_init(&mm_lock, NULL);
    // allocate n_chronos
    timers = new Chronos [n_threads];
    if (!timers) fatal_error("Could not allocate memory for policy timers.");
    waiting_time = 0.0;
  }
  
  virtual void start_mx(unsigned int thID) {
    // start thread chron
    timers[thID].RestartChronos();
    // get semaphore
    pthread_mutex_lock(&mm_lock);
  }
  virtual void end_mx(unsigned int thID) {
    // end thread chron
    waiting_time += timers[thID].ReadChronos();
    // release sempahore
    pthread_mutex_unlock(&mm_lock);
  }

  virtual double mx_time() {
    // return total time
    return waiting_time;
  }
};



#endif

