#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/notebook.h>
#include <wx/listbox.h>
#include <wx/datectrl.h>
#include <wx/combo.h>
#include "wx/aui/aui.h"
#include "wx/cmndata.h"
#include "wx/colordlg.h"
#include "wx/datetime.h"      // wxDateTime
#include "wx/progdlg.h"
#include <wx/listctrl.h>
/************************basic********************/
#include "ControlFrame.h"
#include "Frame.h"
#include "datamanager.h"
#include "Filter.h"
#include "FilteredData.h"
#include "RawData.h"
#include "color.h"
#include "TimeData.h"
#include "LogFile.h"
/************************frames********************/
#include "logframe.h"
#include "HistogramFrame.h"
#include "ParaCoordFrame.h"
#include "DynamicFrame.h"
#include "GraphFrame.h"
#include "TSPlotFrame.h"
/************wxProgressWindows*****************/
#include "wxThread.h"
#include "wx/wxprec.h"
#include "wx/thread.h"
#include "wx/dynarray.h"
#include "wx/numdlg.h"
#include "wx/progdlg.h"

#include <windows.h>
#include <iostream>
#include <sstream>

#define DBOUT( s )            \
{                             \
	std::wostringstream os_;    \
	os_ << s;                   \
	OutputDebugStringW( os_.str().c_str() );  \
}

namespace VIS
{
	static const std::string g_Frames[6] ={"HistogramFrame","ParaCoordFrame","DynamicFrame","logFrame","GraphFrame","TimeSeriesPlotFrame"};

	enum
	{
		/******Menu File************/
		ID_QUIT  = wxID_EXIT,
		ID_LOADDATA = 101,
		ID_SAVEPROJECT,
		ID_SYNCFRAMES,

		/****frameId******/
		ID_HISTOGRAM = HISTOGRAM_ID,
		ID_PARACOORD,
		ID_DYNAMIC,
		ID_LOG,
		ID_GRAPH,
		ID_TIMESERIES,

		/******Brushing************/
		ID_Brush_One,
		ID_Brush_And,
		ID_Brush_OR,
		ID_Brush_One_EX,
		ID_Brush_And_EX,
		ID_Brush_OR_EX,
		ID_Brush_Ne_One_EX,
		ID_Brush_Ne_And_EX,
		ID_Brush_Ne_OR_EX,
		ID_Brush_Toolbar,
		ID_Color,

		ID_Control_TimeSlider,
		ID_Control_HourControl,
		ID_Control_MinuteControl,
		ID_Control_SecondControl,
		ID_TimeStepCombo,
		ID_Skip_CheckBox,
		ID_Range_Ratio,
		ID_Control_RangeSlider,
		ID_Control_Animate,
		ID_Speed_Combo,

		ID_Filter_Selection,
		ID_Filter_ClearThis,
		ID_Filter_ClearAll,
		ID_Query_Save,
		ID_Query_Load,
		ID_Query_Append,
		ID_Filter_SelectionEX,
		ID_Filter_ClearThisEX,
		ID_Filter_ClearAllEX,
		ID_Query_SaveEX,
		ID_Query_LoadEX,
		ID_Query_AppendEX,

		ID_Filter_SelectionNEX,
		ID_Filter_ClearThisNEX,
		ID_Filter_ClearAllNEX,
		ID_Query_SaveNEX,
		ID_Query_LoadNEX,
		ID_Query_AppendNEX,

		TIMER_ID,
	};

	//table event
	BEGIN_EVENT_TABLE(CControlFrame,wxFrame)
		EVT_IDLE    (CControlFrame::OnIdle)
		EVT_MENU    (ID_QUIT,  CControlFrame::OnQuit)
		EVT_MENU    (ID_LOADDATA,  CControlFrame::OnLoadData)

		EVT_MENU    (ID_HISTOGRAM,  CControlFrame::OnHistogram)
		EVT_MENU    (ID_PARACOORD,  CControlFrame::OnParaCoord)
		EVT_MENU    (ID_DYNAMIC,  CControlFrame::OnDynamic)
		EVT_MENU    (ID_LOG,  CControlFrame::OnLog)
		EVT_MENU    (ID_GRAPH,  CControlFrame::OnGraph)

		EVT_MENU    (ID_SYNCFRAMES,  CControlFrame::OnSyncAllFrames)

		EVT_MENU	(WORKER_EVENT, CControlFrame::OnWorkerEvent)
		EVT_TIMER	(TIMER_ID, CControlFrame::OnTimer)

		EVT_CLOSE	(CControlFrame::OnClose)

		EVT_BUTTON ( ID_Control_Animate, CControlFrame::OnAnimate )
		EVT_COMBOBOX( ID_Range_Ratio, CControlFrame::OnRangeRatioSelect)
		EVT_COMMAND_SCROLL  (ID_Control_RangeSlider, CControlFrame::OnRangeSliderScroll)
		EVT_COMMAND_SCROLL  (ID_Control_TimeSlider, CControlFrame::OnTimeSliderScroll)
		EVT_COMMAND_SCROLL_BOTTOM(ID_Control_TimeSlider, CControlFrame::OnTimeSliderScrollBottom) 
		EVT_COMBOBOX( ID_Speed_Combo, CControlFrame::OnAnimationSpeedSelect)
		EVT_COMBOBOX( ID_TimeStepCombo, CControlFrame::OnTimeStepSelect)
		EVT_CHECKBOX( ID_Skip_CheckBox, CControlFrame::OnSkipCheck)

		EVT_LIST_ITEM_SELECTED (ID_Filter_Selection, CControlFrame::OnFilterSelection)
		EVT_BUTTON ( ID_Filter_ClearThis, CControlFrame::OnClearThisFilterSelection )
		EVT_BUTTON ( ID_Filter_ClearAll, CControlFrame::OnClearAllFilterSelection )
		EVT_LIST_ITEM_SELECTED (ID_Filter_SelectionEX, CControlFrame::OnFilterSelectionEX )
		EVT_BUTTON ( ID_Filter_ClearThisEX, CControlFrame::OnClearThisFilterSelectionEX )
		EVT_BUTTON ( ID_Filter_ClearAllEX, CControlFrame::OnClearAllFilterSelectionEX )
		EVT_LIST_ITEM_SELECTED (ID_Filter_SelectionNEX, CControlFrame::OnFilterSelectionNEX )
		EVT_BUTTON ( ID_Filter_ClearThisNEX, CControlFrame::OnClearThisFilterSelectionNEX )
		EVT_BUTTON ( ID_Filter_ClearAllNEX, CControlFrame::OnClearAllFilterSelectionNEX )

		EVT_BUTTON ( ID_Query_Load, CControlFrame::OnQueryLoadButton )
		EVT_BUTTON ( ID_Query_LoadEX, CControlFrame::OnQueryLoadButtonEX )
		EVT_BUTTON ( ID_Query_Save, CControlFrame::OnQuerySaveButton )
		EVT_BUTTON ( ID_Query_SaveEX, CControlFrame::OnQuerySaveButtonEX )
		EVT_BUTTON ( ID_Query_Append, CControlFrame::OnQueryAppendButton )
		EVT_BUTTON ( ID_Query_AppendEX, CControlFrame::OnQueryAppendButtonEX )


		EVT_TOOL ( ID_Brush_One, CControlFrame::OnBrushOne )
		EVT_TOOL ( ID_Brush_And, CControlFrame::OnBrushAnd )
		EVT_TOOL ( ID_Brush_OR, CControlFrame::OnBrushOR )
		EVT_TOOL ( ID_Brush_One_EX, CControlFrame::OnBrushOneEX )
		EVT_TOOL ( ID_Brush_And_EX, CControlFrame::OnBrushAndEX )
		EVT_TOOL ( ID_Brush_OR_EX, CControlFrame::OnBrushOREX )
		EVT_TOOL ( ID_Brush_Ne_One_EX, CControlFrame::OnBrushNeOneEX )
		EVT_TOOL ( ID_Brush_Ne_And_EX, CControlFrame::OnBrushNeAndEX )
		EVT_TOOL ( ID_Brush_Ne_OR_EX, CControlFrame::OnBrushNeOREX )
		EVT_TOOL ( ID_Color, CControlFrame::OnColor )

		END_EVENT_TABLE()

	vector<CControlFrame*> CControlFrame::s_ControlFrameList;

	CControlFrame* CControlFrame::Create(t_configPara& _cfg, long _style)
	{
		//CControlFrame* controlframe = new CControlFrame(_title, _pos, _size, _frames, _style);
		CControlFrame* controlframe = new CControlFrame(_cfg.title, wxPoint(_cfg.pos[0],_cfg.pos[1]), wxSize(_cfg.size[0],_cfg.size[1]), _cfg.subframes, _style);

		controlframe->m_datamanager = new CDataManager();
		controlframe->m_datamanager->m_TimeStepData->SetTimeStep(_cfg.time_step);
		controlframe->m_datamanager->m_TimeRangeData->SetTimeRange(_cfg.range_step);
		CLogFile::GetLogFile();
		//create frames
		CFrame* frame;
		vector<t_FrameInfo>& _frames = _cfg.subframes;
		for(unsigned int i=0; i< _frames.size(); i++)
		{
			if(_frames[i].name == g_Frames[0])
				frame = CHistogramFrame::Create(controlframe,_frames[i].name, wxPoint(_frames[i].pos[0], _frames[i].pos[1]), wxSize(_frames[i].size[0],_frames[i].size[1]));
			else if(_frames[i].name == g_Frames[1])
				frame = CParaCoordFrame::Create(controlframe,_frames[i].name, wxPoint(_frames[i].pos[0], _frames[i].pos[1]), wxSize(_frames[i].size[0],_frames[i].size[1]));
			else if(_frames[i].name == g_Frames[2])
				frame = CDynamicFrame::Create(controlframe,_frames[i].name, wxPoint(_frames[i].pos[0], _frames[i].pos[1]), wxSize(_frames[i].size[0],_frames[i].size[1]));
			else if(_frames[i].name == g_Frames[3])
				frame = CLOGFrame::Create(controlframe,_frames[i].name, wxPoint(_frames[i].pos[0], _frames[i].pos[1]), wxSize(_frames[i].size[0],_frames[i].size[1]));
			else if(_frames[i].name == g_Frames[4])
				frame = CGraphFrame::Create(controlframe,_frames[i].name, wxPoint(_frames[i].pos[0], _frames[i].pos[1]), wxSize(_frames[i].size[0],_frames[i].size[1]));
			else if(_frames[i].name == g_Frames[5])
				frame = CTimeSeriesPlotFrame::Create(controlframe,_frames[i].name, wxPoint(_frames[i].pos[0], _frames[i].pos[1]), wxSize(_frames[i].size[0],_frames[i].size[1]));
			else
				frame = CFrame::Create(controlframe,_frames[i].name, wxPoint(_frames[i].pos[0], _frames[i].pos[1]), wxSize(_frames[i].size[0],_frames[i].size[1]));
			//frame->m_name = _frames[i].name;
			controlframe->m_frames.push_back(frame);

			uint _index = ID_HISTOGRAM;
			for(unsigned int j=0; j<controlframe->m_menuViews->GetMenuItemCount();j++)
			{
				const char *res;
				res = controlframe->m_menuViews->GetLabel(_index+j).mb_str();
				if(strcmp(res,frame->m_name.c_str()) == 0)
				{
					controlframe->m_menuViews->Check(_index+j,true);
					controlframe->m_menuViews->Enable(_index+j, false);
					break;
				}
			}
			
		}
		controlframe->Show(true);

		if(strlen(_cfg.data_folder)>1)
		{
			controlframe->LoadDataProgress(_cfg.data_folder);
		}
		controlframe->SetColor(255,0,0,255);
		return controlframe;
	}


	void CControlFrame::SetColor(unsigned char _red,unsigned char _green,unsigned char _blue,unsigned char _alpha)
	{
		m_ColorData.SetColour(wxColour(_red,_green,_blue,_alpha));
		m_datamanager->SetColor(_red,_green,_blue,_alpha);
	}



	void CControlFrame::CreateToolBar()
	{
		m_brushToolBar = new wxToolBar( this, ID_Brush_Toolbar, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxTB_TOP);

		enum    
		{
			Tool_one = 0,
			Tool_and,
			Tool_or,
			Tool_one_ex,
			Tool_and_ex,
			Tool_or_ex,
			Tool_ne_one_ex,
			Tool_ne_and_ex,
			Tool_ne_or_ex,
			Tool_Max
		};

		wxBitmap toolBarBitmaps[Tool_Max];

#define INIT_TOOL_BMP(bmp) toolBarBitmaps[Tool_##bmp] = wxBITMAP(bmp)

		INIT_TOOL_BMP(one);
		INIT_TOOL_BMP(and);
		INIT_TOOL_BMP(or);
		INIT_TOOL_BMP(one_ex);
		INIT_TOOL_BMP(and_ex);
		INIT_TOOL_BMP(or_ex);
		INIT_TOOL_BMP(ne_one_ex);
		INIT_TOOL_BMP(ne_and_ex);
		INIT_TOOL_BMP(ne_or_ex);

		m_brushToolBar->SetMargins(6, 6);
		m_brushToolBar->AddRadioTool(ID_Brush_One, _T("One"), wxBITMAP(one));
		m_brushToolBar->AddRadioTool(ID_Brush_And, _T("AND"), wxBITMAP(and));
		m_brushToolBar->AddRadioTool(ID_Brush_OR, _T("OR"), wxBITMAP(or));
		m_brushToolBar->AddRadioTool(ID_Brush_One_EX, _T("One"), wxBITMAP(one_ex));
		m_brushToolBar->AddRadioTool(ID_Brush_And_EX, _T("AND"), wxBITMAP(and_ex));
		m_brushToolBar->AddRadioTool(ID_Brush_OR_EX, _T("OR"), wxBITMAP(or_ex));
		m_brushToolBar->AddRadioTool(ID_Brush_Ne_One_EX, _T("One"), wxBITMAP(ne_one_ex));
		m_brushToolBar->AddRadioTool(ID_Brush_Ne_And_EX, _T("AND"), wxBITMAP(ne_and_ex));
		m_brushToolBar->AddRadioTool(ID_Brush_Ne_OR_EX, _T("OR"), wxBITMAP(ne_or_ex));
		m_brushToolBar->AddSeparator();
		m_brushToolBar->AddSeparator();

		wxBitmap bmp(wxBITMAP(color));
		m_brushToolBar->AddTool(ID_Color,_T("color"),bmp);
		m_brushToolBar->Realize();

		m_mgr.AddPane(m_brushToolBar, wxAuiPaneInfo().
			Name(wxT("tool")).Caption(wxT("tool")).
			ToolbarPane().Top().Row(1).
			LeftDockable(false).RightDockable(false));

		m_mgr.GetPane(wxT("tool")).Show();
	}

	CControlFrame::CControlFrame( const wxString& title, const wxPoint& pos,
		const wxSize& size, vector<t_FrameInfo> _frames, long style )
		: wxFrame( NULL, wxID_ANY, title, wxPoint(pos.x+s_ControlFrameList.size()*50, pos.y+s_ControlFrameList.size()*50), size, style ), \
		m_if_animate(false), m_invfps(33.3334f), m_accum_millisec(0),m_timer(this, TIMER_ID)
	{
		m_datamanager = NULL;
		m_mgr.SetManagedWindow(this);

		//menus
		m_menu_bar = new wxMenuBar();
		wxMenu *menuFile = new wxMenu;
		menuFile->Append( ID_LOADDATA, _T("&Load Data file...\tCtrl-L"));
		menuFile->Append( ID_LOADDATA, _T("&Save Current Project\tCtrl-S"));
		menuFile->AppendSeparator();

		menuFile->AppendCheckItem(  ID_SYNCFRAMES, _T("&Time Sync Control"), _T("Time Sync Control"));
		m_syncmenu = menuFile->FindItem( ID_SYNCFRAMES, &menuFile );

		menuFile->Append( ID_QUIT, _T("E&xit\tCtrl-Q"));
		m_menu_bar->Append(menuFile, _T("&File"));

		m_menuViews = new wxMenu;
		m_menuViews->AppendCheckItem(ID_HISTOGRAM, _T("HistogramFrame"));
		m_menuViews->AppendCheckItem(ID_PARACOORD, _T("ParaCoordFrame"));
		m_menuViews->AppendCheckItem(ID_DYNAMIC, _T("DynamicFrame"));
		m_menuViews->AppendCheckItem(ID_LOG, _T("logFrame"));
		m_menuViews->AppendCheckItem(ID_GRAPH, _T("GraphFrame"));
		m_menu_bar->Append(m_menuViews, _T("&Frames"));

		SetMenuBar( m_menu_bar );

		//panels and toolbars
		m_panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
		m_sizer = new wxBoxSizer(wxVERTICAL);

		m_pNotebook = new wxNotebook( m_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_DEFAULT);

		wxPanel *panel;
		wxTextCtrl *control;
		//animation panel
		panel = new wxPanel(m_pNotebook,wxID_ANY);
		m_pNotebook->AddPage( panel, "Animation", false, -1 );
		control = new wxTextCtrl(panel, wxID_ANY,  _T("Current Time:"), wxPoint(0,0), wxSize(80,25), wxBORDER_NONE );
		m_timeStartSlider = new wxSlider( panel, ID_Control_TimeSlider, 1, 1, 10, wxPoint( 0,25 ), wxSize( size.GetWidth()-40, 25) );
		m_DataPicker = new wxDatePickerCtrl( panel, ID_Control_TimeSlider, wxDefaultDateTime, wxPoint( 80,0 ) );
		m_hourCombo = new wxComboBox( panel, ID_Control_HourControl, "", wxPoint( 180,0 ), wxSize(45,25));
		for ( int i = 0; i < 24; i++ )
		{
			char tmp[8];
			sprintf( tmp, "%d", i );
			m_hourCombo->Append( _T(tmp));
		}
		m_minuteCombo = new wxComboBox( panel, ID_Control_MinuteControl, "", wxPoint( 225,0 ), wxSize(45,25) );
		for ( int i = 0; i < 60; i++ )
		{
			char tmp[8];
			sprintf( tmp, "%d", i );
			m_minuteCombo->Append( _T(tmp));
		}
		m_secondCombo = new wxComboBox( panel, ID_Control_SecondControl, "", wxPoint( 270,0 ), wxSize(45,25) );
		for ( int i = 0; i < 60; i++ )
		{
			char tmp[8];
			sprintf( tmp, "%d", i );
			m_secondCombo->Append( _T(tmp));
		}
		control = new wxTextCtrl(panel, wxID_ANY,  _T("Time Step:"), wxPoint(335,5), wxSize(70,25), wxBORDER_NONE );
		m_TimeStepCombo = new wxComboBox( panel, ID_TimeStepCombo, "", wxPoint( 410,0 ), wxSize(50,25) );

		for ( int i = 1; i <= 9; i++ )
		{
			char tmp[8];
			sprintf( tmp, "%d", i );
			m_TimeStepCombo->Append( _T(tmp));
		}

		for ( int i = 10; i <= 120; i+=10 )
		{
			char tmp[8];
			sprintf( tmp, "%d", i );
			m_TimeStepCombo->Append( _T(tmp));
		}
		m_TimeStepCombo->SetSelection(0);
		m_skipCheckBox = new wxCheckBox(panel, ID_Skip_CheckBox, _T("Skip 0 Record Frame"), wxPoint(475, 0), wxSize(150, 25));
		m_skipCheckBox->SetValue( false );

		unsigned int _x = 80, _y = 50;
		control = new wxTextCtrl(panel, wxID_ANY,  _T("Time Range:"), wxPoint(0,_y), wxSize(80,25), wxBORDER_NONE );
		m_rangeRatioCombo = new wxComboBox( panel, ID_Range_Ratio, "", wxPoint( _x+260,_y ), wxSize(60,25) );
		for ( float ratio = 0.05f; ratio <1.01f; ratio+=0.05f )
		{
			char str[8];
			sprintf( str, "%1.2f", ratio );
			m_rangeRatioCombo->Append(_T(str));
		}
		m_rangeRatioCombo->Select(0);
		m_timeRangeSlider = new wxSlider( panel, ID_Control_RangeSlider, 1, 1, 10, wxPoint( 0,_y+25), wxSize( size.GetWidth()-40, 25) );
		m_timeRangeText = new wxTextCtrl( panel, wxID_ANY, "", wxPoint(_x,_y), wxSize(120,25), wxBORDER_NONE );
		m_recordNumberText = new wxTextCtrl( panel, wxID_ANY, "", wxPoint(_x+120,_y), wxSize(120,25), wxBORDER_NONE );
		m_animateButton = new wxButton( panel, ID_Control_Animate, _T("Animation: OFF"), wxPoint( 0, _y+50), wxSize( 150, 25) ); 
		m_SpeedCombo = new wxComboBox( panel, ID_Speed_Combo, "", wxPoint( 160,_y+50), wxSize(100,25) );
		m_SpeedCombo->Append(_T("Faster"));
		for ( int fps = 5; fps <= 30; fps+=5 )
		{
			char str[8];
			sprintf( str, "%d FPS", fps );
			m_SpeedCombo->Append(_T(str));
		}
		m_SpeedCombo->Select(0);


		//selection and exclusion panel
		unsigned int disp = 145;
		wxSize listboxsize( 150, size.GetHeight()-disp );
		wxSize selectionboxsize ( size.GetWidth() - 170, size.GetHeight()-disp );

		panel = new wxPanel(m_pNotebook,wxID_ANY);
		m_pNotebook->AddPage( panel, "Selection", false, -1 );
		m_SelectionCtrl = new wxListCtrl( panel, ID_Filter_Selection, wxPoint(5, 5), selectionboxsize, wxLC_REPORT | wxLC_SINGLE_SEL );
		wxListItem itemCol;
		itemCol.SetText(_T("Filter of Details"));
		itemCol.SetImage(-1);
		m_SelectionCtrl->InsertColumn(0, itemCol);
		itemCol.SetText(_T("Color"));
		itemCol.SetImage(-1);
		m_SelectionCtrl->InsertColumn(1, itemCol);

		m_FilterClearThisButton = new wxButton( panel, ID_Filter_ClearThis, _T("Delete"), wxPoint( selectionboxsize.GetWidth()+15, 5), wxSize( 100, 25) ); 
		m_FilterClearAllButton = new wxButton( panel, ID_Filter_ClearAll, _T("Delete All"), wxPoint( selectionboxsize.GetWidth()+15, 35), wxSize( 100, 25) ); 
		m_query_save = new wxButton( panel, ID_Query_Save, _T("Save"), wxPoint( selectionboxsize.GetWidth()+15, 65), wxSize( 100, 25) ); 
		m_query_load = new wxButton( panel, ID_Query_Load, _T("Load"), wxPoint( selectionboxsize.GetWidth()+15, 95), wxSize( 100, 25) ); 
		m_query_append = new wxButton( panel, ID_Query_Append, _T("Append"), wxPoint( selectionboxsize.GetWidth()+15, 125), wxSize( 100, 25) );  

		panel = new wxPanel(m_pNotebook,wxID_ANY);
		m_pNotebook->AddPage( panel, "Exclusion", false, -1 );
		m_SelectionCtrlEX = new wxListCtrl( panel, ID_Filter_SelectionEX, wxPoint(5, 5), selectionboxsize, wxLC_REPORT | wxLC_SINGLE_SEL );
		itemCol.SetText(_T("Filter of Details"));
		itemCol.SetImage(-1);
		m_SelectionCtrlEX->InsertColumn(0, itemCol);
		itemCol.SetText(_T("Color"));
		itemCol.SetImage(-1);
		m_SelectionCtrlEX->InsertColumn(1, itemCol);
			
		m_FilterClearThisButtonEX = new wxButton( panel, ID_Filter_ClearThisEX, _T("Delete"), wxPoint( selectionboxsize.GetWidth()+15, 5), wxSize( 100, 25) ); 
		m_FilterClearAllButtonEX = new wxButton( panel, ID_Filter_ClearAllEX, _T("Delete All"), wxPoint( selectionboxsize.GetWidth()+15, 35), wxSize( 100, 25) );
		m_query_saveEx = new wxButton( panel, ID_Query_SaveEX, _T("Save"), wxPoint( selectionboxsize.GetWidth()+15, 65), wxSize( 100, 25) ); 
		m_query_loadEx = new wxButton( panel, ID_Query_LoadEX, _T("Load"), wxPoint( selectionboxsize.GetWidth()+15, 95), wxSize( 100, 25) ); 
		m_query_appendEx = new wxButton( panel, ID_Query_AppendEX, _T("Append"), wxPoint( selectionboxsize.GetWidth()+15, 125), wxSize( 100, 25) );  

		panel = new wxPanel(m_pNotebook,wxID_ANY);
		m_pNotebook->AddPage( panel, "Negative Exclusion", false, -1 );
		//m_SelectionBoxNEX = new wxListBox( panel, ID_Filter_SelectionNEX, wxPoint(5, 5), selectionboxsize, 0, NULL, wxLB_SINGLE, wxDefaultValidator );
		m_SelectionCtrlNegEX = new  wxListCtrl( panel, ID_Filter_SelectionNEX, wxPoint(5, 5), selectionboxsize, wxLC_REPORT | wxLC_SINGLE_SEL );
		itemCol.SetText(_T("Filter of Details"));
		itemCol.SetImage(-1);
		m_SelectionCtrlNegEX->InsertColumn(0, itemCol);
		itemCol.SetText(_T("Color"));
		itemCol.SetImage(-1);
		m_SelectionCtrlNegEX->InsertColumn(1, itemCol);

		m_FilterClearThisButtonNEX = new wxButton( panel, ID_Filter_ClearThisNEX, _T("Delete"), wxPoint( selectionboxsize.GetWidth()+15, 5), wxSize( 100, 25) ); 
		m_FilterClearAllButtonNEX = new wxButton( panel, ID_Filter_ClearAllNEX, _T("Delete All"), wxPoint( selectionboxsize.GetWidth()+15, 35), wxSize( 100, 25) );
		m_query_saveNEx = new wxButton( panel, ID_Query_SaveNEX, _T("Save"), wxPoint( selectionboxsize.GetWidth()+15, 65), wxSize( 100, 25) ); 
		m_query_loadNEx = new wxButton( panel, ID_Query_LoadNEX, _T("Load"), wxPoint( selectionboxsize.GetWidth()+15, 95), wxSize( 100, 25) ); 
		m_query_appendNEx = new wxButton( panel, ID_Query_AppendNEX, _T("Append"), wxPoint( selectionboxsize.GetWidth()+15, 125), wxSize( 100, 25) );  



		//the last step to adding the toolbar and notebook to the main panel
		//m_sizer->Add( m_brushToolBar, 0, wxEXPAND, 0 );
		m_sizer->Insert(0, m_pNotebook, wxSizerFlags(5).Expand().Border());
		m_sizer->Show( m_pNotebook );
		m_sizer->Layout();



		m_panel->SetSizer(m_sizer);
		SetExtraStyle(wxWS_EX_PROCESS_IDLE );
		CreateToolBar();
		m_mgr.AddPane(m_panel, wxAuiPaneInfo().Name(wxT("panel")).
			CenterPane().PaneBorder(false));
		m_mgr.GetPane(wxT("panel")).Show();

		m_mgr.Update();
	}

	void CControlFrame::LoadDataProgress(char* _name)
	{
		wxTimer *timePtr;
		timePtr = &m_timer;
		CwxThread *thread = new CwxThread(this,timePtr,m_datamanager,_name);
		if ( thread->Create() != wxTHREAD_NO_ERROR )
		{
			wxLogMessage(wxT("Can't create thread!"));
			return;
		}
		m_loadingData = true;

		m_dlgProgress = new wxProgressDialog
			(
			_T("Progress dialog"),
			_T("Now Loading the data, please waiting... ..."),
			MAX_LOADING_TIME,
			this,
			wxPD_APP_MODAL |
			wxPD_ELAPSED_TIME |
			wxPD_SMOOTH
			);

		thread->Run();
		//m_timer.SetOwner(this,-1);
		m_timer.Start(10); 
		Sleep(50);
	}

	static int g_index = 0;
	void CControlFrame::OnTimer(wxTimerEvent& _event)
	{
		// create any type of command event here
		wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, WORKER_EVENT );

		event.SetInt( 1 );
		g_index++;
		Sleep(50);
		// send in a thread-safe way
		wxPostEvent( this, event );

	}


	void CControlFrame::OnWorkerEvent(wxCommandEvent& event)
	{
		int n = event.GetInt();
		if ( n == -1 )
		{
			m_loadingData = false;
			m_dlgProgress->Destroy();
			m_timer.Stop();
			m_dlgProgress = (wxProgressDialog *)NULL;

			DataRefresh();
			wxEraseEvent();
			//wxWakeUpIdle();
		}
		else
		{
			if(g_index >=MAX_LOADING_TIME)
			{	
				g_index= 0;
				return;
			}

			if( m_dlgProgress != NULL)
			{
				if ( !m_dlgProgress->Update(g_index) )
				{}
			}

		}

	}

	void CControlFrame::OnSyncAllFrames( wxCommandEvent &event)
	{
		bool _ret = event.IsChecked();
		for ( vector<CControlFrame*>::iterator it = s_ControlFrameList.begin(); it != s_ControlFrameList.end(); it++ )
		{
			if ( this != (*it) )
			{
				(*it)->m_syncmenu->Check( _ret );
				if ( _ret && GetDataManager()->m_ifDataReady && (*it)->GetDataManager()->m_ifDataReady )
				{
					(*it)->SetCurrentTime( GetDataManager()->m_CurTimeData->GetCurTime() );
				}
			}
		}
	}


	void CControlFrame::OnHistogram(wxCommandEvent &WXUNUSED(event))
	{
/*		//check the frame is on and close the frame
		if(m_menuViews->IsChecked(ID_HISTOGRAM))
		{
			m_menuViews->Check(ID_HISTOGRAM,true);

			CFrame* frame;
			frame = CHistogramFrame::Create(this,"HistogramFrame", wxPoint(0,0), wxSize(800,600));
			//frame->m_name = "HistogramFrame";
			if(frame->GetCanvas() != NULL)
			{
				frame->m_panels->Refresh(false);
				frame->Refresh(false);
				frame->GetCanvas()->Refresh(false);
			}
			m_frames.push_back(frame);
		}
		else
		{
			m_menuViews->Check(ID_HISTOGRAM,false);
			int index;
			CFrame* frame;
			for(index = 0;index<m_frames.size(); index++)
			{
				if(m_frames[index]->m_name.compare("HistogramFrame") ==0)
					break;
			}
			frame = m_frames[index];
			frame->DataUnRegister();
			frame->Close();
			vector<CFrame*>::iterator it_frame = m_frames.begin();

			int size = m_frames.size();
			for(index=0; index<size; index++)
			{
				if(m_frames[index]->m_name.compare("HistogramFrame") ==0)
					break;
			}
			if(index<size)
				m_frames.erase(it_frame+index);
		}*/
	}

	void CControlFrame::OnParaCoord(wxCommandEvent &WXUNUSED(event))
	{
		//check the frame is on and close the frame
/*		if(m_menuViews->IsChecked(ID_PARACOORD))
		{
			m_menuViews->Check(ID_PARACOORD,true);

			CFrame* frame;
			frame = CParaCoordFrame::Create(this,"ParaCoordFrame", wxPoint(0,0), wxSize(800,600));
			frame->m_name = "ParaCoordFrame";
			frame->SetupData();
			if(frame->m_IData != NULL)
				frame->m_IData->Update();
			if(frame->GetCanvas() != NULL)
			{
				frame->m_panels->Refresh(false);
				frame->Refresh(false);
				frame->GetCanvas()->Refresh(false);
			}
			m_frames.push_back(frame);
		}
		else
		{
			m_menuViews->Check(ID_PARACOORD,false);

			int index =0;
			CFrame* frame;
			for(index = 0;index<m_frames.size(); index++)
			{
				if(m_frames[index]->m_name.compare("ParaCoordFrame") ==0)
					break;
			}
			frame = m_frames[index];
			frame->DataUnRegister();
			frame->Close();
			vector<CFrame*>::iterator it_frame = m_frames.begin();
			int size = m_frames.size();
			for(index=0; index<size; index++)
			{
				if(m_frames[index]->m_name.compare("ParaCoordFrame") ==0)
					break;
			}
			if(index<size)
				m_frames.erase(it_frame+index);
		}*/
	}

	void CControlFrame::OnDynamic(wxCommandEvent &WXUNUSED(event))
	{
		//check the frame is on and close the frame
/*		if(m_menuViews->IsChecked(ID_DYNAMIC))
		{
			m_menuViews->Check(ID_DYNAMIC,true);

			CFrame* frame;
			frame = CDynamicFrame::Create(this,"DynamicFrame", wxPoint(0,0), wxSize(800,600));
			frame->m_name = "DynamicFrame";
			frame->SetupData();
			if(frame->m_IData != NULL)
				frame->m_IData->Update();
			if(frame->GetCanvas() != NULL)
			{
				frame->m_panels->Refresh(false);
				frame->Refresh(false);
				frame->GetCanvas()->Refresh(false);
			}
			m_frames.push_back(frame);
		}
		else
		{
			m_menuViews->Check(ID_DYNAMIC,false);

			int index =0;
			CFrame* frame;
			for(index = 0;index<m_frames.size(); index++)
			{
				if(m_frames[index]->m_name.compare("DynamicFrame") ==0)
					break;
			}
			frame = m_frames[index];
			frame->DataUnRegister();
			frame->Close();
			vector<CFrame*>::iterator it_frame = m_frames.begin();
			int size = m_frames.size();
			for(index=0; index<size; index++)
			{
				if(m_frames[index]->m_name.compare("DynamicFrame") ==0)
					break;
			}
			if(index<size)
				m_frames.erase(it_frame+index);
		}*/
	}

	void CControlFrame::OnLog(wxCommandEvent &WXUNUSED(event))
	{
		//check the frame is on and close the frame
	/*	if(m_menuViews->IsChecked(ID_LOG))
		{
			m_menuViews->Check(ID_LOG,true);

			CFrame* frame;
			frame = CLOGFrame::Create(this,"logFrame", wxPoint(0,0), wxSize(800,600));
			frame->m_name = "logFrame";
			frame->SetupData();
			if(frame->m_IData != NULL)
				frame->m_IData->Update();
			if(frame->GetCanvas() != NULL)
			{
				frame->m_panels->Refresh(false);
				frame->Refresh(false);
				frame->GetCanvas()->Refresh(false);
			}
			m_frames.push_back(frame);
		}
		else
		{
			m_menuViews->Check(ID_LOG,false);

			int index =0;
			CFrame* frame;
			for(index =0;index<m_frames.size(); index++)
			{
				if(m_frames[index]->m_name.compare("logFrame") ==0)
					break;
			}
			frame = m_frames[index];
			frame->DataUnRegister();
			frame->Close();
			vector<CFrame*>::iterator it_frame = m_frames.begin();
			int size = m_frames.size();
			for(index=0; index<size; index++)
			{
				if(m_frames[index]->m_name.compare("logFrame") ==0)
					break;
			}
			if(index<size)
				m_frames.erase(it_frame+index);
		}*/
	}

	void CControlFrame::OnGraph(wxCommandEvent &WXUNUSED(event))
	{
		//check the frame is on and close the frame
	/*	if(m_menuViews->IsChecked(ID_GRAPH))
		{
			m_menuViews->Check(ID_GRAPH,true);

			CFrame* frame;
			frame = CGraphFrame::Create(this,"GraphFrame", wxPoint(0,0), wxSize(800,600));
			frame->m_name = "GraphFrame";
			frame->SetupData();
			if(frame->m_IData != NULL)
				frame->m_IData->Update();
			if(frame->GetCanvas() != NULL)
			{
				frame->m_panels->Refresh(false);
				frame->Refresh(false);
				frame->GetCanvas()->Refresh(false);
			}
			m_frames.push_back(frame);
		}
		else
		{
			m_menuViews->Check(ID_GRAPH,false);

			int index =0;
			CFrame* frame;
			for(index =0;index<m_frames.size(); index++)
			{
				if(m_frames[index]->m_name.compare("GraphFrame") ==0)
					break;
			}
			frame = m_frames[index];
			frame->DataUnRegister();
			frame->Close();
			vector<CFrame*>::iterator it_frame = m_frames.begin();
			int size = m_frames.size();
			for(index=0; index<size; index++)
			{
				if(m_frames[index]->m_name.compare("GraphFrame") ==0)
					break;
			}
			if(index<size)
				m_frames.erase(it_frame+index);
		}*/
	}


	void CControlFrame::OnBrushOne(wxCommandEvent& event)
	{
		m_datamanager->m_BrushType = (t_BrushType)(event.GetId() - ID_Brush_One);
	}

	void CControlFrame::OnBrushAnd(wxCommandEvent& event)
	{
		m_datamanager->m_BrushType = (t_BrushType)(event.GetId() - ID_Brush_One);
	}

	void CControlFrame::OnBrushOR(wxCommandEvent& event)
	{
		m_datamanager->m_BrushType = (t_BrushType)(event.GetId() - ID_Brush_One);
	}

	void CControlFrame::OnBrushOneEX(wxCommandEvent& event)
	{
		m_datamanager->m_BrushType = (t_BrushType)(event.GetId() - ID_Brush_One);
	}

	void CControlFrame::OnBrushAndEX(wxCommandEvent& event)
	{
		m_datamanager->m_BrushType = (t_BrushType)(event.GetId() - ID_Brush_One);
	}

	void CControlFrame::OnBrushOREX(wxCommandEvent& event)
	{
		m_datamanager->m_BrushType = (t_BrushType)(event.GetId() - ID_Brush_One);
	}

	void CControlFrame::OnBrushNeOneEX(wxCommandEvent& event)
	{
		m_datamanager->m_BrushType = (t_BrushType)(event.GetId() - ID_Brush_One);
	}

	void CControlFrame::OnBrushNeAndEX(wxCommandEvent& event)
	{
		m_datamanager->m_BrushType = (t_BrushType)(event.GetId() - ID_Brush_One);
	}

	void CControlFrame::OnBrushNeOREX(wxCommandEvent& event)
	{
		m_datamanager->m_BrushType = (t_BrushType)(event.GetId() - ID_Brush_One);
	}


	void CControlFrame::OnColor(wxCommandEvent &event)
	{
		wxColourDialog dialog(this, &m_ColorData);
		dialog.SetTitle(_T("Choose the background colour"));
		if (dialog.ShowModal() == wxID_OK)
		{
			m_ColorData = dialog.GetColourData();
			m_datamanager->SetColor(
				m_ColorData.GetColour().Red(),
				m_ColorData.GetColour().Green(),
				m_ColorData.GetColour().Blue(),
				m_ColorData.GetColour().Alpha());
		}
	}

	void CControlFrame::OnFilterSelectionEX(wxListEvent& event)
	{
		m_datamanager->m_CurrentFilterEX = event.m_itemIndex;
	}

	void CControlFrame::OnClearThisFilterSelectionEX(wxCommandEvent &event)
	{
		CDataManager *dm = GetDataManager();
		int idx = dm->m_CurrentFilterEX;
		if ( idx >= 0 )
		{
			if(idx + 1 > dm->m_ExclusiveFilter->m_Filter.size())
				return;

			int index  = 0;
			t_FilterSet::iterator it = dm->m_ExclusiveFilter->m_Filter.begin();
			for(; it != dm->m_ExclusiveFilter->m_Filter.end(); it++)
			{
				if(index == idx)
					break;
				index++;
			}
			if(it != dm->m_ExclusiveFilter->m_Filter.end())
				dm->m_ExclusiveFilter->m_Filter.erase(it);

			dm->m_ExclusiveFilter->Update();
			m_SelectionCtrlEX->DeleteItem(dm->m_CurrentFilterEX);
			RefreshAll();
		}
		dm->m_CurrentFilterEX  = -1;
	}

	void CControlFrame::OnClearAllFilterSelectionEX(wxCommandEvent &event)
	{
		CDataManager *dm = m_datamanager;
		dm->m_ExclusiveFilter->m_Filter.clear();
		dm->m_ExclusiveFilter->Update();
		m_SelectionCtrlEX->DeleteAllItems();
		RefreshAll();
	}

	void CControlFrame::OnFilterSelectionNEX(wxListEvent& event)
	{
		m_datamanager->m_CurrentFilterNEX = event.m_itemIndex;
	}

	void CControlFrame::OnClearThisFilterSelectionNEX(wxCommandEvent &event)
	{
		CDataManager *dm = GetDataManager();
		int idx = dm->m_CurrentFilterNEX;
		if ( idx >= 0 )
		{
			if(idx + 1 > dm->m_NegExclusiveFilter->m_Filter.size())
				return;

			int index  = 0;
			t_FilterSet::iterator it = dm->m_NegExclusiveFilter->m_Filter.begin();
			for(; it != dm->m_NegExclusiveFilter->m_Filter.end(); it++)
			{
				if(index == idx)
					break;
				index++;
			}
			if(it != dm->m_NegExclusiveFilter->m_Filter.end())
				dm->m_NegExclusiveFilter->m_Filter.erase(it);

			dm->m_NegExclusiveFilter->Update();
			m_SelectionCtrlNegEX->DeleteItem(dm->m_CurrentFilterNEX);
			RefreshAll();
		}
		dm->m_CurrentFilterNEX  = -1;
	}

	void CControlFrame::OnClearAllFilterSelectionNEX(wxCommandEvent &event)
	{
		CDataManager *dm = m_datamanager;
		dm->m_NegExclusiveFilter->m_Filter.clear();
		dm->m_NegExclusiveFilter->Update();
		m_SelectionCtrlNegEX->DeleteAllItems();
		RefreshAll();
	}

	void CControlFrame::OnFilterSelection(wxListEvent& event)
	{
		CDataManager *dm = GetDataManager();
		dm->m_CurrentFilter = event.m_itemIndex;
		int idx = dm->m_CurrentFilter;
		if ( idx < 0 )
			return;

		int acuNum = 0;
		int curFilterID = 0;
		for(; curFilterID < dm->m_Filters.size(); curFilterID++)
		{
			acuNum += (int)dm->m_Filters[curFilterID]->m_Filter.size();
			if(acuNum >= idx +1)
				break;
		}
		if(acuNum < idx +1)
			return;

		acuNum = acuNum - (int)dm->m_Filters[curFilterID]->m_Filter.size();
		acuNum  = idx - acuNum;
		dm->m_Filters[curFilterID]->SetCurrentAndFilterIdx(acuNum);
	}

	void CControlFrame::OnClearThisFilterSelection(wxCommandEvent &event)
	{
		CDataManager *dm = GetDataManager();
		int idx = dm->m_CurrentFilter;
		if ( idx >= 0 )
		{
			int acuNum = 0;
			int curFilterID = 0;
			for(; curFilterID < dm->m_Filters.size(); curFilterID++)
			{
				acuNum += (int)dm->m_Filters[curFilterID]->m_Filter.size();
				if(acuNum >= idx +1)
					break;
			}
			if(acuNum < idx +1)
				return;

			acuNum = acuNum - (int)dm->m_Filters[curFilterID]->m_Filter.size();
			acuNum  = idx - acuNum;

			int index  = 0;
			t_FilterSet::iterator it = dm->m_Filters[curFilterID]->m_Filter.begin();
			for(; it != dm->m_Filters[curFilterID]->m_Filter.end(); it++)
			{
				if(index == acuNum)
					break;
				index++;
			}
			assert(index == acuNum);
			if(it != dm->m_Filters[curFilterID]->m_Filter.end())
				dm->m_Filters[curFilterID]->m_Filter.erase(it);

			if(dm->m_Filters[curFilterID]->m_Filter.size() == 0)
			{
				EraseFilter(dm->m_Filters[curFilterID]);
			}
			else
				dm->m_Filters[curFilterID]->Update();
			m_SelectionCtrl->DeleteItem(dm->m_CurrentFilter);
			RefreshAll();
		}
		dm->m_CurrentFilter  = -1;
	}

	void CControlFrame::OnClearAllFilterSelection(wxCommandEvent &event)
	{
		CDataManager *dm = m_datamanager;
		vector<CFilter*> vec;
		for(unsigned int i = 0; i < dm->m_Filters.size(); i++)
		{
			dm->m_Filters[i]->m_Filter.clear();
			vec.push_back(dm->m_Filters[i]);
		}
		for(unsigned int i = 0; i < vec.size(); i++)
			EraseFilter(vec[i]);

		dm->m_dataUpdateStyle = UPDATE_STYLE;
		m_SelectionCtrl->DeleteAllItems();
		RefreshAll();
	}

	void CControlFrame::OnAnimationSpeedSelect(wxCommandEvent& event )
	{
		if ( m_if_animate )
			m_animateButton->SetLabel( _T("Animation: OFF"));

		int _sel = m_SpeedCombo->GetSelection();
		if ( _sel == 0 )
			m_invfps = 0.0001f;
		else
			m_invfps = 1000.0f / (_sel * 5.0f);
		m_accum_millisec = 0;
	}

	void CControlFrame::OnTimeStepSelect(wxCommandEvent& event )
	{
		CDataManager *dm = m_datamanager;
		if ( !dm->m_ifDataReady )
			return;

		int sel = m_TimeStepCombo->GetSelection();
		string step = m_TimeStepCombo->GetString(sel);

		dm->m_TimeStepData->SetTimeStep( atoi(step.c_str()) );

		m_timeStartSlider->SetRange(0,(dm->m_MaxTime - dm->m_MinTime)/dm->m_TimeStepData->GetTimeStep());
		m_timeStartSlider->SetValue((int)((dm->m_CurTimeData->GetCurTime() - dm->m_MinTime) / dm->m_TimeStepData->GetTimeStep()));

		dm->m_TimeStepData->Update();
		RefreshAll();

	}

	void CControlFrame::OnSkipCheck( wxCommandEvent& event )
	{
		CDataManager *dm = m_datamanager;

		dm->m_ifSkip = event.IsChecked();

		ResetTimeSlider();
	
		unsigned int _value = m_timeStartSlider->GetValue();
		dm->m_CurrentTimesliderIdx = _value;
		if ( !dm->m_ifDataReady )
			return;

		if ( m_skipCheckBox->IsChecked() )
			dm->m_CurTimeData->SetCurTime( dm->m_TimeWindowData->m_UniqueTimeStamps[_value] );
		else
			dm->m_CurTimeData->SetCurTime( ((time_t) _value) * dm->m_TimeStepData->GetTimeStep() +  dm->m_MinTime);
		UpdateDateTime();
		UpdateDataForTimeChange();
		dm->m_TimeRangeData->Update();
		RefreshAll();
	}


	void CControlFrame::OnTimeSliderScroll( wxScrollEvent &event )
	{
		unsigned int _value = m_timeStartSlider->GetValue();
		CDataManager *dm = GetDataManager();//CDataManager::Instance();
		dm->m_CurrentTimesliderIdx = _value;
		if ( !dm->m_ifDataReady )
			return;

		if ( m_skipCheckBox->IsChecked() )
			dm->m_CurTimeData->SetCurTime( dm->m_TimeWindowData->m_UniqueTimeStamps[_value] );
		else
			dm->m_CurTimeData->SetCurTime( ((time_t) _value) * dm->m_TimeStepData->GetTimeStep() +  dm->m_MinTime);
		UpdateDateTime();
		dm->m_dataUpdateStyle = INCREMENTAL_STYLE;
		UpdateDataForTimeChange();

		if ( m_syncmenu->IsChecked() )
		{
			for ( vector<CControlFrame*>::iterator it = s_ControlFrameList.begin(); it != s_ControlFrameList.end(); it++ )
			{
				if ( this != (*it) )
				{
					if ( GetDataManager()->m_ifDataReady && (*it)->GetDataManager()->m_ifDataReady )
					{
						(*it)->SetCurrentTime( dm->m_CurTimeData->GetCurTime() );
					}
				}
			}
		}
	}

	void CControlFrame::OnTimeSliderScrollBottom( wxScrollEvent &event )
	{
		m_if_animate = false;
		m_animateButton->SetLabel( _T("Animation: OFF"));
	}

	void CControlFrame::OnRangeSliderScroll( wxScrollEvent &event )
	{
		unsigned int _value = m_timeRangeSlider->GetValue();
		CDataManager *dm = m_datamanager;
		if ( !dm->m_ifDataReady )
			return;

		char tmp[32];
		sprintf( tmp, "%d Seconds", _value /** dm->m_RangeStep*/);
		m_timeRangeText->ChangeValue( _T(tmp) );

		dm->m_TimeRangeData->SetTimeRange(m_timeRangeSlider->GetValue());// /** dm->m_RangeStep*/;
		dm->m_TimeRangeData->Update();

		char str[32];
		sprintf( str, "%d records", dm->m_TimeWindowData->GetNumberofQueriedRecords() );
		m_recordNumberText->ChangeValue( _T(str) );

		RefreshAll();
	}

	void CControlFrame::OnIdle( wxIdleEvent &event )
	{
		if( m_if_animate )
		{
			m_datamanager->m_dataUpdateStyle = INCREMENTAL_STYLE;
			bool if_asap = false;
			if (m_SpeedCombo->GetSelection() == 0)
				if_asap = true;

			clock_t _now = clock();
			m_accum_millisec += _now - m_pre_animatetime;
			if( if_asap || m_accum_millisec > m_invfps )
			{
				if ( m_timeStartSlider->GetValue() < m_timeStartSlider->GetMax() )
				{
					m_timeStartSlider->SetValue( m_timeStartSlider->GetValue() + 1 );
					wxScrollEvent _event;
					OnTimeSliderScroll( _event );

					if ( m_syncmenu->IsChecked() )
					{
						for ( vector<CControlFrame*>::iterator it = s_ControlFrameList.begin(); it != s_ControlFrameList.end(); it++ )
						{
							if ( this != (*it) && GetDataManager()->m_ifDataReady && (*it)->GetDataManager()->m_ifDataReady )
							{
								if ( (*it)->m_timeStartSlider->GetValue() <(*it)->m_timeStartSlider->GetMax() && (*it)->m_timeStartSlider->GetValue() > (*it)->m_timeStartSlider->GetMin() )
								{
									(*it)->m_timeStartSlider->SetValue( (*it)->m_timeStartSlider->GetValue() + 1 );
									wxScrollEvent _event;
									(*it)->OnTimeSliderScroll( _event);
								}
								else
								{
									(*it)->m_if_animate = false;
									(*it)->m_animateButton->SetLabel( _T("Animation: OFF"));
								}
							}
						}
					}

				}
				else
				{
					m_if_animate = false;
					m_animateButton->SetLabel( _T("Animation: OFF"));
				}
				m_accum_millisec = 0;
			}
			else
				m_pre_animatetime = _now;
		}
		else
			event.Skip();
		event.RequestMore();
	}

	void CControlFrame::OnAnimate(wxCommandEvent& event)
	{
		if ( m_if_animate )
		{
			m_if_animate = false;
			m_animateButton->SetLabel( _T("Animation: OFF"));
			m_accum_millisec = 0;
		}
		else
		{
			m_if_animate = true;
			m_animateButton->SetLabel( _T("Animation: ON"));
			m_pre_animatetime = clock();
		}

		if ( m_syncmenu->IsChecked() )
		{
			for ( vector<CControlFrame*>::iterator it = s_ControlFrameList.begin(); it != s_ControlFrameList.end(); it++ )
			{
				if ( this != (*it) )
				{
					if ( GetDataManager()->m_ifDataReady && (*it)->GetDataManager()->m_ifDataReady )
					{
						(*it)->m_if_animate = m_if_animate;
						if ( m_if_animate )
						{
							(*it)->m_animateButton->SetLabel( _T("Animation: ON"));
						}
						else
						{
							(*it)->m_animateButton->SetLabel( _T("Animation: OFF"));
						}
					}
				}
			}
		}
	}

	void CControlFrame::OnRangeRatioSelect(wxCommandEvent& event)
	{
		int _sel = m_rangeRatioCombo->GetSelection()+1;
		CDataManager *dm = m_datamanager;

		if ( dm != NULL )
		{
			dm->m_RangeRatio = _sel * 0.05;
			if ( dm->m_RangeRatio > 1.0 )
				dm->m_RangeRatio = 1.0;

			m_timeRangeSlider->SetRange(1, (unsigned int)((dm->m_MaxTime - dm->m_MinTime)*dm->m_RangeRatio));
			m_timeRangeSlider->SetValue(dm->m_TimeRangeData->GetTimeRange());
		}
	}

	void CControlFrame::OnClose( wxCloseEvent& event )
	{
		wxCommandEvent e;
		OnQuit( e );
	}

	void CControlFrame::OnQuit(wxCommandEvent &WXUNUSED(event))
	{
		CDataManager *dm = m_datamanager;

		for( unsigned int i = 0; i < m_frames.size(); i++ )
		{
			if(m_frames[i]->IsShown() == false)
				m_frames[i]->Destroy();
			delete m_frames[i];
		}
		m_frames.clear();
		if ( dm != NULL )
			delete dm;
		dm = NULL;
		Destroy();
	}

	void CControlFrame::RefreshAll()
	{
		for(unsigned int i =0; i < m_frames.size(); i++)
		{
			if(m_frames[i]->GetCanvas() != NULL)
				m_frames[i]->GetCanvas()->Refresh(false);
		}
	}

	void CControlFrame::UpdateDataForTimeChange()
	{
		//update time
		CDataManager *dm = m_datamanager;
		//dm->SelectTimeSpanIndex();
		dm->m_CurTimeData->Update();

		char str[32];
		sprintf( str, "%d records", dm->m_TimeWindowData->GetNumberofQueriedRecords() );
		m_recordNumberText->ChangeValue( _T(str) );
		
		RefreshAll();
	}

	void CControlFrame::ResetTimeSlider()
	{
		CDataManager *dm = GetDataManager();//CDataManager::Instance();
		if(m_skipCheckBox->IsChecked())
			m_timeStartSlider->SetRange(0, dm->m_TimeWindowData->m_UniqueTimeStamps.size()-1);
		else
			m_timeStartSlider->SetRange(0, (dm->m_MaxTime - dm->m_MinTime)/dm->m_TimeStepData->GetTimeStep());
		m_timeStartSlider->SetValue(0);
	}

	void CControlFrame::SetCurrentTime( time_t _curtime )
	{
		if ( _curtime >= m_datamanager->m_MinTime && _curtime <= m_datamanager->m_MaxTime )
		{
			m_datamanager->m_CurTimeData->SetCurTime( _curtime );
			m_timeStartSlider->SetValue((m_datamanager->m_CurTimeData->GetCurTime() - m_datamanager->m_MinTime)/m_datamanager->m_TimeStepData->GetTimeStep());
			UpdateDateTime();
			UpdateDataForTimeChange();
		}
	}

	void CControlFrame::UpdateDateTime()
	{
		CDataManager *dm = m_datamanager;
		time_t curTime = dm->m_CurTimeData->GetCurTime();

		struct tm *timeinfo = gmtime( (const time_t *)(&curTime) );
		wxDateTime _datetime;
		_datetime.Set( *timeinfo );
		m_DataPicker->SetValue( _datetime );
		m_hourCombo->Select( timeinfo->tm_hour );
		m_minuteCombo->Select( timeinfo->tm_min );
		m_secondCombo->Select( timeinfo->tm_sec );
	}

	void CControlFrame::DataRefresh()
	{
		CDataManager *dm = m_datamanager;

		SetColor(255,0,0,255);

		//reflesh the animation panel
		//ResetTimeSlider();
		m_timeRangeSlider->SetRange(1, (uint)(dm->m_MaxTime - dm->m_MinTime)*dm->m_RangeRatio);
		m_timeRangeSlider->SetValue(1);
		m_timeStartSlider->SetRange(0,(dm->m_MaxTime - dm->m_MinTime)/dm->m_TimeStepData->GetTimeStep());
		char tmp[32];
		sprintf(tmp,"%d Seconds", m_timeRangeSlider->GetValue()*(dm->m_TimeRangeData->GetTimeRange()));
		m_datamanager->m_CurTimeData->SetCurTime(dm->m_MinTime);
		m_timeRangeSlider->SetValue(dm->m_TimeRangeData->GetTimeRange());//dm->m_RangeStep
		m_timeRangeText->ChangeValue(_T(tmp));
		UpdateDateTime();
		//SelectTimeSpanIndex();
		ResetTimeSlider();

		for(unsigned int i =0; i < m_frames.size(); i++)
		{
			if(m_frames[i]->GetCanvas() != NULL)
				m_frames[i]->RefleshPanel();
		}


		m_datamanager->m_ifDataReady = true;
		UpdateDataForTimeChange();

		//reflesh the frames control panel
		for(unsigned int i =0; i < m_frames.size(); i++)
		{
			if(m_frames[i]->GetCanvas() != NULL)
			{
				//m_frames[i]->RefleshPanel();
				m_frames[i]->m_panels->Refresh(false);
				m_frames[i]->Refresh(false);
				m_frames[i]->GetCanvas()->Refresh(false);
			}
		}
		dm->m_dataUpdateStyle =INCREMENTAL_STYLE;
	}

	void CControlFrame::OnLoadData(wxCommandEvent &WXUNUSED(event))
	{
		wxString filename = wxFileSelector(_T("Select Data file"));
		if ( !filename )
			return;

		if ( m_if_animate )
		{
			m_if_animate = false;
			m_animateButton->SetLabel( _T("Animation: OFF"));
			m_accum_millisec = 0;
			Sleep(20);
		}

		LoadDataProgress((char *) filename.c_str());

		char title[1024];
		sprintf(title,"Control Panel (%s)",filename);
		this->SetTitle(title);
	}

	void CControlFrame::UpdateFilter()
	{
		CDataManager* dm = GetDataManager();

		if ( dm->m_BrushType <=  Brush_OR)
		{
			m_SelectionCtrl->DeleteAllItems();
			unsigned int index = 0;
			for( unsigned  int i = 0; i < dm->m_Filters.size(); i++ )
			{
				t_FilterSet& _filterset = dm->m_Filters[i]->m_Filter;
				for(unsigned int j=0; j< _filterset.size(); j++)
				{
					wxString str;
					t_AndFilterSet &afs = _filterset[j];
					for ( t_AndFilterSet::iterator it = afs.begin(); it != afs.end(); it++ )
					{
						wxString itemstr;
						if ( dm->m_RawData->m_item_desc[(*it).first].num_values != 0 )
							itemstr = dm->m_RawData->m_item_desc[(*it).first].value_names[(*it).second];
						else
						{
							float value = dm->m_RawData->GetNumDataValue( (*it).second, (*it).first );
							char tmp[32];
							sprintf( tmp, "%f", value );
							itemstr = tmp;
						}
						itemstr = dm->m_RawData->m_item_desc[(*it).first].name + "=" + itemstr;
						if ( it != afs.begin() )
							str += _T(" and ") + itemstr;
						else
							str = itemstr;
					}

					//find the color
					t_color color = dm->GetColor(dm->m_Filters[i]);
					wxColour fontcolor(color.red,color.green, color.blue, color.alpha);
					long tmp = m_SelectionCtrl->InsertItem( index, str, 0 );
					m_SelectionCtrl->SetItem( tmp, 1, _T("color") );
					wxListItem item;
					item.m_itemId = index;
					item.SetTextColour(fontcolor);
					item.SetFont(*wxITALIC_FONT);
					m_SelectionCtrl->SetItem( item );
					m_SelectionCtrl->SetColumnWidth(0,wxLIST_AUTOSIZE );
					index++;
				}
			}
		}

		else if ( (dm->m_BrushType > Brush_OR) && (dm->m_BrushType <= Brush_OR_EX) )
		{
			m_SelectionCtrlEX->DeleteAllItems();

			for ( unsigned int i = 0; i < dm->m_ExclusiveFilter->m_Filter.size(); i++ )
			{
				wxString str;
				t_AndFilterSet &afs = dm->m_ExclusiveFilter->m_Filter[i];
				for ( t_AndFilterSet::iterator it = afs.begin(); it != afs.end(); it++ )
				{
					wxString itemstr;
					if ( dm->m_RawData->m_item_desc[(*it).first].num_values != 0 )
						itemstr = dm->m_RawData->m_item_desc[(*it).first].value_names[(*it).second];
					else
					{
						float value = dm->m_RawData->GetNumDataValue( (*it).second, (*it).first );
						char tmp[32];
						sprintf( tmp, "%f", value );
						itemstr = tmp;
					}
					itemstr = dm->m_RawData->m_item_desc[(*it).first].name + "=" + itemstr;
					if ( it != afs.begin() )
						str += _T(" and ") + itemstr;
					else
						str = itemstr;
				}
				long tmp = m_SelectionCtrlEX->InsertItem( i, str, 0 );
				m_SelectionCtrlEX->SetItem( tmp, 1, _T("color") );
				wxListItem item;
				item.m_itemId = i;
				item.SetTextColour(*wxLIGHT_GREY);
				item.SetFont(*wxITALIC_FONT);
				m_SelectionCtrlEX->SetItem( item );
				m_SelectionCtrlEX->SetColumnWidth(0,wxLIST_AUTOSIZE );
			}
		}

		else if (  (dm->m_BrushType > Brush_OR_EX) && (dm->m_BrushType <= Brush_Neg_OR_EX) )
		{
			m_SelectionCtrlNegEX->DeleteAllItems();

			for ( unsigned int i = 0; i < dm->m_NegExclusiveFilter->m_Filter.size(); i++ )
			{
				wxString str;
				t_AndFilterSet &afs = dm->m_NegExclusiveFilter->m_Filter[i];
				for ( t_AndFilterSet::iterator it = afs.begin(); it != afs.end(); it++ )
				{
					wxString itemstr;
					if ( dm->m_RawData->m_item_desc[(*it).first].num_values != 0 )
						itemstr = dm->m_RawData->m_item_desc[(*it).first].value_names[(*it).second];
					else
					{
						float value = dm->m_RawData->GetNumDataValue( (*it).second, (*it).first );
						char tmp[32];
						sprintf( tmp, "%f", value );
						itemstr = tmp;
					}
					itemstr = dm->m_RawData->m_item_desc[(*it).first].name + "=" + itemstr;
					if ( it != afs.begin() )
						str += _T(" and ") + itemstr;
					else
						str = itemstr;
				}
				long tmp = m_SelectionCtrlNegEX->InsertItem( i, str, 0 );
				m_SelectionCtrlNegEX->SetItem( tmp, 1, _T("color") );
				wxListItem item;
				item.m_itemId = i;
				item.SetTextColour(*wxLIGHT_GREY);
				item.SetFont(*wxITALIC_FONT);
				m_SelectionCtrlNegEX->SetItem( item );
				m_SelectionCtrlNegEX->SetColumnWidth(0,wxLIST_AUTOSIZE );
			}
		}
	}

	void CControlFrame::EraseFilter(CFilter* _filter)
	{
		CDataManager *dm = m_datamanager;
		t_color color  = dm->GetColor(_filter);
		vector<pair<t_color,CFilter*>>::iterator it_color = dm->m_colorTable.begin();
		for(; it_color != dm->m_colorTable.end(); it_color++)
		{
			if(dm->IsSameColor(it_color->first,color))
				break;
		}
		if(it_color != dm->m_colorTable.end())
			dm->m_colorTable.erase(it_color);
		CAbstractData::DeleteObject(_filter, false);
		CAbstractData::EraseIsolatedData();
		dm->CleanDataLists();

		for(int i = 0; i< m_frames.size(); i++)
			m_frames[i]->CleanDataLists();
	}

	void CControlFrame::UpdateBrush(unsigned int _itemidx, unsigned int _valueidx)
	{
		CDataManager *dm = m_datamanager;

		unsigned int nFilters = dm->m_Filters.size();
		CFilter *filter = dm->ProcessBrush( _itemidx, _valueidx );

		if ( filter != NULL )
		{
			if(nFilters + 1  == dm->m_Filters.size())
			{
				for(int i = 0; i< m_frames.size(); i++)
					m_frames[i]->CreateDataForNewFilter();
			}
			
			filter->UpdateSelf();
			if( filter->m_Filter.size() == 0 && filter != dm->m_ExclusiveFilter && filter != dm->m_NegExclusiveFilter) //erase this filter
				EraseFilter(filter);
			else
				filter->Update(); // The filter itself can be updated multiple times, since the m_item and m_Valueidx is reset after the updateself(). Now a filter can have multiple children.
		}
		RefreshAll();
	}

	void CControlFrame::OnQueryLoadButton(wxCommandEvent& event)
	{
/*
		CDataManager *dm = m_datamanager;
		if(dm->m_ifDataReady==false)
		{
			wxMessageBox( _("You need to load data first!"),
				_("Message"),wxOK | wxICON_INFORMATION, this );
			return;
		}

		wxFileDialog 
			openFileDialog(this, _("Open file"), "", "",
			"All files (*.*)|*.*", wxFD_OPEN|wxFD_FILE_MUST_EXIST);

		if (openFileDialog.ShowModal() == wxID_CANCEL)
			return;
		else
		{
			if(dm->LoadQuery((char*)openFileDialog.GetPath().c_str(),true,true))
			{
				UpdateFilter();
				RefreshAll();
			}
		}*/
	}

	void CControlFrame::OnQueryLoadButtonEX(wxCommandEvent& event)
	{
/*
		CDataManager *dm = m_datamanager;
		if(dm->m_ifDataReady==false)
		{
			wxMessageBox( _("You need to load data first!"),
				_("Message"),wxOK | wxICON_INFORMATION, this );
			return;
		}


		wxFileDialog 
			openFileDialog(this, _("Open file"), "", "",
			"All files (*.*)|*.*", wxFD_OPEN|wxFD_FILE_MUST_EXIST);

		if (openFileDialog.ShowModal() == wxID_CANCEL)
			return;
		else
		{
			if(dm->LoadQuery((char*)openFileDialog.GetPath().c_str(),true,false))
			{
				UpdateFilter();
				RefreshAll();
			}
		}*/
	}
	void CControlFrame::OnQuerySaveButton(wxCommandEvent& event)
	{/*
		CDataManager *dm = m_datamanager;
		char str[1024];
		if(!dm->m_ifDataReady)
		{
			wxMessageBox( _("You need to load data first!"),
				_("Message"),wxOK | wxICON_INFORMATION, this );
			return;
		}

		wxFileDialog 
			saveFileDialog(this, _("Save file"), "", ".txt",
			"files (*.txt*)|*.txt*", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

		if (saveFileDialog.ShowModal() == wxID_CANCEL)
			return;
		else
		{
			FILE* savefile;
			savefile = fopen((char*)saveFileDialog.GetPath().c_str(),"w");
			int num = m_SelectionBox->GetCount();
			wxString wx_str;
			char buf[1024];
			for (int i=0;i<num;i++)
			{
				wx_str  =m_SelectionBox->GetString(i);
				strcpy( buf, (const char*)wx_str.mb_str(wxConvUTF8) );
				fprintf(savefile,buf);
				fprintf(savefile,"\n");
			}
			fclose(savefile);
		}*/
	}
	void CControlFrame::OnQuerySaveButtonEX(wxCommandEvent& event)
	{
		/*
		CDataManager *dm = m_datamanager;
		char str[1024];
		if(!dm->m_ifDataReady)
		{
			wxMessageBox( _("You need to load data first!"),
				_("Message"),wxOK | wxICON_INFORMATION, this );
			return;
		}

		wxFileDialog 
			saveFileDialog(this, _("Save file"), "", ".txt",
			"files (*.txt*)|*.txt*", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

		if (saveFileDialog.ShowModal() == wxID_CANCEL)
			return;
		else
		{
			FILE* savefile;
			savefile = fopen((char*)saveFileDialog.GetPath().c_str(),"w");
			int num = m_SelectionBoxEX->GetCount();
			wxString wx_str;
			char buf[1024];
			for (int i=0;i<num;i++)
			{
				wx_str  =m_SelectionBoxEX->GetString(i);
				strcpy( buf, (const char*)wx_str.mb_str(wxConvUTF8) );
				fprintf(savefile,buf);
				fprintf(savefile,"\n");
			}
			fclose(savefile);
		}*/
	}
	void CControlFrame::OnQueryAppendButton(wxCommandEvent& event)
	{/*
		CDataManager *dm = m_datamanager;
		if(dm->m_ifDataReady==false)
		{
			wxMessageBox( _("You need to load data first!"),
				_("Message"),wxOK | wxICON_INFORMATION, this );
			return;
		}
		wxFileDialog 
			openFileDialog(this, _("Open file"), "", "",
			"All files (*.*)|*.*", wxFD_OPEN|wxFD_FILE_MUST_EXIST);

		if (openFileDialog.ShowModal() == wxID_CANCEL)
			return;
		else
		{
			if(dm->LoadQuery((char*)openFileDialog.GetPath().c_str(),false,true))
			{
				UpdateFilter();
				RefreshAll();
			}
		}*/
	}

	void CControlFrame::OnQueryAppendButtonEX(wxCommandEvent& event)
	{/*
		CDataManager *dm = m_datamanager;
		if(dm->m_ifDataReady==false)
		{
			wxMessageBox( _("You need to load data first!"),
				_("Message"),wxOK | wxICON_INFORMATION, this );
			return;
		}
		wxFileDialog 
			openFileDialog(this, _("Open file"), "", "",
			"All files (*.*)|*.*", wxFD_OPEN|wxFD_FILE_MUST_EXIST);

		if (openFileDialog.ShowModal() == wxID_CANCEL)
			return;
		else
		{
			if(dm->LoadQuery((char*)openFileDialog.GetPath().c_str(),false,false))
			{
				UpdateFilter();
				RefreshAll();
			}
		}*/
	}
}