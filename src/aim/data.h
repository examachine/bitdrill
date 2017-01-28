#ifndef __DATA
#define __DATA
/*----------------------------------------------------------------------
  File     : data.h
  Contents : data set management
----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "transaction-db/Transaction_Set.hxx"

class CTransaction 
{
public:
	
	CTransaction(int l) : length(0), count(l) { t = new int[l];}
	~CTransaction(){delete [] t;}
  
	void Init() { length = 0; };

	inline void Add(int i)
	{
		if (length == count)
		{
			int *newArr = new int[count * 2];
			::memcpy(newArr, t, count * sizeof(int));
			delete[] t;
			t = newArr;
			count *= 2;
		}

		t[length++] = i;
	}

	int length;
	int *t;
private:
	int count;
};

class Data
{
public:
	Data(Transaction_Set& ts);
	~Data();

	int isOpen();
	  
	bool getNextTransaction(Transaction &t);
private:
	Transaction_Set& ts;
	Transaction_Set::Scanner* scanner;
};

#endif
