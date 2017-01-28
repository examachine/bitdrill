#ifndef __SORTEDINTARRAY
#define __SORTEDINTARRAY

#include "ObjectPool.h"

#include <cstring>

struct DualItem
{
	int key;
	int val;
};

class CSortedIntArray
{
public:
	CSortedIntArray(int size);
	CSortedIntArray();
	~CSortedIntArray();

	void Add(int key, int val) { pDualItem[Count].key = key; pDualItem[Count].val = val; Count++; }
	void Sort() { ::qsort(pDualItem, Count, sizeof(DualItem), comp); }

	int &operator[](int i) { return pDualItem[i].val; }
	void Clear() { Count = 0; }

	int Count;
	static int DefaultSize;
private:
	static int comp(const void *p1, const void *p2)
	{
		return ((DualItem *)p1)->key - ((DualItem *)p2)->key;
	}

	DualItem *pDualItem;
};

#define BASIC_INT_ARRAY_LENGTH (10)

class CExtendIntArray
{
public:
	CExtendIntArray(): Count(0), length(BASIC_INT_ARRAY_LENGTH) { arr = new int[BASIC_INT_ARRAY_LENGTH]; }
	CExtendIntArray(CExtendIntArray *other):
	  support(other->support),  Count(other->Count), length(other->Count)
	  {
	    arr = new int[length];
	    memcpy(arr, other->arr, Count * sizeof(int));
	  }
	
	~CExtendIntArray() { delete[] arr; }

	CExtendIntArray *Clone() { return new CExtendIntArray(this); }

	void SetAppend(int i);

	void RadixSort();
	void Sort() { ::qsort(arr, Count, sizeof(int), comp); }

	bool IsContaining(CExtendIntArray *other);
	CExtendIntArray *And(CExtendIntArray *other);
	CExtendIntArray *AndNot(CExtendIntArray *other);

	int CountElements() { return Count; }
	void Clear() { Count = 0; }
	int &operator[](int idx) { return arr[idx]; }

	int support;
	int Count;
protected:
	static int comp(const void *p1, const void *p2)
	{
		return *((int *)p1) - *((int *)p2);
	}

	int length;
	int *arr;
};

template<class T> class CExtendingArray
{
public:
	CExtendingArray():
	  Count(0), length(BASIC_INT_ARRAY_LENGTH) {
	  buffer = new T[BASIC_INT_ARRAY_LENGTH]();
	}
	CExtendingArray(CExtendIntArray *other):
	  Count(other->Count), length(other->Count) {
	  buffer = new T[length];
	  memcpy(buffer, other->arr, Count * sizeof(T));
	}
	~CExtendingArray() { delete[] buffer; }

	void Add(T item)
	{
		if (Count == length)
		{
			T *newArr = new T[length * 2];
			::memcpy(newArr, buffer, Count * sizeof(T));
			delete[] buffer;
			buffer = newArr;
			length *= 2;
		}

		buffer[Count++] = item;
	}

	int Count;

	T* buffer;
protected:
	int length;
};

class CCompressedIntArray
{
public:
	static int CompressRation;
	static int CompressMask;

	CCompressedIntArray(CExtendIntArray *other);
	~CCompressedIntArray() { delete[] arr; }

	bool IsContaining(CExtendIntArray *other);

	int CountElements() { return Count; }
	void Clear() { Count = 0; }
	int operator[](int idx) const
	{		
		return (arr[idx/CompressRation]>>((idx%CompressRation)*(32/CompressRation)))&CompressMask; 
	}

	int support;
	int Count;
protected:
	int length;
	int *arr;
};

template <class T> 
class CSimpleVector
{
public:
	CSimpleVector(int init = 2):count(0), length(init)
	{
		vec = new T[init];
	}

	~CSimpleVector()
	{
		delete[] vec;
	}

	void Append(T newItem)
	{
		if (count == length)
		{
			T *newVec = new T[length * 2];
			memcpy(newVec, vec, count * sizeof(T));
			delete[] vec;
			vec = newVec;
			length *= 2;
		}

		vec[count++] = newItem;
	}

	int &GetCount() { return count; }
	T &operator[](int idx) { return vec[idx]; }
private:
	T* vec;
	int count;
	int length;
};
//extern CObjectPool<CExtendIntArray> extendIntArrayPool;

#endif
