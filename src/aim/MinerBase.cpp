#include "StdAfx.h"
#include "MinerBase.h"
#include "data.h"

CMinerBase::CMinerBase(void) : transactionCount(0)
{
	p2ItemsMatrix = NULL;
}

CMinerBase::~CMinerBase(void)
{
	if (p2ItemsMatrix != NULL)
	{
		for (int loop = 1; loop < items.Count; loop++)
			delete[] p2ItemsMatrix[loop];

		delete[] p2ItemsMatrix;
	}
}

void CMinerBase::Init(char *fileName, int minSupport, bool sort)
{
  Transaction_Set ts(fileName, Transaction::ascii);
  Init(ts, minSupport, sort);
}

void CMinerBase::Init(Transaction_Set& ts, int minSupport, bool sort)
{
	LoadDatabase(ts);

		// Build 2-itemsets matrix, if the number of items is not too
		// high and the avg. number of items in transaction is low
	if (((float)dbItemsCount/this->transactionCount) < 15)
	{
			// Remove all un-needed items (e.g. items with support < minSupport
		items.Rebuild(minSupport);

		if (items.Count < 10000)
			Build2ItemsetMatrix(ts);
	}

	if (sort == true)
		items.Sort();
}

int CMinerBase::GetDBCount()
{
	return transactionCount;
}

void CMinerBase::Build2ItemsetMatrix(Transaction_Set& ts)
{
	p2ItemsMatrix = new pInt[items.Count];
	for (int loop = 1; loop < items.Count; loop++)
	{
		p2ItemsMatrix[loop] = new int[loop];
		::memset(p2ItemsMatrix[loop], 0, loop * sizeof(int));
	}

		/****/
	Data db(ts);
	Transaction trans;

	transactionCount = 0;
	while ( db.getNextTransaction(trans) == true)
	{
		for (int loopTrans = 0; loopTrans < trans.size(); loopTrans++)
			trans[loopTrans] = items.IndexOf(trans[loopTrans]);

		for (int loopItem = 0; loopItem < trans.size() - 1; loopItem++)
		{
			int pos = trans[loopItem];
			for (int loopItem2 = loopItem + 1; loopItem2 < trans.size(); loopItem2++)
			{
				int pos2 = trans[loopItem2];
				if ((pos >= 0)&&(pos2 >= 0)&&(pos != pos2))
				{
					if (pos > pos2)
					{
						p2ItemsMatrix[pos][pos2]++;
					}
					else
					{
						p2ItemsMatrix[pos2][pos]++;
					}
				}
			}
		}

		transactionCount++;
	}
}

void CMinerBase::LoadDatabase(Transaction_Set& ts)
{
	Data db(ts);
	Transaction trans;

	transactionCount = 0;
	dbItemsCount = 0;
	while ( db.getNextTransaction(trans) )
	{
		dbItemsCount += trans.size();
		for (int loopItem = 0; loopItem < trans.size(); loopItem++)
		{
			ITEM_NAME i = trans[loopItem];

			int pos = items.IndexOf(i);
			if (pos < 0)
			{
				CItem *newItem = new CItem(i);
				newItem->AddTransaction(transactionCount);
				items.AddItem(newItem);
			}
			else
			{
				items[pos]->AddTransaction(transactionCount);
			}
		}

		++transactionCount;
	}
}
