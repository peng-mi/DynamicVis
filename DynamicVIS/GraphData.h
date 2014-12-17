#pragma once
#include "header.h"
#include "Data.h"
#include "VBOData.h"
namespace VIS
{
	class CGraphIData : public CAbstractData
	{
		public:
			CGraphIData();
			~CGraphIData(){CleanSelf();}
			void UpdateSelf();
			void Init();
			void IncrementalSelf();
			void CleanSelf();
			void NodeMerge();
			void CreateGraph();
			void GraphLayout();
			void SetDataRange(bool _total,bool _merge)
			{ 
				m_data.graphInfo.is_total = _total;
				m_data.graphInfo.is_merge = _merge;
			}
			void SetItems(uint _item[])
			{
				m_data.graphInfo.items[0] = _item[0];
				m_data.graphInfo.items[1] = _item[1];
			}
		public:
			t_Graph		m_data;
		
		private:
			void PrepareRecords();
			void UpdateVector(unsigned int _index);
			void PrepareGraph();
			void PrepareMergeGraph();
 	};

	class CGraphRData : public CAbstractData
	{
		public:
			CGraphRData(CCanvas* _canvas);
			~CGraphRData();
			void UpdateSelf();
			void IncrementalSelf();
			void CleanSelf();
		public:
			//t_HighlightGraph m_data;
			CVBOData *m_vbo;

		private:
			void PrepareVBO();

	};

}

