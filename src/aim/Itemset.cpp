#include "StdAfx.h"
#include "Itemset.h"
#include "ObjectPool.h"

int CItemset::defaultItemCount = 0;
extern CObjectPool<V_BIT_ARRAY> sparseBitArrayPool;

CItemset::CItemset(int itemsCount)
{
//	itemset = new CSimpleBitArray(itemsCount);
	support = -1;
	transactionBitMask = NULL;
}

CItemset::CItemset()
{
//	itemset = new CSimpleBitArray(defaultItemCount);
	support = -1;
	transactionBitMask = NULL;
}

CItemset::CItemset(CItemset *copy)
{
//	itemset = copy->itemset->Clone();
	support = copy->support;
	transactionBitMask = NULL;
}

CItemset::~CItemset(void)
{
	Clear();
//	delete itemset;
}

void CItemset::Clear()
{
	if (transactionBitMask != NULL)
	{
	//	delete transactionBitMask;
		sparseBitArrayPool.Free(transactionBitMask);
	}
	transactionBitMask = NULL;
	support = -1;
}

CItemset *CItemset::Clone()
{
	return new CItemset(this);
}
/*
CItemset* CItemset::And(CItemset *i1, CItemset *i2)
{
	CItemset *i = new CItemset(i1->itemset->Count);
	delete i->itemset;
	i->itemset = i1->itemset->And(i2->itemset);

	return i;
}

CItemset* CItemset::Or(CItemset *i1, CItemset *i2)
{
	CItemset *i = new CItemset(i1->itemset->Count);
	delete i->itemset;
	i->itemset = i1->itemset->Or(i2->itemset);

	return i;
}
*/

