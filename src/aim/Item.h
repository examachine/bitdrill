#ifndef __ITEM
#define __ITEM

#include <string>
#include "SparseBitArray.h"

// CItem - A single item class.
//	includes : name and vertical database for the item.
class CItem
{
public:
	CItem(ITEM_NAME s);
	~CItem(void);

	V_BIT_ARRAY *GetTransactionMask() { return transactionBitMask; }
	inline void AddTransaction(int transactionNum);

	int CompareTo(CItem *o);
	std::string CItem::ToString();

	ITEM_NAME name;
	char s_name[10];
	size_t s_nameLen;

	int index; // Index of the item in the array
private:
	int count;
	V_BIT_ARRAY *transactionBitMask;
};

inline void CItem::AddTransaction(int transactionNum)
{
	++count;
	transactionBitMask->SetAppend(transactionNum);
}

#endif
