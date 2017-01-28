//
//
// C++ Implementation for module: Log
//
// Description: 
//
//
// Author: exa
//
//

#include <iostream>
#include "Log.hxx"


#ifdef LOG

std::ostream & nlog = std::cerr;

#else

nlog_t nlog;

ostream & nlog_t::null_stream = cerr;

#endif

#ifdef ERROR

std::ostream & nerr = std::cerr;

#else

nerr_t nerr;

#endif
