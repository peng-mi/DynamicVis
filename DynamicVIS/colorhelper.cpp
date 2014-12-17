#include "colorhelper.h"

uchar GetRed(uint _value)
{
	uint _tmp = _value >> 24;
	_tmp &= 0x000000ff;
	return (uchar)_tmp;
}

uchar GetGreen(uint _value)
{
	uint _tmp = _value >> 16;
	_tmp &= 0x000000ff;
	return (uchar)_tmp;
}

uchar GetBlue(uint _value)
{
	uint _tmp = _value >> 8;
	_tmp &= 0x000000ff;
	return (uchar)_tmp;
}

uchar GetAlpha(uint _value)
{
	uint _tmp = _value;
	_tmp &= 0x000000ff;
	return (uchar)_tmp;
}

