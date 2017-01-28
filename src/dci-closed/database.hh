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

#ifndef __DCICLOSED_DATABASE_H
#define __DCICLOSED_DATABASE_H

#include <vector>
#include <stdio.h>
#include <unistd.h>
using namespace std;	

#include "transaction-db/Transaction_Set.hxx"

#include "transaction.hh"
#include "items.hh"

namespace DCIClosed {

class Data
{
public:
  Data(Transaction_Set& ts);
  ~Data();

  int isOpen();
	  
  bool getNextTransaction(::Transaction &t);
  inline bool getNextTransaction(vector<unsigned int>& t);
  inline bool getNextTransaction(DCIClosed::Transaction<unsigned int>& t);

private:
  ::Transaction_Set& ts;
  ::Transaction_Set::Scanner* scanner;
};


inline bool Data::getNextTransaction(vector<unsigned int>& t)
{
  ::Transaction ti;
  bool ret = scanner->read(ti);
  if (ret) {
    t.resize(ti.size());  
    copy(ti.begin(), ti.end(), t.begin());
  }
  return ret;
}

  //BUG: THIS IMPL. DOESN'T LOOK RIGHT!!!! --- exa
inline bool Data::getNextTransaction(DCIClosed::Transaction<unsigned int>& t)
{
  ::Transaction ti;
  bool ret = scanner->read(ti);
  t.length = ti.size();
  for (unsigned int i=0; i<ti.size(); i++) {
    t.t[i] = ti[i];
  }

  return ret;
}


class FSout
{
 public:

  FSout(const char *filename, int iter) {
    if (iter == 1)
      out = fopen(filename,"w+");    
    else
      out = fopen(filename,"a");
  };

  ~FSout() {
    if(out) fclose(out);
  };

  int isOpen() {
    if(out) 
      return 1;
    else 
      return 0;
  };

  inline void printSet(int length, unsigned int *iset, int support);
  inline void printSet(char *iset, int sz_str);
  //  inline void printSet_bin(int length, unsigned int *iset, int support);

 private:

  FILE *out;
};


inline void FSout::printSet(int length, unsigned int *iset, int support)
{
  for(int i=0; i<length; i++) 
    fprintf(out, "%d ", iset[i]);
  fprintf(out, "(%d)\n", support);
}

inline void FSout::printSet(char *iset, int sz_str)
{
  fwrite(iset, sz_str, sizeof(char), out);   
}


// inline void FSout::printSet_bin(int length, unsigned int *iset, int support)
// {
// fwrite(iset, length, sizeof(int), out);   
// fwrite(&support, 1, sizeof(int), out);
// }


template <class T>
class binFSout
{
 public:

  binFSout(const char *filename, bool create);
  ~binFSout();

  int isOpen();
  int trunc();
  inline void writeTransaction(vector<T>& t);
  inline void writeTransaction(DCIClosed::Transaction<T>& t);
  inline void writeTransaction(dci_transaction<T>& t);
  unsigned int get_num_of_trans() {return n_tr;}

  unsigned int n_tr;
  long sz;
 private:
  FILE *out;
};


template <class T>
class binFSin
{
 public:

  binFSin(const char *filename);
  ~binFSin();

  int isOpen();

  int getNextTransaction(DCIClosed::Transaction<T>& t);
  int getNextTransaction(dci_transaction<T>& t);

 private:

  FILE *in;
};


class ibmFSin
{
 public:

  ibmFSin(const char *filename);
  ~ibmFSin();

  int isOpen();

  int getNextTransaction(vector<int>& t);

 private:

  FILE *in;
};


class asciiFSout
{
 public:

  asciiFSout(const char *filename);
  ~asciiFSout();

  int isOpen();

  void writeTransaction(vector<int>& t);

 private:

  FILE *out;
};


/////////////////////////////////////


template <class T>
binFSout<T>::binFSout(const char *filename, bool create)
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
inline void binFSout<T>::writeTransaction(DCIClosed::Transaction<T>& t)
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
binFSin<T>::binFSin(const char *filename)
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
int binFSin<T>::getNextTransaction(DCIClosed::Transaction<T>& t)
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


}

#endif
