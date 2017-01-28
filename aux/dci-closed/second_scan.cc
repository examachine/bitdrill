#include "second_scan.hh"

#include "my_defs.hh"
#include "stat.hh"

#include "database.hh"
#include "print.hh"
#include "utils.hh"
#include <math.h>


// instances
 
template 
unsigned int second_scan<unsigned char, unsigned char>( unsigned int &max_trans_len, 
						unsigned int singles,
						int* map,
						unsigned char* singletons_support, 
						unsigned char min_count,
						char* filename,
						float limit);
template 
unsigned int second_scan<unsigned char, unsigned short int>( unsigned int &max_trans_len, 
						     unsigned int singles,
						     int* map,
						     unsigned short int* singletons_support, 
						     unsigned short int min_count,
						     char* filename,
						     float limit);
template 
unsigned int second_scan<unsigned char, unsigned int>( unsigned int &max_trans_len, 
					       unsigned int msingles,
					       int* map,
					       unsigned int* singletons_support, 
					       unsigned int min_count,
					       char* filename,
					       float limit);

template 
unsigned int second_scan<unsigned short int, unsigned char> ( unsigned int &max_trans_len, 
						      unsigned int singles,
						      int* map,
						      unsigned char* singletons_support, 
						      unsigned char min_count,
						      char* filename,
						      float limit);
template  
unsigned int second_scan<unsigned short int, unsigned short int>( unsigned int &max_trans_len, 
							  unsigned int singles,
							  int* map,
							  unsigned short int* singletons_support, 
							  unsigned short int min_count,
							  char* filename,
							  float limit);
template 
unsigned int second_scan<unsigned short int, unsigned int>( unsigned int &max_trans_len,
						    unsigned int singles,
						    int* map,
						    unsigned int* singletons_support, 
						    unsigned int min_count,
						    char* filename,
						    float limit);
 
template  
unsigned int second_scan<unsigned int, unsigned char>( unsigned int &max_trans_len,
					       unsigned int singles,
					       int* map,
					       unsigned char* singletons_support, 
					       unsigned char min_count,
					       char* filename,
					       float limit);
template  
unsigned int second_scan<unsigned int, unsigned short int>( unsigned int &max_trans_len,
						    unsigned int singles,
						    int* map,
						    unsigned short int* singletons_support, 
						    unsigned short int min_count,
						    char* filename,
						    float limit);
template 
unsigned int second_scan <unsigned int, unsigned int>( unsigned int &max_trans_len,
					       unsigned int singles,
					       int* map,
					       unsigned int* singletons_support, 
					       unsigned int min_count,
					       char* filename,
					       float limit);


/* given side, it creates an upper triangular
 * matrix without the diagonal
 */ 
template <class T1> class partial_tri_upper {
public:
  partial_tri_upper(long items, long start, long end) {
    n = items;
    // size of the memory buffer
    int sz = (items-start)*(items-start-1)/2 - (items-end)*(items-end-1)/2;
    // skipped item w.r.t full tri matrix
    offset = (items)*(items-1)/2 - (items-start)*(items-start-1)/2;
    buf    = new T1[sz];
    if (!buf) fatal_error("Could not allocate memory of upper triangular matrix.");
    //bzero(buf, sz * sizeof(T1));
    memset(buf, 0x0, sz * sizeof(T1));
  }
  ~partial_tri_upper() {delete [] buf;}

  inline T1& operator()(long i, long j) {
    return buf[direct_position2(i, j)];
  }

  inline T1& operator[](long i) {
    return buf[i];
  }

  inline long direct_position2_init(long x) { 
    return ((x)*((n)-1)-((x)*((x)+1))/2-1) -offset; 
  }
  
  inline int direct_position2(long x, long y) {
    return ((x)*((n)-1)-((x)*((x)+1))/2+(y)-1) -offset;
  }
  
private:
  T1 *buf;
  int offset;
  int n;
};


template<class T, class T1>
unsigned int MakePartitions( unsigned int m1, 
			     T1* singletons_support, 
			     T1 min_count,
			     partial_tri_upper<T1> &c,
			     partial_tri_upper<T1> &trs,
			     float limit,
			     T lb,
			     T ub );

template <class T, class T1> 
unsigned int scan_data( unsigned int &max_trans_len, 
			unsigned int singles,  
			int* map,
			T1* singletons_support,
			T1 min_count,
			char* filename,
			float limit,
			T lb,
		 	T ub,
			vector<bool> &to_output  );


template <class T, class T1> 
unsigned int second_scan( unsigned int &max_trans_len, 
			  unsigned int singles,  
			  int* map,
			  T1* singletons_support,
			  T1 min_count,
			  char* filename,
			  float limit)
{ 

#ifdef VERBOSE
  Chronos elapsed_time;
  elapsed_time.StartChronos();

  synch_printf( "#\n"
		"# ## STEP 2 . Extract Frequent 2-Itemsets and Dataset Statistics.\n");
#endif


#ifdef MY_STATS
  SECOND_SCAN_TIME.StartChronos();  
#endif 

  unlink(MAPPED_OUTPUT); 
  unlink(PARTITIONS_FILE);

  unsigned int n_part = 0; 
  vector<bool> to_output(singles); 
  for (unsigned int i=0; i<singles; i++) 
    to_output[i]=true; 


  // lauch partitioning processes such that
  // they fit into the main memory
  float max_mem = limit / sizeof(T1); // words
  T lb = 0;
  T ub = 0;

  // while there are unpartitioned items

  while (ub < singles) {
    float skipped_mem = ub*ub-(2*singles-1)*ub;
    float delta = (2*singles-1)*(2*singles-1) - 4*(max_mem-skipped_mem);
    if (delta < 0)    
      ub = singles;
    else
      ub = lb + (int)( (2.0*singles-1.0)-sqrt(delta) )/2;
    if (lb == ub) {
      ub++;
      synch_printf( "# ## !! WARNING !! a statistics table is predicted to exceed the memory limit\n" );
    }

    n_part += scan_data<T,T1>(
			      max_trans_len, 
			      singles,  
			      map,
			      singletons_support,
			      min_count,
			      filename,
			      limit,
			      lb, 
			      ub,
			      to_output);
    lb = ub;
  }
  
//   n_part += scan_data<T,T1>(
// 					 max_trans_len, 
// 					 singles,  
// 					 map,
// 					 singletons_support,
// 					 min_count,
// 					 filename,
// 					 limit,
// 					 0, 
// 					 singles,
// 					 to_output);


#ifdef VERBOSE
  synch_printf( "# ## Number of Partitions               : %d\n", n_part);
#endif

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  //  Output frequent Itemsets of length 1  
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  FILE* output_file = fopen(MAPPED_OUTPUT,"wb");
  if (!output_file) fatal_error("Could not open output file for writing singletons.");

  for (T i=0; i < singles; i++) { 
    if (to_output[i]) {
      T lung = 1;
      fwrite(&lung, sizeof(T), 1, output_file);
      fwrite(singletons_support+i, sizeof(T1), 1, output_file);
      fwrite(&i, sizeof(T), 1, output_file);
    }  
  } 
  fclose (output_file);

#ifdef MY_STATS
  SECOND_SCAN_TIME.StopChronos();
#endif 

#ifdef VERBOSE
    synch_printf( "# ## Elapsed Time: %.2f secs.\n", elapsed_time.ReadChronos());
#endif

  return n_part;
} 


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Scan data
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <class T, class T1> 
unsigned int scan_data( unsigned int &max_trans_len, 
			unsigned int singles,  
			int* map,
			T1* singletons_support,
			T1 min_count,
			char* filename,
			float limit,
			T lb,
			T ub,
			vector<bool> &to_output )
{
  T m1 = (T) singles;  

  partial_tri_upper<T1> c   (singles, lb, ub); 
  partial_tri_upper<T1> trs (singles, lb, ub);

  binFSin<unsigned int> ii(TEMPORARY_DATASET); 
  if(!ii.isOpen()) fatal_error("Could not open binary dataset file for reading.");

  binFSin<T> ii2(TEMPORARY_DATASET); 
  if(!ii2.isOpen()) fatal_error("Could not open binary dataset file for reading.");

  binFSout<T> oo(TEMPORARY_DATASET, false); 
  if(!oo.isOpen()) fatal_error("Could not open binary dataset file for writing.");

  Transaction<unsigned int> t_in(max_trans_len); 
  Transaction<T> t_out(max_trans_len); 
 
  if (lb == 0)
    max_trans_len = 0;

#ifdef VERBOSE
  int totalsize=0; 
#endif

  // database scan 
  while (true) {
    if (lb == 0) {
      if (!ii.getNextTransaction(t_in))
	break;
      prune_and_map_and_ord_first_iter(t_in, t_out, map); 
    }
    else
      if (!ii2.getNextTransaction(t_out))
	break;

    // only transaction useful from the second iteration are saved 
    if (t_out.length > 0) { 
      // incr candidate count 
      T x0; long index_init; 
      for (T t0=0; t0 < t_out.length-1; t0++) { 
	if (t_out.t[t0] >= lb && t_out.t[t0] < ub ) {
	  x0 = t_out.t[t0]; 
	  index_init = c.direct_position2_init(x0); 
	  for (T t1=t0+1; t1 < t_out.length; t1++) 
	    c[index_init + t_out.t[t1]]++; 
// 	  for (T t1=t0+1; t1 < t_out.length; t1++) 
// 	    c( t_out.t[t0], t_out.t[t1])++; 
	}
      } 

      // change and improve this !!!
      // count transactions for each partition
      T ti = 0;
      for (T i=0; i<m1-1; i++) {
	if (i==t_out.t[ti]) {
	  // do not update singletons
	  // we already have such information
	  ti++;
	  if (ti==t_out.length)
	    break;
	} else {
	  // update transactions
	  if ( i >= lb && i < ub ) {
	    trs(i,t_out.t[ti])++;
	  }
	}
      } 
      
      // devo scrivere solo la prima volta
      // write the trans on disk
      if (lb == 0) {
	oo.writeTransaction(t_out);
	if ( t_out.length > max_trans_len )
	  max_trans_len = t_out.length;
      }

#ifdef VERBOSE
      if (lb == 0)
	totalsize += t_out.length;
#endif
    } 
  } 

  ii.close();
  ii2.close();

  // solo la prima volta
  if (lb == 0)
    oo.trunc();
  oo.close();

  // informazioni sul dataset solo la prima volta
#ifdef VERBOSE
  if (lb == 0) {
    unsigned int n_tr = oo.get_num_of_trans();
  synch_printf( "# ## DataSize : %d * %d = %d KB, dens: %.3e. \n",
		m1, n_tr,
		(totalsize + n_tr) * sizeof(T) /1024, 
		(float)totalsize / (float)(n_tr*m1) );
  synch_printf( "# ## Bytes used to store one item       : %d \n", sizeof(T));
  synch_printf( "# ## Bytes used to store item's support : %d \n", sizeof(T1));
  }
#endif

  
  T1 count; 
  int  k=0; 
    // loop over the first item 
  for (T i=lb; i < ub; i++) { 
    // loop over the second item 
    for (T j=i+1; j < m1; j++) { 
      count = c[k++];
      if (count >= min_count){  
	// yes, itemset (i,j) is frequent 
	// check closure 
	if (count == singletons_support[i]) {
	  to_output[i] = false; 
	} 
	if (count == singletons_support[j]) {
	  to_output[j] = false; 
	} 
      } 
    } 
  } 

  return MakePartitions<T1>( m1, singletons_support, min_count,
			     c, trs, limit,
			     lb, ub);
}


  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Make Partitions
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <class T, class T1>
unsigned int MakePartitions( unsigned int m1, 
			     T1* singletons_support, 
			     T1 min_count,
			     partial_tri_upper<T1> &c,
			     partial_tri_upper<T1> &trs,
			     float limit,
			     T lb,
			     T ub ) 
{
  // cout << "# ## Partition mapping from " << (int) lb << " to " << (int) ub << endl;


  // open file for storing partitioning info
  FILE* out = fopen(PARTITIONS_FILE,"ab");
  if (!out) fatal_error("Could not open partitioning info file for writing.");

  unsigned int num_of_part = 0;
  unsigned int p_trans = 0;  // no. of transactions of the new partition
  unsigned int p_items = 0;  // no. of items of the new partition
  T i = lb;

  limit *= 8; // bits

  // try to add some other singleton
  while (i < ub) { 
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // increment the no. of transactions of the new partition
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    unsigned int new_p_trans = p_trans;
    if (lb == i) 
      new_p_trans = singletons_support[lb];  // this is the first item
    else
      new_p_trans += trs(lb,i);
      
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // increment no. of items of the new partition
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    unsigned int new_p_items = p_items;
    bool is_useful = false;
    // look for new frequently co-occurring singletons
    for (T j=i+1; j<m1; j++) {
      if ( c(i,j) >= min_count ) {
	is_useful = true;
	// if we have never seen j before
	if ( i==lb || c(lb,j)==0 ) {
	  new_p_items++;
	}
      }
    }

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // check partition size
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    if (new_p_items == 0) {
      //skip this item and start partition from the next
      lb++;
      i++;
    } else {
      // add current item if never seen before
      if (is_useful)
	if ( i==lb || c(lb,i)==0 ) {
	  new_p_items++;
	}

      // check size
      if (i==lb || (float)new_p_items * (float)new_p_trans <= limit) {
	// size is ok => add the new item i to the partition
	// update dimensions
	p_items = new_p_items;
	p_trans = new_p_trans;
	// update valid items
	if (i!=lb && is_useful)
	    c(lb,i) = 1;
	for (T j=i+1; j<m1; j++) {
	  if (i==lb) 
	    c(lb,j) = c(i,j) >= min_count ? 1 : 0;
	  else
	    c(lb,j) = c(i,j) >= min_count ? 1 : c(lb,j);
	}
	i++;
      } else {
	// the item cannot fit into the partition
	// stop this partition and start another one
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// partition is [lb,i]
        num_of_part++;
	if ((float)p_items * (float)p_trans > limit) {
	  synch_printf( "# ## !! WARNING !! a partition is predicted to exceed the memory limit \n");
	} 

	// write partitions in a binary file
	unsigned int aux[] = {(unsigned int)lb, (unsigned int)i, (unsigned int) p_items, (unsigned int)p_trans, (unsigned int) lb}; 
	fwrite(&(aux[0]), 5, sizeof(unsigned int), out);
	for (unsigned int j=lb+1; j<m1; j++) 
	  if (  c(lb,j)!=0 ) 
	    fwrite(&j,  1, sizeof(unsigned int), out);	

	// restart
	lb = i;
	p_items = p_trans = 0;
      }
    }
  }

  if (p_items > 0) {
    num_of_part++;
  
    if ((float)p_items * (float)p_trans > limit) {
      synch_printf( "# ## !! WARNING !! a partition is predicted to exceed the memory limit" );
    } 
    
    // write partitioning in a binary file
    unsigned int aux[] = {(unsigned int)lb, (unsigned int)i, (unsigned int) p_items, (unsigned int)p_trans, (unsigned int) lb}; 
    fwrite(&(aux[0]), 5, sizeof(unsigned int), out);

    for (unsigned int j=lb+1; j<m1; j++) 
      if (  c(lb,j)!=0 ) 
	fwrite(&j,  1, sizeof(unsigned int), out);	
  }
  
  fclose(out);
  
  return num_of_part;
}
