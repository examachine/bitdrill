#ifndef __SIMPLEBITARRAY
#define __SIMPLEBITARRAY

class CSimpleBitArray
{
public:
	CSimpleBitArray(int size);
	CSimpleBitArray(CSimpleBitArray *copy);

	~CSimpleBitArray(void);

	int FindFirstItemReverse(int lastValue);
	int FindFirstItem(int startIndex);

	bool IsOneItemDiff(CSimpleBitArray *other);
	void Set(int i, bool val);
	bool Get(int i);
	int CountElements();

	bool IsContaining(CSimpleBitArray *other);
	bool Equals(CSimpleBitArray *obj);
	int CompareTo(CSimpleBitArray *i);

	CSimpleBitArray *Or(CSimpleBitArray *other);
	CSimpleBitArray *Xor(CSimpleBitArray *other);
	CSimpleBitArray *And(CSimpleBitArray *other);
	CSimpleBitArray *Clone();

	int GetItemSum() { return itemSum; }

	int support;

	int Count;
	int minValue, maxValue;
private:	
	MY_ULONG *bitArray;
	int length;


	int itemSum;

	static bool firstRun;
	static int oneCount[256];
};

#endif
