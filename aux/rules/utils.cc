/*
  Copyright (C) 2009, Claudio Lucchese

  This file is part of FINDRULES.

  FINFRULES is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  FINDRULES is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with FINFRULES.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <iostream>
using namespace std;	

#include "utils.hh"
Chronos GLOBAL_CLOCK;


void Chronos::StartChronos()
{
	/*
 if(!( (State==READY_CHRONOS || Time==0) ||
       (State==STOPPED_CHRONOS) ) ) 
   ResetChronos();
*/
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
  Time += (double)((sec2-sec1)) + 
    (double) ((usec2-usec1))/1000000.0;
//	if ((usec2-usec1) == 0)
//		Time += 0.0005;
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

