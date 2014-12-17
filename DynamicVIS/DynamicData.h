#pragma once
#include "Data.h"

namespace VIS
{
	class CVBOData;

	class CDynamicIData : public CAbstractData
	{
	public:
		CDynamicIData() : m_data(NULL), m_size(0), m_Item(-1),m_SpecialItem(-1),m_numParents(0){}
		~CDynamicIData() {CleanSelf();}
		void UpdateSelf();
		void IncrementalSelf();
		void CleanSelf();
		void AllocateSelf();
	public:
		float *m_data;
		unsigned int m_size;
		static float s_maxvalue;
		static float s_minvalue;
		int m_Item;
		int m_SpecialItem; 
	private:
		void PrepareData();
		unsigned int m_numParents;
	};

	class CDynamicRData : public CAbstractData
	{
	public:
		CDynamicRData(CCanvas* _canvas);
		~CDynamicRData();
		void UpdateSelf();
		void IncrementalSelf();
		void CleanSelf();
		void AllocateSelf();
		void SetLogScale(bool _log){m_if_logScale = _log;}
	public:
		float *m_acuHeight;
		bool  *m_if_defalut_value;
		CVBOData* m_vbo;
		bool	m_if_logScale;
	private:
		CCanvas* m_canvas;
		unsigned int m_numParents;
		void PrepareVBO(float *_preacc, float *_cur, unsigned int _size, float _maxValue, float _minValue, bool _isNumerical);
		void PrepareData();
	};
}
