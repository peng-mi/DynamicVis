#pragma once

#include "Data.h"

#include <vector>
#include <string>
using namespace std;

namespace VIS
{
//class CAbstractData;

typedef vector<unsigned int> t_uintvector;

typedef struct{
	string name;
	unsigned int num_bytes;
	unsigned int offset;
	unsigned int num_values;
	vector<string> value_names;
	t_uintvector value_counts;
	float min;
	float max;
	float logcoeff;
} t_Item;
typedef vector<t_Item> t_Item_Desc;

class CRawData : public CAbstractData
{
private:
	char *m_Filename;
	void Load_Item_Desc_File(char *_filename);
public:
	CRawData();
	CRawData(char *_filename);
	virtual ~CRawData(void){ CleanSelf();}

	void UpdateSelf();
	void CleanSelf();

	unsigned int GetDataValue( unsigned int _recordIdx, unsigned int _itemIdx );
	float GetNumDataValue( unsigned int _recordIdx, unsigned int _itemIdx );
	time_t GetRecordTime( unsigned int _recordIdx );

	void SetFilename( char *_filename );

	t_Item_Desc m_item_desc;
	//data description
	unsigned int m_TotalBytes;
	unsigned char *m_DataBuffer;
	unsigned int m_NumberOfRecords;
};

}