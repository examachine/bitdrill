//
//
// C++ Interface for module: lcm_freq
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef lcm_freq_Interface
#define lcm_freq_Interface

#include "generic/General.hxx"
#include "transaction-db/Transaction_Set.hxx"

void lcm_freq_cli( int argc, char *argv[] );
void lcm_freq(Transaction_Set& ts, int support, const char* out_fn);

#endif
