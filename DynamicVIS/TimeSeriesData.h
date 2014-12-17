#pragma once
#include "Data.h"

namespace VIS
{
class CVBOData;

class CTimeSeriesIData : public CAbstractData
{
public:
	CTimeSeriesIData(void);
	~CTimeSeriesIData(void) {CleanSelf();}
	void UpdateSelf();
	void CleanSelf();

	int m_CurrentBuildKey;
	int m_CurrentItem;

	unsigned int m_numTSs;
	unsigned int m_numPoints;

	float *m_data;
	bool *m_ifHasData;
private:
	void PrepareData();
};

class CTimeSeriesIIndicesData : public CAbstractData
{
public:
	CTimeSeriesIIndicesData(void){}
	~CTimeSeriesIIndicesData(void) {CleanSelf();}
	void UpdateSelf();
	void CleanSelf(){m_TSIndices.clear();}

	vector<unsigned int> m_TSIndices;
private:
	void PrepareData();
};

class CTimeSeriesRData : public CAbstractData
{
public:
	CTimeSeriesRData(CCanvas* _canvas);
	~CTimeSeriesRData(void);

	void UpdateSelf();
	void CleanSelf();

	void SetLogScale ( bool _value ) { m_if_logScale = _value; }

	CVBOData *m_vbo;

	bool m_if_logScale;
private:
	void PrepareData();
};

class CTimeSeriesRColorData : public CAbstractData
{
public:
	CTimeSeriesRColorData() : m_ColorIndices(NULL), m_numTSs(0) {}
	~CTimeSeriesRColorData(void) {CleanSelf();}

	void UpdateSelf();
	void CleanSelf();

	unsigned int *m_ColorIndices;
	unsigned int m_numTSs;
private:
	void PrepareData();
};

}