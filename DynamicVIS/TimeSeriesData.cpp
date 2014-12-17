#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "TimeSeriesData.h"
#include "datamanager.h"
#include "PerformanceTimer.h"
#include "RawData.h"
#include "colorhelper.h"
#include "Filter.h"
#include "VBOData.h"
#include "TimeData.h"

namespace VIS
{

CTimeSeriesIData::CTimeSeriesIData(void) : 	m_CurrentBuildKey(-1), m_CurrentItem(-1), m_numTSs(0), m_numPoints(0), m_data(NULL), m_ifHasData(NULL)
{
}

void CTimeSeriesIData::UpdateSelf()
{
	PerformanceTimer _pTimer;
	_pTimer.StartTimer();

	CleanSelf();
	PrepareData();

	m_dataProcessingTime = (float)_pTimer.GetTimeElapsed();
}

void CTimeSeriesIData::PrepareData()
{
	if(m_parent.size() == 0 || m_CurrentBuildKey == -1 || m_CurrentItem == -1 )
		return;

	CDataManager* dm = GetDataManager();
	CRawData* rd = dynamic_cast<CRawData*>(m_parent[0]);
	CFilter* exfilter = dynamic_cast<CFilter*>(m_parent[1]);

	m_numPoints = dm->m_TimeWindowData->m_UniqueTimeStamps.size();
	m_numTSs = rd->m_item_desc[m_CurrentBuildKey].num_values;
	m_data = (float *) malloc(m_numTSs*m_numPoints*sizeof(float));
	m_ifHasData = (bool*) malloc( m_numTSs*sizeof(bool));
	memset( m_ifHasData, false, m_numTSs*sizeof(bool) );
	
	float *_accum = (float *) malloc(m_numTSs*sizeof(float));
	unsigned int *_num = (unsigned int *) malloc(m_numTSs*sizeof(unsigned int));

	unsigned int recordIdx = 0;

	for ( unsigned int pointidx = 0; pointidx < m_numPoints; pointidx++ )
	{
		time_t _curtime = dm->m_TimeWindowData->m_UniqueTimeStamps[pointidx];

		memset( _accum, 0, m_numTSs*sizeof(float) );
		memset( _num, 0, m_numTSs*sizeof(unsigned int) );

		while ( recordIdx < rd->m_NumberOfRecords && rd->GetRecordTime( recordIdx ) == _curtime )
		{
			if ( !dm->SatisfyFilter( recordIdx, exfilter->m_Filter ) )
			{
				unsigned int keyindex = rd->GetDataValue( recordIdx, m_CurrentBuildKey );
				_accum[keyindex] += rd->GetNumDataValue( recordIdx, m_CurrentItem );
				_num[keyindex]++;
				recordIdx++;
			}
		}

		for ( unsigned int i = 0; i < m_numTSs; i++ )
		{
			m_data[i*m_numPoints+pointidx] = _accum[i] / _num[i]; // What if _num[i] == 0???
			if ( _num[i] != 0 )
				m_ifHasData[i] = true;
		}
	}

	free( _accum );
	free( _num );
}

void CTimeSeriesIData::CleanSelf(void)
{
	m_numTSs = 0;
	m_numPoints = 0;
	if (m_data != NULL)
		free(m_data);
	m_data = NULL;

	if ( m_ifHasData != NULL )
		free( m_ifHasData );
	m_ifHasData = NULL;
}

void CTimeSeriesIIndicesData::UpdateSelf()
{
	PerformanceTimer _pTimer;
	_pTimer.StartTimer();

	CleanSelf();
	PrepareData();

	m_dataProcessingTime = (float)_pTimer.GetTimeElapsed();
}

void CTimeSeriesIIndicesData::PrepareData()
{
/*	CDataManager* dm = GetDataManager();

	CTimeSeriesIData* idata; 
	CFilter* filter = dynamic_cast<CFilter*>(m_parent[0]);
	if ( fdata == NULL )
	{
		idata = dynamic_cast<CTimeSeriesIData*>(m_parent[0]);
		filter = dynamic_cast<CFilter*>(m_parent[1]);
	}
	else
		idata = dynamic_cast<CTimeSeriesIData*>(m_parent[1]);

	unsigned int tsidx;
	for ( set<unsigned int>::iterator it = fdata->m_filteredData.begin(); it != fdata->m_filteredData.end(); it++ )
	{
		tsidx = dm->m_RawData->GetDataValue( *it, idata->m_CurrentBuildKey );
		m_TSIndices.push_back(tsidx);
	}*/
}

/****************************************************************************************
***************************** CTimeSeriesRdata*******************************************
****************************************************************************************/

CTimeSeriesRData::CTimeSeriesRData(CCanvas* _canvas) : m_if_logScale(false), m_vbo(NULL)
{
	m_vbo = new CVBOData(_canvas);
}

CTimeSeriesRData::~CTimeSeriesRData()
{
	CleanSelf();
	if(m_vbo != NULL)
	{
		delete m_vbo;
		m_vbo = NULL;
	}
}

void CTimeSeriesRData::CleanSelf()
{
	if(m_vbo != NULL)
	{
		m_vbo->CleanData();
	}
}

void CTimeSeriesRData::PrepareData()
{
	CTimeSeriesIData *tsidata = dynamic_cast<CTimeSeriesIData*>(m_parent[0]);

	if ( tsidata == NULL || tsidata->m_CurrentBuildKey == -1 || tsidata->m_CurrentItem == -1 )
		return;

	CDataManager *dm = GetDataManager();

	float _max = dm->m_RawData->m_item_desc[tsidata->m_CurrentItem].max;
	float _min = dm->m_RawData->m_item_desc[tsidata->m_CurrentItem].min;
	float _coeff = 1.0f / (_max-_min);
	float _coefflog = 1.0f / log10(_max-_min+1.0f);

	m_vbo->m_if_GPU = false;
	m_vbo->m_vertexNum = tsidata->m_numPoints * tsidata->m_numTSs;
	m_vbo->m_vertexFromat = 2;
	m_vbo->m_vertexData = (float*) malloc(m_vbo->m_vertexNum*m_vbo->m_vertexFromat*sizeof(float));
	
	for ( unsigned int i = 0; i < tsidata->m_numTSs; i++ )
	{
		float step = 1.0f / (tsidata->m_numPoints-1.0f);
		float *_rddata = m_vbo->m_vertexData + 2*i*tsidata->m_numPoints;
		float *_data = tsidata->m_data + i*tsidata->m_numPoints;
		for ( unsigned int j = 0; j < tsidata->m_numPoints; j++ )
		{
			_rddata[2*j] = j*step;
			if ( m_if_logScale )
				_rddata[2*j+1] = log10(_data[j]-_min+ 1.0f) * _coefflog;
			else
				_rddata[2*j+1] =_data[j] * _coeff;
		}
	}

	m_vbo->BindVBO();
}

void CTimeSeriesRData::UpdateSelf()
{
	PerformanceTimer _pTimer;
	_pTimer.StartTimer();

	PrepareData();

	m_dataProcessingTime = (float)_pTimer.GetTimeElapsed();
}

void CTimeSeriesRColorData::CleanSelf()
{
	if ( m_ColorIndices != NULL )
		free(m_ColorIndices);
	m_ColorIndices = NULL;
	m_numTSs = 0;
}

void CTimeSeriesRColorData::UpdateSelf()
{
	PerformanceTimer _pTimer;
	_pTimer.StartTimer();

	PrepareData();

	m_dataProcessingTime = (float)_pTimer.GetTimeElapsed();
}

void CTimeSeriesRColorData::PrepareData()
{
	CTimeSeriesIIndicesData *iidata = dynamic_cast<CTimeSeriesIIndicesData*> (m_parent[0]);
	CTimeSeriesIData *idata = dynamic_cast<CTimeSeriesIData*> (iidata->m_parent[0]);
	if ( idata == NULL )
		idata = dynamic_cast<CTimeSeriesIData*> (iidata->m_parent[1]);

	m_numTSs = idata->m_numTSs;
	m_ColorIndices = (unsigned int*)malloc(m_numTSs*sizeof(unsigned int));
	memset( m_ColorIndices, 0, m_numTSs*sizeof(unsigned int) );

	for ( unsigned int i = 0; i < m_parent.size(); i++ )
	{
		iidata = dynamic_cast<CTimeSeriesIIndicesData*> (m_parent[i]);
		for ( unsigned int j = 0; j < iidata->m_TSIndices.size(); j++ )
			m_ColorIndices[iidata->m_TSIndices[j]] = i+1;
	}
}

}