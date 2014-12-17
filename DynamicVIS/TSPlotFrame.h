#pragma once
#include "Frame.h"
#include "TimeSeriesData.h"

namespace VIS
{

class CTimeSeriesPlotFrame: public CFrame
{
	friend class CTimeSeriesPlotCanvas;

public:
	static CTimeSeriesPlotFrame *Create(CControlFrame* _cf, const wxString& title, const wxPoint& pos,
        const wxSize& size, long style = wxDEFAULT_FRAME_STYLE );

    CTimeSeriesPlotFrame(CControlFrame* _cf, const wxString& title, const wxPoint& pos,
        const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);
	~CTimeSeriesPlotFrame(){};

	void OnTSPlotBuildKeySelect(wxListEvent &event);
	void OnTSPlotItemSelect(wxListEvent &event);
	void OnBuildTimeSeries(wxCommandEvent& event);
	void OnTimeSeriesPlotLogScaleCheck( wxCommandEvent& event );

    CTimeSeriesPlotCanvas *GetCanvas() { return (CTimeSeriesPlotCanvas*)m_canvas; }

	void RefleshPanel();
	void CreateDataForNewFilter();

private:
	wxListCtrl	*m_TSPlotBuildKeySelectBox;
	wxListCtrl	*m_TSPlotItemSelectBox;
	wxButton	*m_buildTS_Button;
	wxCheckBox	*m_buildAllcheckbox;
	wxCheckBox	*m_timeseriesplot_scalecheckbox;

    DECLARE_EVENT_TABLE()
};

class CTimeSeriesPlotCanvas: public CCanvas
{
    friend class CTimeSeriesPlotFrame;
public:
    CTimeSeriesPlotCanvas( wxWindow *parent, wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0, const wxString& name = _T("CTimeSeriesPlotCanvas") );

	~CTimeSeriesPlotCanvas(){};
	void SetLogScale ( bool _value );
private:
	void InitOrtho()
	{
		m_Ortho[0] = -0.1f;
		m_Ortho[1] = 1.06f;
		m_Ortho[2] = -0.1f;
		m_Ortho[3] = 1.06f;
		m_Ortho[4] = -1.0f;
		m_Ortho[5] = 1.0f;
	}


	void DrawAxis();
	void DrawCanvas();
	void SelectData();
	void InitGL();

	void RenderVBO();
	void DrawIndication();

	void SetShader();

	DECLARE_EVENT_TABLE()
};

}
