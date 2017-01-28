#include "StdAfx.h"
#include "SortedIntArray.h"

int CSortedIntArray::DefaultSize = 0;
CObjectPool<CExtendIntArray> extendIntArrayPool;

CSortedIntArray::CSortedIntArray(int size)
{ 
	Count = 0;
	pDualItem = new DualItem[size]; 
}

CSortedIntArray::CSortedIntArray()
{ 
	Count = 0;
	pDualItem = new DualItem[DefaultSize]; 
}

CSortedIntArray::~CSortedIntArray() 
{ 
	delete pDualItem; 
}


int backets[16][10];
int backetsCount[16];

int arraySort[1024];

bool radix (int byte, int N, int *source, int *dest)
{
	static int count[256];
	static int index[256];
	memset (count, 0, sizeof (count));
	int i;
	for ( i=0; i<N; i++ ) count[((source[i])>>(byte*8))&0xff]++;

	index[0]=0;
	for ( i=1; i<256; i++ ) index[i]=index[i-1]+count[i-1];
	if (index[1] == index[255]) return false;
	for ( i=0; i<N; i++ ) dest[index[((source[i])>>(byte*8))&0xff]++] = source[i];

	return true;
}

void radixsort (int *source, int *temp, int N)
{
	radix (0, N, source, temp);
	if (radix (1, N, temp, source) == false) return;
	if (radix (2, N, source, temp) == false) return;
	if (radix (3, N, temp, source) == false) return;
}

void CExtendIntArray::RadixSort()
{
	radixsort(arr, arraySort, Count);
	// Sort using fixed size array
	/*
	memset(arraySort, 0, 1024 * sizeof(int));
	for (int loop = 0; loop < Count; loop++)
	{
		arraySort[arr[loop]] = 1;
	}

	int po = 0;
	for (int loop = 0; loop < 1024; loop++)
	{
		if (arraySort[loop] > 0)
			arr[pos++] = loop;
	}*/
}

void CExtendIntArray::SetAppend(int i)
{
	if (Count == length) 
	{
		int *newArr = new int[length * 2];
		::memcpy(newArr, arr, Count * sizeof(int));
		delete[] arr;
		arr = newArr;
		length *= 2;
	}
	arr[Count++] = i;
}

bool CExtendIntArray::IsContaining(CExtendIntArray *other)
{
	int pos = 0;
	int otherPos = 0;
	int otherCount = other->Count;

	if (Count < otherCount)
		return false;

	while ( (pos < Count)&&(otherPos < otherCount) )
	{
		if (arr[pos] > other->arr[otherPos])
			return false;

		if (arr[pos] < other->arr[otherPos])
		{
			pos++;
			continue;
		}

		pos++;
		otherPos++;
	}

	if ((pos == Count)&&(otherPos < otherCount))
		return false;

	return true;
}

CExtendIntArray *CExtendIntArray::And(CExtendIntArray *other)
{
	CExtendIntArray *res = extendIntArrayPool.Alloc();

	int pos = 0;
	int otherPos = 0;
	int itemCount = Count;
	int otherItemCount = other->Count;

	while ((pos < itemCount)&&(otherPos < otherItemCount))
	{
		if (arr[pos] < other->arr[otherPos])
		{
			pos++;
		}
		else if (arr[pos] > other->arr[otherPos])
		{
			otherPos++;
		}
		else
		{
			res->SetAppend(arr[pos]);

			pos++;
			otherPos++;
		}
	}

	return res;
}

CExtendIntArray *CExtendIntArray::AndNot(CExtendIntArray *other)
{
	CExtendIntArray *res = extendIntArrayPool.Alloc();

	int pos = 0;
	int otherPos = 0;
	int itemCount = Count;
	int otherItemCount = other->Count;

	while ((pos < itemCount)&&(otherPos < otherItemCount))
	{
		if (arr[pos] < other->arr[otherPos])
		{
			res->SetAppend(arr[pos]);
			pos++;
		}
		else if (arr[pos] > other->arr[otherPos])
		{
			otherPos++;
		}
		else
		{
			pos++;
			otherPos++;
		}
	}

	while (pos < itemCount)
	{
		res->SetAppend(arr[pos]);
		pos++;
	}

	return res;
}

int CCompressedIntArray::CompressRation = 4;
int CCompressedIntArray::CompressMask = 0xFF;

CCompressedIntArray::CCompressedIntArray(CExtendIntArray *other)
{
	int i;
	length = (other->Count/CompressRation) + 1;
	arr = new int[length];
	::memset(arr, 0, length * sizeof(int));
	Count = 0;
	for (int loop = 0; loop < other->Count; loop++)
	{
		i = (*other)[loop];
		i <<= (Count%CompressRation)*(32/CompressRation);
		arr[Count++/CompressRation] |= i;
	}

	support = other->support;
}

bool CCompressedIntArray::IsContaining(CExtendIntArray *other)
{
	int pos = 0;
	int otherPos = 0;
	int otherCount = other->Count;
	
	if (Count < otherCount)
		return false;

	while ( (pos < Count)&&(otherPos < otherCount) )
	{
		if ((*this)[pos] > (*other)[otherPos])
			return false;

		if ((*this)[pos] < (*other)[otherPos])
		{
			pos++;
			continue;
		}

		pos++;
		otherPos++;
	}

	if ((pos == Count)&&(otherPos < otherCount))
		return false;

	return true;
}
