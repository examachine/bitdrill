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


template class binFSout<unsigned int>;
template class binFSout<unsigned short int>;
template class binFSout<unsigned char>;

template class binFSin<unsigned int>;
template class binFSin<unsigned short int>;
template class binFSin<unsigned char>;



/////////////////////////////////////


template <class T>
binFSout<T>::binFSout(char *filename, bool create)
{
  if (create)
    out = fopen(filename,"w+");
  else
    out = fopen(filename,"r+");
  n_tr=0;
  sz=0;
}


template <class T>
binFSout<T>::~binFSout()
{
  if(out) fclose(out);
}

template <class T>
int binFSout<T>::isOpen()
{
  if(out) return 1;
  else return 0;
}


template <class T>
int binFSout<T>::trunc()
{
  int ret;
  ret = ftruncate (fileno(out), ftell(out));
  if(ret) return 1;
  else return 0;
}

template <class T>
inline void binFSout<T>::writeTransaction(vector<T>& t)
{
  int tsz;
  if ((tsz = t.size()) != 0) {
    n_tr++; 
    sz += (tsz+1) * sizeof(T);
    fwrite(&tsz, 1,sizeof(T), out);   
    fwrite(&t[0], tsz, sizeof(T), out); 
  }
}



template <class T>
inline void binFSout<T>::writeTransaction(Transaction<T>& t)
{
  if (t.length != 0) {
    n_tr++; 
    sz += (t.length+1) * sizeof(T);
    fwrite(&t.length, 1,sizeof(T), out);   
    fwrite(&t.t[0], t.length, sizeof(T), out); 
  }
}

template <class T>
inline void binFSout<T>::writeTransaction(dci_transaction<T>& t)
{
  if (t.t_len != 0) {
    n_tr++;
    sz += (t.t_len+1) * sizeof(T);
    fwrite(&t.t_len, 1, sizeof(T), out);   
    fwrite(&t.elements[0], t.t_len, sizeof(T), out); 
  }
}

///////////////////////////////////

template <class T>
binFSin<T>::binFSin(char *filename)
{
  in = fopen(filename,"r");
}

template <class T>
binFSin<T>::~binFSin()
{
  if(in) fclose(in);
}

template <class T>
int binFSin<T>::isOpen()
{
  if(in) return 1;
  else return 0;
}

template <class T>
int binFSin<T>::getNextTransaction(Transaction<T>& t)
{
  int ret;
  ret = fread(&t.length, sizeof(T), 1, in);
  if (ret != 1)
    return 0;
  ret = fread(&t.t[0], sizeof(T), t.length, in);

  return 1;

}


template <class T>
int binFSin<T>::getNextTransaction(dci_transaction<T>& t)
{
  int ret;
  ret = fread(&t.t_len, sizeof(T), 1, in);
  if (ret != 1)
    return 0;
  ret = fread(t.elements, sizeof(T), t.t_len, in);


  return 1;
}





///////////////////////////////////

ibmFSin::ibmFSin(char *filename)
{
  in = fopen(filename,"r");
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
  out = fopen(filename,"w+");
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

