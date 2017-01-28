#include "StdAfx.h"
#include "Items.h"

typedef CItem* pCItem;

CItems::CItems(void)
{
	itemArray = new pCItem[35];
	length = 35;
	Count = 0;
}

CItems::~CItems(void)
{
	for (int loop = 0; loop < Count; loop++)
		delete itemArray[loop];

	delete[] itemArray;
}

void CItems::Sort()
{
	for (int loop = 0; loop < Count; loop++)
		itemArray[loop]->GetTransactionMask()->support = itemArray[loop]->GetTransactionMask()->CountElements();

	::qsort(itemArray, Count, sizeof(CItem *), rev_comp);

	itemsArrayHash.clear();

	for (int loop = 0; loop < Count; loop++)
		itemsArrayHash[itemArray[loop]->name] = loop + 1;
}

int CItems::rev_comp(const void *p1, const void *p2)
{
	return (*((CItem **)p2))->GetTransactionMask()->support - (*((CItem **)p1))->GetTransactionMask()->support;
}

void CItems::AddItem(CItem *newItem)
{
	if (Count >= length)
	{
		CItem **newItemArray = new pCItem[length * 2];
		length = length * 2;
		::memcpy(newItemArray, itemArray, Count * sizeof(pCItem));
		delete[] itemArray;
		itemArray = newItemArray;
	}

	itemsArrayHash[newItem->name] = Count + 1;

	itemArray[Count] = newItem;
	Count++;
}

void CItems::Rebuild(int minSupport)
{
	CItem **newItemArray = new pCItem[Count];
	int newCount = 0;

	itemsArrayHash.clear();

	for (int loop = 0; loop < Count; loop++)
	{
		int support = itemArray[loop]->GetTransactionMask()->CountElements();		
		if (support >= minSupport)
		{
			newItemArray[newCount] = itemArray[loop];
			itemsArrayHash[itemArray[loop]->name] = newCount + 1;
			newCount++;
		}
		else
		{
			delete itemArray[loop];
		}
	}

	length = Count;
	Count = newCount;
	delete[] itemArray;
	itemArray = newItemArray;
}

int CItems::IndexOf(ITEM_NAME &s)
{
#ifdef USE_ITEMS_HASH_LOOKUP
	hash_map <int, int> :: iterator pIter;

	pIter = itemsArrayHash.find(s);
	if ( pIter == itemsArrayHash.end( ) )
		return -1;

	return pIter->second - 1;
#else
	return itemsArrayHash.Get(s) - 1;
#endif
}
