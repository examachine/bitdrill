#ifndef __FILEWRITER_H
#define __FILEWRITER_H

#include <stdio.h>

class CFileWriter
{
public:
	CFileWriter(const char *fileName);
	~CFileWriter();

	void Write(char *buf, size_t bufLen);
	void WriteLine(char *buf, size_t bufLen);
private:
	FILE *out;

	char *writeBuffer;
	size_t writeBufferSize;
};

#endif
