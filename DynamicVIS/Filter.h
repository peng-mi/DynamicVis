#pragma once
#include "header.h"
#include "Data.h"

namespace VIS
{
//class CAbstractData;
typedef enum{
	ONE = 0,
	AND,
	OR,
	OTHER
} t_LogicType; 

class CFilter :	public CAbstractData
{
private:
	t_LogicType		m_LogicType;
	int	m_ItemIdx;
	int	m_ValueIdx;
	int	m_CurrentAndFilterIdx;

public:
	CFilter();
	~CFilter() {CleanSelf();}
	t_FilterSet m_Filter;
	void SetLogicType (t_LogicType _lt) { m_LogicType = _lt; }
	void SetIndex (unsigned int _itemidx, unsigned int _valueidx )
	{
		m_ItemIdx = _itemidx;
		m_ValueIdx = _valueidx;
	}
	void SetCurrentAndFilterIdx( unsigned int _idx ) { m_CurrentAndFilterIdx = _idx; }
	unsigned int GetCurrentAndFilterIdx() { return m_CurrentAndFilterIdx;}

	void UpdateSelf();
	void CleanSelf(){ m_Filter.clear(); }
};

}