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

#ifndef __DATABASE_H
#define __DATABASE_H

#include <vector>
#include <stdio.h>
#include <unistd.h>

#include "transaction.hh"
#include "items.hh"


using namespace std;	

class Data
{
 public:
	
  Data(char *filename) {
    in = fopen(filename,"rt");
  }

  ~Data() {
    if(in) fclose(in);
  }

  int isOpen(){
    if(in) return 1;
    else return 0;
  }

  
  inline int getNextTransaction(vector<unsigned int>& t);
  inline int getNextTransaction(Transaction<unsigned int>& t);
  
 private:
  
  FILE *in;
};


inline int Data::getNextTransaction(vector<unsigned int>& t)
{
  t.resize(0);

  int c;

  // read list of items
  do {
    c = fgetc(in);

    if ((c >= '0') && (c <= '9')) {
      unsigned int item = c-'0';
      c = fgetc(in);
      while((c >= '0') && (c <= '9')) {
	item *=10;
	item += c-'0';
	c = fgetc(in);
      }
      t.push_back(item);
    }
  } while(c != '\n' && !feof(in));

  // if end of file is reached, return 0;
  if(feof(in)){
    return 0;
  }
  
  return 1;
}



inline int Data::getNextTransaction(Transaction<unsigned int>& t)
{
  int c;

  t.length = 0;

  // read list of items
  do {
    c = fgetc(in);

    if ((c >= '0') && (c <= '9')) {
      unsigned int item = c-'0';
      c = fgetc(in);
      while((c >= '0') && (c <= '9')) {
	item *=10;
	item += c-'0';
	c = fgetc(in);
      }

      t.t[t.length] = item;
      t.length++;
    }
  } while(c != '\n' && !feof(in));

  // if end of file is reached, return 0;
  if(feof(in)){
    return 0;
  }
  
  return 1;
}


class FSout
{
 public:

  FSout(char *filename, int iter) {
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

  binFSout(char *filename, bool create);
  ~binFSout();

  int isOpen();
  int trunc();
  inline void writeTransaction(vector<T>& t);
  inline void writeTransaction(Transaction<T>& t);
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

  binFSin(char *filename);
  ~binFSin();

  int isOpen();

  int getNextTransaction(Transaction<T>& t);
  int getNextTransaction(dci_transaction<T>& t);

 private:

  FILE *in;
};








class ibmFSin
{
 public:

  ibmFSin(char *filename);
  ~ibmFSin();

  int isOpen();

  int getNextTransaction(vector<int>& t);

 private:

  FILE *in;
};



class asciiFSout
{
 public:

  asciiFSout(char *filename);
  ~asciiFSout();

  int isOpen();

  void writeTransaction(vector<int>& t);

 private:

  FILE *out;
};



#endif




