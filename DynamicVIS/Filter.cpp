#include "Filter.h"
#include "Data.h"

namespace VIS
{

CFilter::CFilter() : m_LogicType(OTHER), m_ItemIdx(-1), m_ValueIdx(-1), m_CurrentAndFilterIdx(-1)
{
}

void CFilter::UpdateSelf(void)
{
	if ( m_LogicType == OTHER || m_ItemIdx == -1 || m_ValueIdx == -1 )
		return;

	switch (m_LogicType)
	{
	case ONE:
		{
			m_Filter.clear();
			t_AndFilterSet filterset;
			filterset.insert( pair<unsigned int, unsigned int> (m_ItemIdx, m_ValueIdx ) );
			m_Filter.push_back(filterset);
		}
		break;
	case AND:
		if ( m_CurrentAndFilterIdx < 0 || m_Filter.size() == 0 )
		{
			t_AndFilterSet filterset;
			filterset.insert( pair<unsigned int, unsigned int> (m_ItemIdx, m_ValueIdx ) );
			m_Filter.push_back(filterset);
		}
		else
		{
			t_AndFilterSet &curFilter = m_Filter[m_CurrentAndFilterIdx];
			t_AndFilterSet::iterator it;
			for ( it = curFilter.begin(); it != curFilter.end(); it++ )
			{
				if ( m_ItemIdx == (*it).first && m_ValueIdx != (*it).second )
					return;
			}
			pair<t_AndFilterSet::iterator, bool> ret;

			ret = curFilter.insert( pair<unsigned int, unsigned int> (m_ItemIdx, m_ValueIdx ) );
			if ( ret.second == false )
			{
				curFilter.erase( ret.first );
				if ( curFilter.size() == 0 )
					m_Filter.pop_back();
				m_CurrentAndFilterIdx = m_Filter.size() - 1;
			}
		}
		break;
	case OR:
		bool found = false;
		for ( t_FilterSet::iterator it = m_Filter.begin(); it != m_Filter.end(); it++ )
		{
			if ( (*it).size() == 1 && (*it).begin()->first == m_ItemIdx && (*it).begin()->second == m_ValueIdx )
			{
				m_Filter.erase( it );
				found = true;
				break;
			}
		}
		if ( !found )
		{
			t_AndFilterSet filterset;
			filterset.insert( pair<unsigned int, unsigned int> (m_ItemIdx, m_ValueIdx ) );
			m_Filter.push_back(filterset);
		}
		m_CurrentAndFilterIdx = m_Filter.size() - 1;
		break;
	}
	m_ItemIdx = -1;
	m_ValueIdx = -1;
}


}