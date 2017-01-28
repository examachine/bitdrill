#include "StdAfx.h"
#include "DFSMiner.h"
#include "SortedIntArray.h"
#include "ObjectPool.h"
#include <math.h>
#include "SimpleBitArray.h"

	// Object pools
CObjectPool<SFixedIntArray> intBufferPool;
CObjectPool<SFixedCItemsetArray> itemsetBufferPool;
CObjectPool<CItemset> itemsetPool;
CObjectPool<CSortedIntArray> sortedArrayPool;
CObjectPool<CExtendIntArray> smallSparseBitArrayPool;
extern CObjectPool<CExtendIntArray> extendIntArrayPool;
extern CObjectPool<V_BIT_ARRAY> sparseBitArrayPool;

int SFixedIntArray::arraySize = 0;
int SFixedCItemsetArray::arraySize = 0;

CDFSMiner::CDFSMiner(): fileWriter(NULL)
{
}

void CDFSMiner::SetOutputFile(const char *fileName)
{
	if (fileName != NULL)
	{
		fileWriter = new CFileWriter(fileName);
	}
	else
	{
		fileWriter = NULL;
	}
}

CDFSMiner::~CDFSMiner()
{
	if (fileWriter != NULL)
		delete fileWriter;
}

void CDFSMiner::SetStaticParameters()
{
	SFixedIntArray::arraySize = items.Count;
	SFixedCItemsetArray::arraySize = items.Count;
	CItemset::defaultItemCount = items.Count;
	CSortedIntArray::DefaultSize = items.Count;

	if (items.Count <= 0xFF)
	{
		CCompressedIntArray::CompressRation = 4;
		CCompressedIntArray::CompressMask = 0xFF;
	}
	else if (items.Count <= 0xFFFF)
	{
		CCompressedIntArray::CompressRation = 2;
		CCompressedIntArray::CompressMask = 0xFFFF;
	}
	else
	{
		CCompressedIntArray::CompressRation = 1;
		CCompressedIntArray::CompressMask = 0xFFFFFFFF;
	}
}

void CDFSMiner::PrintEmptyTransaction()
{
	(*frequentItemsetsFound)[0]++;
	if (fileWriter != NULL)
	{
		char buf[15];

		sprintf(buf,"(%d)%s", transactionCount, CRLF);
		fileWriter->WriteLine(buf,strlen(buf));
	}
}

int CDFSMiner::FindFrequentPatterns(int minSupport)
{
	levelInfo = new SDFSLevelInfo[items.Count + 2];
	SetStaticParameters();
	frequentItemsetsFound = new SFixedIntArray(items.Count + 1);

	SFixedCItemsetArray *tails = itemsetBufferPool.Alloc();
		// Create initial tail
	CItemset *tail;
	int tailCount = 0;
	int maxSupport = -1;
	for (int loop = 0; loop < items.Count; loop++)
	{
		int support = items[loop]->GetTransactionMask()->CountElements();
		if (minSupport > support)
			continue;

		maxSupport = (support > maxSupport)?support:maxSupport;

		tail = itemsetPool.Alloc();//new CItemset(items.Count);
		tail->support = support;
		tail->transactionBitMask = items[loop]->GetTransactionMask();
		tail->lastItem = loop;

		tails->buffer[tailCount++] = tail;
	}

#ifdef MINE_FCI
	// Print the empty transaction only NONE of the support of single
	// items equals to the transaction count
	if ((maxSupport < transactionCount)&&(transactionCount > minSupport))
		PrintEmptyTransaction();
#else
	if (transactionCount > minSupport)
		PrintEmptyTransaction();
#endif

	int patternCount = 0;
	RecurseMine(0, NULL, tails, tailCount, 0, minSupport, patternCount, false, false, false /* IsHUT */);
	for (int loop = 0; loop < tailCount; loop++)
	{
		tails->buffer[loop]->transactionBitMask = NULL;
		delete tails->buffer[loop];
	}
	itemsetBufferPool.Free(tails);

	int maxLevel = 0;
	int totalFI = 0;
	for (int loop = 0; loop < frequentItemsetsFound->length; loop++)
	{
		totalFI += (*frequentItemsetsFound)[loop];
		if ((*frequentItemsetsFound)[loop] > 0)
			maxLevel = loop;
	}

	// EO: comment out the stdout output for parallel code
	// Print the total number of itemsets found
// 	printf("%d\n", totalFI);

	// Print the number of FI found - one line per itemsets length
	for (int loop = 0; loop <= maxLevel; loop++)
	{
// 		printf("%d\n",(*frequentItemsetsFound)[loop]);
	}

	delete[] levelInfo;

	return patternCount;
}

bool CDFSMiner::RecurseMine(int level, CItemset *head, SFixedCItemsetArray *tail, int tailCount, int tailStartPos, int minSupport, int &patternCount, bool useDiffset, bool firstTimeDiffset, bool IsHUT)
{
	CItemset *newHead;
	SFixedCItemsetArray *newHeads = itemsetBufferPool.Alloc();
	V_BIT_ARRAY *bitArray = NULL;
	CSortedIntArray *sortedSupport = sortedArrayPool.Alloc();//(tailCount);

	levelInfo[level + 1].PEPItemCount = 0;
	levelInfo[level + 1].PEPItems = intBufferPool.Alloc();

		// Loop over the tail
	for (int loopTail = tailStartPos; loopTail < tailCount; loopTail++)
	{
		CItemset *i = tail->buffer[loopTail];
		int support = 0;

				// 1-itemset
		if (head == NULL)
		{
			bitArray = i->transactionBitMask->Clone();
			support = bitArray->CountElements();
		}
		else
		{
				// 2-itemsets matrix - check if 2-itemset is frequent
			if ((level == 1)&&(p2ItemsMatrix != NULL))
			{
				int sup;
				if (head->lastItem > i->lastItem)
					sup = p2ItemsMatrix[head->lastItem][i->lastItem];
				else
					sup = p2ItemsMatrix[i->lastItem][head->lastItem];

				if (minSupport > sup)
					continue;
			}

					// For 2-itemsets don't use diffsets
			if (useDiffset == false)
			{
				if ((float)transactionCount * 0.10 > head->support )
							// Simple bit vector AND
					bitArray = i->transactionBitMask->And(head->transactionBitMask);
				else
							// Projection
					bitArray = i->transactionBitMask->Project(head->transactionBitMask);

				support = bitArray->CountElements();
			}
						// Starting to use diffsets
			else if (firstTimeDiffset == true)
			{
				bitArray = head->transactionBitMask->AndNot(i->transactionBitMask);
				support = head->support - bitArray->CountElements();
			}
			else		// diffsets already in use
			{
				bitArray = i->transactionBitMask->AndNot(head->transactionBitMask);
				support = head->support - bitArray->CountElements();
			}

				// PEP
			if (support == head->support)
			{
				levelInfo[level].PEPItems->buffer[levelInfo[level].PEPItemCount] = i->lastItem;
				levelInfo[level].PEPItemCount++;

				sparseBitArrayPool.Free(bitArray);
				continue;				
			}
		}

			// New FI found, add to list
		if (support > minSupport)
		{
			newHead = itemsetPool.Alloc();
			newHead->support = support;
			newHead->transactionBitMask = bitArray;
			newHead->lastItem = i->lastItem;

			sortedSupport->Add(support, loopTail);
			newHeads->buffer[loopTail] = newHead;
		}
		else
			sparseBitArrayPool.Free(bitArray);
	}

		// Resort the tail - Dynamic reordering
	sortedSupport->Sort();
	SFixedCItemsetArray *newerTail = itemsetBufferPool.Alloc();
	for (int loop = 0; loop < sortedSupport->Count; loop++)
		newerTail->buffer[loop] = newHeads->buffer[(*sortedSupport)[loop]];//tail->buffer[sortedSupport[loop]];

		// Init diffset selection mode
	bool nextUseDiffset, nextFirstTimeDiffset;
	float baseSupport;
	if (head == NULL)
		baseSupport = (float)this->transactionCount;
	else
		baseSupport = (float)head->support;

	bool BREAK_OUT = false;
	for (int loopHeads = 0; loopHeads < sortedSupport->Count; loopHeads++)
	{
		newHead = newHeads->buffer[(*sortedSupport)[loopHeads]];
		levelInfo[level].item = newHead->lastItem;

			// Check if diffsets should be used
		nextUseDiffset = useDiffset;
		nextFirstTimeDiffset = firstTimeDiffset;

		if (firstTimeDiffset == true)
			nextFirstTimeDiffset = false;

		if (useDiffset == false)
		{
			if ((float)newHead->support / baseSupport > 0.5)
			{
				nextUseDiffset = true;
				nextFirstTimeDiffset = true;
			}
		}
				// Recurse only if there are items in the tail
		if (loopHeads < sortedSupport->Count - 1)
		{
			// This is the left most item & all children are freqeunt
#ifdef MINE_MFI
			bool MyIsHUT = (loopHeads == 0)&&(sortedSupport->Count == tailCount - tailStartPos);

			bool res = RecurseMine(level + 1, newHead, newerTail, sortedSupport->Count, loopHeads + 1, minSupport, patternCount, nextUseDiffset, nextFirstTimeDiffset, MyIsHUT);
#else
			bool res = RecurseMine(level + 1, newHead, newerTail, sortedSupport->Count, loopHeads + 1, minSupport, patternCount, nextUseDiffset, nextFirstTimeDiffset, true);
#endif

#ifdef MINE_MFI
			// FHUT
			// Breakout ...
			if ((res == true)&&(IsHUT == true))
			{
				levelInfo[level + 1].PEPItemCount = 0;				
				
				for (; loopHeads < sortedSupport->Count; loopHeads++)
				{
					newHead = newHeads->buffer[(*sortedSupport)[loopHeads]];
					itemsetPool.Free(newHead);
				}

				BREAK_OUT = true;
				break;
			}
#endif			
		}
		else
		{
			if (tailCount - tailStartPos == 1)
				BREAK_OUT = true;
		}

#ifdef MINE_FCI
		int FCI_ItemsetLength = IsFCI(level, newHead);
		if (FCI_ItemsetLength >= 0)
			(*frequentItemsetsFound)[FCI_ItemsetLength]++;
#else
#ifdef MINE_MFI

#else // Frequent itemset
				// Print frequent itemsets found
		if (fileWriter != NULL)
			CreateFI(level, newHead->support);

				// Count frequent itemsets
		int count = 1;
		int countPEP = 0;
		for (int loop = 1; loop < level + 2; loop++)
		{
			count *= 1<<levelInfo[loop].PEPItemCount;
			countPEP += levelInfo[loop].PEPItemCount;
		}
		patternCount += count;

		int baseLength = level + 1;
		int baseLengthInc = 1;
		(*frequentItemsetsFound)[baseLength]++;
		for (int loop = 0; loop < countPEP; loop++)
		{
			baseLengthInc = (baseLengthInc * (countPEP - loop)) / (loop + 1);
			(*frequentItemsetsFound)[baseLength + 1 + loop] += baseLengthInc;
		}
#endif
#endif

		levelInfo[level + 1].PEPItemCount = 0;

		itemsetPool.Free(newHead);
	}

	sortedArrayPool.Free(sortedSupport);
	itemsetBufferPool.Free(newerTail);
	itemsetBufferPool.Free(newHeads);

	intBufferPool.Free(levelInfo[level + 1].PEPItems);
	levelInfo[level + 1].PEPItems = NULL;

	return BREAK_OUT;
}

int CDFSMiner::CreateFI(int level, int support)
{
	return RecurseBuildFI(0, -1, level, support, 0);
}

// Fast int to string convertor (using lookup table
CIntToString its;

// Recursively build all frequent subsets of the PEPed items, and generate frequent itemsets.
int CDFSMiner::RecurseBuildFI(int currLevel, int currPEPItem, int maxLevel, int support, int bufPos)
{
	static char buf[10240];
	static CItem *p;

	int count = 0;

	if (currLevel > maxLevel)
	{
		int len;
		char *refBuff;
		
		refBuff = /*m_IntToString*/its.Convert(support, len);

		if (refBuff != NULL)
		{
			fileWriter->Write(buf, bufPos);
			fileWriter->WriteLine(refBuff, len);
		}
		else
		{
			// Print FI
			// Add "(support)"
			sprintf(&(buf[bufPos]),"(%d)%s", support, CRLF);
			bufPos += strlen(&(buf[bufPos]));

			fileWriter->WriteLine(buf, bufPos);
		}
		return 1;
	}

	if (currPEPItem == -1)
	{
		p = items[levelInfo[currLevel].item];
		memcpy(&(buf[bufPos]), p->s_name, 10);
		count += RecurseBuildFI(currLevel, 0, maxLevel, support, bufPos + p->s_nameLen);
	}
	else if (levelInfo[currLevel + 1].PEPItemCount > currPEPItem)
	{
			// With
		p = items[levelInfo[currLevel + 1].PEPItems->buffer[currPEPItem]];
		memcpy(&(buf[bufPos]), p->s_name, 10);
		count += RecurseBuildFI(currLevel, currPEPItem + 1, maxLevel, support, bufPos + p->s_nameLen);

			// Without
		count += RecurseBuildFI(currLevel, currPEPItem + 1, maxLevel, support, bufPos);
	}
	else
		count += RecurseBuildFI(currLevel + 1, -1, maxLevel, support, bufPos);

	return count;
}


//#include <hash_map>

typedef CExtendingArray<CCompressedIntArray *> SimpleBitArrayVector;
struct hash_cell
{
	SimpleBitArrayVector v;
	CExtendingArray<int> intArr;
};

//std::hash_map<int, hash_cell *> fciHash;
#define HASH_SIZE (40960)
hash_cell *fciHash[HASH_SIZE];
static bool firstTime = true;

int CDFSMiner::IsFCI(int level, CItemset *itemset)
{
	if (firstTime)
	{
		memset(fciHash, 0, HASH_SIZE * sizeof(hash_cell *));
		firstTime = false;
	}

	CExtendIntArray *newItem = smallSparseBitArrayPool.Alloc();

		// Create an array for the current FCI
	for (int loop = 0; loop <= level; loop++)
	{
		newItem->SetAppend(levelInfo[loop].item);

		for (int loopPEP = 0; loopPEP < levelInfo[loop + 1].PEPItemCount; loopPEP++)
			newItem->SetAppend(levelInfo[loop + 1].PEPItems->buffer[loopPEP]);
	}

	newItem->Sort();
	newItem->support = itemset->support;

	int hashVal = itemset->support + itemset->transactionBitMask->itemSum;
	hashVal = hashVal%HASH_SIZE;

		// Look-up the hash table for the hash-value
	//std::hash_map <int, hash_cell *> :: iterator pIter;
	//pIter = fciHash.find(hashVal);

	//if (pIter != fciHash.end())
	if (fciHash[hashVal] != NULL)
	{
			// Check the list of items with the same hash value, if
			// one contains the candidate
		hash_cell *v = fciHash[hashVal];//pIter->second;
		for (int loop = 0; loop < v->v.Count; loop++)
		{
			if (v->intArr.buffer[loop] == newItem->support)
			if (v->v.buffer[loop]->IsContaining(newItem) == true)
			{
				smallSparseBitArrayPool.Free(newItem);

				// Not an FCI
				return -1;
			}
		}

			// No FCI contains the candidate - new FCI
		v->v.Add(new CCompressedIntArray(newItem));
		v->intArr.Add(newItem->support);
	}
	else // hash not found - new FCI
	{
		hash_cell *v = new hash_cell;
		v->v.Add(new CCompressedIntArray(newItem));
		v->intArr.Add(newItem->support);
		fciHash[hashVal] = v;
	}

	if (fileWriter != NULL)
		CreateFCI(newItem);

	int FCILength = newItem->Count;
	smallSparseBitArrayPool.Free(newItem);

	return FCILength;
}

void CDFSMiner::CreateFCI(CExtendIntArray *itemset)
{
	static char buf[10240];
	static CItem *p;

	int bufPos = 0;

	for (int loop = 0; loop < itemset->Count; loop++)
	{
		p = items[(*itemset)[loop]];
		memcpy(&(buf[bufPos]), p->s_name, 10);
		bufPos += p->s_nameLen;
	}

	int len;
	char *refBuff;
	
	refBuff = its.Convert(itemset->support, len);

	if (refBuff != NULL)
	{
		fileWriter->Write(buf, bufPos);
		fileWriter->WriteLine(refBuff, len);
	}
	else
	{
		// Print FI
		// Add "(support)"
		sprintf(&(buf[bufPos]),"(%d)%s", itemset->support, CRLF);
		bufPos += strlen(&(buf[bufPos]));

		fileWriter->WriteLine(buf, bufPos);
	}
}

