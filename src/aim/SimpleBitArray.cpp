#include "StdAfx.h"
#include "SimpleBitArray.h"

#define min(a,b) (((a) < (b))? (a) : (b))
#define max(a,b) (((a) > (b))? (a) : (b))

bool CSimpleBitArray::firstRun = true;
int CSimpleBitArray::oneCount[256];

CSimpleBitArray::CSimpleBitArray(int size)
{
	if (firstRun == true)
	{
		firstRun = false;
		for (unsigned int loop = 0; loop < 256; loop++)
		{
			int count = 0;
			unsigned int diff = loop;
			while (diff > 0)
			{
				if ( (diff & 0x01) == 0x01)
					count++;
				diff >>= 1;
			}	

			oneCount[loop] = count;
		}
	}

	Count = size;
	length = Count/BITS_IN_ULONG + 1;
	bitArray = new MY_ULONG[length];
	::memset(bitArray, 0, length * sizeof(MY_ULONG));
	maxValue = -1;
	minValue = 0x7FFFFFFF;
	itemSum = 0;
}

CSimpleBitArray::CSimpleBitArray(CSimpleBitArray *copy)
{
	Count = copy->Count;
	length = copy->length;
	bitArray = new MY_ULONG[length];
	::memcpy(bitArray,copy->bitArray,length * sizeof(MY_ULONG));
	maxValue = copy->maxValue;
	minValue = copy->minValue;
	itemSum = copy->itemSum;
}

CSimpleBitArray::~CSimpleBitArray(void)
{
}

CSimpleBitArray *CSimpleBitArray::Clone()
{
	return new CSimpleBitArray(this);
}

int CSimpleBitArray::FindFirstItemReverse(int lastValue)
{
	int loop;

	lastValue = min(lastValue, maxValue);
	loop = lastValue / BITS_IN_ULONG;

	for (; loop >= 0; loop--)
	{
		if (bitArray[loop] > 0)
		{
			MY_ULONG bitMask = (MY_ULONG)0x01 << (BITS_IN_ULONG - 1);
			int bitPos = BITS_IN_ULONG - 1;

			while (true)
			{
				if ( (bitMask & bitArray[loop]) != 0)
					return bitPos + (loop * BITS_IN_ULONG);

				bitPos--;
				bitMask >>= 1;
			}
		}
	}

	return -1;
}

int CSimpleBitArray::FindFirstItem(int startIndex)
{
	int maxIndex = maxValue/BITS_IN_ULONG + 1;
	int loop;
	int initShift;

	startIndex = max(startIndex, minValue);
	loop = startIndex / BITS_IN_ULONG;
	initShift = startIndex % BITS_IN_ULONG;

	for (; loop < maxIndex; loop++)
	{
		MY_ULONG diff = bitArray[loop] >> initShift;
		int bitPos = initShift;
		initShift = 0;

		while (diff > 0)
		{
			if ( (diff & 0x01) == 0x01)
				return bitPos + (loop * BITS_IN_ULONG);

			bitPos++;
			diff >>= 1;
		}
	}

	return -1;
}

bool CSimpleBitArray::IsOneItemDiff(CSimpleBitArray *other)
{
	bool foundOneBit = false;
	for (int loop = 0; loop < length; loop++)
	{
		MY_ULONG diff = bitArray[loop] ^ other->bitArray[loop];
		if (diff == 0)
			continue;

		if (foundOneBit == true)
			return false;

		// Find the first '1' in the number
		while ((diff & 0x01) == 0x00) // <==> (n & 0x01) != 0x01
			diff = diff >> 1;

		// The first bit is '1' (n & 0x01 == 0x01), if it's the only bit than
		// n == 1
		// Another option is to check with XOR : (n ^ 0x01) == 0
		if (diff == 1)
			foundOneBit = true;
		else
			return false; // More then one bit on in this ulong
	}

	return true;
}

void CSimpleBitArray::Set(int i, bool val)
{
	// Find the ulong & bit of 'i'
	int arrayPos = i/BITS_IN_ULONG;
	int bitPos = i%BITS_IN_ULONG;

	if (val == true)
	{
		minValue = min(minValue,i);
		maxValue = max(maxValue,i);

		bitArray[arrayPos] = bitArray[arrayPos] | (1UL << bitPos);
		itemSum += i;
	}
	else
	{
		bitArray[arrayPos] = bitArray[arrayPos] ^ (1UL << bitPos);
		if (i == maxValue)
 			maxValue = this->FindFirstItemReverse(maxValue - 1);
		if (i == minValue)
		{
			minValue = this->FindFirstItem(minValue + 1);
			if (minValue == -1)
				minValue = 0x7FFFFFFF;
		}

		itemSum -= i;
	}
}

bool CSimpleBitArray::Get(int i)
{
	// Find the ulong & bit of 'i'
	int arrayPos = i/BITS_IN_ULONG;
	int bitPos = i%BITS_IN_ULONG;
	
	return (bitArray[arrayPos] & (1UL << bitPos)) > 0;
}


CSimpleBitArray *CSimpleBitArray::Or(CSimpleBitArray *other)
{
	CSimpleBitArray *res = new CSimpleBitArray(Count);

	res->minValue = min(minValue,other->minValue);
	res->maxValue = max(maxValue,other->maxValue);

	for (int loop = 0; loop < length; loop++)
		res->bitArray[loop] = bitArray[loop] | other->bitArray[loop];

	return res;			
}

CSimpleBitArray *CSimpleBitArray::And(CSimpleBitArray *other)
{
	CSimpleBitArray *res = new CSimpleBitArray(Count);

	res->minValue = max(minValue,other->minValue);
	res->maxValue = min(maxValue,other->maxValue);

	for (int loop = 0; loop < length; loop++)
		res->bitArray[loop] = bitArray[loop] & other->bitArray[loop];

	return res;	
}

CSimpleBitArray *CSimpleBitArray::Xor(CSimpleBitArray *other)
{
	CSimpleBitArray *res = new CSimpleBitArray(Count);

	for (int loop = 0; loop < length; loop++)
		res->bitArray[loop] = bitArray[loop] ^ other->bitArray[loop];

	return res;	
}

int CSimpleBitArray::CountElements()
{
	int loopMin = minValue / BITS_IN_ULONG;
	int loopMax = maxValue / BITS_IN_ULONG + 1;
	int count = 0;
	for (int loop = loopMin; loop < loopMax; loop++)
	{
		MY_ULONG diff = bitArray[loop];
		while (diff > 0)
		{
//					if ( (diff & 0x01) == 0x01)
//						count++;
//					diff >>= 1;

			count += oneCount[(unsigned int)(diff & 0x000000FF)];
			diff >>= 8;
		}
	}

	return count;
}

bool CSimpleBitArray::IsContaining(CSimpleBitArray *other)
{
	for (int loop = 0; loop < length; loop++)
		if ( (bitArray[loop] & other->bitArray[loop]) != other->bitArray[loop])
			return false;

	return true;
}

bool CSimpleBitArray::Equals(CSimpleBitArray *obj)
{
	return CompareTo(obj) == 0;
}

//		public static bool reverseSort = false;
int CSimpleBitArray::CompareTo(CSimpleBitArray *i)
{

	for (int loop = length - 1; loop >= 0; loop--)
	{
		if (bitArray[loop] > i->bitArray[loop])
			return 1;
		if (bitArray[loop] < i->bitArray[loop])
			return -1;
	}

	return 0;
}
