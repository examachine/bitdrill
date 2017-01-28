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

#ifndef __UTILS_H
#define __UTILS_H

#include <unistd.h>
#include <sys/time.h>

#include "database.hh"

template <class T>
void printt(Transaction<T>& t)
{
  for (T i=0; i< t.length; i++)
    cout << (int) t.t[i] << " ";
  cout << endl;
}



void print_statistics(char *algo, int iter, 
		      int num_cand, int num_freq, double time);




const int RUNNING_CHRONOS = 2;
const int STOPPED_CHRONOS = 1;
const int READY_CHRONOS	= 0;


class Chronos {

public:
  Chronos() {Time=0; State=READY_CHRONOS;}
  
  void   StartChronos();
  void   StopChronos();
  double ReadChronos();
  void   ResetChronos();
  void   RestartChronos();

private:
  struct timeval tv;
  struct timezone tz;
  long sec1,sec2,sec3;
  long usec1,usec2,usec3;
  double Time;
  int State;
};




#endif