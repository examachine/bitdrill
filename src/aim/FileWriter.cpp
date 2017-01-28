#include "FileWriter.h"
#include <memory.h>

#define WRITE_BUFFER_SIZE (32*1024)

CFileWriter::CFileWriter(const char *fileName)
{
	writeBuffer = new char[WRITE_BUFFER_SIZE + 1024 * 5];
	writeBufferSize = 0;

		// "w" - Write
		// "b" - Binary
		// "n" - No-Commit
	out = ::fopen(fileName, "wbn");
	setvbuf( out, NULL, _IONBF, 0 );
}

CFileWriter::~CFileWriter(void)
{
	if (writeBufferSize > 0)
	{
        ::fwrite(writeBuffer, sizeof(char), writeBufferSize, out);
		writeBufferSize = 0;
	}

	::fclose(out);

	delete[] writeBuffer;
}

void CFileWriter::Write(char *buf, size_t bufLen)
{
	::memcpy(&(writeBuffer[writeBufferSize]), buf, bufLen);
	writeBufferSize += bufLen;
}

void CFileWriter::WriteLine(char *buf, size_t bufLen)
{
	::memcpy(&(writeBuffer[writeBufferSize]), buf, bufLen);
	writeBufferSize += bufLen;

	if (writeBufferSize > WRITE_BUFFER_SIZE)
	{
        ::fwrite(writeBuffer, sizeof(char), writeBufferSize, out);
		writeBufferSize = 0;
	}
}
