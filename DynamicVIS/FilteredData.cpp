#include "Data.h"
#include "RawData.h"
#include "datamanager.h"
#include "FilteredData.h"
#include "Filter.h"
#include "PerformanceTimer.h"
#include "TimeData.h"
#include "LogFile.h"
#define NUM_PARENT 4

namespace VIS
{
	/******************************************************************************************************
	********************FilteredData***********************************************************************
	******************************************************************************************************/

	CFilteredData::CFilteredData() : CAbstractData()
	{
		m_incremental.dir = OTHERS;
		m_incremental.preStart = 0;
		m_incremental.preEnd = 0;
	}

	void CFilteredData::CleanSelf()
	{
		m_filteredData.clear();
	}

	void CFilteredData::IncrementalSelf(t_IncrementalType _dir)
	{
/*		if(_dir == SAME_DIR)
			return;

		CDataManager* dm = GetDataManager();//CDataManager::Instance();

		if(_dir == OTHERS)
		{
			//erase all of the previous data
			m_filteredData.clear();

			//filling the new data
			if(dm->m_EndIdx >0 && dm->m_StartIdx < dm->m_NumberOfRecords)
			{
				for(uint i = dm->m_StartIdx; i<= dm->m_EndIdx; i++)
				{
					if (dm->SatisfyFilter(i, dm->m_excludedset))
						continue;
					if(dm->m_neExcludedset.size() !=0)
					{	
						if(! dm->SatisfyFilter(i, dm->m_neExcludedset))
							continue;
					}
					m_filteredData.insert(i);
				}
			}

			return;
		}

		set<uint>::iterator _start = m_filteredData.begin();
		uint _valueStart = (*_start);
		uint _valueEnd	 = (*m_filteredData.rbegin());

		if(_dir == LARGE_DIR)
		{
			//increase the head
			for(uint i=_valueStart-1; i>=dm->m_StartIdx; i--)
			{
				if(dm->SatisfyFilter(i,dm->m_excludedset))
					continue;
				if(dm->m_neExcludedset.size() !=0)
				{
					if(! dm->SatisfyFilter(i,dm->m_neExcludedset))
						continue;
				}
				m_filteredData.insert(i);
				if(i==0)
					break;
			}
			//increase the tail
			for(uint i=_valueEnd+1; i<=dm->m_EndIdx; i++)
			{
				if(dm->SatisfyFilter(i,dm->m_excludedset))
					continue;
				if(dm->m_neExcludedset.size() !=0)
				{
					if(! dm->SatisfyFilter(i,dm->m_neExcludedset))
						continue;
				}
				m_filteredData.insert(i);
			}
		}
		else if(_dir == SMALL_DIR)
		{
			//decrease the head
			set<uint>::iterator low;
			if(m_filteredData.size()>0)
			{
				low  = m_filteredData.lower_bound(dm->m_StartIdx);
				m_filteredData.erase(m_filteredData.begin(), low);
			}
			//decrease the tail
			if(m_filteredData.size()>0)
			{
				low = m_filteredData.upper_bound(dm->m_EndIdx);
				m_filteredData.erase(low,m_filteredData.end());
			}

		}

		else if(_dir == LEFT_DIR)
		{
			//increase the head
			for(uint i=_valueStart-1; i>=dm->m_StartIdx; i--)
			{
				if(dm->SatisfyFilter(i,dm->m_excludedset))
					continue;
				if(dm->m_neExcludedset.size() !=0)
				{
					if(! dm->SatisfyFilter(i,dm->m_neExcludedset))
						continue;
				}
				m_filteredData.insert(i);
				_start = m_filteredData.begin();
				if(i==0)
					break;
			}
			//decrease the tail
			set<uint>::iterator low;
			if(m_filteredData.size()>0)
			{
				low = m_filteredData.upper_bound(dm->m_EndIdx);
				m_filteredData.erase(low, m_filteredData.end());
			}
		}

		else if(_dir == RIGHT_DIR)
		{
			//decrease the head
			set<uint>::iterator low;
			if(m_filteredData.size()>0)
			{
				low= m_filteredData.lower_bound(dm->m_StartIdx);
				m_filteredData.erase(m_filteredData.begin(),low);
			}
			//increase the tail
			for(uint i=_valueEnd+1; i<=dm->m_EndIdx; i++ )
			{
				if(dm->SatisfyFilter(i,dm->m_excludedset))
					continue;
				if(dm->m_neExcludedset.size() !=0)
				{
					if(! dm->SatisfyFilter(i,dm->m_neExcludedset))
						continue;
				}
				m_filteredData.insert(i);
			}
		}*/
	}

	t_IncrementalType CFilteredData::GetDirection()
	{
		if ( m_parent.size() != 3 )
			return OTHERS;
		CRawData *_rawdata = dynamic_cast<CRawData*>(m_parent[0]);
		if ( _rawdata == NULL )
			return OTHERS;

		t_IncrementalType res;
		res = OTHERS;
		if(m_filteredData.size()<=1)
			return res;

		CTimeWindow* timewindow = NULL;
		for(unsigned int i = 0; i < m_parent.size(); i++)
		{
			timewindow = dynamic_cast<CTimeWindow*>(m_parent[i]);
			if(timewindow != NULL)
				break;
		}


		if(timewindow->m_EndIdx >=0 && timewindow->m_StartIdx < _rawdata->m_NumberOfRecords)
		{
			set<uint>::iterator _itBegin = m_filteredData.begin();
			//set<uint>::reverse_iterator _itEnd	= m_filteredData.rbegin();
			uint _begin = (*_itBegin);
			uint _end = (*m_filteredData.rbegin());

			if((_begin > timewindow->m_StartIdx) && (_end < timewindow->m_EndIdx ))
				res = LARGE_DIR;
			else if((_begin == timewindow->m_StartIdx) && (_end == timewindow->m_EndIdx ))
				res = SAME_DIR;
			else if((_begin <= timewindow->m_StartIdx) && (_end >= timewindow->m_EndIdx ))
				res = SMALL_DIR;
			else if((_begin <= timewindow->m_StartIdx) && (_end < timewindow->m_EndIdx ) &&(_end>timewindow->m_StartIdx))
				res = RIGHT_DIR;
			else if ((_begin > timewindow->m_StartIdx) && (_end >= timewindow->m_EndIdx ) &&(_begin<timewindow->m_EndIdx))
				res  = LEFT_DIR;
			else
				res = OTHERS;
		}
		return res;
	}

	void CFilteredData::UpdateSelf()
	{
		PerformanceTimer _pTimer;
		_pTimer.StartTimer();
		//clear the previous dataset
		CleanSelf();

		CDataManager* dm = GetDataManager();//CDataManager::Instance();

		if ( m_parent.size() > 2 ) // for exclusive filtered data
		{
			CRawData *_rawdata = dynamic_cast<CRawData*>(m_parent[0]);
			CFilter *_excludeFilter = dynamic_cast<CFilter*>(m_parent[1]);
			CFilter *_negExcludeFilter = dynamic_cast<CFilter*>(m_parent[2]);
			CTimeWindow* _windowtime = dynamic_cast<CTimeWindow*>(m_parent[3]);

			if ( _rawdata == NULL || _excludeFilter == NULL || _negExcludeFilter == NULL || _windowtime == NULL)
				return;
			if(_windowtime->m_EndIdx >=0 && _windowtime->m_StartIdx < _rawdata->m_NumberOfRecords)
			{
				for(uint i = _windowtime->m_StartIdx; i<= _windowtime->m_EndIdx; i++)
				{
					if (dm->SatisfyFilter(i, _excludeFilter->m_Filter))
						continue;
					if(_negExcludeFilter->m_Filter.size() !=0)
					{	
						if(! dm->SatisfyFilter(i, _negExcludeFilter->m_Filter))
							continue;
					}
					m_filteredData.insert(i);
				}
			}
		}
		else if ( m_parent.size() == 2 ) // For highlighted data
		{
			CFilteredData *_filteredData = dynamic_cast<CFilteredData*> (m_parent[0]);
			CFilter *_highlightFilter;
			if ( _filteredData != NULL )
			{
				_highlightFilter = dynamic_cast<CFilter*> (m_parent[1]);
			}
			else
			{
				_filteredData = dynamic_cast<CFilteredData*> (m_parent[1]);
				_highlightFilter = dynamic_cast<CFilter*> (m_parent[0]);
			}

			set<unsigned int>::iterator it;
			for( it = _filteredData->m_filteredData.begin(); it != _filteredData->m_filteredData.end(); it++)
			{
				if (dm->SatisfyFilter(*it, _highlightFilter->m_Filter ))
					m_filteredData.insert(*it);
			}
		}
		CLogFile::GetLogFile()->Add("NumOfRecords",m_filteredData.size()*1.0f);
		m_dataProcessingTime = (float)_pTimer.GetTimeElapsed();

	}

	void CFilteredData::IncrementalSelf()
	{
		m_dataProcessingTime = 0.0f;

		
		uint _size  = m_filteredData.size();
		UpdateSelf();
		/*if( _size ==0)
		{
		UpdateSelf();
		return;
		}
		else*/
		
		/*
		{
			PerformanceTimer _pTimer;
			_pTimer.StartTimer();

			m_incremental.dir = GetDirection();
			if(_size>0)
			{
				m_incremental.preStart = (*m_filteredData.begin());
				m_incremental.preEnd = (*(m_filteredData.rbegin()));
			}
			IncrementalSelf(m_incremental.dir); 

			m_dataProcessingTime = (float)_pTimer.GetTimeElapsed();
		}
		*/
	}

	/******************************************************************************************************
	********************HighlightData***********************************************************************
	******************************************************************************************************/
/*
	void CHighlightData::UpdateSelf()
	{
		m_dataProcessingTime = 0.0f;

		if(m_parent.size() == 0)
		{
			m_highlightData.clear();
			return;
		}
		else
		{
			PerformanceTimer _pTimer;
			_pTimer.StartTimer();

			CleanSelf(); 
			FilteredData* parent = (FilteredData*)m_parent[0];
			CDataManager* dm = GetDataManager();//CDataManager::Instance();

			uint _color = 0;
			set<uint>::iterator set_it;
			for(set_it = parent->m_filteredData.begin(); set_it != parent->m_filteredData.end(); set_it++)
			{
				if(dm->SatisfySelectFilter(*set_it,_color))
				{
					bool foundColor = false;
					t_highlightData::iterator it;
					for(it = m_highlightData.begin(); it!= m_highlightData.end(); it++)
					{
						if(it->first == _color)
						{
							foundColor = true;
							it->second.insert(*set_it);
							break;
						}
					}
					if(!foundColor)
					{
						set<uint> _tmp;
						_tmp.insert(*set_it);
						m_highlightData[_color] = _tmp;
					}
				}	
			}
			HighlightNum();
			m_dataProcessingTime = (float)_pTimer.GetTimeElapsed();
		}
	}

	void CHighlightData::CleanSelf()
	{
		t_highlightData::iterator _it;
		for(_it = m_highlightData.begin(); _it != m_highlightData.end(); _it++)
		{
			_it->second.clear();
		}
		m_highlightData.clear();
	}

	void CHighlightData::IncrementalSelf(t_IncrementalType _dir)
	{
		if(_dir == SAME_DIR)
			return;

		FilteredData* _parent  = (FilteredData*)(m_parent[0]);
		CDataManager* dm = GetDataManager();//CDataManager::Instance();

		if(_parent->m_filteredData.size() ==0)
		{
			m_highlightData.clear();
			return;
		}

		uint _color = 0;
		set<uint> &_filterData =_parent->m_filteredData;
		t_FilterIncremental &_inc = _parent->m_incremental;

		if(_dir == OTHERS)
		{
			//erase all the previous data
			t_highlightData::iterator _it;
			for(_it = m_highlightData.begin(); _it!= m_highlightData.end(); _it++)
				_it->second.clear();

			//filling the new data
			set<uint>::iterator _itStart = _filterData.begin();
			set<uint>::iterator _itEnd   = _filterData.end();
			set<uint>::iterator _itFilter;
			for(_itFilter = _itStart; _itFilter != _itEnd; _itFilter++)
			{
				if(dm->SatisfySelectFilter((*_itFilter),_color))
				{
					bool _foundColor = false;
					t_highlightData::iterator _hit;
					for(_hit = m_highlightData.begin(); _hit!= m_highlightData.end(); _hit++)
					{
						if(_hit->first == _color)
						{
							_foundColor = true;
							_hit->second.insert((*_itFilter));
						}
					}
					if(!_foundColor)
					{
						set<uint>  _tmp;
						_tmp.insert((*_itFilter));
						m_highlightData[_color] = _tmp;
					}
				}
			}
		}
		else if(_dir == LARGE_DIR)
		{
			//increase the head
			set<uint>::iterator _itStart = _filterData.begin();
			uint _value = (*_itStart);
			while(_value<_inc.preStart)
			{
				if(dm->SatisfySelectFilter(_value,_color))
				{
					bool _foundColor = false;
					t_highlightData::iterator _hit;
					for(_hit = m_highlightData.begin(); _hit!= m_highlightData.end(); _hit++)
					{
						if(_hit->first == _color)
						{
							_foundColor = true;
							_hit->second.insert(_value);
						}
					}
					if(!_foundColor)
					{
						set<uint>  _tmp;
						_tmp.insert(_value);
						m_highlightData[_color] = _tmp;
					}
				}
				_itStart++;
				_value = (*_itStart);
			}

			//increase the tail
			set<uint>::reverse_iterator _itEnd = _filterData.rbegin();
			_value =(*_itEnd);
			while(_value>_inc.preEnd)
			{
				if(dm->SatisfySelectFilter(_value,_color))
				{
					bool _foundColor = false;
					t_highlightData::iterator _hit;
					for(_hit = m_highlightData.begin(); _hit!= m_highlightData.end(); _hit++)
					{
						if(_hit->first == _color)
						{
							_foundColor = true;
							_hit->second.insert(_value);
						}
					}
					if(!_foundColor)
					{
						set<uint>  _tmp;
						_tmp.insert(_value);
						m_highlightData[_color] = _tmp;
					}
				}
				_itEnd++;
				_value = (*_itEnd);
			}
		}//end of the large
		else if(_dir == SMALL_DIR)
		{
			//decrease the head and tail
			t_highlightData::iterator _it;
			uint _valueBegin  = (*_filterData.begin());
			uint _valueEnd	  = (*_filterData.rbegin());


			for(_it = m_highlightData.begin(); _it!= m_highlightData.end(); _it++)
			{
				set<uint> &_tmp = _it->second;
				set<uint>::iterator _uit;

				if(_tmp.size()>0)
				{
					_uit = _tmp.begin();
					if ((*_uit)<_valueBegin)
					{
						set<uint>::iterator low = _tmp.lower_bound(_valueBegin);
						_tmp.erase(_tmp.begin(),low);
					}
					if(_tmp.size()>0)
					{
						set<uint>::reverse_iterator _reuit = _tmp.rbegin();
						if((*_reuit) > _valueEnd)
						{
							set<uint>::iterator upper = _tmp.upper_bound(_valueEnd);
							_tmp.erase(upper,_tmp.end());
						}
					}
				}
			}
		}//end of the small

		else if(_dir == LEFT_DIR)
		{
			//increase the head
			set<uint>::iterator _itStart = _filterData.begin();
			uint _value = (*_itStart);
			while(_value<_inc.preStart)
			{
				if(dm->SatisfySelectFilter(_value,_color))
				{
					bool _foundColor = false;
					t_highlightData::iterator _hit;
					for(_hit = m_highlightData.begin(); _hit!= m_highlightData.end(); _hit++)
					{
						if(_hit->first == _color)
						{
							_foundColor = true;
							_hit->second.insert(_value);
						}
					}
					if(!_foundColor)
					{
						set<uint>  _tmp;
						_tmp.insert(_value);
						m_highlightData[_color] = _tmp;
					}
				}
				_itStart++;
				_value = (*_itStart);
			}

			//decrease the tail
			t_highlightData::iterator _it;
			uint _valueEnd	  = (*(_filterData.rbegin()));

			for(_it = m_highlightData.begin(); _it!= m_highlightData.end(); _it++)
			{
				set<uint> &_tmp = _it->second;
				if(_tmp.size()>0)
				{
					set<uint>::reverse_iterator _uit = _tmp.rbegin();
					if((*_uit) > _valueEnd)
					{
						set<uint>::iterator set_it = _tmp.upper_bound(_valueEnd);
						_tmp.erase(set_it,_tmp.end());
					}
				}
			}
		}//end of the left

		else if(_dir == RIGHT_DIR)
		{
			//decrease the head
			t_highlightData::iterator _it;
			uint _valuesStart	  = (*_filterData.begin());

			for(_it = m_highlightData.begin(); _it!= m_highlightData.end(); _it++)
			{
				set<uint> &_tmp = _it->second;
				if(_tmp.size()>0)
				{
					set<uint>::iterator _uit = _tmp.begin();
					if((*_uit) < _valuesStart)
					{
						set<uint>::iterator low = _tmp.lower_bound(_valuesStart);
						_tmp.erase(_tmp.begin(),low);
					}
				}
			}

			//increase the tail

			set<uint>::reverse_iterator _itEnd = _filterData.rbegin();
			uint _value = (*_itEnd);
			while(_value>_inc.preEnd)
			{
				if(dm->SatisfySelectFilter(_value,_color))
				{
					bool _foundColor = false;
					t_highlightData::iterator _hit;
					for(_hit = m_highlightData.begin(); _hit!= m_highlightData.end(); _hit++)
					{
						if(_hit->first == _color)
						{
							_foundColor = true;
							_hit->second.insert(_value);
						}
					}
					if(!_foundColor)
					{
						set<uint>  _tmp;
						_tmp.insert(_value);
						m_highlightData[_color] = _tmp;
					}
				}
				_itEnd++;
				_value = (*_itEnd);
			}
		}//end of the right
	}

	void CHighlightData::IncrementalSelf()
	{
		m_dataProcessingTime = 0.0f;

		if(m_parent.size() == 0)
		{
			m_highlightData.clear();
			return;
		}
		else
		{
			FilteredData* _parent  = (FilteredData*)(m_parent[0]);
			if(_parent->m_filteredData.size() ==0)
			{
				m_highlightData.clear();
				HighlightNum();
				return;
			}

			PerformanceTimer _pTimer;
			_pTimer.StartTimer();
			//UpdateSelf();
			IncrementalSelf(_parent->m_incremental.dir);
			HighlightNum();
			m_dataProcessingTime = (float)_pTimer.GetTimeElapsed();

		}
	}

	void CHighlightData::HighlightNum()
	{
		CDataManager* dm = GetDataManager();

		//clear the previous highlight information 
		if(dm->m_NumFilteredRecord.size()>0)
		{
			map<uint, uint>::iterator filterRecord = dm->m_NumFilteredRecord.begin();
			for(filterRecord = dm->m_NumFilteredRecord.begin();filterRecord != dm->m_NumFilteredRecord.end(); filterRecord++)
				filterRecord->second = 0;
		}

		t_highlightData::iterator it;
		for(it = m_highlightData.begin(); it != m_highlightData.end(); it++)
		{
			dm->m_NumFilteredRecord[it->first] = it->second.size();
		}
	}
	*/
}