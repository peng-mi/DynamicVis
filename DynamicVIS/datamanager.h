#ifndef _DATAMANAGER_H_
#define _DATAMANAGER_H_
#include "header.h"
#include "TimeData.h"
namespace VIS
{
	class CRawData;
	class CFilter;
	class CFilteredData;
	class CAbstractData;


	class CDataManager
	{
	public:
		~CDataManager();
		CDataManager();


		void LoadData(char *_filename );
		void ProcessFliter(uint id);
		CFilter* ProcessBrush( unsigned int _itemidx, unsigned int _valueidx );
		//void SelectTimeSpanIndex();
		bool SatisfyFilter( uint _recordIdx, t_FilterSet &_filterSet );
		//unsigned int GetNumberofQueriedRecords() { return m_EndIdx - m_StartIdx + 1; } 
		bool LoadQuery(char *filename,bool kind_load, bool kind_selection);//kind is true when load, kind is false when append
		void CleanDataLists();
		t_color GetColor(CFilter* _filter);

	public:
		bool m_ifDataReady;

		//t_TimeStamps m_UniqueTimeStamps;
		time_t m_MinTime, m_MaxTime;
		//time_t m_CurTime;
		//uint m_TimeRange;
		uint m_CurrentTimesliderIdx;
		bool m_ifSkip;

		CRawData *m_RawData;
		vector<CFilteredData*> m_FilteredDataList;
		CFilteredData* m_BasedFilteredData;


		vector<CFilter*> m_Filters; // First filter is ExcludeFilter, second one is NegExcludeFilter, the rest are select filters
		CFilter* m_ExclusiveFilter;
		CFilter* m_NegExclusiveFilter;

		t_color m_color;
		void SetColor(unsigned char _red, unsigned char _green, unsigned char _blue, unsigned char _alpha)
		{ m_color.red = _red; m_color.green = _green; m_color.blue = _blue; m_color.alpha = _alpha;}
		bool IsSameColor(t_color& _color1, t_color& _color2);

		//time data
		CCurTime* m_CurTimeData;
		CTimeRange* m_TimeRangeData;
		CTimeWindow* m_TimeWindowData;
		CTimeStep* m_TimeStepData;

		//animation panel
		float m_RangeRatio;

		t_BrushType m_BrushType;
		t_DataUpdateStyle m_dataUpdateStyle;
		t_FilterType m_filterType;

		//data
		//unsigned int m_StartIdx, m_EndIdx;
		//map<uint, uint> m_NumFilteredRecord;
		vector<pair<t_color, CFilter*>> m_colorTable;

		int m_CurrentFilter;
		int m_CurrentFilterEX;
		int m_CurrentFilterNEX;

		//t_TimeStamps m_TimeStamps;
		//Intermediate Data
		vector<CAbstractData*> m_HighlightIDataList;
		vector<CAbstractData*> m_BasedIDataList;

	private:
		void Load_Item_Desc_File(char *_filename);
		bool ColorConflict(unsigned int _itemidx, unsigned int _valueidx, t_color &_color);
		void LoadHistoryFilter(char* buf, bool kind); //kind is true for selection and false for exclusion.
	};

}

#endif