#ifndef __SPARSEBITARRAY
#define __SPARSEBITARRAY

#include "SortedIntArray.h"
#include "ObjectPool.h"

class CExtendIntArray;

#define ARRAY_INIT_SIZE (35)

class CSparseBitArray
{
public:
	int support;

	CSparseBitArray();
	CSparseBitArray(int initSize);
	CSparseBitArray(CSparseBitArray *other);
	~CSparseBitArray(void);

	CSparseBitArray *Clone();

	bool Equals(CSparseBitArray *other);
	int CompareTo(CSparseBitArray *other);

	void Clear();

	bool GetFromPos(int i, int &startPos);
	bool Get(int i);
	void Set(int i, bool val);
	void SetAppend(int i);

		// Project current vector on a given bit vector
	CSparseBitArray *Project(CSparseBitArray *projectOn);

		// Project current vector on a given tid-list
	CSparseBitArray *Project(CExtendIntArray *extendIntArray);
	void ToIntArray(CExtendIntArray *extendIntArray);

	bool IsContaining(CSparseBitArray *other);

		// Bitwise vertical operations : And, AndNot (A & ~B), Xor, Or
	CSparseBitArray *And(CSparseBitArray *other);
	int AndCount(CSparseBitArray *other);
	CSparseBitArray *Or(CSparseBitArray *other);
	CSparseBitArray *Xor(CSparseBitArray *other);
	CSparseBitArray *AndNot(CSparseBitArray *other);

	int CountElements();
	int SumElements();
	int CountAndSumElements(int &sum);

	bool IsOneItemDiff(CSparseBitArray *other);
	int GetItemSum() { return itemSum; }
protected:
	void Insert(int pos, int key);
	void RemoveAt(int pos);

//protected:
public:
	static void BuildProjectionLookupTable();
	static void BuildCountLookupTable();

	int maxUsedPos;

	int *keys;
	MY_ULONG *values;
	int length;

	int itemSum;

			// Lookup-Tables
	static bool firstRun; 

			// Number of '1's in each byte
	static unsigned short oneCount[256]; 

			// Sum value of '1's in each byte
	static unsigned short oneSum[256];

			// Projection result of Y on X 
	static unsigned short projectionValues[256/*X*/][256/*Y*/];

	static int CountElement(MY_ULONG element)
	{
		int count = 0;
		while (element > 0)
		{
			count += oneCount[element & 0x000000FF];
			element >>= 8;
		}
		return count;
	}

};



struct SIntArray
{
public:
	SIntArray()
	{
		_buffer = new int[1];
		_bufferSize = 1;
		Clear();		
	}

	~SIntArray()
	{
		delete[] _buffer;
	}

	int &operator[](int i) const
	{ 
		return _buffer[i]; 
	}

	int &operator[](int i) 
	{ 
		if (i >= _bufferSize)
		{
			int newSize = (i >= _bufferSize * 2)? i + 10:_bufferSize * 2;
			int *tmp = new int[newSize * sizeof(int)];
			::memset(tmp, 0, newSize * sizeof(int));
			::memcpy(tmp, _buffer, _bufferSize * sizeof(int));
			delete[] _buffer;
			_buffer = tmp;
			_bufferSize = newSize;
		}

		return _buffer[i]; 
	}

	int Get(int &idx)
	{
		if (idx >= _bufferSize)
			return 0;

		return _buffer[idx];
	}

	void Clear() { ::memset(_buffer, 0, _bufferSize * sizeof(int)); }
	void clear() { ::memset(_buffer, 0, _bufferSize * sizeof(int)); }

private:
	int		*_buffer;
	int		_bufferSize;
};

#endif
