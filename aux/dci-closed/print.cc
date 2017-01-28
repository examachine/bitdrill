#include "print.hh"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <pthread.h>

using namespace std;

//synchronization mutex
pthread_mutex_t __errors_output_lock;
bool __errors_output_lock_init = false;

void synch_printf(char* format, ...) {
  // lock
  pthread_mutex_lock( &__errors_output_lock );

  va_list vl;
  va_start(vl,format);
  vprintf(format, vl);
  va_end(vl);

  // unlock
  pthread_mutex_unlock( &__errors_output_lock );
}

void fatal_error(char* s) {
  // lock
  pthread_mutex_lock( &__errors_output_lock );

  cout << endl
       << " !! !!!!!!!!!!!!!!!!!!!! !! " << endl
       << " !! Fatal Error          !! " << endl
       << " !! -------------------- !! " << endl;

  if (s) cout << " !! " << s << endl;
  else cout << " !! Default Message: Could not allocate memory." << endl;

  cout << " !!" << endl
       << endl;

  exit(1);
}


void synch_init() {
  if (!__errors_output_lock_init)
    pthread_mutex_init(&__errors_output_lock, NULL);
}
inline void synch_destroy() {
  pthread_mutex_destroy(&__errors_output_lock);
}

