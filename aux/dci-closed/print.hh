#ifndef __PRINT_HH
#define __PRINT_HH

void synch_init();
void synch_destroy();
void synch_printf(char* format, ...);

void fatal_error(char*);


#endif
