#pragma once
#include "header.h"
#include "Data.h"

namespace VIS
{
	class CCurTime : public CAbstractData
	{
	public :
		CCurTime();
		~CCurTime(){CleanSelf();}
		void SetCurTime(time_t _time) { m_CurTime = _time;}
		time_t GetCurTime(){ return m_CurTime; }
	private:
		time_t m_CurTime;
		void UpdateSelf(){};
		void CleanSelf(){};
	};

	class CTimeRange : public CAbstractData
	{
	public :
		CTimeRange();
		~CTimeRange(){CleanSelf();}
		void SetTimeRange(unsigned int _range){ m_TimeRange = _range;}
		unsigned int GetTimeRange(){ return m_TimeRange; }
	private:
		unsigned int m_TimeRange;
		void UpdateSelf(){};
		void CleanSelf(){};
	};

	class CTimeWindow : public CAbstractData
	{
	public:
		CTimeWindow(void);
		~CTimeWindow(void){CleanSelf();}
		unsigned int GetNumberofQueriedRecords() { return m_EndIdx - m_StartIdx + 1; }  
	public:
		unsigned int m_StartIdx;
		unsigned int m_EndIdx;
		t_TimeStamps m_TimeStamps;
		t_TimeStamps m_UniqueTimeStamps;
	
	private:
		void UpdateSelf();
		void CleanSelf(){m_TimeStamps.clear(); m_UniqueTimeStamps.clear();};
	};

	class CTimeStep : public CAbstractData
	{
	public:
		CTimeStep();
		~CTimeStep(){CleanSelf();}
		void SetTimeStep(unsigned int _step){m_TimeStep = _step;}
		unsigned int GetTimeStep(){ return m_TimeStep;}
	private:
		unsigned int m_TimeStep;
		void UpdateSelf(){};
		void CleanSelf(){};
	};
}