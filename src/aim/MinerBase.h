#ifndef __MINERBASE // -*-c++-*-
#define __MINERBASE

#include <vector>
#include <string>
#include <set>
#include "data.h"
#include "Items.h"
#include "IntToString.h"
#include "transaction-db/Transaction_Set.hxx"

#define SEPERATOR ' '

typedef int* pInt;

class CMinerBase
{
public:
	CMinerBase(void);
	~CMinerBase(void);

	void Init(char *fileName, int minSupport, bool sort = false);
	void Init(Transaction_Set& ts, int minSupport, bool sort = false);
	void LoadDatabase(Transaction_Set& ts);
	int GetDBCount();
	void Build2ItemsetMatrix(Transaction_Set& ts);

	// vertical database
	CItems items;

protected:
		// 2 Items upper matrix
	int **p2ItemsMatrix;

		// pure database
	unsigned int transactionCount;
	unsigned int dbItemsCount;
};

#endif
