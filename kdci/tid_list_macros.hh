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
#ifndef __KDCI_TID_LIST_MACROS_H
#define __KDCI_TID_LIST_MACROS_H

namespace KDCI {


static inline void and_tid_list_set_index(unsigned int *p_out, 
				   const unsigned int *p1_in, 
				   const unsigned int *p2_in, 
				   int *index, int sz)
{
  int k=2;
  bool start=false;
  index[1] = 0;

  for(int i=0; i<sz; i++) {
    p_out[i] = p1_in[i] & p2_in[i];
    if (start && (p_out[i] == 0)) {
      index[k++] = i;
      index[1] += index[k-1] - index[k-2];
      start=false;
    } 
    else if ((!start) && (p_out[i] != 0)) {
      index[k++] = i;
      start=true;
    }
  }

  if (start) {
    index[k++] = sz;
    index[1] += index[k-1] - index[k-2];
  }

  index[0]=(k-2)/2;
}


static inline void and_tid_list_use_index(unsigned int *p_out, 
				   const unsigned int *p1_in, 
				   const unsigned int *p2_in, 
				   const int *index, int sz)
{
  for(int i=0; i<index[0]; i++)
    for(int j=index[2+i*2]; j<index[3+i*2]; j++) 
      p_out[j] = p1_in[j] & p2_in[j];
}


static inline void or_tid_list(unsigned int *p_out, 
				  const unsigned int *p1_in, 
				  const unsigned int *p2_in, 
				  int sz)
{
  for(int i=0; i<sz; i++)
    p_out[i] = p1_in[i] | p2_in[i];
}


static inline void or_tid_list_use_index(unsigned int *p_out, 
				  const unsigned int *p1_in, 
				  const unsigned int *p2_in, 
				  const int *index, int sz)
{
  for(int i=0; i<index[0]; i++)
    for(int j=index[2+i*2]; j<index[3+i*2]; j++)
      p_out[j] = p1_in[j] | p2_in[j];
}



static inline void and_tid_list(unsigned int *p_out, 
			 const unsigned int *p1_in, 
			 const unsigned int *p2_in, 
			 int sz)
{
  for(int i=0; i<sz; i++)
    p_out[i] = p1_in[i] & p2_in[i];
}


static inline void and_tid_list_use_section(unsigned int *p_out, 
			 const unsigned int *p1_in, 
			 const unsigned int *p2_in, 
			 int start,
			 int sz)
{
  for(int i=start; i<sz; i++)
    p_out[i] = p1_in[i] & p2_in[i];
}



typedef unsigned int TUL;
static const TUL ONES = (TUL) (-1);


static inline TUL TWO(int k) {return (TUL) (1 << k);}
static inline TUL CYCL(int k) {return ONES/(1 + (TWO(TWO(k)))); }

static const TUL TWO_0 = (TUL)1;
static const TUL TWO_1 = (TUL)2;
static const TUL CYCL_0 = ONES/3;
static const TUL CYCL_1 = ONES/5;


static inline void BSUM(int& x, const int k)
{
  x +=  x >> ((TUL)1 << k); 
  TUL var;
  var = ((TUL)1 << k);
  var = ((TUL)1 << var);
  x &= (ONES/(1 + var));
}


static inline int BitCount(int x) {
  x=(x & CYCL_0) + ((x>>1) & CYCL_0);
  x=(x & CYCL_1) + ((x>>2) & CYCL_1);
  BSUM(x,2);
  BSUM(x,3);
  BSUM(x,4);
  return x;
}

static inline int and_tid_list_count_1_use_index(
					   unsigned int *p_out, 
					   const unsigned int *p1_in, 
					   const unsigned int *p2_in, 
					   const int *index, int sz)
{
  int cc=0;
  for(int i=0; i<index[0]; i++)
    for(int j=index[2+i*2]; j<index[3+i*2]; j++) {
      p_out[j] = p1_in[j] & p2_in[j];
      cc += BitCount(p_out[j]);
    }
  return cc;
}

static inline int count_1_bits(const unsigned int *p, int sz)
{
  int cc = 0;
  for(int i=0; i< sz; i++)
     cc += BitCount(p[i]);
  return cc;
}

static inline int and_tid_list_count_1(unsigned int *p_out, 
				       const unsigned int *p1_in, 
				       const unsigned int *p2_in, 
				       int sz)
{
  int cc=0;
  for (int i=0; i<sz; i++) {
    p_out[i] = p1_in[i] & p2_in[i];
    cc += BitCount(p_out[i]);
  }
  return cc;
}



static inline int and_tid_list_count_1_use_section(unsigned int *p_out, 
						   const unsigned int *p1_in, 
						   const unsigned int *p2_in, 
						   int start, int sz)
{
  int cc=0;
  for(int i=start; i<sz; i++) {
    p_out[i] = p1_in[i] & p2_in[i];
    cc += BitCount(p_out[i]);
  }
  return cc;
}

} // namespace

#endif
