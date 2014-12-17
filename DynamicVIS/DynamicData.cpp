#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "DynamicData.h"
#include "datamanager.h"
#include "PerformanceTimer.h"
#include "colorhelper.h"
#include "FilteredData.h"
#include "RawData.h"
#include "VBOData.h"
#include "TimeData.h"
#include "Filter.h"

namespace VIS
{
	/******************************************************************************************************
	********************CDynamicIData*******************************************************************
	******************************************************************************************************/

	float CDynamicIData::s_minvalue = 0.0f;
	float CDynamicIData::s_maxvalue = 6.0f;
	void CDynamicIData::CleanSelf()
	{
		if(m_data != NULL)
		{
			free(m_data);
			m_data = NULL;
		}
	}

	void CDynamicIData::PrepareData()
	{
		
		CDataManager* dm = GetDataManager();
		
		CFilter* filter = NULL;
		filter = dynamic_cast<CFilter*>(CAbstractData::s_UpdateObjects[0]);
		CDynamicIData* idata = NULL;
		idata = dynamic_cast<CDynamicIData*>(CAbstractData::s_UpdateObjects[0]);
		if(CAbstractData::s_UpdateObjects[0] == dm->m_TimeStepData || CAbstractData::s_UpdateObjects[0] == dm->m_TimeRangeData ||m_data == NULL || filter != NULL || idata != NULL || m_child[0]->m_parent.size() == m_numParents -1)
			AllocateSelf();
		 
		m_numParents = m_child[0]->m_parent.size();

		unsigned int idx;
		if ( !dm->m_ifSkip )
			idx = (dm->m_CurTimeData->GetCurTime() - dm->m_MinTime) / dm->m_TimeStepData->GetTimeStep();
		else
			idx = dm->m_CurrentTimesliderIdx;

		if( m_Item != -1)
		{
			if(m_SpecialItem == -1)
			{
				if(dm->m_RawData->m_item_desc[m_Item].num_values == 0) //numerical item
				{
					CFilteredData* filtereddata = dynamic_cast<CFilteredData*>(m_parent[0]);
					float _accumlated_value = 0.0f;
					unsigned int _num_values = 0;
					
					set<unsigned int>::iterator it = filtereddata->m_filteredData.begin();
					for(; it != filtereddata->m_filteredData.end(); it++)
					{
						float _value = dm->m_RawData->GetNumDataValue(*it,m_Item);
						_num_values++;
						_accumlated_value += _value;
					}
					m_data[idx] = _accumlated_value / _num_values;

					if ( m_data[idx] > CDynamicIData::s_maxvalue )
						CDynamicIData::s_maxvalue = m_data[idx];
					if ( m_data[idx] < CDynamicIData::s_minvalue )
						CDynamicIData::s_minvalue = m_data[idx];
				}
			}
			else //special item 
			{
				CDynamicIData::s_minvalue = 0.0f;
				CDynamicIData::s_maxvalue = 9e10f;

				for(unsigned int i = 0; i < m_size; i++)
				{
					float _accumlated_value = 0.0f;
					unsigned int _num_values = 0; 
					CFilteredData* filtereddata = dynamic_cast<CFilteredData*>(m_parent[0]);
					set<unsigned int>::iterator it = filtereddata->m_filteredData.begin();
					for(; it != filtereddata->m_filteredData.end(); it++)
					{
						float _value = (dm->m_RawData->GetNumDataValue(*it, 7))*(dm->m_RawData->GetNumDataValue(*it,12));
						_num_values++;
						_accumlated_value += _value;
					}
					m_data[idx] = _accumlated_value / _num_values;

					if ( m_data[idx] > CDynamicIData::s_maxvalue )
						CDynamicIData::s_maxvalue = m_data[idx];
					if( m_data[idx] < CDynamicIData::s_minvalue )
						CDynamicIData::s_minvalue = m_data[idx];
				}
			}
		}
		else
		{
			unsigned int value;
			if(m_parent[0] == dm->m_BasedFilteredData) //base 
				value = dm->m_TimeWindowData->GetNumberofQueriedRecords(); //dm->GetNumberofQueriedRecords();
			else
			{
				CFilteredData* filtereddata = dynamic_cast<CFilteredData*>(m_parent[0]);
				if(filtereddata != NULL)
					value = filtereddata->m_filteredData.size();
			}
			m_data[idx] = (float)value;

			if ( value > CDynamicIData::s_maxvalue )
				CDynamicIData::s_maxvalue = value;
		}
		
		
	}


	void CDynamicIData::UpdateSelf()
	{
		PerformanceTimer _pTimer;
		_pTimer.StartTimer();

		PrepareData();

		m_dataProcessingTime = (float)_pTimer.GetTimeElapsed();
	}

	void CDynamicIData::IncrementalSelf()
	{
		PerformanceTimer _pTimer;
		_pTimer.StartTimer();

		PrepareData();

		m_dataProcessingTime = (float)_pTimer.GetTimeElapsed();
	}

	void CDynamicIData::AllocateSelf()
	{
		CDataManager* dm = GetDataManager();//CDataManager::Instance();

		
		m_size = 0;

		unsigned int _size = 0;
		if(!dm->m_ifSkip)
			_size = (dm->m_MaxTime - dm->m_MinTime)/dm->m_TimeStepData->GetTimeStep() + 1;
		else
		{	
			_size = dm->m_TimeWindowData->m_UniqueTimeStamps.size();//dm->m_UniqueTimeStamps.size();

		}
		if ( m_data != NULL )
		{
			free( m_data );
			m_data = NULL;
		}

		m_size = _size;
		m_data = (float*) malloc( _size*sizeof(float));
		memset( m_data, 0.0f, _size*sizeof(float) );
		
	}

	/******************************************************************************************************
	********************CDynamicRData*******************************************************************
	******************************************************************************************************/
	CDynamicRData::CDynamicRData(CCanvas* _canvas): m_if_logScale(false), m_vbo(NULL),m_acuHeight(NULL),m_if_defalut_value(NULL), m_numParents(0)
	{
		m_vbo = new CVBOData(_canvas);
	}

	CDynamicRData::~CDynamicRData()
	{
		CleanSelf();
		if(m_vbo != NULL)
		{
			delete m_vbo;
			m_vbo = NULL;
		}
	}

	void CDynamicRData::UpdateSelf()
	{
		PerformanceTimer _pTimer;
		_pTimer.StartTimer();

		PrepareData();

		m_dataProcessingTime = (float)_pTimer.GetTimeElapsed();
	}

	void CDynamicRData::CleanSelf()
	{
		if(m_vbo != NULL)
		{
			m_vbo->CleanData();
		}
		if(m_acuHeight != NULL)
		{
			free(m_acuHeight);
			m_acuHeight = NULL;
		}
		if(m_if_defalut_value != NULL)
		{
			free(m_if_defalut_value);
			m_if_defalut_value = NULL;
		}
	}

	void CDynamicRData::PrepareData()
	{
		
		CDataManager* dm = GetDataManager();
		CFilter* filter = NULL;
		filter = dynamic_cast<CFilter*>(CAbstractData::s_UpdateObjects[0]);
		CDynamicIData* idata = NULL;
		idata = dynamic_cast<CDynamicIData*>(CAbstractData::s_UpdateObjects[0]);
		if(CAbstractData::s_UpdateObjects[0] == dm->m_TimeStepData || CAbstractData::s_UpdateObjects[0] == dm->m_TimeRangeData ||m_vbo == NULL || m_acuHeight == NULL || m_if_defalut_value == NULL || filter != NULL || idata != NULL || m_parent.size() == m_numParents -1)
			AllocateSelf();

		m_numParents = m_parent.size();

		unsigned int idx;
		if ( !dm->m_ifSkip )
			idx = (dm->m_CurTimeData->GetCurTime() - dm->m_MinTime) / dm->m_TimeStepData->GetTimeStep();
		else
			idx = dm->m_CurrentTimesliderIdx;

		m_acuHeight[idx] =0.0f;
		m_if_defalut_value[idx] = false;
		for(int i = 0; i < m_parent.size()-1; i++)
		{
			CDynamicIData *thisdata = dynamic_cast<CDynamicIData*>(m_parent[i]);
			m_acuHeight[idx] += thisdata->m_data[idx];
		}
		CDynamicIData *thisdata = dynamic_cast<CDynamicIData*>(m_parent[m_parent.size()-1]);
		bool isNumerical = false;
		if(thisdata->m_Item != -1)
			isNumerical = true;

		PrepareVBO(m_acuHeight,thisdata->m_data,thisdata->m_size,CDynamicIData::s_maxvalue,CDynamicIData::s_minvalue,isNumerical);

		m_vbo->BindVBO();
	}

	void CDynamicRData::IncrementalSelf()
	{
		PerformanceTimer _pTimer;
		_pTimer.StartTimer();

		PrepareData();
		m_dataProcessingTime = (float)_pTimer.GetTimeElapsed();
	}

	void CDynamicRData::AllocateSelf()
	{
		CDynamicIData* idata  = dynamic_cast<CDynamicIData*>(m_parent[0]);
		CleanSelf();

		m_acuHeight = (float*) malloc(idata->m_size*sizeof(float));
		memset(m_acuHeight,0.0f,idata->m_size*sizeof(float));
		
		m_if_defalut_value = (bool*) malloc(idata->m_size*sizeof(bool));
		memset(m_if_defalut_value, true, idata->m_size*sizeof(bool));
	}

	void CDynamicRData::PrepareVBO(float *_preacc, float *_cur, unsigned int _size, float _maxValue, float _minValue,bool _isNumerical)
	{
		m_vbo->CleanData();
		m_vbo->m_if_GPU = false;
		m_vbo->m_vertexNum = _size;
		m_vbo->m_vertexFromat = 3;
		m_vbo->m_vertexData = (float*) malloc(m_vbo->m_vertexNum*m_vbo->m_vertexFromat*sizeof(float));
	 
		if( fabs(_maxValue - _minValue) <= PRECISION)
			_maxValue += 1.0f;

		float _tmp,_height;
		if(m_if_logScale)
			_tmp = 1.0f / log10(_maxValue - _minValue +1.0f);
		else
			_tmp = 1.0f/(_maxValue -_minValue);
		float _step = 1.0f/(_size-1);
		float _depth = -0.15f;
		float _tmpValue;
		for(uint i=0; i<_size; i++)
		{
			if(m_if_defalut_value[i])
				_tmpValue = _minValue;
			else
				_tmpValue = _cur[i];

			
			if(m_if_logScale)
			{
				if(_isNumerical)
					_height = log10(_tmpValue - _minValue +1.0f)*_tmp;
				else
				_height = log10(_tmpValue +_preacc[i] - _minValue +1.0f)*_tmp;
			}
			else
			{
				if(_isNumerical)
					_height = (_tmpValue -  _minValue)*_tmp;
				else
					_height = (_tmpValue + _preacc[i] -  _minValue)*_tmp;
			}

			m_vbo->m_vertexData[3*i + 0] = i*_step;
			m_vbo->m_vertexData[3*i + 1] = _height;
			m_vbo->m_vertexData[3*i + 2] = _depth;
		}
	}
}
