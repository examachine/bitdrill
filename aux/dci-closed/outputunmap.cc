#include "outputunmap.hh"
#include <iostream>
using namespace std;

#include "utils.hh"
#include "print.hh"

template void output_unmap <unsigned char, unsigned char> ( char *mapped, char* map, char* output, unsigned char m1, unsigned int num_of_parts, unsigned int no_of_threads );
template void output_unmap <unsigned char, unsigned short int> ( char *mapped, char* map, char* output, unsigned char m1, unsigned int num_of_parts, unsigned int no_of_threads );
template void output_unmap <unsigned char, unsigned int> ( char *mapped, char* map, char* output, unsigned char m1, unsigned int num_of_parts, unsigned int no_of_threads );

template void output_unmap <unsigned int, unsigned char> ( char *mapped, char* map, char* output, unsigned int m1, unsigned int num_of_parts, unsigned int no_of_threads );
template void output_unmap <unsigned int, unsigned short int> ( char *mapped, char* map, char* output, unsigned int m1, unsigned int num_of_parts, unsigned int no_of_threads );
template void output_unmap <unsigned int, unsigned int> ( char *mapped, char* map, char* output, unsigned int m1, unsigned int num_of_parts, unsigned int no_of_threads );

template void output_unmap <unsigned short int, unsigned char> ( char *mapped, char* map, char* output, unsigned short int m1, unsigned int num_of_parts, unsigned int no_of_threads );
template void output_unmap <unsigned short int, unsigned short int> ( char *mapped, char* map, char* output, unsigned short int m1, unsigned int num_of_parts, unsigned int no_of_threads );
template void output_unmap <unsigned short int, unsigned int> ( char *mapped, char* map, char* output, unsigned short int m1, unsigned int num_of_parts, unsigned int no_of_threads );


template <class T, class T1>
void output_unmap( char *mapped, char* map, char* output, T m1, unsigned int num_of_parts, unsigned int no_of_threads ) {
#ifdef VERBOSE
  Chronos elapsed_time;
  elapsed_time.StartChronos();

  cout << "# " << endl;
  cout << "# ## STEP 5 . Writing Output." << endl;
#endif

  // read mapping of items
  char* unmap = new char[m1*MAX_ASCII_ITEM_LEN];
  if (!unmap) fatal_error("Could not allocate memory for singletons ascii unmapping.");
  int* unmap_len = new int [m1];
  if (!unmap_len) fatal_error("Could not allocate memory for singletons ascii unmapping lengths.");

  FILE* map_file = fopen(map, "rb");
  for (T i=0; i<m1;i++) {
    unsigned int item;
    fread(&item, sizeof(unsigned int), 1, map_file);
    unmap_len[i] = sprintf(unmap+i*MAX_ASCII_ITEM_LEN, "%d ", item);
  }
  fclose(map_file);

  // read itemsets and output
  FILE* input_file = fopen(mapped,"rb");
  if (!input_file) fatal_error("Could not open unmapping file for reading.");

  FILE* output_file = fopen(output,"ab");
  if (!output_file) fatal_error("Could not open output file.");
  if ( num_of_parts >1 )
    no_of_threads = 0;

  unsigned int num_of_files_read = 0;

  while (1) {
    T lung;
    T1 support;
    if (fread(&lung, sizeof(T), 1, input_file)==0) {
      fclose(input_file);
      if (num_of_files_read<no_of_threads) {
	// open new file
	char filename[30];
	sprintf(filename, BIN_ITEMSETS_FILE_FMT, num_of_files_read++);
	input_file = fopen(filename, "rb");
	if(!input_file) fatal_error("Could not open partial mining output file for reading.");
	continue;
      } else 
	break;
    }
    // support
    fread(&support, sizeof(T1), 1, input_file);

    //items
    T* itemset = new T [lung];
    if (!itemset) fatal_error(NULL);
    fread(itemset, sizeof(T), lung, input_file);

    for (T i=0; i<lung; i++) 
      // fprintf (output_file, "%s ", unmap + MAX_ASCII_ITEM_LEN*itemset[i]); 
      // fprintf (output_file, unmap + MAX_ASCII_ITEM_LEN*itemset[i]); 
      fwrite (unmap + MAX_ASCII_ITEM_LEN*itemset[i], 1, unmap_len[itemset[i]], output_file); 
    fprintf (output_file, "(%d)\n", support);

    delete [] itemset;

    // small stats
    if ((unsigned int)lung > MAX_ITEMSET_LENGTH)
    MAX_ITEMSET_LENGTH = (unsigned int)lung;
    ITEMSETS_BY_LENGTH[lung]++;
  }

  fclose(output_file);

  delete [] unmap;
  delete [] unmap_len;

#ifdef VERBOSE
  synch_printf("# ## Elapsed Time: %.2f secs.\n", elapsed_time.ReadChronos() );
#endif
}
