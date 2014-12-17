#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include <GL/glew.h>
#include "ParaCoordData.h"
#include "FilteredData.h"
#include "Filter.h"
#include "RawData.h"

#include "datamanager.h"
#include "Frame.h"

#include "color.h"
#include "PerformanceTimer.h"
#include "colorhelper.h"
#include "LogFile.h"

#include <windows.h>
#include <iostream>
#include <fstream>
#include <conio.h>
#include <stdio.h>

#include "guicon.h"
#include <crtdbg.h>

#include <windows.h>
#include <iostream>
#include <sstream>

 #define DBOUT( s )            \
{                             \
   std::wostringstream os_;    \
   os_ << s;                   \
   OutputDebugStringW( os_.str().c_str() );  \
}

namespace VIS
{
	extern "C" void cuda_Init();
	extern "C" void cuda_CleanConstant();
	extern "C" void cuda_TransferConstant(unsigned int* _item_desc_offset,	unsigned int* _item_desc_bytes,
										  unsigned int* _item_desc_values,	unsigned char* _buffer_data,
										  unsigned int _itemNum,			unsigned int _bufferSize);
	extern "C" void cuda_CleanDynamic();
	extern "C" void cuda_TransferDynamic(unsigned int* _filteredData,	unsigned int _filterDataSize,
										 unsigned int* _selected_items,	unsigned int _selectedItemsNum,
										 unsigned int _max_dimension,	unsigned int _total_bytes);

	extern "C" void cuda_GetCompactListSize(unsigned int& _compactNum);
	extern "C" void cuda_GetCompactList(unsigned int* compactList);

	extern "C" void cuda_GetVertexIndexSize(unsigned int& _vertexBufferSize, unsigned int& _indexBufferSize);
	extern "C" void cuda_GenerateVertexBuffers(float* _vertexBuffer_ptr, unsigned int* indexBuffer_ptr,
												unsigned int _vertexBufferSize, unsigned int _indexBufferSize,
												unsigned int _depth);



	bool CParaCoordIData::m_loadData = true;
	/******************************************************************************************************
	********************CParaCoordIData********************************************************************
	******************************************************************************************************/
	CParaCoordIData::CParaCoordIData():m_compactNum(0), m_compactRecords(NULL),m_if_GPU(true)
	{
	}

	CParaCoordIData::~CParaCoordIData()
	{
		CleanSelf();
		if(m_if_GPU)
		{
			cuda_CleanDynamic();
			cuda_CleanConstant();
		}
	}
	
	void CParaCoordIData::SetGPU(bool _gpu)
	{
		CleanSelf();
		if(_gpu == true)
			GPU_TransferConstantData();
		m_if_GPU = _gpu;
	}

	void CParaCoordIData::CleanSelf()
	{
		if(!m_if_GPU)
			m_records.clear();
		else
			cuda_CleanDynamic();
	}

	void CParaCoordIData::IncrementalSelf()
	{
		UpdateSelf();
	}

	void CParaCoordIData::PrepareData()
	{
		assert(m_parent.size() == 1);
		PerformanceTimer timer;
		timer.StartTimer();
		CFilteredData* filterdata  = dynamic_cast<CFilteredData*>(m_parent[0]);
		CDataManager* dm = GetDataManager();

		t_uintvector &si = m_selectedItems;
		if ( si.size() < 2 )
		{
			CleanSelf();
			return;
		}

		pair<t_recordset::iterator,bool> ret;
		set<unsigned int>::iterator it;
		for(it = filterdata->m_filteredData.begin(); it != filterdata->m_filteredData.end(); it++ )
		{
			t_uintvector vec;
			for(uint j = 0; j < si.size(); j++)
			{
				uint idx = si[j];
				if ( dm->m_RawData->m_item_desc[idx].num_values == 0 ) // Numerical
					vec.push_back(*it);
				else
				{
					uint _value = dm->m_RawData->GetDataValue(*it,idx);
					vec.push_back(_value);
				}
			}
			ret = m_records.insert( pair<t_uintvector, unsigned int>(vec, 1));
			if ( ret.second == false )
				m_records[vec] = m_records[vec] + 1;
		}
		CLogFile::GetLogFile()->Add("CPU-CompactList",timer.GetTimeElapsed()*1000.0f);
	}


	void CParaCoordIData::UpdateSelf()
	{
		PerformanceTimer _pTimer;
		_pTimer.StartTimer();

		CleanSelf();


		if(!m_if_GPU)
			PrepareData();
		else
		{
			if(m_loadData)
			{
				GPU_TransferConstantData();
				m_loadData = false;
			}
			GPU_PrepareData();
		}

		m_dataProcessingTime = (float)_pTimer.GetTimeElapsed();
		CLogFile::GetLogFile()->Add("IData Generation",m_dataProcessingTime*1000.0f);
	}

/*********************************************************************************************
************************ GPU Codes **********************************************************
*********************************************************************************************/

	void CParaCoordIData::GPU_Init()
	{
		CDataManager* dm = GetDataManager();
		if(!dm->m_ifDataReady)
			return;

		cuda_Init();
		cuda_CleanConstant();
		GPU_TransferConstantData();
	}

	void CParaCoordIData::GPU_TransferConstantData()
	{
		CDataManager* dm = GetDataManager();
		unsigned int *item_desc_offset, *item_desc_bytes, *item_desc_values;
		unsigned int itemNum = dm->m_RawData->m_item_desc.size();
		item_desc_offset = (unsigned int*)malloc(sizeof(unsigned int)*itemNum);
		item_desc_bytes = (unsigned int*)malloc(sizeof(unsigned int)*itemNum);
		item_desc_values = (unsigned int*)malloc(sizeof(unsigned int)*itemNum);

		item_desc_offset[0] = 0;
		item_desc_bytes[0] = 8;
		item_desc_values[0] = dm->m_RawData->m_NumberOfRecords;

		for(unsigned int i = 1; i < itemNum; i++)
		{
			item_desc_offset[i] = dm->m_RawData->m_item_desc[i].offset;
			item_desc_bytes[i] = dm->m_RawData->m_item_desc[i].num_bytes;
			item_desc_values[i] = dm->m_RawData->m_item_desc[i].num_values;
		}

		unsigned int bufferSize = dm->m_RawData->m_NumberOfRecords*dm->m_RawData->m_TotalBytes;
		cuda_TransferConstant(item_desc_offset,item_desc_bytes, item_desc_values, 
			dm->m_RawData->m_DataBuffer, itemNum, bufferSize);

		free(item_desc_bytes);
		free(item_desc_offset);
		free(item_desc_values);
	}

	void CParaCoordIData::GPU_PrepareData() //generate the compactlist
	{
		CFilteredData* filteredData = NULL;
		m_compactNum = 0;
		filteredData = dynamic_cast<CFilteredData*>(m_parent[0]);
		assert(filteredData != NULL);
		CDataManager* dm = GetDataManager();
		if(m_selectedItems.size() < 2)
			return;
		
		set<unsigned int>& data = filteredData->m_filteredData;
		if(data.size() > 0)
		{
			PerformanceTimer timer;
			timer.StartTimer();
			//selected items;
			unsigned int selectedItemsNum = m_selectedItems.size();
			unsigned int *selected_items;
			unsigned int max_dimension = 0;
			selected_items = (unsigned int*)malloc(sizeof(unsigned int)*selectedItemsNum);
			for(unsigned int i = 0; i < m_selectedItems.size(); i++)
			{
				selected_items[i] = m_selectedItems[i];
				if(max_dimension < dm->m_RawData->m_item_desc[selected_items[i]].num_values)
					max_dimension = dm->m_RawData->m_item_desc[selected_items[i]].num_values;
			}

			//filtered data
			unsigned int* filterData;
			unsigned int index = 0;
			filterData = (unsigned int*)malloc(sizeof(unsigned int)*data.size());
			for(set<unsigned int>::iterator it = data.begin(); it != data.end(); it++)
			{
				filterData[index] = *it;
				index++;
			}
			CLogFile::GetLogFile()->Add("PrepareCopyFilterdData",timer.GetTimeElapsed()*1000.0f);
			timer.StartTimer();
			

			cuda_CleanDynamic();
			cuda_TransferDynamic(filterData, data.size(), selected_items, selectedItemsNum,
								max_dimension, dm->m_RawData->m_TotalBytes);
			CLogFile::GetLogFile()->Add("CopyFilterdData",timer.GetTimeElapsed()*1000.0f);
			timer.StartTimer();

			//get the size of compactList
			cuda_GetCompactListSize(m_compactNum);
			CLogFile::GetLogFile()->Add("GetCompactListNum",timer.GetTimeElapsed()*1000.0f);
			timer.StartTimer();

			if(m_compactNum > 0)
			{
				if( m_compactRecords != NULL)
				{
					free(m_compactRecords);
					m_compactRecords = NULL;
				}

				m_compactRecords = (unsigned int*)malloc(sizeof(unsigned int)*m_compactNum*selectedItemsNum);
			
				//generate the compactList
				cuda_GetCompactList(m_compactRecords);
			
			}
			CLogFile::GetLogFile()->Add("CopyCompactList",timer.GetTimeElapsed()*1000.0f);
			free(selected_items);
			free(filterData);
		}
	}

	/******************************************************************************************************
	********************CParaCoordRData********************************************************************
	******************************************************************************************************/
	CParaCoordRData::~CParaCoordRData()
	{
		CleanSelf();
		if ( m_vbo != NULL )
		{
			delete m_vbo;
			m_vbo = NULL;
		}
	}

	CParaCoordRData::CParaCoordRData(CCanvas* _canvas,bool _gpu) : m_if_GPU(_gpu), m_vbo(NULL)
	{
		m_vbo = new CVBOData(_canvas);
		m_vbo->m_if_GPU = _gpu;
	}

	void CParaCoordRData::SetGPU(bool _gpu)
	{
		CleanSelf();
		m_if_GPU = _gpu;
		m_vbo->m_if_GPU = _gpu;
	}

	void CParaCoordRData::CleanSelf()
	{
		if(!m_if_GPU)
		{
			assert(m_vbo != NULL);
			m_vbo->CleanData();
			m_edges.clear();
			m_vertices.clear();
		}
		else
			m_vbo->CleanData();

	}

	void CParaCoordRData::IncrementalSelf()
	{
		UpdateSelf();
	}

	void CParaCoordRData::UpdateSelf()
	{
		PerformanceTimer _pTimer;
		_pTimer.StartTimer();

		if(!m_if_GPU)
			PrepareData();
		else 
			GPU_PrepareData();

		m_dataProcessingTime = (float)_pTimer.GetTimeElapsed();
		CLogFile::GetLogFile()->Add("RData Generation",m_dataProcessingTime*1000.0f);
	}


	void CParaCoordRData::PrepareVertexEdge(CParaCoordIData* _idata, vector<t_ref>& _vertices, t_EdgeSet& _edges)
	{	
		t_uintvector &si = _idata->m_selectedItems;
		if(si.size() < 2)
			return;
		_vertices.resize(si.size());
		_edges.clear();

		unsigned int vertexNum = 0;
		for(unsigned int siidx = 0; siidx < si.size(); siidx++)
		{
			pair<t_ref::iterator,bool> ret;
			for(t_recordset::iterator it = _idata->m_records.begin(); it != _idata->m_records.end(); it++)
			{
				ret = _vertices[siidx].insert(pair<unsigned int, unsigned int>((it->first)[siidx],vertexNum));
				if(ret.second == true)
					vertexNum++;
			}	
		}

		for(t_recordset::iterator it = _idata->m_records.begin(); it != _idata->m_records.end(); it++)
		{
			for(unsigned int siidx = 0; siidx < si.size()-1; siidx++)
				_edges.insert(pair<unsigned int, unsigned int>(_vertices[siidx][(it->first)[siidx]], _vertices[siidx+1][(it->first)[siidx+1]]));
		}
	}

	void CParaCoordRData::PrepareData()
	{
		CParaCoordIData* idata = NULL;
		idata = dynamic_cast<CParaCoordIData*>(m_parent[0]);
		if(idata->m_selectedItems.size() < 2)
		{
			CleanSelf();
			return;
		}
		PerformanceTimer timer;
		timer.StartTimer();
		CleanSelf();
		PrepareVBO();
		CLogFile::GetLogFile()->Add("PrepareVBO-Total",timer.GetTimeElapsed()*1000.0f);
		timer.StartTimer();
		m_vbo->BindVBO();
		CLogFile::GetLogFile()->Add("BindingVBO",timer.GetTimeElapsed()*1000.0f);
	}

	t_color CParaCoordRData::GetColor(CParaCoordIData* _idata)
	{
		CFilter* filter = NULL;
		filter  = dynamic_cast<CFilter*>(_idata->m_parent[0]->m_parent[0]);
		CDataManager* dm = GetDataManager();
		return dm->GetColor(filter);
	}

	void CParaCoordRData::PrepareColorBuffer()
	{
		memset(m_vbo->m_colorData,GRAY[0]*255, m_vbo->m_colorFormat * m_vbo->m_colorNum * sizeof(unsigned char));
		
		for(unsigned int i = 1; i < m_parent.size(); i++)
		{
			CParaCoordIData* idata;
			idata = dynamic_cast<CParaCoordIData*>(m_parent[i]);
			assert(idata);

			vector<t_ref> vertices;
			t_EdgeSet edges;
			PrepareVertexEdge(idata,vertices,edges);
			t_color color_strut  =  GetColor(idata);
			unsigned char color[4];
			color[0]  = color_strut.red;
			color[1]  = color_strut.green;
			color[2]  = color_strut.blue;
			color[3]  = color_strut.alpha;

			for( unsigned int i = 0; i < idata->m_selectedItems.size(); i++)
			{
				for( t_ref::iterator it = vertices[i].begin(); it != vertices[i].end(); it++)
				{
					unsigned int index = m_vertices[i][it->first];
					memcpy(m_vbo->m_colorData + index* m_vbo->m_colorFormat, color, m_vbo->m_colorFormat*sizeof(unsigned char));
				}
			}
		}
	}

	unsigned int CParaCoordRData::GetDepth()
	{
		CParaCoordRData* rdata = NULL;
		unsigned int order = 1;
		CDataManager* dm = GetDataManager();
		for(unsigned int i = 0; i< dm->m_BasedIDataList.size(); i++)
		{
			rdata = dynamic_cast<CParaCoordRData*>(dm->m_BasedIDataList[i]->m_child[0]);
			if(rdata != NULL)
			{
				if(rdata == this)
					break;
			}

		}
		if(rdata == NULL)
		{
			for(unsigned int i = 0; i < dm->m_HighlightIDataList.size(); i++)
			{
				rdata = dynamic_cast<CParaCoordRData*>(dm->m_HighlightIDataList[i]->m_child[0]);
				if(rdata != NULL)
				{
					order++;
					if(rdata == this)
						break;
				}
			}
		}
		return order;
	}

	void CParaCoordRData::PrepareVBO()
	{
		PerformanceTimer timer;
		timer.StartTimer();
		CDataManager* dm = GetDataManager();
		CParaCoordIData* idata = NULL;
		idata = dynamic_cast<CParaCoordIData*>(m_parent[0]);
		assert(idata != NULL);
		PrepareVertexEdge(idata,m_vertices,m_edges);
		CLogFile::GetLogFile()->Add("PrepareVertexEdge",timer.GetTimeElapsed()*1000.0f);
		timer.StartTimer();

		t_uintvector &si = idata->m_selectedItems;
		float step  = 0.8f /(si.size() -1);
		float start = 0.1f;
		vector<float> steps;
		steps.resize(si.size());
		unsigned int vertexNum  = 0;
		for(unsigned int i = 0; i < si.size(); i++)
		{
			vertexNum += m_vertices[i].size();
			if(dm->m_RawData->m_item_desc[si[i]].num_values != 0)
				steps[i] = 0.9f / (dm->m_RawData->m_item_desc[si[i]].num_values -1);
		}

		
		unsigned int order = GetDepth();
		m_vbo->m_vertexNum = vertexNum;
		m_vbo->m_vertexFromat = 3;
		m_vbo->m_indexNum = m_edges.size();
		m_vbo->m_indexFormat = 2;
		m_vbo->m_vertexData = (float*) malloc(m_vbo->m_vertexNum * m_vbo->m_vertexFromat * sizeof(float));
		m_vbo->m_indexData = (unsigned int*) malloc(m_vbo->m_indexNum * m_vbo->m_indexFormat*sizeof(unsigned int));

		for(unsigned int siidx = 0; siidx < si.size(); siidx++)
		{
			float pos[3] = {0.0f, 0.0f, 0.0f};
			pos[0] = start + siidx*step;
			pos[2] = order*0.1f;

			if(dm->m_RawData->m_item_desc[si[siidx]].num_values != 0) // CAT
			{
				for(t_ref::iterator it = m_vertices[siidx].begin(); it != m_vertices[siidx].end(); it++)
				{
					pos[1] = it->first* steps[siidx] + 0.05f;
					memcpy(m_vbo->m_vertexData + it->second*m_vbo->m_vertexFromat, pos, m_vbo->m_vertexFromat*sizeof(float));
				}
			}
			else //NUM
			{
				for(t_ref::iterator it = m_vertices[siidx].begin(); it != m_vertices[siidx].end(); it++)
				{
					float value = dm->m_RawData->GetNumDataValue(it->first, si[siidx]);
					value = log10(value - dm->m_RawData->m_item_desc[si[siidx]].min + 1.0f)*dm->m_RawData->m_item_desc[si[siidx]].logcoeff;
					pos[1] = value*0.9f +0.05;
					memcpy(m_vbo->m_vertexData + it->second*m_vbo->m_vertexFromat, pos, m_vbo->m_vertexFromat*sizeof(float));
				}
			}
		}
		CLogFile::GetLogFile()->Add("CPU-GenerateVertexBuffer",timer.GetTimeElapsed()*1000.0f);
		timer.StartTimer();

		unsigned int *pid = m_vbo->m_indexData;
		for(t_EdgeSet::iterator it = m_edges.begin(); it != m_edges.end(); it++, pid+=2)
		{
			*pid = it->first;
			*(pid +1 ) = it->second;
		}
		CLogFile::GetLogFile()->Add("CPU-GenerateIndexBuffer",timer.GetTimeElapsed()*1000.0f);
	}


	/*****************************************************************************************
	******************** GPU Codes ***********************************************************
	******************************************************************************************/

	
	void CParaCoordRData::GPU_PrepareData()
	{
		CParaCoordIData* idata = NULL;
		idata = dynamic_cast<CParaCoordIData*>(m_parent[0]);
		m_vbo->m_vertexNum = 0;
		m_vbo->m_indexNum = 0;
		if(idata->m_compactNum == 0)
			return;
		
		PerformanceTimer timer;
		timer.StartTimer();
		cuda_GetVertexIndexSize(m_vbo->m_vertexNum, m_vbo->m_indexNum);
		CLogFile::GetLogFile()->Add("GetVertexIndexSize",timer.GetTimeElapsed()*1000.0f);
		timer.StartTimer();
		 
		m_vbo->m_vertexFromat = 3;
		m_vbo->m_indexFormat = 2;
		
		if(m_vbo->m_vertexNum ==0 || m_vbo->m_indexNum ==0)
			return;

		m_vbo->CreateVBO();
		m_vbo->PreKernel();
		cuda_GenerateVertexBuffers(m_vbo->m_vertexData,m_vbo->m_indexData,
			m_vbo->m_vertexNum*m_vbo->m_vertexFromat, m_vbo->m_indexNum*m_vbo->m_indexFormat,GetDepth());
		m_vbo->AfterKernel();
		CLogFile::GetLogFile()->Add("GenerateVertexIndexBuffer",timer.GetTimeElapsed()*1000.0f);
	}
}
