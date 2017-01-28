/*
  Copyright (C) 2005  Claudio.Lucchese@ISTI.CNR.it
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
*/


#include "fourth_scan.hh"

#include <iostream>
#include <algorithm>
#include <set>
#include <sys/time.h>
#include <unistd.h>

using namespace std;

#include "utils.hh"
#include "print.hh"

struct IF {
  unsigned int fid;
  char* data;
};

template <class T, class T1>
struct _ItemsetSort
{ 
  bool operator() (char* const& a, char* const& b) { 
    T1 a_supp = *( (T1*) (a+sizeof(unsigned int)) );
    T1 b_supp = *( (T1*) (b+sizeof(unsigned int)) );
    if (a_supp < b_supp) return true;
    if (a_supp > b_supp) return false;

    T a_lung = *((T*)a);
    T b_lung = *((T*)b);
    T lung = a_lung <= b_lung ? a_lung : b_lung;

    T* a_items = (T*) (a+2*sizeof(unsigned int));
    T* b_items = (T*) (b+2*sizeof(unsigned int));
    for (T i=0; i<lung; i++) {
      if (b_items[i]>a_items[i]) return true;
      if (b_items[i]<a_items[i]) return false;
    }
    return a_lung < b_lung;
  } 
}; 


template <class T, class T1>
struct _IFSort
{ 
  bool operator() (IF const& a, IF const& b) { 
    return _ItemsetSort<T,T1>()(a.data, b.data);
  } 
}; 


template void fourth_scan <unsigned char, unsigned char> (float limit, unsigned int parts, unsigned int no_of_threads, char* output);
template void fourth_scan <unsigned char, unsigned short int> (float limit, unsigned int parts, unsigned int no_of_threads, char* output);
template void fourth_scan <unsigned char, unsigned int> (float limit, unsigned int parts, unsigned int no_of_threads, char* output);

template void fourth_scan <unsigned int, unsigned char> (float limit, unsigned int parts, unsigned int no_of_threads, char* output);
template void fourth_scan <unsigned int, unsigned short int> (float limit, unsigned int parts, unsigned int no_of_threads, char* output);
template void fourth_scan <unsigned int, unsigned int> (float limit, unsigned int parts, unsigned int no_of_threads, char* output);

template void fourth_scan <unsigned short int, unsigned char> (float limit, unsigned int parts, unsigned int no_of_threads, char* output);
template void fourth_scan <unsigned short int, unsigned short int> (float limit, unsigned int parts, unsigned int no_of_threads, char* output);
template void fourth_scan <unsigned short int, unsigned int> (float limit, unsigned int parts, unsigned int no_of_threads, char* output);



template <class T, class T1>
void partialflush (char** start, char** end, unsigned int &num);

template void partialflush <unsigned char, unsigned char> (char** start, char** end, unsigned int &num);
template void partialflush <unsigned char, unsigned short int> (char** start, char** end, unsigned int &num);
template void partialflush <unsigned char, unsigned int> (char** start, char** end, unsigned int &num);

template void partialflush <unsigned int, unsigned char> (char** start, char** end, unsigned int &num);
template void partialflush <unsigned int, unsigned short int> (char** start, char** end, unsigned int &num);
template void partialflush <unsigned int, unsigned int> (char** start, char** end, unsigned int &num);

template void partialflush <unsigned short int, unsigned char> (char** start, char** end, unsigned int &num);
template void partialflush <unsigned short int, unsigned short int> (char** start, char** end, unsigned int &num);
template void partialflush <unsigned short int, unsigned int> (char** start, char** end, unsigned int &num);




template <class T, class T1>
void partialflush(char** start, char** end, unsigned int &num) {
  // flush 
  char pfilename[30];
  sprintf(pfilename, "%s-%d", PARTIAL_RESULTS, num);
  FILE* o = fopen(pfilename, "wb");
  if(!o) fatal_error("Could not open file for writing partially ordered itemsets.");
  
  for (; start<end; start++) {
    fwrite (*start, sizeof(T), 1, o);
    fwrite (*start+sizeof(unsigned int), sizeof(T1), 1, o);
    fwrite (*start+2*sizeof(unsigned int), sizeof(T), *((T*)*start), o);
  }
  
  fclose(o);
}

// ===================================
// ===================================




template <class T, class T1> 
void fourth_scan(float limit, unsigned int parts, unsigned int no_of_threads, char* output)
{
#ifdef VERBOSE
  Chronos elapsed_time;
  elapsed_time.StartChronos();

  synch_printf( "# \n"
		"# ## STEP 4 . Remove Spurious Itemsets.\n"
		"# ## Sorting %d Collections of Frequent Itemsets.\n", parts);
  double no_spurious = 0.0;
#endif

  //  no duplicates to remove if there is only one partition
  if (parts == 1) {
#ifdef VERBOSE
    synch_printf( "# ## No Spurious Itemsets to remove\n");
#endif
    return;
  }

  T max_len = 0;
  unsigned int num_of_parts = 0;
  unsigned int num_of_partials = 0;


  // input files
  char filename[50];
  sprintf(filename, BIN_ITEMSETS_FILE_FMT, num_of_parts++);
  FILE* input = fopen(filename, "rb");
  if(!input) fatal_error("Could not open partial mining output file for reading.");
  
  unsigned int STATIC_MEM_SIZE = (unsigned int)(limit); 
  char* STATIC_MEM = new char [STATIC_MEM_SIZE];               // memory pointer
  if (!STATIC_MEM) fatal_error("Could not allocate memory for ordering partial itemsets collections.");
  char* BEGIN      = STATIC_MEM;                            // itemsets' data
  char** END       = (char**)(STATIC_MEM+STATIC_MEM_SIZE);  // data pointers
  char** STOP      = (char**)(STATIC_MEM+STATIC_MEM_SIZE);  // end of allocated memory

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // READ DATA
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  while(1) {   
    T lung;
    T1 support;
    
    if (fread(&lung, sizeof(T), 1, input)==0) {
      fclose(input);
      if (num_of_parts<no_of_threads) {
	// open new file
	sprintf(filename, BIN_ITEMSETS_FILE_FMT, num_of_parts++);
	input = fopen(filename, "rb");
	if(!input) fatal_error("Could not open partial mining output file for reading.");
	continue;
      } else 
	break;
    }

    // support
    fread(&support, sizeof(T1), 1, input);

    if (lung>1) {
      if (lung>max_len)
	max_len = lung;
      
      // if there is not enough memoery
      if ( (void*)(BEGIN+2*sizeof(unsigned int)+sizeof(T)*(lung+1)+sizeof(char*)) >= (void*)END ) {
	// sort
	sort(END, STOP, _ItemsetSort<T,T1>());
	partialflush<T,T1>(END, STOP, num_of_partials);
	num_of_partials++;
	
	//restart
	BEGIN = STATIC_MEM;
	END   = STOP;
      }

      // Store
      END--;
      *END = BEGIN;

      *((T*) (BEGIN)) = lung;    BEGIN += sizeof(unsigned int);
      *((T1*)(BEGIN)) = support; BEGIN += sizeof(unsigned int);
      fread(BEGIN, sizeof(T), lung, input);
      // sort in decreasing order
      sort((T*)BEGIN, (T*)(BEGIN+sizeof(T)*lung), greater<T>());
      BEGIN += (sizeof(T)*lung+3) & 0xFFFFFFFC; // word align
      
      if (BEGIN > (char*) END)
	fatal_error("Fatal error in memory management during merging");
    } else {
      synch_printf( "LUNG <= 1 !!!!!!!!!!!! doing nothing\n" );
    }
  }

  // flush last itemsets  
  if (END != STOP) { 
    // sort
    sort(END, STOP, _ItemsetSort<T,T1>());
    partialflush<T,T1>(END, STOP, num_of_partials);
    num_of_partials++;
  }

  delete [] STATIC_MEM;

  // remove itemsets file
  for (unsigned int i=0; i<parts; i++) {
    sprintf(filename, BIN_ITEMSETS_FILE_FMT, i+1);
    unlink (filename);
  }

  
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // MERGE PARTIAL and GENERATE OUTPUT
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifdef VERBOSE
  synch_printf( "# ## Merging %d collections of sorted itemsets.\n", num_of_partials);
#endif
#ifdef DEBUG_FOURTH
  unsigned int * spurious_per_len = new unsigned int [max_len+1];
  for (unsigned int i=0; i<=max_len; i++)
    spurious_per_len[i] = 0;
#endif

  unsigned int size = 2*sizeof(unsigned int) + (max_len+1)*sizeof(T); // lung, supp, items

  if (2.0*size*num_of_partials > limit)
    synch_printf(" warning too much memory\n");

  char* global_itemsets = new char [2*size*num_of_partials];
  if (!global_itemsets) fatal_error("Could not allocate memory for itemsets merging.");

  if (FOPEN_MAX <= num_of_partials)
    synch_printf( "!\n"
		  "! !! Warning: Partial files are more than FOPEN_MAX\n"
		  "!\n" );

  // Open input files
  FILE** iFiles = new FILE* [num_of_partials];
  if (!iFiles) fatal_error("Could not allocate memory for storing files descriptors.");
  for (unsigned int i=0; i<num_of_partials; i++) {
    char pfilename[50];
    sprintf(pfilename, "%s-%d", PARTIAL_RESULTS, i);
    iFiles[i] = fopen(pfilename, "rb");
    if(!iFiles[i]) fatal_error("Could not open partially ordered itemsets file for reading.");
  }

  FILE* output_file = fopen(output, "ab");
  if(!output_file) fatal_error("Coult not open binary output file.");

  // since duplicates may be in the same file
  // we must read 2 itemsets per file

  // is this true ?!???

  set<IF, _IFSort<T,T1> > current;
  for (unsigned int i=0; i<num_of_partials; i++) {
    for (unsigned int j=0; j<2; j++) {
      IF el;
      el.fid  = i;
      el.data = global_itemsets+(2*i+j)*size;

      if (fread(el.data, sizeof(T), 1, iFiles[i]) != 0) {
	fread(el.data+sizeof(unsigned int), sizeof(T1), 1, iFiles[i]);
	fread(el.data+2*sizeof(unsigned int),sizeof(T),(*( (T*) el.data )), iFiles[i]);
	current.insert(el);
      }
    }
  }

  // merge ..............................
  while(current.size()>0) {

    // take out first one
    IF removed;
    removed.fid  = current.begin()->fid;
    removed.data = current.begin()->data;
    current.erase(current.begin());

    // now take the smallest
    typename set<IF, _IFSort<T,T1> >::iterator smallest = current.begin();

    // check for output
    bool isprefix = 
      // current is not empty
      current.size()!=0 && 
      // supp
      *( (T1*) (removed.data+sizeof(unsigned int)) ) ==  *( (T1*) (smallest->data+sizeof(unsigned int)) ) &&
      // lung
      *( (T*) removed.data ) <  *( (T*) smallest->data );

    for (T i=0; isprefix && i<*( (T*) removed.data ); i++)
      isprefix = 
	*( (T*) (removed.data   +2*sizeof(unsigned int)+sizeof(T)*i) ) ==
	*( (T*) (smallest->data +2*sizeof(unsigned int)+sizeof(T)*i) );

    if (!isprefix) {
      // print
      fwrite(removed.data                        , sizeof(T),  1, output_file);
      fwrite(removed.data+   sizeof(unsigned int), sizeof(T1), 1, output_file);
      fwrite(removed.data+ 2*sizeof(unsigned int), sizeof(T) ,*( (T*) removed.data ), output_file);
    }
    else {
#ifdef VERBOSE
      no_spurious++;
#endif
#ifdef DEBUG_FOURTH
      spurious_per_len[*((T*) removed.data)]++;
#endif
    }

    // read next itemset
    IF el;
    el.fid  = removed.fid;
    el.data = removed.data;

    if (fread(el.data, sizeof(T), 1, iFiles[el.fid])!=0) {
      fread(el.data+sizeof(unsigned int), sizeof(T1), 1, iFiles[el.fid]);
      fread(el.data+2*sizeof(unsigned int), sizeof(T), (*( (T*) el.data )), iFiles[el.fid]);
      current.insert(el);
    }
  }
  fclose(output_file);

  for (unsigned int i=0; i<num_of_partials; i++) {
    fclose(iFiles[i]);
    char pfilename[50];
    sprintf(pfilename, "%s-%d", PARTIAL_RESULTS, i);
    unlink(pfilename);
  }

  delete [] global_itemsets;
  delete [] iFiles;

#ifdef DEBUG_FOURTH
  synch_printf("# ## ## Spurious Itemsets Histogram:\n");
  for (unsigned int i=0; i<=max_len; i++)
    synch_printf("# ## ## %10d\n", spurious_per_len[i] );
  delete [] spurious_per_len;
#endif

#ifdef VERBOSE
  synch_printf("# ## Spurious Itemsets : %d.\n", no_spurious );
  synch_printf("# ## Elapsed Time: %.2f secs.\n", elapsed_time.ReadChronos() );
#endif

}


