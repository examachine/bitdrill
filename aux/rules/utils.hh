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

#ifndef __UTILS_H
#define __UTILS_H

#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>

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

  static const int RUNNING_CHRONOS = 2;
  static const int STOPPED_CHRONOS = 1;
  static const int READY_CHRONOS   = 0;
};



// this is not what i really wanted !!!!!!!
// timed lock and ulock are not thread safe !!!!!!!
class TimedMutex {
  Chronos timer;
  pthread_mutex_t sem;
  double waiting_time;

public:
  TimedMutex(){
    pthread_mutex_init(&sem, NULL);
    waiting_time = 0;
  }

  ~TimedMutex(){
    pthread_mutex_destroy(&sem);
  }
  
  inline void lock() {
    pthread_mutex_lock(&sem);
  }
  inline void timedlock() {
    timer.RestartChronos();
    lock();
  }
  inline void unlock() {
    pthread_mutex_unlock(&sem);
  }
  inline void timedunlock() {
    waiting_time += timer.ReadChronos();
    unlock();
  }

  double gettime() {
    return waiting_time;
  }
};



#endif
