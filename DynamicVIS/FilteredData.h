#pragma once

namespace VIS
{
typedef struct
{
	unsigned int preStart;
	unsigned int preEnd;
	t_IncrementalType dir;
}t_FilterIncremental;

	class CAbstractData;
	
	class CFilteredData : public CAbstractData
	{
	private:
		void UpdateSelf();
		void IncrementalSelf(t_IncrementalType);
		void IncrementalSelf();
		void CleanSelf();
		t_IncrementalType GetDirection();

	public:
		set<unsigned int> m_filteredData; //has the indices for the records in the raw data
		t_FilterIncremental m_incremental;

		CFilteredData();
		~CFilteredData(){CleanSelf();}
	};

}