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
#include <stdlib.h>
#include <string.h>

#include "memory.hh"

/*
  Name: mem_rss

  Version: 1.0
  Date: Tue Aug 22 15:50:47 CEST 2000
  Author: Paolo Palmerini <paolo.palmerini@cnuce.cnr.it>
  Description: Read file /proc/pid/status and return VmRSS entry 
  or -1 if some error occurs.
*/

unsigned int mem_rss(pid_t pid)
{
  char fname[128];
  char tmp[128];
  FILE * f;
  unsigned int rss=0;
  
  strcpy(&fname[0], "");
  sprintf(&fname[0], "/proc/%d/status", (int) pid);
  
  f = (FILE *) fopen (&fname[0], "r");
  if (f==NULL) {
    fprintf(stderr, "Cannot open %s\n", &fname[0]);
    return(0);
  }
  
  while (!feof(f)) {
    strcpy(&tmp[0],"");
    fscanf(f,"%s %u", &tmp[0], &rss);
    //    if (strcmp("VmRSS:",&tmp[0])==0) break;
    if (strcmp("VmData:",&tmp[0])==0) break;
  }
  
  fclose(f);
  return rss;
}

/*
  Name: mem_total

  Version: 1.0
  Date: Tue Aug 22 15:50:47 CEST 2000
  Author: Paolo Palmerini <paolo.palmerini@cnuce.cnr.it>
  Description: Returns total physical memory available from /proc/meminfo 
  or -1 if some error occurs.
*/



unsigned int mem_total()
{
  char fname[128];
  char tmp[128];
  FILE * f;
  unsigned int tot=0;
  
  strcpy(&fname[0], "");
  sprintf(&fname[0], "/proc/meminfo");
  
  // if there is no /proc file system support on the 
  // machine (as in the case of cygwin)
  // we return a fixed predefined value. 

  f = (FILE *) fopen (&fname[0], "r");
  if (f==NULL) {
    return __MIN_MEMORY_AVAILABLE__;
  }
  
  while (!feof(f)) {
    strcpy(&tmp[0],"");
    fscanf(f,"%s %u", &tmp[0], &tot);
    if (strcmp("MemTotal:",&tmp[0])==0) { tot *= 1024; break;}
  }
  
  fclose(f);
  return tot;
}

unsigned int mem_free()
{
  char fname[128];
  char tmp[128];
  FILE * f;
  unsigned int tot=0;
  
  strcpy(&fname[0], "");
  sprintf(&fname[0], "/proc/meminfo");
  
  // if there is no /proc file system support on the 
  // machine (as in the case of cygwin)
  // we return a fixed predefined value. 

  f = (FILE *) fopen (&fname[0], "r");
  if (f==NULL) {
    return __MIN_MEMORY_AVAILABLE__;
  }
  
  while (!feof(f)) {
    strcpy(&tmp[0],"");
    fscanf(f,"%s %u", &tmp[0], &tot);
    if (strcmp("MemFree:",&tmp[0])==0) { tot *= 1024; break;}
  }
  
  fclose(f);
  return tot;
}
