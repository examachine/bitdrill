#ifndef __INTTOSTRING_H
#define __INTTOSTRING_H

class CIntToString
{
public:
	CIntToString() 
	{ 
		for (int loop = 0; loop < LOOKUP_SIZE; loop++)
		{
			sprintf(lookupTable[loop],"(%d)%s", loop, CRLF);
			lookupLengths[loop] = strlen(lookupTable[loop]);
		}
	}
	char *Convert(int num, int& length)
	{
		if (num < LOOKUP_SIZE)
		{
			length = lookupLengths[num];
			return lookupTable[num];
		}

		return NULL;
	}
private:
	const static int LOOKUP_SIZE = (1024 * 64);
	char lookupTable[LOOKUP_SIZE][10];
	int lookupLengths[LOOKUP_SIZE];
};

#endif
