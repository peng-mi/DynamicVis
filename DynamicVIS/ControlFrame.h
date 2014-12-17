#pragma once

#include "wx/wx.h"
#include "wx/progdlg.h"
#include <wx/timer.h>
#include <vector>
#include "header.h"
using namespace std;

class wxNotebook;
class wxListBox;
class wxPanel;
class wxBoxSizer;
class wxComboBox;
class wxDatePickerCtrl;
class wxColourData;

namespace VIS
{
	class CFrame;
	class CDataManager;
	class CFilter;
	class CControlFrame : public wxFrame
	{
	public:
		static CControlFrame* Create(t_configPara& _cfg, long style = wxDEFAULT_FRAME_STYLE);

		static vector<CControlFrame*> s_ControlFrameList;

		CControlFrame( const wxString& title, const wxPoint& pos,
			const wxSize& size, vector<t_FrameInfo> _frames, long style = wxDEFAULT_FRAME_STYLE);

		void OnQuit(wxCommandEvent &WXUNUSED(event));
		void OnLoadData(wxCommandEvent &WXUNUSED(event));
		void OnClose( wxCloseEvent &event );
		void OnIdle( wxIdleEvent &event );
		void OnTimeSliderScroll( wxScrollEvent &event );
		void OnTimeSliderScrollBottom( wxScrollEvent &event );
		void OnAnimate(wxCommandEvent& event);
		void OnRangeRatioSelect(wxCommandEvent& event );
		void OnRangeSliderScroll( wxScrollEvent &event );
		void OnAnimationSpeedSelect(wxCommandEvent& event );
		void OnTimeStepSelect(wxCommandEvent& event );
		void OnSkipCheck( wxCommandEvent& event );

		void OnHistogram(wxCommandEvent &WXUNUSED(event));
		void OnParaCoord(wxCommandEvent &WXUNUSED(event));
		void OnDynamic(wxCommandEvent &WXUNUSED(event));
		void OnLog(wxCommandEvent &WXUNUSED(event));
		void OnGraph(wxCommandEvent &WXUNUSED(event));

		void OnFilterSelection(wxListEvent& event);
		void OnClearThisFilterSelection(wxCommandEvent &event);
		void OnClearAllFilterSelection(wxCommandEvent &event);
		void OnFilterSelectionEX(wxListEvent& event);
		void OnClearThisFilterSelectionEX(wxCommandEvent &event);
		void OnClearAllFilterSelectionEX(wxCommandEvent &event);
		void OnFilterSelectionNEX(wxListEvent& event);
		void OnClearThisFilterSelectionNEX(wxCommandEvent &event);
		void OnClearAllFilterSelectionNEX(wxCommandEvent &event);

		void OnBrushOne(wxCommandEvent& event);
		void OnBrushAnd(wxCommandEvent& event);
		void OnBrushOR(wxCommandEvent& event);
		void OnBrushOneEX(wxCommandEvent& event);
		void OnBrushAndEX(wxCommandEvent& event);
		void OnBrushOREX(wxCommandEvent& event);
		void OnBrushNeOneEX(wxCommandEvent& event);
		void OnBrushNeAndEX(wxCommandEvent& event);
		void OnBrushNeOREX(wxCommandEvent& event);
		void OnColor(wxCommandEvent& event);

		void OnQueryLoadButton(wxCommandEvent& event);
		void OnQueryLoadButtonEX(wxCommandEvent& event);
		void OnQuerySaveButton(wxCommandEvent& event);
		void OnQuerySaveButtonEX(wxCommandEvent& event);
		void OnQueryAppendButton(wxCommandEvent& event);
		void OnQueryAppendButtonEX(wxCommandEvent& event);

		void OnWorkerEvent(wxCommandEvent& event);
		void OnTimer(wxTimerEvent& event);

		void SetCurrentTime( time_t _curtime );
		void RefreshAll();
		void UpdateDataForTimeChange();
		void UpdateDateTime();
		void UpdateFilter();
		void UpdateBrush(unsigned int _itemidx, unsigned int _valueidx);
		CDataManager* GetDataManager(){return m_datamanager;}
		void LoadDataProgress(char* _name);


		bool m_if_animate;
		wxProgressDialog *m_dlgProgress;
		vector<CFrame*> m_frames;

		wxPanel *m_panel;
		wxBoxSizer* m_sizer;
		wxNotebook *m_pNotebook;
		wxToolBar *m_brushToolBar;
		wxAuiManager m_mgr;
		wxColourData m_ColorData;
		wxMenu *m_menuViews;
		wxMenuBar *m_menu_bar;
		wxMenuItem	*m_syncmenu;

		//animation panel
		wxSlider	*m_timeStartSlider, *m_timeRangeSlider;
		wxDatePickerCtrl *m_DataPicker;
		wxComboBox *m_hourCombo, *m_minuteCombo, *m_secondCombo;
		wxCheckBox	*m_skipCheckBox;
		wxComboBox	*m_TimeStepCombo;
		wxComboBox	*m_rangeRatioCombo;
		wxTextCtrl *m_timeRangeText, *m_recordNumberText, *m_selectedValue;
		wxButton	*m_animateButton;
		wxComboBox	*m_SpeedCombo;

		//selection and exclusion panel
		wxListCtrl  *m_SelectionCtrl, *m_SelectionCtrlEX,*m_SelectionCtrlNegEX;
		wxButton	*m_FilterClearAllButton,*m_FilterClearThisButton,*m_FilterClearAllButtonEX,*m_FilterClearThisButtonEX, *m_FilterClearAllButtonNEX,*m_FilterClearThisButtonNEX;
		wxButton *m_query_save;
		wxButton *m_query_load;
		wxButton *m_query_append;
		wxButton *m_query_saveEx;
		wxButton *m_query_loadEx;
		wxButton *m_query_appendEx;
		wxButton *m_query_saveNEx;
		wxButton *m_query_loadNEx;
		wxButton *m_query_appendNEx;

	private :
		void CreateToolBar();
		void ResetTimeSlider();
		void SetColor(unsigned char _red, unsigned char _green, unsigned char _blue, unsigned char _alpha);
		void DataRefresh();
		void OnSyncAllFrames( wxCommandEvent &event);
		void EraseFilter(CFilter* _filter);


		CDataManager *m_datamanager;
		bool m_loadingData;
		wxTimer m_timer;
		unsigned int m_accum_millisec;
		clock_t m_pre_animatetime;
		float m_invfps;

		DECLARE_EVENT_TABLE()
	};


}