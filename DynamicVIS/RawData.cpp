#include <wx/log.h>

#include "RawData.h"
#include <string.h>
#include "filebuf.h"
#include "stringhelper.h"
#include "Data.h"
#include "header.h"
#include "math.h"
#include <stdio.h>
#include <time.h>

namespace VIS
{
CRawData::CRawData() :  m_DataBuffer(NULL), m_TotalBytes(0), m_NumberOfRecords(0), m_Filename(NULL)
{
}

CRawData::CRawData(char *_filename) : m_DataBuffer(NULL), m_TotalBytes(0), m_NumberOfRecords(0), m_Filename(NULL)
{
	m_Filename = _strdup(_filename);
}

void CRawData::CleanSelf()
{
	if(m_Filename != NULL)
	{
		free(m_Filename);
		m_Filename = NULL;
		m_item_desc.clear();
		if ( m_DataBuffer != NULL )
		{
			free(m_DataBuffer);
			m_DataBuffer = NULL;
		}
	}
}

void CRawData::SetFilename( char *_filename )
{
	if ( m_Filename != NULL )
		free(m_Filename);
	m_Filename = _strdup(_filename);
}

void CRawData::UpdateSelf()
{
	char filename [256];
	strncpy(filename,m_Filename,(int)strlen(m_Filename));
	filename[(int)strlen(m_Filename)] ='\0';

	//clear the previous data
	m_item_desc.clear();
	if ( m_DataBuffer != NULL )
	{
		free(m_DataBuffer);
		m_DataBuffer = NULL;
	}

	char item_desc_filename[256], tmp[256];
	int len = (int)strlen(filename);
	strncpy(tmp,filename,len-5);
	tmp[len-5] = 0;
	sprintf(item_desc_filename,"%s.stats",tmp);

	Load_Item_Desc_File(item_desc_filename);

	char tmp_name[256];
	sprintf(tmp_name,"%s.data",tmp);

	FILE *infile;
	uint fsize  = 0;
	infile = fopen(tmp_name,"rb");
	if ( infile != NULL )
	{
		fseek(infile,0, SEEK_END);
		fsize = ftell(infile);
		fseek(infile,0, SEEK_SET);

		if( m_NumberOfRecords != fsize / m_TotalBytes)
		{
			CleanSelf();
			return;
		}

		m_DataBuffer = (uchar*) malloc(fsize);
		fread( m_DataBuffer,1,fsize,infile );

		fclose(infile);
	}
	else
		wxLogMessage("Can not find data file %s.\n", tmp_name);
}

void CRawData::Load_Item_Desc_File(char *_filename)
{
	CFileBuf infile;
	unsigned int start = 0, end;
	char buffer[4096], tag[1024];

	infile.open(_filename,"r");
	infile.ReadNextLine(buffer);
	end = get_char_end(buffer, 0, ':');
	m_NumberOfRecords = atoi(buffer+end+1);
	

	infile.ReadNextLine(buffer);
	end = get_char_end(buffer, 0, ':');
	unsigned int num_items = atoi( buffer+end+1 );
	m_item_desc.resize( num_items );

	//item names
	for(unsigned int i = 0; i < num_items; i++)
	{
		infile.ReadNextLine(buffer);
		start = 0;
		end = get_end_string( buffer, start);
		strncpy( tag, buffer+start, end-start );
		tag[end-start] = 0;
		m_item_desc[i].name = tag;
	}

	//for each items
	while(infile.ReadNextLine(buffer) >0)
	{
		start = 0;
		end = get_char_end( buffer, start, ':');
		strncpy( tag, buffer+start, end-start );
		tag[end-start] = 0;
		unsigned int item_idx = atoi( tag+5 );

		start = end+1;
		end = get_end_string(buffer, start);
		strncpy(tag,buffer+start,end-start);
		tag[end-start] = 0;
		unsigned int _num_values = atoi(tag);
		m_item_desc[item_idx].num_values = _num_values;
		if( _num_values == 0 )
			m_item_desc[item_idx].num_bytes = 4;//numerical values
		else if( _num_values < 256 )
			m_item_desc[item_idx].num_bytes = 1;
		else if( _num_values < 65535 )
			m_item_desc[item_idx].num_bytes = 2;
		else
			m_item_desc[item_idx].num_bytes = 4;

		m_item_desc[item_idx].value_names.resize(_num_values);
		m_item_desc[item_idx].value_counts.resize(_num_values);

		if( _num_values !=0 )
		{
			for ( unsigned int i = 0; i < _num_values; i++ )
			{
				infile.ReadNextLine(buffer);
				end = get_char_end_last(buffer,0,':');
				strncpy(tag, buffer,end);
				tag[end] = 0;
				m_item_desc[item_idx].value_names[i] = tag;

				start  = end + 1;
				end = get_end_string(buffer,start);
				strncpy(tag,buffer+start,end-start);
				tag[end-start] = 0;
				m_item_desc[item_idx].value_counts[i] = atoi(tag);
			}
		}
		else
		{
			infile.ReadNextLine(buffer);
			end = get_char_end_last(buffer,0,':');
			strncpy(tag,buffer,end);
			tag[end] = 0;
			m_item_desc[item_idx].min = atof(tag);

			start = end + 1;
			end = get_end_string( buffer, start );
			strncpy( tag, buffer + start, end - start);
			tag[end - start] = 0;
			m_item_desc[item_idx].max = atof(tag);
			m_item_desc[item_idx].logcoeff = 1.0f/log10( m_item_desc[item_idx].max - m_item_desc[item_idx].min + 1.0f );
		}
		if(item_idx == m_item_desc.size()-1 )
			break;
	}

	unsigned int _offset = sizeof(time_t);
	for( t_Item_Desc::iterator it = m_item_desc.begin()+1; it != m_item_desc.end(); it++)
	{
		(*it).offset = _offset;
		_offset += (*it).num_bytes;
	}

	for (t_Item_Desc::iterator it = m_item_desc.begin()+1; it != m_item_desc.end();)
	{
		if((*it).num_values == 1)
			it = m_item_desc.erase(it);
		else
			it++;
	}

	//the total size of the data
	m_TotalBytes = sizeof(time_t);
	for( uint i = 1; i < m_item_desc.size(); i++)
		m_TotalBytes += m_item_desc[i].num_bytes;

	infile.close();
}

unsigned int CRawData::GetDataValue( unsigned int _recordIdx, unsigned int _itemIdx ) 
{
	unsigned int byteoffset = m_item_desc[_itemIdx].offset;

	if ( m_item_desc[_itemIdx].num_values != 0 )
	{
		uint numbytes = m_item_desc[_itemIdx].num_bytes;
		if ( numbytes == 1 )
		{
			uchar item = *((uchar *) (m_DataBuffer+_recordIdx*m_TotalBytes+byteoffset));
			return (unsigned int)item;
		}
		else if ( numbytes == 2 )
		{
			ushort item = *((ushort *) (m_DataBuffer+_recordIdx*m_TotalBytes+byteoffset));
			return (unsigned int)item;
		}
		else
		{
			uint item = *((unsigned int *) (m_DataBuffer+_recordIdx*m_TotalBytes+byteoffset));
			return item;
		}
	}
	else
	{
		return 0;
	}
}

float CRawData::GetNumDataValue( unsigned int _recordIdx, unsigned int _itemIdx )
{
	if ( m_item_desc[_itemIdx].num_values != 0 )
		return 0.0f;
	else
	{
		float value = *((float *) (m_DataBuffer+_recordIdx*m_TotalBytes+m_item_desc[_itemIdx].offset));
		//if ( _iflogscaled )
		//	value = log10( value - m_item_desc[_itemIdx].min + 1.0f ) * m_item_desc[_itemIdx].logcoeff;
		return value;
	}
}

time_t CRawData::GetRecordTime( unsigned int _recordIdx )
{
	time_t _value = *((time_t *) (m_DataBuffer+_recordIdx*m_TotalBytes));
	return _value;
}

}