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
#include <stdio.h>
#include <iostream>
using namespace std;	

#include "utils.hh"

namespace KDCI {

void print_statistics(char *algo, int iter, 
		      int num_cand, int num_freq, double time)
{
  printf("# %s\t|F%d| = %d\t\t(|C%d| =%d,  time=%f)\n", 
	 algo, iter, num_freq, iter, num_cand, time);
}




//////////////////////////////////
void Chronos::StartChronos()
{
 if(!( (State==READY_CHRONOS || Time==0) ||
       (State==STOPPED_CHRONOS) ) ) 
   ResetChronos();

 State=RUNNING_CHRONOS;

 gettimeofday(&tv,&tz);
 sec1=tv.tv_sec;
 usec1=tv.tv_usec;
}



void Chronos::StopChronos()
{
  State=STOPPED_CHRONOS;

  gettimeofday(&tv,&tz);
  sec2=tv.tv_sec;
  usec2=tv.tv_usec;
  Time = (double)((sec2-sec1)) + 
    (double) ((usec2-usec1))/1000000.0;
}


double Chronos::ReadChronos()
{
 if(State==STOPPED_CHRONOS || State==READY_CHRONOS) 
   return Time;
 else {
   gettimeofday(&tv,&tz);
   sec3=tv.tv_sec;
   usec3=tv.tv_usec;
   Time = (double)((sec3-sec1)) + 
     (double) ((usec3-usec1))/1000000.0;
   return Time;
 }
}


void Chronos::ResetChronos()
{
 Time=0;
 State=READY_CHRONOS;
}



void Chronos::RestartChronos()
{
  ResetChronos();
  StartChronos();
}

} // namespace
