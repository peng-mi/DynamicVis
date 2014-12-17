#include "filebuf.h"

CFileBuf::~CFileBuf()
{
	close();
	delete[] buf;
}

CFileBuf::CFileBuf( size_t _BufSize )
{
	BufSize = _BufSize;
	buf = new char[BufSize];
	fp = NULL;
	end = NULL;
}

FILE* CFileBuf::open( char *filename, char *mode )
{
	close();
	if ( (fp = fopen( filename, mode )) == NULL )
		return fp;
	ReadFile();
	return fp;
}

int CFileBuf::ReadFile()
{
	pos = buf;
	size_t i = fread(buf, sizeof(char), BufSize, fp);
	if (i<BufSize)
		end = &(buf[i-1]);
	else 
		end = NULL;
	return (int)i;
}

int CFileBuf::close()
{
	int success;

	if ( fp != NULL )
	{
		success = fclose( fp );
		fp = NULL;
		return success;
	}
	else
		return 0;
}

int CFileBuf::ReadNextLine( char *dest )
{
	int i=0;

	while ( pos != end )
	{
		dest[i] = *pos;
		pos++;
		if (pos == &buf[BufSize])
			ReadFile();
		if (dest[i] == '\n')
		{
			i++;
			break;
		}
		i++;
	}
	dest[i]=0;
	return i;
}
