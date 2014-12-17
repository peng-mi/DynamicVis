#include "TimeData.h"
namespace VIS
{
	CCurTime::CCurTime() : m_CurTime(0)
	{
	}

	CTimeRange::CTimeRange() : m_TimeRange(0)
	{
	}

	CTimeStep::CTimeStep() :  m_TimeStep(0)
	{
	}

	CTimeWindow::CTimeWindow(void) :  m_StartIdx(0), m_EndIdx(0)
	{
	}

	void CTimeWindow::UpdateSelf()
	{
		CCurTime* curtime_ptr = NULL;
		CTimeRange* timerange_ptr = NULL;
		t_TimeStamps::iterator low, up;
		
		curtime_ptr = dynamic_cast<CCurTime*>(m_parent[0]);
		if( curtime_ptr != NULL)
			timerange_ptr = dynamic_cast<CTimeRange*>(m_parent[1]);
		else
		{
			curtime_ptr = dynamic_cast<CCurTime*>(m_parent[1]);
			timerange_ptr = dynamic_cast<CTimeRange*>(m_parent[0]);
		}
	
		low = std::lower_bound (m_TimeStamps.begin(), m_TimeStamps.end(), curtime_ptr->GetCurTime());
		up = std::upper_bound (low, m_TimeStamps.end(), curtime_ptr->GetCurTime() + timerange_ptr->GetTimeRange());
		m_StartIdx = low - m_TimeStamps.begin();
		m_EndIdx = up - m_TimeStamps.begin();
		m_EndIdx--;
	}
}
