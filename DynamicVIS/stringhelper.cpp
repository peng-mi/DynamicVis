#include "string.h"
#include "stringhelper.h"

unsigned int get_char_end(const char *str, unsigned int start, char _c )
{
	unsigned int idx = start;
	while (str[idx] != _c && str[idx] != 0)
		idx++;
	return idx;
}

unsigned int get_char_end_last(const char *str, unsigned int start, char _c )
{
	unsigned int len = strlen( str+start );
	unsigned int idx = start+len-1;
	while (str[idx] != _c && str[idx] != 0)
		idx--;
	return idx;
}

unsigned int get_end_string(const char *str, unsigned int start )
{
	unsigned int idx = start;
	while (str[idx] != ',' && str[idx] != 10 && str[idx] != 13 && str[idx] != 0 )
		idx++;
	return idx;
}

unsigned int get_string_end(const char *str,  unsigned int start, const char *found, unsigned int length)
{
	unsigned int idx = start;
	unsigned int idy=0;
	unsigned int idz=0;

	while(str[idx]!=0)
	{
		idy =idx;
		idz =0;
		while(str[idy]==found[idz]&&idz<length)
		{
			idy++;
			idz++;
		}
		if(idz==length)
			return idx;
		else
			idx++;
	}
	return idx;
}