#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


//#include <cutil_inline.h>    // includes cuda.h and cuda_runtime_api.h
//#include <cutil_gl_inline.h> // includes cuda_gl_interop.h// includes cuda_gl_interop.h
#include <cuda_gl_interop.h>
//#include <rendercheck_gl.h>
//
//#include <cuda_gl_interop.h>

#include <stdlib.h>
#include <stdio.h>
#include <cuda.h>
#include <cuda_runtime_api.h>
#include <time.h>

#include "filebuf.h"
#include "datamanager.h"
#include "PerformanceTimer.h"
#include "Data.h"
#include "FilteredData.h"
#include "RawData.h"
#include "colorhelper.h"
#include "Filter.h"
#include "stringhelper.h"
#include "color.h"
#include "LogFile.h"


#include<iostream>
#include<fstream>
#include<string> 

namespace VIS
{
	int gpuDeviceInit(int devID)
	{
		int deviceCount;
		cudaGetDeviceCount(&deviceCount);

		if (deviceCount == 0)
		{
			wxLogError( "gpuDeviceInit() CUDA error: no devices supporting CUDA.\n" );
			exit(-1);
		}

		if (devID < 0)
			devID = 0;

		if (devID > deviceCount-1)
		{
			wxLogError( "\n");
			wxLogError( ">> %d CUDA capable GPU device(s) detected. <<\n", deviceCount);
			wxLogError( ">> gpuDeviceInit (-device=%d) is not a valid GPU device. <<\n", devID);
			wxLogError( "\n");
			return -devID;
		}

		cudaDeviceProp deviceProp;
		cudaGetDeviceProperties(&deviceProp, devID);

		if (deviceProp.major < 1)
		{
			wxLogError( "gpuDeviceInit(): GPU device does not support CUDA.\n");
			return -1;
		}

		cudaSetDevice(devID);

		return devID;
	}

	CDataManager::CDataManager() :	m_ifDataReady(false), m_MinTime(0), m_MaxTime(0), /*m_CurTime(0), m_TimeRange(0),*/ \
		m_CurTimeData(NULL),m_TimeRangeData(NULL), m_TimeWindowData(NULL), m_TimeStepData(NULL), \
		/*m_StartIdx(0), m_EndIdx(0),*/ /*m_TimeStep(TIMESTEP),*/ /*m_RangeStep(RANGESTEP), */\
		m_RangeRatio(RANGERATIO), m_BrushType(Brush_One), m_filterType(NONE), m_CurrentTimesliderIdx(0), \
		m_ifSkip(false),m_dataUpdateStyle(UPDATE_STYLE), m_RawData(NULL), \
		m_CurrentFilter(-1),m_CurrentFilterEX(-1),m_CurrentFilterNEX(-1), \
		m_BasedFilteredData(NULL), m_ExclusiveFilter(NULL), m_NegExclusiveFilter(NULL)
	{
		gpuDeviceInit(0);
		cudaGLSetGLDevice(0);

		m_ExclusiveFilter = new CFilter();
		m_NegExclusiveFilter = new CFilter();
	
		m_RawData = new CRawData();
		m_RawData->SetDataManager(this);

		m_BasedFilteredData = new CFilteredData();
		
		m_CurTimeData = new CCurTime();
		m_TimeRangeData = new CTimeRange();
		m_TimeWindowData = new CTimeWindow();
		m_TimeStepData = new CTimeStep();
		AddRelation( m_CurTimeData, m_TimeWindowData, false);
		AddRelation( m_TimeRangeData, m_TimeWindowData, false);
		m_TimeStepData->SetTimeStep(TIMESTEP);

		//Registeration
		AddRelation( m_RawData, m_BasedFilteredData, true );
		AddRelation( m_ExclusiveFilter, m_BasedFilteredData, false );
		AddRelation( m_NegExclusiveFilter, m_BasedFilteredData, false );
		AddRelation( m_TimeWindowData, m_BasedFilteredData, false);
	}

	CDataManager::~CDataManager()
	{
		CAbstractData::DeleteObject( m_RawData, true );
		for ( unsigned int i = 0; i < m_Filters.size(); i++ )
				delete m_Filters[i];

		delete m_ExclusiveFilter;
		delete m_NegExclusiveFilter;
		delete m_CurTimeData;
		delete m_TimeRangeData;
		delete m_TimeWindowData;
		delete m_TimeStepData;
		CLogFile::GetLogFile()->Write();
	}

	void CDataManager::LoadData(char *_filename )
	{
		if ( m_RawData != NULL ) // Reload a dataset
		{
			CAbstractData::CleanAllObjects();
			
			for ( unsigned int i = 0; i < m_Filters.size(); i++ )
				CAbstractData::DeleteObject( m_Filters[i], true );
			m_Filters.resize(0);

			m_colorTable.resize(0);
			m_RawData->SetFilename( _filename );
			m_RawData->UpdateSelf();
		}
		else
		{
			//something is wrong
		}

		//get the unique time stamps
		set<time_t> ut_set;
		m_TimeWindowData->m_TimeStamps.resize(m_RawData->m_NumberOfRecords);
 		for( uint i =0; i < m_RawData->m_NumberOfRecords; i++)
		{
			m_TimeWindowData->m_TimeStamps[i] = *((time_t *)(m_RawData->m_DataBuffer + i*m_RawData->m_TotalBytes));
			ut_set.insert(m_TimeWindowData->m_TimeStamps[i]);
		}
		for(set<time_t>::iterator it = ut_set.begin(); it != ut_set.end(); ++it)
			m_TimeWindowData->m_UniqueTimeStamps.push_back(*it);

		m_MinTime = *((time_t *) m_RawData->m_DataBuffer);
		m_MaxTime = *((time_t *) (m_RawData->m_DataBuffer + (m_RawData->m_NumberOfRecords-1)*m_RawData->m_TotalBytes));

		m_CurTimeData->SetCurTime(m_MinTime);
		//m_TimeRange = m_RangeStep;

		m_TimeWindowData->m_StartIdx = 0;
		m_TimeWindowData->m_EndIdx = m_RawData->m_NumberOfRecords - 1;
	}
/*
	void CDataManager::SelectUniqueTimeSpanIndex()
	{
		if ( !m_ifBuildAllHistoryData )
		{
			t_TimeStamps::iterator low, up;
			
			low = std::lower_bound (m_UniqueTimeStamps.begin(), m_UniqueTimeStamps.end(), m_CurTime);
			up = std::upper_bound (low, m_UniqueTimeStamps.end(), m_CurTime + m_TimeRange);
			m_UniqueStartIdx = low - m_UniqueTimeStamps.begin();
			m_UniqueEndIdx = up - m_UniqueTimeStamps.begin();
			m_UniqueEndIdx--;
		}
		else
		{
			m_StartIdx = 0;
			m_EndIdx = m_UniqueTimeStamps.size() - 1;
		}
	}

	void CDataManager::SelectTimeSpanIndex()
	{
		t_TimeStamps::iterator low, up;
		
		low = std::lower_bound (m_TimeStamps.begin(), m_TimeStamps.end(), m_CurTimeData->GetCurTime());
		up = std::upper_bound (low, m_TimeStamps.end(), m_CurTimeData->GetCurTime() + m_TimeRangeData->GetTimeRange());
		m_StartIdx = low - m_TimeStamps.begin();
		m_EndIdx = up - m_TimeStamps.begin();
		m_EndIdx--;
	}
*/
	bool CDataManager::SatisfyFilter( uint _recordIdx, t_FilterSet &_filterSet )
	{
		for ( uint i = 0; i < _filterSet.size(); i++ )
		{
			t_AndFilterSet &curFilter = _filterSet[i];
			bool satified = true;
			for ( t_AndFilterSet::iterator it = curFilter.begin(); it != curFilter.end(); it++ )
			{
				uint itemidx = it->first;
				
				if ( m_RawData->m_item_desc[itemidx].num_values != 0 )
				{
					uint value = m_RawData->GetDataValue( _recordIdx, itemidx );
					if ( value != it->second )
					{
						satified = false;
						break;
					}
				}
				else
				{
					float value = m_RawData->GetNumDataValue( _recordIdx, itemidx );
					if ( value != m_RawData->GetNumDataValue( it->second, itemidx ))
					{
						satified = false;
						break;
					}
				}
			}
			if ( satified )
				return true;
		}
		return false;
	}

	CFilter* CDataManager::ProcessBrush( unsigned int _itemidx, unsigned int _valueidx )
	{
		CFilter *curFilter = NULL;
		CFilteredData *filteredData = NULL;

		if ( m_BrushType >= Brush_One_EX && m_BrushType <= Brush_OR_EX )
			curFilter = m_ExclusiveFilter;
		else if (m_BrushType >= Brush_Neg_One_EX && m_BrushType <= Brush_Neg_OR_EX )
			curFilter = m_NegExclusiveFilter;
		else // select filters
		{
			vector<pair<t_color,CFilter*>>::iterator it_color = m_colorTable.begin();
			for(; it_color != m_colorTable.end(); it_color++)
			{
				if(IsSameColor(it_color->first,m_color))
					break;
			}

			t_color color;
			if ( ColorConflict( _itemidx, _valueidx, color) )
			{
				if(!IsSameColor(color,m_color))
					return NULL;
			}

			if(it_color == m_colorTable.end()) // new color
			{
				curFilter = new CFilter();
				filteredData = new CFilteredData();
				AddRelation(curFilter, filteredData, true);
				AddRelation(m_BasedFilteredData, filteredData, false);
				m_FilteredDataList.push_back( filteredData);
				m_Filters.push_back(curFilter);

				m_colorTable.push_back(pair<t_color,CFilter*>(m_color,curFilter));
			}
			else
				curFilter = it_color->second;
		}
		
		curFilter->SetIndex(_itemidx, _valueidx);
		curFilter->SetLogicType((t_LogicType)((unsigned int)m_BrushType % 3));

		return curFilter;
	}

	t_color CDataManager::GetColor(CFilter *_filter)
	{
		unsigned int j = 0;
		for(; j < m_colorTable.size(); j++)
		{
			if(m_colorTable[j].second == _filter)
				break;
		}
		if(j != m_colorTable.size())
		{
			t_color &color = m_colorTable[j].first;
			return color;
		}
		else
		{
			t_color color;
			color.red = GRAY[0] * 255;
			color.green = GRAY[1] * 255;
			color.blue = GRAY[2] * 255;
			color.alpha = GRAY[3] * 255;
			return color;
		}
	}

	bool CDataManager::ColorConflict(unsigned int _itemidx, unsigned int _valueidx, t_color &_color)
	{
		for ( unsigned int i = 0; i < m_colorTable.size(); i++ )
		{
			CFilter* filter = m_colorTable[i].second;
			for(unsigned int j = 0; j < filter->m_Filter.size(); j++)
			{
				t_AndFilterSet &_andfilter = filter->m_Filter[j];
				for ( t_AndFilterSet::iterator it = _andfilter.begin(); it != _andfilter.end(); it++ )
				{
					if((it->first == _itemidx) && (it->second == _valueidx))
					{
						_color.red = m_colorTable[i].first.red;
						_color.green = m_colorTable[i].first.green;
						_color.blue = m_colorTable[i].first.blue;
						_color.alpha = m_colorTable[i].first.alpha;
						return true;
					}
				}
			}
		}
		return false;
	}

	void CDataManager::CleanDataLists() // Remove bad pointers
	{
		for(vector<CFilteredData*>::iterator it = m_FilteredDataList.begin(); it != m_FilteredDataList.end();)
		{
			if(CAbstractData::IsDeletedObject(*it))
				it = m_FilteredDataList.erase(it);
			else
				it++;
		}	
		for(vector<CFilter*>::iterator it = m_Filters.begin(); it != m_Filters.end();)
		{
			if(CAbstractData::IsDeletedObject(*it))
				it = m_Filters.erase(it);
			else
				it++;
		}

		for( vector<CAbstractData*>::iterator it = m_HighlightIDataList.begin() ; it != m_HighlightIDataList.end();)
		{
			if(CAbstractData::IsDeletedObject(*it))
				it = m_HighlightIDataList.erase(it);
			else
				it++;
		}
	}

	bool CDataManager::LoadQuery(char *filename,bool kind_load,bool kind_select)//kind is for append or load
	{
/*
		FILE *inputfile;
		char buf[1024];
		unsigned int lines=0;

		inputfile =fopen(filename,"r");
		while(fgets (buf,1024,inputfile) != NULL )
			lines++;

		fseek( inputfile , 0 , SEEK_SET);

		if(kind_select)
		{
			if(kind_load)
			{
				m_selectsets.clear();
				m_CurrentFilter = -1;
			}
			m_BrushType = Brush_OR;
		}
		else
		{
			if(kind_load)
			{
				m_excludedset.clear();
				m_CurrentFilterEX = -1;
			}
			m_BrushType = Brush_OR_EX;
		}

		for(int i=0;i<lines;i++)
		{
			fgets (buf,1024 ,inputfile);
			LoadHistoryFilter(buf,kind_select);
		}

		//UpdateAllDataForFilter();
		fclose(inputfile);
		*/
		return true;
	}


	void CDataManager::LoadHistoryFilter(char* buf, bool kind)
	{
		/*
		int _sel=-1;
		int _value=-1;
		unsigned int start=0;
		unsigned int end =0;
		unsigned int length=0;
		unsigned int and_num =1;

		length = strlen(buf);
		end  = get_string_end(buf,start," and ",5);
		while(end!=length)	
		{
			and_num++;
			start = end+1;
			end  = get_string_end(buf,start," and ",5);
		}
		buf[end-1]=0;
		start =0;
		if(kind)
			m_BrushType = Brush_OR;
		else
			m_BrushType = Brush_OR_EX;
		for(int j=0;j<and_num;j++)
		{
			_sel=-1;
			_value=-1;

			if(j>0)
			{
				if(kind)
					m_BrushType = Brush_And;
				else
					m_BrushType = Brush_And_EX;
			}
			end  = get_string_end(buf,start," and ",5);
			buf[end]=0;
			length =get_char_end(buf,start,'=');
			buf[length]=0;

			for (int k=0;k<m_item_desc.size();k++)
			{
				if(strcmp(buf+start,m_item_desc[k].name.c_str())==0)
				{
					_sel =k;
					break;
				}
			}
			if(_sel>=0)
			{
				for (int k=0;k<m_item_desc[_sel].num_values;k++)
				{
					if(strcmp( buf+length+1,m_item_desc[_sel].value_names[k].c_str())==0)
					{
						_value = k;
						break;
					}
				}
				if(_value>=0)
					ProcessBrush( _sel, _value);
			}
			start  = end +5;
		}
		*/
	}

	bool CDataManager::IsSameColor(t_color& _color1, t_color& _color2)
	{
		if(_color1.red == _color2.red && _color1.green == _color2.green && _color1.blue == _color2.blue && _color1.alpha == _color2.alpha)
			return true;
		return false;
	}


}