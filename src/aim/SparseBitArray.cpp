#include "StdAfx.h"
#include "SparseBitArray.h"
#include "ObjectPool.h"
#include <stdlib.h>

bool CSparseBitArray::firstRun = true;
unsigned short CSparseBitArray::oneCount[256];
unsigned short CSparseBitArray::oneSum[256];
unsigned short CSparseBitArray::projectionValues[256][256];
CObjectPool<V_BIT_ARRAY> sparseBitArrayPool;

void CSparseBitArray::BuildProjectionLookupTable()
{
	for (int loopProjectOn = 0; loopProjectOn < 256; loopProjectOn++)
	{
		for (int loopProjectedFrom = 0; loopProjectedFrom < 256; loopProjectedFrom++)
		{
			int val = loopProjectOn & loopProjectedFrom;
			int bitMask = loopProjectOn;
			int bitPos = 0;
			int res = 0;

			while (val > 0)
			{
					// Skip on bit that doesn't contain '1'
				if ( (bitMask & 0x01) > 0)
				{
					if ((val & 0x01) > 0)
						res += 1<<bitPos;
					bitPos++;
				}

				val>>=1;
				bitMask >>=1;
			}

			projectionValues[loopProjectOn][loopProjectedFrom] = res;
		}
	}
}

void CSparseBitArray::BuildCountLookupTable()
{
	for (unsigned int loop = 0; loop < 256; loop++)
	{
		int count = 0;
		int sum = 0;
		int pos = 0;

		unsigned int diff = loop;
		while (diff > 0)
		{
			if ( (diff & 0x01) == 0x01)
			{
				count++;
				sum += pos;
			}

			diff >>= 1;
			pos++;
		}	

		oneCount[loop] = count;
		oneSum[loop] = sum;
	}
}

CSparseBitArray::CSparseBitArray(int initSize)
{
	if (firstRun == true)
	{
		firstRun = false;

		BuildProjectionLookupTable();

				// Build oneCount & oneSum static lookup tables
		BuildCountLookupTable();
	}

	keys = new int[initSize/BITS_IN_ULONG + 1];
	values = new MY_ULONG[initSize/BITS_IN_ULONG + 1];
	length = initSize/BITS_IN_ULONG + 1;
	maxUsedPos = 0;
	itemSum = 0;
}

CSparseBitArray::CSparseBitArray()
{
	if (firstRun == true)
	{
		firstRun = false;

		BuildProjectionLookupTable();

				// Build oneCount & oneSum static lookup tables
		BuildCountLookupTable();
	}

	keys = new int[ARRAY_INIT_SIZE];
	values = new MY_ULONG[ARRAY_INIT_SIZE];
	length = ARRAY_INIT_SIZE;
	maxUsedPos = 0;
	itemSum = 0;
}

CSparseBitArray::CSparseBitArray(CSparseBitArray *other)
{
	length = other->length;
	keys = new int[length];
	values = new MY_ULONG[length];
	::memcpy(keys, other->keys, length * sizeof(int));
	::memcpy(values, other->values, length * sizeof(MY_ULONG));
	maxUsedPos = other->maxUsedPos;
	itemSum = other->itemSum;
}

CSparseBitArray::~CSparseBitArray(void)
{
	delete[] keys;
	delete[] values;
}

void CSparseBitArray::Clear()
{
	maxUsedPos = 0;
	itemSum = 0;
}

CSparseBitArray *CSparseBitArray::Clone()
{
	return new CSparseBitArray(this);
}

bool CSparseBitArray::Equals(CSparseBitArray *other)
{
	return CompareTo(other) == 0;
}

int CSparseBitArray::CompareTo(CSparseBitArray *other)
{
	int otherPos = other->maxUsedPos - 1;
	int pos = maxUsedPos - 1;
	
	while ( (pos >= 0)&&(otherPos >= 0) )
	{
		if (keys[pos] > other->keys[otherPos])
			return 1;

		if (keys[pos] < other->keys[otherPos])
			return -1;

		if (values[pos] > other->values[otherPos])
			return 1;
		if (values[pos] < other->values[otherPos])
			return -1;

		pos--;
		otherPos--;
	}

	if ((pos == -1)&&(otherPos == -1))
		return 0;

	if (otherPos == -1)
		return 1;

	return -1;
}

int int_comp( const int *arg1, const int *arg2 )
{
   return *arg1 - *arg2;
}

bool CSparseBitArray::GetFromPos(int i, int &startPos)
{
	int searchFor = i/BITS_IN_ULONG;
	int bitPos = i%BITS_IN_ULONG;

	if (startPos == maxUsedPos)
		return false;

	if (keys[startPos] > searchFor)
		return false;

	for (; startPos < maxUsedPos; startPos++)
	{
		if (keys[startPos] == searchFor)
			break;

		if (keys[startPos] > searchFor)
		{
			startPos--;
			return false;
		}
	}
	
	if (startPos == maxUsedPos)
		return false;

	return	 (values[startPos] & ((MY_ULONG)1 << bitPos)) > 0;
}

bool CSparseBitArray::Get(int i)
{
	int searchFor = i/BITS_IN_ULONG;
	int *p_pos = (int *)::bsearch(&searchFor, keys, maxUsedPos, sizeof(int), (int (*)(const void*, const void*))int_comp);
	if (p_pos == NULL)
		return false;

	int pos = (int)(p_pos - keys);
	int bitPos = i%BITS_IN_ULONG;

	return (values[pos] & ((MY_ULONG)1 << bitPos)) > 0;
}

void CSparseBitArray::Insert(int pos, int key)
{
	if (length <= maxUsedPos + 1)
	{
		int *newKeys = new int[length * 2];
		MY_ULONG *newValues = new MY_ULONG[length * 2];

		::memcpy(newKeys, keys, length * sizeof(int));
		::memcpy(newValues, values, length * sizeof(MY_ULONG));
		length = length * 2;

		delete[] keys;
		delete[] values;
		keys = newKeys;
		values = newValues;
	}

	for (int loop = maxUsedPos - 1; loop >= pos; loop--)
	{
		keys[loop + 1] = keys[loop];
		values[loop + 1] = values[loop];
	}

	keys[pos] = key;
	values[pos] = 0;
	maxUsedPos++;
}

void CSparseBitArray::RemoveAt(int pos)
{
	for (int loop = pos; loop < maxUsedPos; loop++)
	{
		keys[loop] = keys[loop + 1];
		values[loop] = values[loop + 1];
	}

	maxUsedPos--;
}

// ASSUMPTION : The support value is filled.
CSparseBitArray *CSparseBitArray::Project(CSparseBitArray *projectOn)
{
	CSparseBitArray *res = sparseBitArrayPool.Alloc();
	int projectedVectorLength = (projectOn->support / BITS_IN_ULONG) + 1;

		// If the existing vector isn't long enough - enlarge it
	if (res->length < projectedVectorLength)
	{
		delete[] res->keys;
		delete[] res->values;
		res->keys = new int[projectedVectorLength];
		res->values = new MY_ULONG[projectedVectorLength];
		res->length = projectedVectorLength;
	}


	::memset(res->values, 0, projectedVectorLength * sizeof(MY_ULONG));		

		// Update the values array
	int pos = 0;
	int projectOnPos = 0;

	int projectedBitPos = 0;

	int projectOnItemCount = projectOn->maxUsedPos;
	int projectOnKey;

	MY_ULONG projectOnValue;
	MY_ULONG projectFromValue;
	
		// Loop on both arrays
	while ((pos < maxUsedPos)&&(projectOnPos < projectOnItemCount))
	{
		projectOnKey = projectOn->keys[projectOnPos];

		if (keys[pos] < projectOnKey)
		{
			++pos;
		}
		else if (keys[pos] > projectOnKey)
		{
				// Advance the bit position marker in the projection result vector
			projectOnValue = projectOn->values[projectOnPos];
			while (projectOnValue > 0)
			{
				projectedBitPos += oneCount[projectOnValue & 0xFF];
				projectOnValue >>=8;
			}
			++projectOnPos;
		}
		else	// Found a key in both vector
		{
			projectOnValue = projectOn->values[projectOnPos];
			projectFromValue = values[pos];
			while ((projectFromValue > 0)||(projectOnValue > 0))
			{
					// Check the value of projection of projectFromValue on projectOnValue
				MY_ULONG val = projectionValues[projectOnValue & 0xFF][projectFromValue & 0xFF];

					// Check for overflow - check if the new bits position overflow from the current key
					//		to the next key, if so write the overflow to the next key
				if (projectedBitPos % BITS_IN_ULONG + oneCount[projectOnValue & 0xFF] > BITS_IN_ULONG)
				{
					int overflowSize = (projectedBitPos % BITS_IN_ULONG + oneCount[projectOnValue & 0xFF]) - BITS_IN_ULONG;
					MY_ULONG overflow = val>>(oneCount[projectOnValue & 0xFF]-overflowSize);

					res->values[projectedBitPos / BITS_IN_ULONG + 1] |= overflow;
				}

					// Concat the new projected value to the projection result vector
				val <<= projectedBitPos % BITS_IN_ULONG;
				res->values[projectedBitPos / BITS_IN_ULONG] |= val;

				projectedBitPos += oneCount[projectOnValue & 0xFF];
				projectOnValue >>=8;
				projectFromValue >>=8;
			}

				// Advance the bit position marker in the projection result vector
			while (projectOnValue > 0)
			{
				projectedBitPos += oneCount[projectOnValue & 0xFF];
				projectOnValue >>=8;
			}

			++pos;
			++projectOnPos;
		}
	}

		// Update the keys array
	for (int loopKeys = 0; loopKeys < projectedVectorLength; loopKeys++)
		res->keys[loopKeys] = loopKeys;
			
	res->maxUsedPos = projectedVectorLength;

	return res;
}

	// Simple projection of bit-vector on tid-list
CSparseBitArray *CSparseBitArray::Project(CExtendIntArray *extendIntArray)
{
	CSparseBitArray *res = sparseBitArrayPool.Alloc();
	res->support = 0;
	int startPos = 0;

	for (int loop = 0; loop < extendIntArray->CountElements(); ++loop)
	{
		if (GetFromPos((*extendIntArray)[loop], startPos) == true)
		{
			res->SetAppend(loop);
			++res->support;
		}
	}

	return res;
}

	// convert to tid-list
void CSparseBitArray::ToIntArray(CExtendIntArray *extendIntArray)
{
	for (int loop = 0; loop < maxUsedPos; loop++)
	{
		MY_ULONG val = values[loop];
		int pos = 0;

		while (val > 0)
		{
			if ( (val & 0xFF) == 0 )
			{
				val >>= 8;
				pos += 8;
				continue;
			}

			if ( (val & 0x01) > 0 )
				extendIntArray->SetAppend((keys[loop]*BITS_IN_ULONG) + pos);

			pos++;
			val >>= 1;
		}
	}
}

void CSparseBitArray::SetAppend(int i)
{
	if (maxUsedPos == 0)
	{
		Insert(0, i/BITS_IN_ULONG);
	}
	else if (keys[maxUsedPos - 1] != i/BITS_IN_ULONG)
	{
		Insert(maxUsedPos, i/BITS_IN_ULONG);
		itemSum += i/BITS_IN_ULONG;
	}

	values[maxUsedPos - 1] |= (MY_ULONG)1<<(i%BITS_IN_ULONG);
}

void CSparseBitArray::Set(int i, bool val)
{
	int searchFor = i/BITS_IN_ULONG;
	int *p_pos = (int *)::bsearch(&searchFor, keys, maxUsedPos, sizeof(int), (int (*)(const void*, const void*))int_comp);
	int pos;

	if (p_pos == NULL)
	{
		// The item doesn't exist, don't add it if all you got to do is to 
		// remove it...
		if (val == false)
			return;

		if (maxUsedPos == 0)
		{
			pos = 0;
			Insert(pos, i/BITS_IN_ULONG);
		}
		else
		{
				// BinarySearch & Insert
			int minItem = 0;
			int maxItem = maxUsedPos - 1;

			int mid;
			int comp;
			while (::abs(minItem - maxItem) > 0)
			{
				mid = (minItem + maxItem)/2;
				comp = keys[mid] - (i/BITS_IN_ULONG);

				if (comp > 0)
				{
					if (maxItem == mid)
						mid--;
					maxItem = mid;
				}
				else if (comp < 0)
				{
					if (minItem == mid)
						mid++;
					minItem = mid;
				}
				else //(comp == 0)
					comp = 0; // HA?
			}
		
			comp = keys[minItem] - (i/BITS_IN_ULONG);					
			if (comp > 0)
				pos = minItem;
			else // comp < 0
				pos = minItem + 1;

			Insert(pos, i/BITS_IN_ULONG);
			itemSum += i/BITS_IN_ULONG;
		}
	}
	else
	{
		pos = (int)(p_pos - keys);
	}

	if (val == true)
	{
		values[pos] |= (MY_ULONG)1<<(i%BITS_IN_ULONG);
	}
	else
	{
		// Assumes the value exists
		values[pos] ^= (MY_ULONG)1<<(i%BITS_IN_ULONG);
		if (values[pos] == 0)
		{
			RemoveAt(pos);
			itemSum -= i/BITS_IN_ULONG;
		}
	}
}

bool CSparseBitArray::IsContaining(CSparseBitArray *other)
{
	int pos = 0;
	int otherPos = 0;
	int count = maxUsedPos;
	int otherCount = other->maxUsedPos;
	
	while ( (pos < count)&&(otherPos < otherCount) )
	{
		if (keys[pos] > other->keys[otherPos])
			return false;

		if (keys[pos] < other->keys[otherPos])
		{
			pos++;
			continue;
		}

		if ( (values[pos] & other->values[otherPos]) != other->values[otherPos])
			return false;

		pos++;
		otherPos++;
	}

	if ((pos == count)&&(otherPos < otherCount))
		return false;

	return true;
}

CSparseBitArray *CSparseBitArray::And(CSparseBitArray *other)
{
	CSparseBitArray *res = sparseBitArrayPool.Alloc();// new CSparseBitArray();
	res->support = 0;

	int pos = 0;
	int otherPos = 0;

	int otherItemCount = other->maxUsedPos;
	int otherKey;

	while ((pos < maxUsedPos)&&(otherPos < otherItemCount))
	{
		otherKey = other->keys[otherPos];

		if (keys[pos] < otherKey)
		{
			++pos;
		}
		else if (keys[pos] > otherKey)
		{
			++otherPos;
		}
		else
		{
			if ( (values[pos] & other->values[otherPos]) != 0)
			{
				res->Insert(res->maxUsedPos, keys[pos]);
				res->values[res->maxUsedPos-1] = (values[pos] & other->values[otherPos]);
				res->itemSum += keys[pos];
			}
			++pos;
			++otherPos;
		}
	}

	return res;
}

int CSparseBitArray::AndCount(CSparseBitArray *other)
{
	int count = 0;

	int pos = 0;
	int otherPos = 0;

	int otherItemCount = other->maxUsedPos;
	int otherKey;

	while ((pos < maxUsedPos)&&(otherPos < otherItemCount))
	{
		otherKey = other->keys[otherPos];

		if (keys[pos] < otherKey)
		{
			++pos;
		}
		else if (keys[pos] > otherKey)
		{
			++otherPos;
		}
		else
		{
			if ( (values[pos] & other->values[otherPos]) != 0)
				count += CountElement(values[pos] & other->values[otherPos]);

			++pos;
			++otherPos;
		}
	}

	return count;
}

CSparseBitArray *CSparseBitArray::AndNot(CSparseBitArray *other)
{
	CSparseBitArray *res = sparseBitArrayPool.Alloc();// new CSparseBitArray();
	res->support = 0;

	int pos = 0;
	int otherPos = 0;
	int otherKey;
	int otherItemCount = other->maxUsedPos;

	while ((pos < maxUsedPos)&&(otherPos < otherItemCount))
	{
		otherKey = other->keys[otherPos];
		if (keys[pos] < otherKey)
		{
			res->Insert(res->maxUsedPos, keys[pos]);
			res->values[res->maxUsedPos-1] = values[pos];;
			res->itemSum += keys[pos];

			++pos;
		}
		else if (keys[pos] > otherKey)
		{
			++otherPos;
		}
		else
		{
			if ( (values[pos] & ~other->values[otherPos]) != 0)
			{
				res->Insert(res->maxUsedPos, keys[pos]);
				res->values[res->maxUsedPos-1] = (values[pos] & ~other->values[otherPos]);
				res->itemSum += keys[pos];
			}
			++pos;
			++otherPos;
		}
	}

	while (pos < maxUsedPos)
	{
		res->Insert(res->maxUsedPos, keys[pos]);
		res->values[res->maxUsedPos-1] = values[pos];
		res->itemSum += keys[pos];

		++pos;
	}

	return res;
}

CSparseBitArray* CSparseBitArray::Or(CSparseBitArray *other)
{
	CSparseBitArray *res = new CSparseBitArray();
	int pos = 0;
	int otherPos = 0;

	while ((pos < maxUsedPos)||(otherPos < other->maxUsedPos))
	{
		int key;
		MY_ULONG val;

		if (otherPos == other->maxUsedPos)
		{
			key = keys[pos];
			val = values[pos];
			pos++;
		}
		else if (pos == maxUsedPos)
		{
			key = other->keys[otherPos];
			val = other->values[otherPos];
			otherPos++;
		}
		else if (keys[pos] < other->keys[otherPos])
		{
			key = keys[pos];
			val = values[pos];
			pos++;
		}
		else if (keys[pos] > other->keys[otherPos])
		{
			key = other->keys[otherPos];
			val = other->values[otherPos];
			otherPos++;
		}
		else
		{
			key = keys[pos];
			val = values[pos] | other->values[otherPos];
			pos++;
			otherPos++;
		}

		res->Insert(res->maxUsedPos, key);
		res->values[res->maxUsedPos - 1] = val;
	}

	return res;
}

CSparseBitArray *CSparseBitArray::Xor(CSparseBitArray *other)
{
	CSparseBitArray *res = new CSparseBitArray();
	int pos = 0;
	int otherPos = 0;

	while ((pos < maxUsedPos)||(otherPos < other->maxUsedPos))
	{
		int key;
		MY_ULONG val;

		if (otherPos == other->maxUsedPos)
		{
			key = keys[pos];
			val = values[pos];
			pos++;
		}
		else if (pos == maxUsedPos)
		{
			key = other->keys[otherPos];
			val = other->values[otherPos];
			otherPos++;
		}
		else if (keys[pos] < other->keys[otherPos])
		{
			key = keys[pos];
			val = values[pos];
			pos++;
		}
		else if (keys[pos] > other->keys[otherPos])
		{
			key = other->keys[otherPos];
			val = other->values[otherPos];
			otherPos++;
		}
		else
		{
			key = keys[pos];
			val = values[pos] ^ other->values[otherPos];
			pos++;
			otherPos++;
		}

		res->Insert(res->maxUsedPos, key);
		res->values[res->maxUsedPos - 1] = val;
	}

	return res;
}

int CSparseBitArray::SumElements()
{
	int sum = 0;
	int bitArrayCount = maxUsedPos;
	int baseValue;

	for (int loop = 0; loop < bitArrayCount; loop++)
	{
		baseValue = keys[loop];
		MY_ULONG diff = values[loop];

		while (diff > 0)
		{
			sum += (oneCount[diff & 0x000000FF] * baseValue) + (oneSum[diff & 0x000000FF]);
			diff >>= 8;
			baseValue += 8;
		}
	}

	return sum;
}

int CSparseBitArray::CountElements()
{
	support = 0;
	int bitArrayCount = maxUsedPos;

	for (int loop = 0; loop < bitArrayCount; ++loop)
	{
		MY_ULONG diff = values[loop];

		while (diff > 0)
		{
			support += oneCount[diff & 0x000000FF];
			diff >>= 8;
		}
	}

	return support;
}

int CSparseBitArray::CountAndSumElements(int &sum)
{
	int count = 0;
	sum = 0;
	int bitArrayCount = maxUsedPos;
	int baseValue;

	for (int loop = 0; loop < bitArrayCount; ++loop)
	{
		MY_ULONG diff = values[loop];
		baseValue = keys[loop] * BITS_IN_ULONG;

		while (diff > 0)
		{
			sum += (oneCount[diff & 0x000000FF] * baseValue) + (oneSum[diff & 0x000000FF]);
			count += oneCount[diff & 0x000000FF];
			diff >>= 8;
			baseValue += 8;
		}
	}

	return count;
}

bool CSparseBitArray::IsOneItemDiff(CSparseBitArray *other)
{
	CSparseBitArray *tmp = Xor(other);
	bool res = tmp->CountElements() == 1;
	delete tmp;

	return res;
}

