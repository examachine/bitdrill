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
#ifndef _TID_LIST_MACROS_H
#define _TID_LIST_MACROS_H

#include "my_defs.hh"
#include "items.hh"

#include <stdlib.h>

using namespace std;

/*
 * define data typ for tid_lists operations
 * using SSE instruction set.
 */

inline void aligned16_malloc (tid_list* &aligned, void* &real_inimg, size_t spaceneeded) {
  real_inimg = malloc(spaceneeded + 0x10); 
  if (real_inimg==NULL)
    aligned = NULL;
  else
    aligned  = (tid_list*) (reinterpret_cast<intptr_t>(  ((unsigned char*)real_inimg + 0x10))  & (~0xf) );
}

#ifdef MY_SET_BIT
static __inline__ void my_set_bit(int nr, volatile void * addr)
{
        __asm__ __volatile__(
                "btsl %1,%0"
                :"=m" (*(volatile long *) addr)
                :"Ir" (nr));
}
#endif


#ifdef USE_SSE

#ifdef USE_MMX
/* 
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * SSE 64-Bit operations
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

void sse_init_masks(void) {
  cout << " i'm actually using 64 bits" << endl;
}


inline int count_1_bits(const tid_list *p, int sz, tid_list* SSE_RESULT)
{
  mmx_t constants[4];
  constants[0].ud[0] = constants[0].ud[1] = 0x55555555;
  constants[1].ud[0] = constants[1].ud[1] = 0x33333333;
  constants[2].ud[0] = constants[2].ud[1] = 0x0F0F0F0F;
  constants[3].ud[0] = constants[3].ud[1] = 0x0000003F;

  // reset counter
  __asm__ __volatile__ ( 
			// // Loading masks
			"\n\t movq %0,%%mm4       \t# load mu1  "
 			"\n\t movq %1,%%mm5       \t# load mu2  "
 			"\n\t movq %2,%%mm6       \t# load mu3  "
 			"\n\t movq %3,%%mm7       \t# load mu4  "
			// reset
			"\n\t pxor  %%mm3, %%mm3   " 
			//
			:
			: "m" (constants[0]),  // and %1   
 			"m" (constants[1]),   // and %2   
 			"m" (constants[2]),   // and %3   
 			"m" (constants[3])   // and %4   
			);


  for (int i=0; i<sz; i++) {
    for (int j=0; j<2; j++) {
    __asm__ __volatile__ ( 
			  "\n\t movq %0,%%mm0       \t# u  load ref  "

			  // b = (b & 0x55555555) + (b >> 1 & 0x55555555);
			  "\n\t movq  %%mm0, %%mm1   "
			  "\n\t psrld $0x01, %%mm1   "
			  "\n\t pand  %%mm4, %%mm0   "
			  "\n\t pand  %%mm4, %%mm1   "
			  "\n\t paddd %%mm1, %%mm0   "
			  
			  // b = (b & 0x33333333) + (b >> 2 & 0x33333333);
			  "\n\t movq  %%mm0, %%mm1   "
			  "\n\t psrld $0x02, %%mm1   "
			  "\n\t pand  %%mm5, %%mm0   "
			  "\n\t pand  %%mm5, %%mm1   "
			  "\n\t paddd %%mm1, %%mm0   "
			  
			  // b = (b + (b >> 4)) & 0x0F0F0F0F;
			  "\n\t movq  %%mm0, %%mm1   "
			  "\n\t psrld $0x04, %%mm1   "
			  "\n\t paddd %%mm1, %%mm0   "
			  "\n\t pand   %%mm6, %%mm0   "

			  // b = b + (b >> 8);
			  "\n\t movq  %%mm0, %%mm1   "
			  "\n\t psrld $0x08, %%mm1   "
			  "\n\t paddd %%mm1, %%mm0   "
			  
			  // b = (b + (b >> 16)) & 0x0000003F;
			  "\n\t movq  %%mm0, %%mm1   "
			  "\n\t psrld $0x10, %%mm1   "
			  "\n\t paddd %%mm1, %%mm0   "
			  "\n\t pand  %%mm7, %%mm0   "
			  
			  "\n\t paddd %%mm0, %%mm3   "
			  : 
			  : "m" (p[i].f[j*2])
			  );
    }
  } 

  __asm__ __volatile__ ( 
			"\n\t movq %%mm3, %0   \t# tmp2"
			: "=m" (SSE_RESULT[0].f[0])  // this is %0,  output
			:
			);

  __asm__ __volatile__ ( "\n\t emms " : : );

  return  SSE_RESULT[0].f[0]+SSE_RESULT[0].f[1];
}

int and_tid_list_count_1(tid_list *p_out, 
			 const tid_list *p1_in, 
			 const tid_list *p2_in, 
			 int sz,
			 tid_list* SSE_RESULT )
{
  mmx_t constants[4];
  constants[0].ud[0] = constants[0].ud[1] = 0x55555555;
  constants[1].ud[0] = constants[1].ud[1] = 0x33333333;
  constants[2].ud[0] = constants[2].ud[1] = 0x0F0F0F0F;
  constants[3].ud[0] = constants[3].ud[1] = 0x0000003F;

  // reset counter
  __asm__ __volatile__ ( 
			// // Loading masks
			"\n\t movq %0,%%mm4       \t# load mu1  "
 			"\n\t movq %1,%%mm5       \t# load mu2  "
 			"\n\t movq %2,%%mm6       \t# load mu3  "
 			"\n\t movq %3,%%mm7       \t# load mu4  "
			// reset
			"\n\t pxor  %%mm3, %%mm3   " 
			//
			:
			: "m" (constants[0]),  // and %1   
 			"m" (constants[1]),   // and %2   
 			"m" (constants[2]),   // and %3   
 			"m" (constants[3])   // and %4   
			);


  for (int i=0; i<sz; i++) {
    for (int j=0; j<2; j++) {

    __asm__ __volatile__ ( 
			  "\n\t movq %1,%%mm0 "
			  "\n\t pand %2,%%mm0 "
			  "\n\t movq %%mm0,%0 "
			  : "=m" (p_out[i].f[j*2]) 
			  : "m" (p1_in[i].f[j*2]), 
			  "m" (p2_in[i].f[j*2])    
			  );

    __asm__ __volatile__ ( 
			  // b = (b & 0x55555555) + (b >> 1 & 0x55555555);
			  "\n\t movq  %%mm0, %%mm1   "
			  "\n\t psrld $0x01, %%mm1   "
			  "\n\t pand  %%mm4, %%mm0   "
			  "\n\t pand  %%mm4, %%mm1   "
			  "\n\t paddd %%mm1, %%mm0   "
			  
			  // b = (b & 0x33333333) + (b >> 2 & 0x33333333);
			  "\n\t movq  %%mm0, %%mm1   "
			  "\n\t psrld $0x02, %%mm1   "
			  "\n\t pand  %%mm5, %%mm0   "
			  "\n\t pand  %%mm5, %%mm1   "
			  "\n\t paddd %%mm1, %%mm0   "
			  
			  // b = (b + (b >> 4)) & 0x0F0F0F0F;
			  "\n\t movq  %%mm0, %%mm1   "
			  "\n\t psrld $0x04, %%mm1   "
			  "\n\t paddd %%mm1, %%mm0   "
			  "\n\t pand   %%mm6, %%mm0   "

			  // b = b + (b >> 8);
			  "\n\t movq  %%mm0, %%mm1   "
			  "\n\t psrld $0x08, %%mm1   "
			  "\n\t paddd %%mm1, %%mm0   "
			  
			  // b = (b + (b >> 16)) & 0x0000003F;
			  "\n\t movq  %%mm0, %%mm1   "
			  "\n\t psrld $0x10, %%mm1   "
			  "\n\t paddd %%mm1, %%mm0   "
			  "\n\t pand  %%mm7, %%mm0   "
			  
			  "\n\t paddd %%mm0, %%mm3   "
			  : 
			  : 
			  );
    }
  } 

  __asm__ __volatile__ ( 
			"\n\t movq %%mm3, %0   \t# tmp2"
			: "=m" (SSE_RESULT[0].f[0])  // this is %0,  output
			:
			);

  __asm__ __volatile__ ( "\n\t emms " : : );

  return  SSE_RESULT[0].f[0]+SSE_RESULT[0].f[1];

}

template <class T1>
inline bool tid_list_inclusion( tid_list* i, 
				tid_list* j, 
				T1 &kk,
				T1 end,
				tid_list* SSE_RESULT )
{
  bool match = true;

  for (; match && kk<end; kk++) {

    __asm__ __volatile__ ( 
			  "\n\t movq %1,%%mm0       \t# u  load ref  "
			  "\n\t movq %2,%%mm1       \t# u  load input "
			  "\n\t pand %%mm0,%%mm1    \t# v copy ref "
			  "\n\t pandn %%mm0,%%mm1  \t# compare"
			  "\n\t movq %%mm1,%0       \t# u store result, "
			  : "=m" (SSE_RESULT[0].f[0])  // this is %0,  output
			  : "m" (*((mmx_t*)(i+kk))),     // and %1   reference 
			    "m" (*((mmx_t*)(j+kk)))      // and %2   per-pixel-threshold
			  );

    __asm__ __volatile__ ( 
			  "\n\t movq %1,%%mm0       \t# u  load ref  "
			  "\n\t movq %2,%%mm1       \t# u  load input "
			  "\n\t pand %%mm0,%%mm1    \t# v copy ref "
			  "\n\t pandn %%mm0,%%mm1  \t# compare"
			  "\n\t movq %%mm1,%0       \t# u store result, "
			  : "=m" (SSE_RESULT[0].f[2])  // this is %0,  output
			  : "m" (*((mmx_t*)(i+kk)+1)),     // and %1   reference 
			    "m" (*((mmx_t*)(j+kk)+1))      // and %2   per-pixel-threshold
			  );

    match = !(SSE_RESULT[0].f[0] || SSE_RESULT[0].f[1] || 
	      SSE_RESULT[0].f[2] || SSE_RESULT[0].f[3]);
  }

  __asm__ __volatile__ ( "\n\t emms \t# clean up," : : );

  return match;
}



#else
/* 
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * SSE 128-Bit operations
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

void sse_init_masks(void) {
  tid_list mu[4];
  mu[0].f[0] = mu[0].f[1] = mu[0].f[2] = mu[0].f[3] = 0x55555555;
  mu[1].f[0] = mu[1].f[1] = mu[1].f[2] = mu[1].f[3] = 0x33333333;
  mu[2].f[0] = mu[2].f[1] = mu[2].f[2] = mu[2].f[3] = 0x0F0F0F0F;
  mu[3].f[0] = mu[3].f[1] = mu[3].f[2] = mu[3].f[3] = 0x0000003F;
  
  __asm__ __volatile__ ( 
			// // Loading masks
			"\n\t movups %0,%%xmm4       \t# load mu1  "
 			"\n\t movups %1,%%xmm5       \t# load mu2  "
 			"\n\t movups %2,%%xmm6       \t# load mu3  "
 			"\n\t movups %3,%%xmm7       \t# load mu4  "
			:
			: "m" (mu[0]),  // and %1   
 			"m" (mu[1]),   // and %2   
 			"m" (mu[2]),   // and %3   
 			"m" (mu[3])   // and %4   
			);
}


inline int count_1_bits(const tid_list *p, int sz)
{
  // reset counter
  __asm__ __volatile__ ( "\n\t xorps  %%xmm3, %%xmm3   " : :);

  for (int i=0; i<sz; i++) {
    __asm__ __volatile__ ( 
			  "\n\t movaps %0,%%xmm0       \t# u  load ref  "

			  // b = (b & 0x55555555) + (b >> 1 & 0x55555555);
			  "\n\t movaps %%xmm0, %%xmm1   \t# tmp1, tmp2"
			  "\n\t psrld  $0x01,%%xmm1      \t# shift tmp2"
			  "\n\t andps  %%xmm4, %%xmm0   "
			  "\n\t andps  %%xmm4, %%xmm1   "
			  "\n\t paddd  %%xmm1, %%xmm0     "
			  
			  // b = (b & 0x33333333) + (b >> 2 & 0x33333333);
			  "\n\t movaps %%xmm0, %%xmm1   \t# tmp1, tmp2"
			  "\n\t psrld  $0x02,%%xmm1      \t# shift tmp2"
			  "\n\t andps  %%xmm5, %%xmm0   "
			  "\n\t andps  %%xmm5, %%xmm1   "
			  "\n\t paddd  %%xmm1, %%xmm0     "
			  
			  // b = (b + (b >> 4)) & 0x0F0F0F0F;
			  "\n\t movaps %%xmm0, %%xmm1   \t# tmp1, tmp2"
			  "\n\t psrld  $0x04,%%xmm1      \t# shift tmp2"
			  "\n\t paddd  %%xmm1, %%xmm0     "
			  "\n\t andps  %%xmm6, %%xmm0   "

			  // b = b + (b >> 8);
			  "\n\t movaps %%xmm0, %%xmm1   \t# tmp1, tmp2"
			  "\n\t psrld  $0x08,%%xmm1      \t# shift tmp2"
			  "\n\t paddd  %%xmm1, %%xmm0     "
			  
			  // b = (b + (b >> 16)) & 0x0000003F;
			  "\n\t movaps %%xmm0, %%xmm1   \t# tmp1, tmp2"
			  "\n\t psrld  $0x10,%%xmm1      \t# shift tmp2"
			  "\n\t paddd  %%xmm1, %%xmm0     "
			  "\n\t andps  %%xmm7, %%xmm0   "
			  
			  "\n\t paddd  %%xmm0, %%xmm3     "
			  : 
			  : "m" (p[i]) 
			  );
  } 

  // EVERYTHING IN THE REGISTER
// 			// b = (b + (b >> 32))
// 			// b = (b + (b >> 64)) movss
 
// LOCAL MEMORY
  sse_t aux;
  __asm__ __volatile__ ( "\n\t movaps %%xmm3, %0   \t# tmp2" : "=m" (aux) : );

  return  aux.f[0] + aux.f[1] + aux.f[2] + aux.f[3];
}

int and_tid_list_count_1(tid_list *p_out, 
			 const tid_list *p1_in, 
			 const tid_list *p2_in, 
			 int sz)
{
  // reset counter
  __asm__ __volatile__ ( "\n\t xorps  %%xmm3, %%xmm3   " : :);
  
  for (int i=0; i<sz; i++) {
    __asm__ __volatile__ ( 
			  "\n\t movaps %1,%%xmm0       \t# u  load ref  "
			  "\n\t andps  %2,%%xmm0    \t# v copy ref "
			  "\n\t movaps %%xmm0,%0       \t# u store result, "
			  : "=m" (p_out[i])  // this is %0,  output
			  : "m" (p1_in[i]),     // and %1   reference 
			  "m" (p2_in[i])      // and %2   per-pixel-threshold
			  // the "=m" implies it is output.. just "m" is input
			  // see the gcc info pages for more details...
			  );
  
    __asm__ __volatile__ ( 
			  // b = (b & 0x55555555) + (b >> 1 & 0x55555555);
			  "\n\t movaps %%xmm0, %%xmm1   \t# tmp1, tmp2"
			  "\n\t psrld  $0x01,%%xmm1      \t# shift tmp2"
			  "\n\t andps  %%xmm4, %%xmm0   "
			  "\n\t andps  %%xmm4, %%xmm1   "
			  "\n\t paddd  %%xmm1, %%xmm0     "
			  
			  // b = (b & 0x33333333) + (b >> 2 & 0x33333333);
			  "\n\t movaps %%xmm0, %%xmm1   \t# tmp1, tmp2"
			  "\n\t psrld  $0x02,%%xmm1      \t# shift tmp2"
			  "\n\t andps  %%xmm5, %%xmm0   "
			  "\n\t andps  %%xmm5, %%xmm1   "
			  "\n\t paddd  %%xmm1, %%xmm0     "
			  
			  // b = (b + (b >> 4)) & 0x0F0F0F0F;
			  "\n\t movaps %%xmm0, %%xmm1   \t# tmp1, tmp2"
			  "\n\t psrld  $0x04,%%xmm1      \t# shift tmp2"
			  "\n\t paddd  %%xmm1, %%xmm0     "
			  "\n\t andps  %%xmm6, %%xmm0   "

			  // b = b + (b >> 8);
			  "\n\t movaps %%xmm0, %%xmm1   \t# tmp1, tmp2"
			  "\n\t psrld  $0x08,%%xmm1      \t# shift tmp2"
			  "\n\t paddd  %%xmm1, %%xmm0     "
			  
			  // b = (b + (b >> 16)) & 0x0000003F;
			  "\n\t movaps %%xmm0, %%xmm1   \t# tmp1, tmp2"
			  "\n\t psrld  $0x10,%%xmm1      \t# shift tmp2"
			  "\n\t paddd  %%xmm1, %%xmm0     "
			  "\n\t andps  %%xmm7, %%xmm0   "
			  
			  "\n\t paddd  %%xmm0, %%xmm3     "
			  : : );
  } 

// LOCAL MEMORY
  sse_t aux;
  __asm__ __volatile__ ( "\n\t movaps %%xmm3, %0 ": "=m" (aux) : );

  return  aux.f[0] + aux.f[1] + aux.f[2] + aux.f[3];
}

template <class T1>
inline bool tid_list_inclusion( tid_list* i, 
				tid_list* j, 
				T1 &kk,
				T1 end)
{
  sse_t aux; 
  bool match = true;

  for (; match && kk<end; kk++) {

    __asm__ __volatile__ ( 
			  "\n\t movaps %1,%%xmm0       \t# u  load ref  "
			  "\n\t movaps %2,%%xmm1       \t# u  load input "
			  "\n\t andps  %%xmm0,%%xmm1    \t# v copy ref "
			  "\n\t andnps %%xmm0,%%xmm1  \t# compare"
			  "\n\t movaps %%xmm1,%0       \t# u store result, "
			  : "=m" (aux)  
			  : "m" (i[kk]),
			  "m" (j[kk])   
			  );

    match = !(aux.f[0] || aux.f[1] || aux.f[2] || aux.f[3]);
  }
  return match;
}
#endif

#else
/* 
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Just 32-Bit operations
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

inline int BitCount(int b) {
  b = (b & 0x55555555) + (b >> 1 & 0x55555555);
  b = (b & 0x33333333) + (b >> 2 & 0x33333333);
  b = (b + (b >> 4)) & 0x0F0F0F0F;
  b = b + (b >> 8);
  b = (b + (b >> 16)) & 0x0000003F;
  return b;
}

inline int count_1_bits(const tid_list *pp, int sz)
{
  unsigned int* p = (unsigned int*) pp;
  int ssz = sz*sizeof(tid_list)/sizeof(unsigned int);


  int cc = 0;
  for(int i=0; i< ssz; i++)
     cc += BitCount(p[i]);
  return cc;
}

inline int and_tid_list_count_1(tid_list *pp_out, 
				       const tid_list *pp1_in, 
				       const tid_list *pp2_in, 
				       int ssz)
{
  unsigned int* p_out = (unsigned int*) pp_out;
  unsigned int* p1_in = (unsigned int*) pp1_in;
  unsigned int* p2_in = (unsigned int*) pp2_in;
  int sz = ssz*sizeof(tid_list)/sizeof(unsigned int);
  
  int cc=0;
  for (int i=0; i<sz; i++) {
    p_out[i] = p1_in[i] & p2_in[i];
    cc += BitCount(p_out[i]);
  }
  return cc;
}

template <class T1>
inline bool tid_list_inclusion(tid_list* i, 
			       tid_list* j, 
			       T1 &kk,
			       T1 end)
{
  bool match = true;
  for (; match && kk<end; kk++) {
    unsigned int* aux_i = (unsigned int*)(i+kk);
    unsigned int* aux_j = (unsigned int*)(j+kk);
    for (unsigned int iii=0; iii<sizeof(tid_list)/sizeof(unsigned int); iii++)
      match = match && ( ( *(aux_j+iii) & *(aux_i+iii) ) == *(aux_i+iii) );
    // #ifdef MY_STATS 
    //       BYTE_INCL++;
    // #endif
  }
  return match;
} 

#endif

#endif
