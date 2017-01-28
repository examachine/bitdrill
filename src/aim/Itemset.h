#ifndef __ITEMSET
#define __ITEMSET

#include "SparseBitArray.h"
#include "ObjectPool.h"

// CItemset - Used as intermidate class to store the newly created frequent 
//	itemsets. In practice it doesn't hold the itemset it self only the last item
//	in the itemset, and the vertical tidset/diffset
class CItemset
{
public:
	CItemset::CItemset(); // using defaultItemCount;
	CItemset(int itemCount);
	CItemset(CItemset *copy);

	~CItemset(void);

	CItemset *Clone();
	void Clear();

	static int defaultItemCount;

	int itemSum;
	int support;

		// Vertical tidset or diffset.
	V_BIT_ARRAY *transactionBitMask;

		// Last item in the 
	int lastItem;
};

#endif
