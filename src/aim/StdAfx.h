// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#ifndef __STDAXF
#define __STDAFX

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <memory.h>
#include <stdlib.h>

// #define WIN32_DEBUG

/*
  CItems item name lookup method
  ------------------------------
  CItems has a lookup structure to find items in the items array. For speed-up
  reasons a simple array is used. This array is good ONLY when there highest item
  has a reasonable value (e.g. 100,000 or so).

  When the highest number is VERY high (e.g. 1,000,000,000) use this define - it 
  switches from array lookup to hash lookup.
*/
//#define USE_ITEMS_HASH_LOOKUP

#define CRLF "\r\n"

//#define MINE_FCI
//#define MINE_MFI

	// container of a single position in the vertical bit vector
#define MY_ULONG unsigned long
#define BITS_IN_ULONG (sizeof(MY_ULONG)*8)

#define ITEM_NAME int

	// The class used for the vertical bit-vector
#define V_BIT_ARRAY CSparseBitArray
//#define V_BIT_ARRAY CExtendIntArray

#endif
