#pragma once
#include "Data.h"

namespace VIS
{
	class CVBOData;

	class CHistogramIData : public CAbstractData
	{
	public:
		static unsigned int s_maxValue;
		CHistogramIData() : m_item(1),m_histogram(NULL),m_index(NULL),m_reference(NULL), m_order(NULL),m_numHistogram(0) {}
		~CHistogramIData() {CleanSelf();}
		void UpdateSelf();
		void IncrementalSelf();
		void CleanSelf();
		void SetItem(unsigned int _item){ m_item = _item;}
		unsigned int GetItem(){return m_item;}
	public:
		unsigned int* m_histogram;
		unsigned int* m_index;
		unsigned int* m_reference;
		unsigned int* m_order;
		unsigned int  m_numHistogram;
	private:		
		unsigned int m_item;
	};

	class CHistogramRData : public CAbstractData
	{
	public:
		CHistogramRData(CCanvas* _canvas);
		~CHistogramRData(); 
		void UpdateSelf();
		void IncrementalSelf();
		void CleanSelf();
		void SetLogScale(bool _logScale){m_if_logScale = _logScale;}
		void SetVerticalScale(float _scale){m_verticalScale = _scale;}
	public:
		CVBOData *m_vbo;
		unsigned int *m_acuHeight;
		bool m_if_logScale;
	private:
		float m_verticalScale;

		void PrepareVBO( unsigned int *_preacc, unsigned int *_cur, unsigned int _size, unsigned int _maxValue);
	};

}