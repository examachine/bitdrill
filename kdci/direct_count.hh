// Copyright (C) 2003 salvatore orlando <salvatore.orlando@unive.it>
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
#ifndef _KDCI_DIRECT_COUNT_H
#define _KDCI_DIRECT_COUNT_H

#include <stdint.h>

namespace KDCI {

inline uint64_t direct_position2_init(const  uint64_t x, const  uint64_t n) 
{ 
  return ((x)*((n)-1)-((x)*((x)+1))/2-1); 
}

inline uint64_t direct_position2(const  uint64_t x, const  uint64_t y, const  uint64_t n) {
  return ((x)*((n)-1)-((x)*((x)+1))/2+(y)-1);
}

}

#endif
