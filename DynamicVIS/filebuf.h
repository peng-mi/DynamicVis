#ifndef __FILEBUF_H_
#define __FILEBUF_H_

#include <stdio.h>

class CFileBuf
{
private:
	FILE *fp;
	char *buf;
	size_t BufSize;
	char *pos, *end;
	virtual int ReadFile();
public:
	CFileBuf( size_t _BufSize = 1024*1024 );
	virtual ~CFileBuf();
	virtual FILE* open(char *filename, char *mode);
	virtual int close();
	virtual int ReadNextLine(char *dest);
};

#endif