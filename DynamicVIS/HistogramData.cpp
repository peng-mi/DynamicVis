#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "HistogramData.h"
#include "datamanager.h"
#include "PerformanceTimer.h"
#include "colorhelper.h"
#include "VBOData.h"
#include "RawData.h"
#include "FilteredData.h"

namespace VIS
{	 
	extern "C"
	void cuda_HistogramOrder(unsigned int* _histogramData, unsigned int* _reference, unsigned int* _order,unsigned int _size);
	extern "C"
	unsigned int binarySearch(unsigned int* _data, unsigned int _value, unsigned int _size);

	/******************************************************************************************************
	********************CHistogramIData*******************************************************************
	******************************************************************************************************/
	unsigned int CHistogramIData::s_maxValue = 0;
	void CHistogramIData::CleanSelf()
	{
		if(m_histogram != NULL)
		{
			free(m_histogram);
			m_histogram = NULL;
		}
		if(m_index != NULL)
		{
			free(m_index);
			m_index = NULL;
		}
		if(m_reference != NULL)
		{
			free(m_reference);
			m_reference = NULL;
		}
		if(m_order != NULL)
		{
			free(m_order);
			m_order = NULL;
		}
		m_numHistogram = 0;
	}

	void CHistogramIData::UpdateSelf()
	{
		PerformanceTimer _pTimer;
		_pTimer.StartTimer();

		CleanSelf();
		if(m_parent.size() ==0)
			return;

		if(m_item <1)
			return;

		CDataManager* dm = GetDataManager();
		CRawData* _rawdata = dm->m_RawData;
		CFilteredData *_filtereddata = dynamic_cast<CFilteredData*>(m_parent[0]);
		if(_filtereddata == NULL)
			return;

	
		set<uint>::iterator it;
		unsigned int _maxValue = 6;
		map<unsigned int, unsigned int> histogram_data;
		for(it = _filtereddata->m_filteredData.begin(); it != _filtereddata->m_filteredData.end(); it++)
		{
			uint _value = _rawdata->GetDataValue(*it,m_item);
		
			if(histogram_data.find(_value) != histogram_data.end())
				histogram_data[_value]++;
			else
				histogram_data[_value] = 1;
			if(_maxValue < histogram_data[_value])
				_maxValue = histogram_data[_value];
		}
		if ( _maxValue > CHistogramIData::s_maxValue )
			CHistogramIData::s_maxValue = _maxValue;

		if(m_parent.size() ==1)
		{
			m_numHistogram = histogram_data.size();
			m_histogram = (unsigned int*)malloc(sizeof(unsigned int)*m_numHistogram);
			m_index = (unsigned int*)malloc(sizeof(unsigned int)*m_numHistogram);
			m_reference = (unsigned int*)malloc(sizeof(unsigned int)*m_numHistogram);
			m_order = (unsigned int*)malloc(sizeof(unsigned int)*m_numHistogram);
			unsigned int index = 0;
			for(map<unsigned int,unsigned int>::iterator it = histogram_data.begin(); it != histogram_data.end(); it++)
			{
				m_reference[index] = index;
				m_order[index] = index;
				m_histogram[index] = it->second;
				m_index[index] = it->first;
				index++;
			}
			cuda_HistogramOrder(m_histogram, m_reference, m_order, m_numHistogram);
		}
		else
		{
			CHistogramIData* idata = dynamic_cast<CHistogramIData*>(m_parent[1]);
			m_numHistogram = idata->m_numHistogram;
			m_histogram = (unsigned int*)malloc(sizeof(unsigned int)*m_numHistogram);
			m_index = (unsigned int*)malloc(sizeof(unsigned int)*m_numHistogram);
			memset(m_histogram,0, sizeof(unsigned int)*m_numHistogram);
			memcpy(m_index,idata->m_index,sizeof(unsigned int)*m_numHistogram);
			
			unsigned int reference;
			for(map<unsigned int,unsigned int>::iterator it = histogram_data.begin(); it != histogram_data.end(); it++)
			{
				reference = binarySearch(idata->m_index,it->first,m_numHistogram);
				reference = idata->m_order[reference];
				m_histogram[reference] = it->second;
			}
		}
 
		m_dataProcessingTime = (float)_pTimer.GetTimeElapsed();
	}

	void CHistogramIData::IncrementalSelf()
	{
		UpdateSelf();
	}

	/******************************************************************************************************
	********************CHistogramRData*******************************************************************
	******************************************************************************************************/
	
	CHistogramRData::CHistogramRData(CCanvas* _canvas):m_if_logScale(true),m_verticalScale(1.0f), m_vbo(NULL),m_acuHeight(NULL)
	{
		m_vbo = new CVBOData(_canvas);
	}

	void CHistogramRData::CleanSelf()
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
	}

	CHistogramRData::~CHistogramRData()
	{
		CleanSelf();
		if(m_vbo != NULL)
		{
			delete m_vbo;
			m_vbo = NULL;
		}
	}

	void CHistogramRData::IncrementalSelf()
	{
		UpdateSelf();
	}

	void CHistogramRData::UpdateSelf()
	{
		PerformanceTimer _pTimer;
		_pTimer.StartTimer();

		CleanSelf();

		CHistogramIData *hidata = dynamic_cast<CHistogramIData*>(m_parent[0]);
		if(m_acuHeight == NULL)
			m_acuHeight = (unsigned int*) malloc(hidata->m_numHistogram*sizeof(unsigned int));
		memset( m_acuHeight, 0, hidata->m_numHistogram*sizeof(unsigned int) );
		if ( m_parent.size() == 1 ) // Base histogram
		{
			PrepareVBO( m_acuHeight, hidata->m_histogram, hidata->m_numHistogram, CHistogramIData::s_maxValue );
			 m_vbo->BindVBO();
		}
		else
		{
			for ( unsigned int i = 0; i < m_parent.size()-1; i++ )
			{
				CHistogramIData *_thisdata = dynamic_cast<CHistogramIData*> (m_parent[i]);
				for ( unsigned int j = 0; j < hidata->m_numHistogram; j++ )
					m_acuHeight[j] += _thisdata->m_histogram[j];
			}
			CHistogramIData *_thisdata = dynamic_cast<CHistogramIData*> (m_parent[m_parent.size()-1]);

			PrepareVBO( m_acuHeight, _thisdata->m_histogram, hidata->m_numHistogram, CHistogramIData::s_maxValue );
			m_vbo->BindVBO();
		}

		m_dataProcessingTime = (float)_pTimer.GetTimeElapsed();
	}


	void CHistogramRData::PrepareVBO( unsigned int *_preacc, unsigned int *_cur, unsigned int _size, unsigned int _maxValue)
	{
		m_vbo->CleanData();
		m_vbo->m_if_GPU = false;
		m_vbo->m_vertexNum = 2*_size;
		m_vbo->m_vertexFromat = 3;
		m_vbo->m_vertexData = (float*) malloc(m_vbo->m_vertexNum*m_vbo->m_vertexFromat*sizeof(float));

		unsigned int _nValues = _size;
		float step = 1.0f / _nValues;
		float delta = 0.1f*step;
		float depth = -0.15f;

		for( unsigned int i = 0; i < _size; i++)
		{
			unsigned int value;
			float height;
			value = _cur[i] + _preacc[i];

			if(m_if_logScale)
				height = log10( (float)(value + 1.0f)) / log10(_maxValue + 1.0f);
			else
				height = (float) value / _maxValue;
			height *= m_verticalScale;
			m_vbo->m_vertexData[6*i + 0] = i*step + delta;
			m_vbo->m_vertexData[6*i + 1] = height;
			m_vbo->m_vertexData[6*i + 2] = depth;
			m_vbo->m_vertexData[6*i + 3] = (i+1)*step - delta;
			m_vbo->m_vertexData[6*i + 4] = height;
			m_vbo->m_vertexData[6*i + 5] = depth;
		}
	}

}