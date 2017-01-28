#ifndef __DFSMINER // -*-c++-*-
#define __DFSMINER

#include "MinerBase.h"
#include "Itemset.h"
#include "FileWriter.h"

	// General purpose fixed size int array
struct SFixedIntArray
{
public:
	static int arraySize;
	SFixedIntArray()
	{
		length = arraySize;
		buffer = new int[arraySize];
		::memset(buffer, 0, arraySize * sizeof(int));
	}

	SFixedIntArray(int length)
	{
		this->length = length;
		buffer = new int[length];
		::memset(buffer, 0, length * sizeof(int));
	}

	~SFixedIntArray()
	{
		delete[] buffer;
	}

	void Clear() {}
	int &operator[](int i) { return buffer[i]; }

	int length;
	int *buffer;
};

typedef CItemset* pCItemset;

	// Array of itemsets - used to store the tail
struct SFixedCItemsetArray
{
public:
	static int arraySize;
	SFixedCItemsetArray()
	{
		buffer = new pCItemset[arraySize];
	}

	~SFixedCItemsetArray()
	{
		delete[] buffer;
	}

	void Clear() {};

	pCItemset *buffer;
};

	// Structure that holds the DFS information for a cetain level in the DFS traversal
struct SDFSLevelInfo
{
	SDFSLevelInfo(): item(-1), PEPItems(NULL), PEPItemCount(0) {}
	~SDFSLevelInfo() { if (PEPItems != NULL) delete PEPItems; }

	int item;

	SFixedIntArray *PEPItems;
	int PEPItemCount;
};

class CDFSMiner :
	public CMinerBase
{
public:
	CDFSMiner();
 	~CDFSMiner();

	int FindFrequentPatterns(int minSupport);
	void SetOutputFile(const char *fileName);

protected:
	CFileWriter *fileWriter;
	SFixedIntArray *frequentItemsetsFound;

	bool RecurseMine(int level, CItemset *head, SFixedCItemsetArray *tail, int tailCount, int tailStartPos, int minSupport, int &patternCount, bool useDiffset, bool firstTimeDiffset, bool IsHUT);
	
	void SetStaticParameters();

	void PrintEmptyTransaction();

	int CreateFI(int level, int support);
	int RecurseBuildFI(int currLevel, int currPEPItem, int maxLevel, int support, int bufPos);

	void CreateFCI(CExtendIntArray *itemset);
	int IsFCI(int level, CItemset *itemset);

	SDFSLevelInfo *levelInfo;
};

#endif
