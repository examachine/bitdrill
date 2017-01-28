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



#include "database.hh"


ibmFSin::ibmFSin(char *filename)
{
  in = fopen(filename,"rb");
}


ibmFSin::~ibmFSin()
{
  if(in) fclose(in);
}

int ibmFSin::isOpen()
{
  if(in) return 1;
  else return 0;
}

int ibmFSin::getNextTransaction(vector<int>& t)
{
  int ret;

  int prologue[3]; // tid +  clientid + length;

  t.resize(0);

  ret = fread(prologue, sizeof(int), 3, in);
  if (ret != 3)
    return 0;
  
  int length = prologue[2];
  int item;
  for (int i=0; i<length; i++) {
    if ((ret = fread(&item, sizeof(int), 1, in)) != 1)
      return 0;
    t.push_back(item);
  }

  return 1;
}







/////////////////////////////////////


asciiFSout::asciiFSout(char *filename)
{
  out = fopen(filename,"wb");
}



asciiFSout::~asciiFSout()
{
  if(out) fclose(out);
}



int asciiFSout::isOpen()
{
  if(out) return 1;
  else return 0;
}


void asciiFSout::writeTransaction(vector<int>& t)
{
  int tsz;
  if ((tsz = t.size()) != 0) {
    for (int i=0; i<tsz; i++)
      fprintf(out, "%d ", t[i]);
    fprintf(out, "\n");
  } 
}

