
#include "StdAfx.h"
#include "SparseBitArray.h"
#include "DFSMiner.h"
#include <stdio.h>
#ifdef WIN32
#include <windows.h>
#endif

int main(int argc, char ** argv)
{
	if ((argc != 3)&&(argc != 4))
	{
		printf("Usage: %s <InputFile> <MinSupport> [OutputFile]\n", argv[0]);
		return 0;
	}

#ifdef WIN32_DEBUG
	DWORD a = ::GetTickCount();
#endif

	CDFSMiner miner;
	if (argc == 4)
		miner.SetOutputFile(argv[3]);

	miner.Init(argv[1], ::atoi(argv[2]) - 1, false);

	miner.FindFrequentPatterns(::atoi(argv[2]) - 1);

#ifdef WIN32_DEBUG
	printf("%d\n",::GetTickCount() - a);
#endif

	return 0;
}

