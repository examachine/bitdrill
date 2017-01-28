#ifndef __ITEMS
#define __ITEMS

#ifdef USE_ITEMS_HASH_LOOKUP
#include <hash_map>
using namespace std;
#endif
//using namespace stdext;
#include "Item.h"

// CItems - The set of items in the database
class CItems
{
public:
	CItems(void);
	~CItems(void);

		// Add new item
	void AddItem(CItem *newItem);

	CItem *operator[](int idx) { return itemArray[idx]; }

		// Find the index of an item by its name
	int IndexOf(ITEM_NAME &s);

		// Rebuild by support
	void Rebuild(int minSupport);

		// Sort the items by support, from high to low
	void Sort();

	int Count;
private:
		// Sort comperator
	static int rev_comp(const void *p1, const void *p2);

		// Internal data structure variables
	int length;
	CItem **itemArray;

#ifdef USE_ITEMS_HASH_LOOKUP
	hash_map<int, int> itemsArrayHash;
#else
	SIntArray itemsArrayHash;
#endif
};

#endif
