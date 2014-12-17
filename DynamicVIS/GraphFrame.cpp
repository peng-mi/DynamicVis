
#include <GL/glew.h>

//wxwidgets
#include <wx/notebook.h>
#include <wx/listbox.h>
#include <wx/datectrl.h>
#include <wx/combo.h>
#include <wx/listctrl.h>

//graph info
#include "GraphFrame.h"
#include "GraphData.h"

//helpers
#include "color.h"
#include "glhelper.h"
#include "PerformanceTimer.h"
#include "colorhelper.h"

//data and control
#include "ControlFrame.h"
#include "datamanager.h"
#include "RawData.h"
#include "FilteredData.h"
#include "Filter.h"
namespace VIS
{

	/******************************************************************************************************
	********************CGraphFrame*******************************************************************
	******************************************************************************************************/

	enum
	{
		ID_QUIT  = wxID_EXIT,
		ID_HIDE,

		ID_ListBox,
		ID_ListCtrl,
		ID_Check_Whole,
		ID_Check_Merge,
		ID_CleanGraph,

		ID_Control_Layout,
		ID_Control_CreateGraphButton,
		ID_Control_Layout_att,
		ID_Control_Layout_rep,
		ID_Control_Layout_gra,
		ID_Control_Layout_range,
		ID_Control_Layout_pointSize,
	};

	BEGIN_EVENT_TABLE(CGraphFrame, CFrame)
		EVT_MENU(ID_QUIT, CFrame::OnExit)
		EVT_SIZE(CFrame::OnSize)
		EVT_MENU(ID_HIDE, CFrame::OnHide)
		EVT_CHAR_HOOK(CFrame::OnKeyDown)

		EVT_IDLE    (CGraphFrame::OnIdle)
		EVT_LIST_ITEM_SELECTED ( ID_ListCtrl, CGraphFrame::OnListboxSelect )
		EVT_BUTTON ( ID_Control_CreateGraphButton, CGraphFrame::OnGraphCreate )
		EVT_BUTTON ( ID_Control_Layout, CGraphFrame::OnLayout )
		EVT_BUTTON ( ID_CleanGraph, CGraphFrame::OnCleanGraph )



		EVT_COMMAND_SCROLL  (ID_Control_Layout_att, CGraphFrame::OnLayoutAttScroll)
		EVT_COMMAND_SCROLL  (ID_Control_Layout_rep, CGraphFrame::OnLayoutRepScroll)
		EVT_COMMAND_SCROLL  (ID_Control_Layout_gra, CGraphFrame::OnLayoutGraScroll)
		EVT_COMMAND_SCROLL  (ID_Control_Layout_range, CGraphFrame::OnLayoutRangeScroll)
		EVT_COMMAND_SCROLL  (ID_Control_Layout_pointSize, CGraphFrame::OnLayoutSizeScroll)

		END_EVENT_TABLE()

		CGraphFrame *CGraphFrame::Create(CControlFrame* _cf, const wxString& title, const wxPoint& pos,
		const wxSize& size, long style )
	{
		CGraphFrame *rf = new CGraphFrame(_cf, title, pos, size, style);
		rf->Show(true);
		return rf;
	}

	CGraphFrame::CGraphFrame(CControlFrame* _cf, const wxString& title, const wxPoint& pos,
		const wxSize& size, long style)
		: CFrame(_cf,title, pos, size, style ),m_ifLayout(false)
	{
		CDataManager *dm = GetDataManager();
		m_baseIData = new CGraphIData();
		dm->m_BasedIDataList.push_back(m_baseIData);
		AddRelation(dm->m_BasedFilteredData, m_baseIData, true);

		m_GraphItems[0] = -1;
		m_GraphItems[1] = -1;

		m_panelScale = 0.4f;
		m_canvas = new CGraphCanvas(this, wxID_ANY,wxDefaultPosition, wxSize(size.GetWidth(),size.GetWidth()));

		m_mgr.AddPane(m_canvas, wxAuiPaneInfo().
			Name(wxT("canvas")).Caption(wxT("canvas")).
			MaxSize(size).Layer(0).
			CloseButton(true).MaximizeButton(true));

		wxSize panelSize(size.GetWidth(),size.GetHeight()*m_panelScale);
		m_panels = new wxPanel(this, wxID_ANY, wxDefaultPosition, panelSize, wxTAB_TRAVERSAL);
		m_panels->SetBackgroundColour(wxColour(255,255,255));

		wxSize listboxsize(150,size.GetHeight()*m_panelScale-30);
		wxPanel* panel;
		m_pNotebook = new wxNotebook( m_panels, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_DEFAULT);


		//page one
		panel = new wxPanel(m_pNotebook,wxID_ANY);
		m_pNotebook->AddPage( panel, "General", false, -1 );
		new wxStaticText(panel, wxID_ANY, _T("Item Names:"), wxPoint(0,3), wxDefaultSize, wxBORDER_NONE);
		m_pListCtrl = new wxListCtrl( panel, ID_ListCtrl, wxPoint(0, 25), listboxsize, wxLC_REPORT | wxLC_SINGLE_SEL );
		wxListItem itemCol;
		itemCol.SetText(_T("Name"));
		itemCol.SetImage(-1);
		m_pListCtrl->InsertColumn(0, itemCol);
		itemCol.SetText(_T("Type"));
		itemCol.SetImage(-1);
		m_pListCtrl->InsertColumn(1, itemCol);

		new wxStaticText(panel, wxID_ANY, _T("Selected Items:"), wxPoint(160,3), wxDefaultSize, wxBORDER_NONE);
		m_pListBox = new wxListBox( panel, ID_ListBox, wxPoint(160, 25), listboxsize, 0, NULL, wxLB_MULTIPLE|wxWANTS_CHARS );
		m_GraCreate = new wxButton( panel, ID_Control_CreateGraphButton, _T("Create Graph"), wxPoint( 320, 25), wxSize( 125, 25) ); 
		m_Layout = new wxButton( panel, ID_Control_Layout, _T("Layout:Off"), wxPoint( 320, 55), wxSize( 125, 25) ); 
		m_clean = new wxButton( panel, ID_CleanGraph, _T("Clean Graph"), wxPoint( 320, 85), wxSize( 125, 25) ); 
		m_whole = new wxCheckBox(panel, ID_Check_Whole, _T("Whole Data"), wxPoint(460, 25), wxSize(100, 25));
		m_whole->SetValue( false );
		m_merge = new wxCheckBox(panel, ID_Check_Merge, _T("Merge Nodes"), wxPoint(560, 25), wxSize(100, 25));
		m_merge->SetValue( false );
		//m_DragNode = new wxButton( panel, ID_Control_Layout, _T("Layout:Off"), wxPoint( 320, 55), wxSize( 125, 25) );
		//m_Layout = new wxButton( panel, ID_Control_Layout, _T("Layout:Off"), wxPoint( 320, 55), wxSize( 125, 25) );



		//page two
		panel = new wxPanel(m_pNotebook,wxID_ANY);
		m_pNotebook->AddPage( panel, "Force Directed", false, -1 );
		//new wxStaticText(panel, wxID_ANY, _T("hello:"), wxPoint(0,3), wxDefaultSize, wxBORDER_NONE);
		int begin_offset = 10;
		int text_offset = 180;
		int end_offset = 300;
		m_Layout_att = new wxSlider( panel, ID_Control_Layout_att, 1, 1, 10, wxPoint( begin_offset+text_offset,30), wxSize( size.GetWidth()-end_offset, 25) );
		m_Layout_att_text = new wxTextCtrl (panel, wxID_ANY, wxT("Attractive Force"),wxPoint(begin_offset, 30),wxSize(120,25), wxBORDER_NONE);
		m_Layout_rep = new wxSlider( panel, ID_Control_Layout_rep, 1, 1, 10, wxPoint( begin_offset+text_offset,60), wxSize( size.GetWidth()-end_offset, 25) );
		m_Layout_rep_text = new wxTextCtrl (panel, wxID_ANY, wxT("Repulsive  Force"),wxPoint(begin_offset, 60),wxSize(120,25), wxBORDER_NONE);
		m_Layout_gra = new wxSlider( panel, ID_Control_Layout_gra, 1, 1, 10, wxPoint( begin_offset+text_offset,90), wxSize( size.GetWidth()-end_offset, 25) );
		m_Layout_gra_text = new wxTextCtrl (panel, wxID_ANY, wxT("Gravity    Force"),wxPoint(begin_offset, 90),wxSize(120,25), wxBORDER_NONE);
		m_Layout_pointSize = new wxSlider( panel, ID_Control_Layout_pointSize, 1, 1, 10, wxPoint( begin_offset+text_offset,120), wxSize( size.GetWidth()-end_offset, 25) );
		m_Layout_pointSize_text = new wxTextCtrl (panel, wxID_ANY, wxT("Point    Size"),wxPoint(begin_offset, 120),wxSize(120,25), wxBORDER_NONE);
		m_Layout_range = new wxSlider( panel, ID_Control_Layout_range, 1, 1, 10, wxPoint( begin_offset+text_offset,150), wxSize( size.GetWidth()-end_offset, 25) );
		m_Layout_range_text = new wxTextCtrl (panel, wxID_ANY, wxT("Range        Size"),wxPoint(begin_offset, 150),wxSize(120,25), wxBORDER_NONE);


		m_sizer = new wxBoxSizer(wxVERTICAL);
		m_sizer->Insert(0, m_pNotebook, wxSizerFlags(5).Expand().Border());
		m_sizer->Show(m_pNotebook);
		m_sizer->Layout();
		m_panels->SetSizer(m_sizer);
		SetExtraStyle(wxWS_EX_PROCESS_IDLE );

		m_mgr.AddPane(m_panels, wxAuiPaneInfo().
			Name(wxT("panel")).Caption(wxT("panel")).
			MaxSize(panelSize).Layer(0).
			CloseButton(true).MaximizeButton(false));


		m_mgr.GetPane(wxT("canvas")).Show().Center().Layer(0).Position(0);
		m_mgr.GetPane(wxT("panel")).Show().Center().Layer(0).Position(0);
		m_mgr.Update();

	}

	CGraphFrame::~CGraphFrame()
	{
		CGraphIData* base = dynamic_cast<CGraphIData*>(m_baseIData);
		if(base->m_data.graphPara.graphReady)
		{
			wxCommandEvent ent;
			OnCleanGraph(ent);
		}
	}

	void CGraphFrame::OnIdle( wxIdleEvent& event )
	{
		if(m_ifLayout)
		{
			CDataManager* dm = GetDataManager();
			 
			CGraphIData* idata = NULL;
			idata = dynamic_cast<CGraphIData*>(m_baseIData);
			idata->GraphLayout();

			for(unsigned int i = 0; i < dm->m_BasedIDataList.size(); i++)
			{
				idata = dynamic_cast<CGraphIData*>(dm->m_BasedIDataList[i]);
				if(idata != NULL)
					idata->UpdateSelf();
			}
			for(unsigned int i = 0; i < dm->m_HighlightIDataList.size(); i++)
			{
				idata = dynamic_cast<CGraphIData*>(dm->m_HighlightIDataList[i]);
				if(idata != NULL)
					idata->UpdateSelf();
			}
			for(unsigned int i = 0; i < m_canvas->m_RDataList.size(); i++)
				m_canvas->m_RDataList[i]->Update();

			m_canvas->Refresh(false);
		}
	}

	 
	void CGraphFrame::OnListboxSelect(wxListEvent& event)
	{
		int nSel = event.m_itemIndex+1;
		if(nSel != m_GraphItems[1])
		{
			m_GraphItems[0] = m_GraphItems[1];
			m_GraphItems[1] = nSel;
		}

		CDataManager* dm = GetDataManager();
		m_pListBox->Clear();
		if(m_GraphItems[0] != -1)
			m_pListBox->Append( wxString::FromUTF8(dm->m_RawData->m_item_desc[m_GraphItems[0]].name.c_str()));
		if(m_GraphItems[1] != -1)
		m_pListBox->Append( wxString::FromUTF8(dm->m_RawData->m_item_desc[m_GraphItems[1]].name.c_str()));
	}

	void CGraphFrame::OnCleanGraph(wxCommandEvent& event)
	{
		CDataManager* dm = GetDataManager();
		
		CGraphIData* idata = NULL;
		idata = dynamic_cast<CGraphIData*>(m_baseIData);
		idata->CleanSelf();
		idata->Update();
		idata->m_data.graphPara.graphReady = false;

		for(unsigned int i = 0; i < dm->m_HighlightIDataList.size(); i++)
		{
			idata = dynamic_cast<CGraphIData*>(dm->m_HighlightIDataList[i]);
			if(idata != NULL)
				idata->m_data.graphPara.graphReady = false;
		}
		Refresh(false);
	}

	void CGraphFrame::OnGraphCreate(wxCommandEvent& event)
	{
		CDataManager* dm = GetDataManager();
		if(dm->m_ifDataReady)
		{
			//m_whole

			CGraphIData* idata = NULL;
			idata = dynamic_cast<CGraphIData*>(m_baseIData);
			idata->CleanSelf();
			idata->SetDataRange(m_whole->GetValue(),m_merge->GetValue());
			idata->SetItems(m_GraphItems);
			idata->CreateGraph();
			idata->m_data.graphPara.graphReady = true;

			for(int i = 0; i < dm->m_HighlightIDataList.size(); i++)
			{
				idata = dynamic_cast<CGraphIData*>(dm->m_HighlightIDataList[i]);
				if( idata != NULL)
				{
					idata->CleanSelf();
					idata->SetDataRange(m_whole->GetValue(),m_merge->GetValue());
					idata->SetItems(m_GraphItems);
					idata->m_data.graphPara.graphReady = true;
				}
			}

			char tmp[32];
			m_Layout_gra->SetRange(0,100);
			m_Layout_gra->SetValue(25);
			m_Layout_att->SetRange(0,2000);
			m_Layout_att->SetValue(517);
			m_Layout_rep->SetRange(0,1000);
			m_Layout_rep->SetValue(0);
			m_Layout_pointSize->SetRange(0,1000);
			m_Layout_pointSize->SetValue(110);

			sprintf(tmp,"Attractive Force %.2f",m_Layout_att->GetValue()/100.0);
			m_Layout_att_text->ChangeValue(_T(tmp));
			sprintf(tmp,"Repulsive Force %.2f",m_Layout_rep->GetValue()/100.0);
			m_Layout_rep_text->ChangeValue(_T(tmp));
			sprintf(tmp,"Gravity     Force %.2f",m_Layout_gra->GetValue()/100.0);
			m_Layout_gra_text->ChangeValue(_T(tmp));
			sprintf(tmp,"Point         Size  %.2f",m_Layout_pointSize->GetValue()/1000.0);
			m_Layout_pointSize_text->ChangeValue(_T(tmp));

			idata = dynamic_cast<CGraphIData*>(m_baseIData);
			m_Layout_range->SetRange(1,idata->m_data.graphPara.numNode);
			m_Layout_range->SetValue((int)(sqrt((float)(idata->m_data.graphPara.numNode))));
			idata->m_data.graphPara.range = m_Layout_range->GetValue();
			sprintf(tmp,"Range       Size  %.2f",(float)m_Layout_range->GetValue());
			m_Layout_range_text->ChangeValue(_T(tmp));

			idata->m_data.graphPara.range = m_Layout_range->GetValue();
			idata->m_data.graphPara.attExponent = m_Layout_att->GetValue()/100.0;
			idata->m_data.graphPara.repExponent = m_Layout_rep->GetValue()/100.0;
			idata->m_data.graphPara.graScale =  m_Layout_gra->GetValue()/100.0;
			idata->m_data.graphPara.nodeSize =  m_Layout_pointSize->GetValue()/1000.0;

			GetCanvas()->m_Ortho[0] = GetCanvas()->m_Ortho[2] = 0- idata->m_data.graphPara.range;
			GetCanvas()->m_Ortho[1] = GetCanvas()->m_Ortho[3] = idata->m_data.graphPara.range;
			((CGraphCanvas*)GetCanvas())->m_OrthoBackup[0] = GetCanvas()->m_Ortho[0];
			((CGraphCanvas*)GetCanvas())->m_OrthoBackup[1] = GetCanvas()->m_Ortho[1];
			((CGraphCanvas*)GetCanvas())->m_OrthoBackup[2] = GetCanvas()->m_Ortho[2];
			((CGraphCanvas*)GetCanvas())->m_OrthoBackup[3] = GetCanvas()->m_Ortho[3];

			sprintf(tmp,"Duplicated Nodes: %d",idata->m_data.duplicatedD_S.size());
			new wxStaticText(m_panels, wxID_ANY, _T(tmp), wxPoint(470,90), wxDefaultSize, wxBORDER_NONE);

			idata->UpdateSelf();
			idata = NULL;
			for(unsigned int i = 0; i < dm->m_HighlightIDataList.size(); i++)
			{
				idata = dynamic_cast<CGraphIData*>(dm->m_HighlightIDataList[i]);
				if(idata != NULL)
					idata->UpdateSelf();
			}
			for(unsigned int i = 0; i < m_canvas->m_RDataList.size(); i++)
				m_canvas->m_RDataList[i]->Update();

			Refresh(false);
		}
	}

	void CGraphFrame::OnLayout(wxCommandEvent& event)
	{
		CDataManager* dm = GetDataManager();
		if(!m_ifLayout)
		{
			m_Layout->SetLabel("Layout:On");
			m_ifLayout = true;
		}
		else
		{
			m_Layout->SetLabel("Layout:Off");
			m_ifLayout = false;
		}

		CGraphIData* idata = NULL;
		idata = dynamic_cast<CGraphIData*>(m_baseIData);
		idata->m_data.is_layout = m_ifLayout;
		for(unsigned int i = 0; i < dm->m_HighlightIDataList.size(); i++)
		{
			idata = dynamic_cast<CGraphIData*>(dm->m_HighlightIDataList[i]);
			if(idata != NULL)
				idata->m_data.is_layout = m_ifLayout;
		}
	}

	void CGraphFrame::RefleshPanel()
	{
		CDataManager *dm =  GetDataManager();//CDataManager::Instance();
		
		m_pListBox->Clear();
		if(m_GraphItems[0] != -1)
			m_pListBox->Append( wxString::FromUTF8(dm->m_RawData->m_item_desc[m_GraphItems[0]].name.c_str()));
		if(m_GraphItems[1] != -1)
		m_pListBox->Append( wxString::FromUTF8(dm->m_RawData->m_item_desc[m_GraphItems[1]].name.c_str()));

		for ( unsigned int i = 1; i < dm->m_RawData->m_item_desc.size(); i++ )
		{
			long _tmp = m_pListCtrl->InsertItem(i-1,wxString::FromUTF8(dm->m_RawData->m_item_desc[i].name.c_str()), 0 );
			if ( dm->m_RawData->m_item_desc[i].num_values != 0 )
				m_pListCtrl->SetItem( _tmp, 1, _T("CAT") );
			else
			{
				m_pListCtrl->SetItem( _tmp, 1, _T("NUM") );
				wxListItem item;
				item.m_itemId = i-1;
				item.SetTextColour(*wxLIGHT_GREY);
				item.SetFont(*wxITALIC_FONT);
				m_pListCtrl->SetItem( item );
			}
		}
	}

	void CGraphFrame::OnLayoutAttScroll( wxScrollEvent &event )
	{
		CDataManager *dm = GetDataManager();
		CGraphIData* idata = dynamic_cast<CGraphIData*>(m_baseIData);
		if ( !dm->m_ifDataReady || !idata->m_data.graphPara.graphReady )
			return;

		char tmp[32];
		sprintf(tmp,"Attractive Force %.2f",m_Layout_att->GetValue()/100.0);
		m_Layout_att_text->ChangeValue(_T(tmp));

		idata->m_data.graphPara.attExponent = m_Layout_att->GetValue()/100.0;
		GetCanvas()->Refresh(false);

	}
	void CGraphFrame::OnLayoutRepScroll( wxScrollEvent &event )
	{
		CDataManager *dm = GetDataManager();
		CGraphIData* idata = dynamic_cast<CGraphIData*>(m_baseIData);
		if ( !dm->m_ifDataReady || !idata->m_data.graphPara.graphReady )
			return;

		char tmp[32];
		sprintf(tmp,"Repulsive Force %.2f",m_Layout_rep->GetValue()/100.0);
		m_Layout_rep_text->ChangeValue(_T(tmp));

		idata->m_data.graphPara.repExponent = m_Layout_rep->GetValue()/100.0;
		GetCanvas()->Refresh(false);
	}
	void CGraphFrame::OnLayoutGraScroll( wxScrollEvent &event )
	{
		CDataManager *dm = GetDataManager();
		CGraphIData* idata = dynamic_cast<CGraphIData*>(m_baseIData);
		if ( !dm->m_ifDataReady || !idata->m_data.graphPara.graphReady )
			return;

		char tmp[32];
		sprintf(tmp,"Gravity     Force %.2f",m_Layout_gra->GetValue()/100.0);
		m_Layout_gra_text->ChangeValue(_T(tmp));

		idata->m_data.graphPara.graScale = m_Layout_gra->GetValue()/100.0;
		GetCanvas()->Refresh(false);
	}
	void CGraphFrame::OnLayoutRangeScroll( wxScrollEvent &event )
	{
		CDataManager *dm = GetDataManager();
		CGraphIData* idata = dynamic_cast<CGraphIData*>(m_baseIData);
		if ( !dm->m_ifDataReady || !idata->m_data.graphPara.graphReady )
			return;

		char tmp[32];
		sprintf(tmp,"Range       Size %.2f",(float)m_Layout_range->GetValue());
		m_Layout_range_text->ChangeValue(_T(tmp));

		idata->m_data.graphPara.range = m_Layout_range->GetValue();
		((CGraphCanvas*)GetCanvas())->m_OrthoBackup[0] = GetCanvas()->m_Ortho[0] = 0-m_Layout_range->GetValue();
		((CGraphCanvas*)GetCanvas())->m_OrthoBackup[1] = GetCanvas()->m_Ortho[1] = m_Layout_range->GetValue();
		((CGraphCanvas*)GetCanvas())->m_OrthoBackup[2] = GetCanvas()->m_Ortho[2] = 0-m_Layout_range->GetValue();
		((CGraphCanvas*)GetCanvas())->m_OrthoBackup[3] = GetCanvas()->m_Ortho[3] = m_Layout_range->GetValue();

		GetCanvas()->Refresh(false);

	}
	void CGraphFrame::OnLayoutSizeScroll( wxScrollEvent &event )
	{
		CDataManager *dm = GetDataManager();
		CGraphIData* idata = dynamic_cast<CGraphIData*>(m_baseIData);
		if ( !dm->m_ifDataReady || !idata->m_data.graphPara.graphReady )
			return;

		char tmp[32];
		sprintf(tmp,"Point         Size %.2f",m_Layout_pointSize->GetValue()/1000.0);
		m_Layout_pointSize_text->ChangeValue(_T(tmp));

		idata->m_data.graphPara.nodeSize = m_Layout_pointSize->GetValue()/1000.0;
		GetCanvas()->Refresh(false);
	}


	void CGraphFrame::CreateDataForNewFilter()
	{
		CDataManager *dm = GetDataManager();
		CGraphIData* base = dynamic_cast<CGraphIData*>(m_baseIData);
		CGraphIData* idata = new CGraphIData();
		idata->m_data.is_layout = base->m_data.is_layout;
		idata->m_data.graphPara.graphReady = base->m_data.graphPara.graphReady;
		idata->SetItems(m_GraphItems);
		idata->SetDataRange(m_whole->GetValue(),m_merge->GetValue());

		
		dm->m_HighlightIDataList.push_back(idata);
		AddRelation(dm->m_FilteredDataList[dm->m_FilteredDataList.size()-1], idata, true);

		CGraphRData* _rdata = new CGraphRData(m_canvas);
		m_canvas->m_RDataList.push_back(_rdata);
		
		AddRelation(m_baseIData, _rdata, false);
		AddRelation( idata, _rdata, true );
	}

	/******************************************************************************************************
	********************CGraphCanvas*******************************************************************
	******************************************************************************************************/

	BEGIN_EVENT_TABLE(CGraphCanvas, CCanvas)
		EVT_SIZE(CCanvas::OnSize)
		EVT_PAINT(CCanvas::OnPaint)
		EVT_ERASE_BACKGROUND(CCanvas::OnEraseBackground)
		EVT_KEY_DOWN(CCanvas::OnKeyDown)
		EVT_MOUSE_EVENTS(CGraphCanvas::OnMouse)
		EVT_IDLE(CCanvas::OnIdle) 
		END_EVENT_TABLE()

		CGraphCanvas::CGraphCanvas( wxWindow *parent, wxWindowID id,
		const wxPoint& pos, const wxSize& size, long style, const wxString& name)
		: CCanvas(parent, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE , name )
	{
		rf = (CGraphFrame*)parent;
		CGraphRData* rdata = new CGraphRData(this);
		m_RDataList.push_back(rdata);
		AddRelation(rf->m_baseIData, rdata, true);

		m_OrthoBackup[0] = -1.0f;
		m_OrthoBackup[1] =  1.0f;
		m_OrthoBackup[2] = -1.0f;
		m_OrthoBackup[3] =  1.0f;
		rf = (CGraphFrame*) parent;
		m_dragNodes = false;
		m_nodeMove = false;
		m_nodeTranslation[0] = m_nodeTranslation[1] =0.0f;
		InitOrtho();
	}

	void CGraphCanvas::OnMouse(wxMouseEvent& event)
	{
		CCanvas::OnMouse(event);
		SetCurrent();
		int mouse_x = event.GetX();
		int mouse_y = event.GetY();

		if(event.RightDown())
		{
			if(!m_dragNodes)
			{
				m_if_selecting = false;
				SetCurrent();
				GetWorldCoordinate( m_newOrtho[0], m_newOrtho[1], mouse_x, mouse_y );
				m_newOrtho[2] = m_newOrtho[0]; m_newOrtho[3] = m_newOrtho[1];
				m_nodeTranslation[0] = m_newOrtho[2];
				m_nodeTranslation[1] = m_newOrtho[3];
			}
		}
		else if(event.Dragging() && event.RightIsDown())
		{
			if(m_nodeMove == false)
			{
				m_dragNodes = true;
				m_if_selecting = true;
				GetWorldCoordinate( m_newOrtho[2], m_newOrtho[3], mouse_x, mouse_y );
				Refresh(false);
				m_nodeTranslation[0] = m_newOrtho[2];
				m_nodeTranslation[1] = m_newOrtho[3];
			}
			else
			{
				CGraphFrame *gframe =  NULL;
				gframe = dynamic_cast<CGraphFrame*>(rf);
				t_Graph& _data = ((CGraphIData *)(gframe->m_baseIData))->m_data;
				GetWorldCoordinate( m_newOrtho[2], m_newOrtho[3], mouse_x, mouse_y );
				if (fabs(m_nodeTranslation[0]-0.0f)<0.000000001)
				{
					m_nodeTranslation[0] = m_newOrtho[2];
					m_nodeTranslation[1] = m_newOrtho[3];

				}
				else
				{
					set<uint>::iterator set_it;
					float tran[2];
					tran[0] = m_newOrtho[2] - m_nodeTranslation[0];
					tran[1] = m_newOrtho[3] - m_nodeTranslation[1];
					for(set_it = m_nodes.begin(); set_it != m_nodes.end(); set_it++)
					{
						_data.node[*set_it].pos[0] +=  tran[0];
						_data.node[*set_it].pos[1] +=  tran[1];
					}
					m_nodeTranslation[0] = m_newOrtho[2];
					m_nodeTranslation[1] = m_newOrtho[3];

					Refresh(false);
				}

			}
		}
		else if(event.RightUp())
		{
			if(m_dragNodes)
			{
				if(!m_nodeMove)
				{
					GetWorldCoordinate( m_newOrtho[2], m_newOrtho[3], mouse_x, mouse_y );
					if ( m_newOrtho[0] > m_newOrtho[2] )
					{
						float tmp = m_newOrtho[0];
						m_newOrtho[0] = m_newOrtho[2];
						m_newOrtho[2] = tmp;
					}

					if ( m_newOrtho[1] > m_newOrtho[3] )
					{
						float tmp = m_newOrtho[3];
						m_newOrtho[3] = m_newOrtho[1];
						m_newOrtho[1] = tmp;
					}
					this->SelectData();

					m_dragNodes = false;
					m_if_selecting = false;
					rf->m_cf->RefreshAll();
				}
			}
			else
				m_nodeMove = false;
		}



	}

	CGraphCanvas::~CGraphCanvas()
	{
	}

	void CGraphCanvas::DrawAxis()
	{
	}

	void CGraphCanvas::RenderVBO()
	{
		glEnableClientState(GL_VERTEX_ARRAY);
		CDataManager *dm = GetDataManager();
		CGraphIData* idata = NULL;
		CGraphRData* rdata = NULL;
		CGraphFrame* frame = NULL;
		frame = dynamic_cast<CGraphFrame*>(rf);
		idata = dynamic_cast<CGraphIData*>(frame->m_baseIData);
		glPointSize( idata->m_data.graphPara.nodeSize*40 );
		glEnable( GL_POINT_SMOOTH );
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

		for(unsigned int i = 0; i < m_RDataList.size(); i++)
		{
			
			rdata = dynamic_cast<CGraphRData*>(m_RDataList[i]);
			if(i == 0)
				glColor4fv(GRAY);
			else
			{
				int size = dm->m_colorTable.size();
				unsigned char colors[4];
				colors[0] = dm->m_colorTable[0].first.red;
				colors[1] = dm->m_colorTable[0].first.green;
				colors[2] = dm->m_colorTable[0].first.blue;
				colors[3] = dm->m_colorTable[0].first.alpha;

				glColor4ub(dm->m_colorTable[i-1].first.red,dm->m_colorTable[i-1].first.green,dm->m_colorTable[i-1].first.blue,dm->m_colorTable[i-1].first.alpha);
			}
			
			glBindBuffer(GL_ARRAY_BUFFER, rdata->m_vbo->m_vertexBufferId);
			glVertexPointer(rdata->m_vbo->m_vertexFromat,GL_FLOAT,0,0);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rdata->m_vbo->m_indexBufferId);
			glDrawElements(GL_LINES, rdata->m_vbo->m_indexNum, GL_UNSIGNED_INT, NULL);

			glDrawArrays(GL_POINTS,0,rdata->m_vbo->m_vertexNum);
		}
		glDisableClientState(GL_VERTEX_ARRAY);
	}

	void CGraphCanvas::DrawIndication()
	{
		float pos[3] = {0.0f, 0.0f, 0.6f};
		char str[256];
		CFilter* filter;
		CDataManager* dm = GetDataManager();
		CGraphIData* idata = NULL;
		CGraphFrame* frame = NULL;
		frame = dynamic_cast<CGraphFrame*>(rf);
		idata = dynamic_cast<CGraphIData*>(frame->m_baseIData);
		
		for(unsigned int i = 0; i < dm->m_colorTable.size(); i++)
		{
			filter = dm->m_colorTable[i].second;

			for(unsigned int j = 0; j < filter->m_Filter.size(); j++)
			{
				t_AndFilterSet &afs = filter->m_Filter[j];
				if(afs.size() != 1)
					continue;
				else if(afs.begin()->first == idata->m_data.graphInfo.items[0])
				{
					t_ref::iterator it = idata->m_data.vertex_ValueToId.find(afs.begin()->second);
					if(it != idata->m_data.vertex_ValueToId.end())
					{
						pos[0] = idata->m_data.node[it->second].pos[0];
						pos[1] = idata->m_data.node[it->second].pos[1];
						
						glColor4fv(BLACK);
						glRasterPos3fv(pos);

						if(dm->m_RawData->m_item_desc[afs.begin()->first].num_values != 0)
							sprintf(str,"%s",(char*)(dm->m_RawData->m_item_desc[afs.begin()->first].value_names[afs.begin()->second].c_str()));
						else
						{
							float value = dm->m_RawData->GetNumDataValue(afs.begin()->second, afs.begin()->first);
							sprintf(str, "%f", value);
						}
						glPrint("%s",str);
					}
				}
				else if(afs.begin()->first == idata->m_data.graphInfo.items[1])
				{
					t_ref::iterator it = idata->m_data.vertex_ValueToId.find(afs.begin()->second + idata->m_data.offset);
					if(it != idata->m_data.vertex_ValueToId.end())
					{
						pos[0] = idata->m_data.node[it->second].pos[0];
						pos[1] = idata->m_data.node[it->second].pos[1];

						glColor4fv(BLACK);
						glRasterPos3fv(pos);

						if(dm->m_RawData->m_item_desc[afs.begin()->first].num_values != 0)
							sprintf(str,"%s",(char*)(dm->m_RawData->m_item_desc[afs.begin()->first].value_names[afs.begin()->second].c_str()));
						else
						{
							float value = dm->m_RawData->GetNumDataValue(afs.begin()->second, afs.begin()->first);
							sprintf(str, "%f", value);
						}
						glPrint("%s",str);
					}
				}
			}
		}
	}

	void CGraphCanvas::DrawCanvas()
	{
		RenderVBO();
		DrawIndication();
	}

	void CGraphCanvas::SelectData()
	{
		CGraphIData* idata = NULL;
		CGraphFrame* frame = NULL;
		frame = dynamic_cast<CGraphFrame*>(rf);
		idata = dynamic_cast<CGraphIData*>(frame->m_baseIData);
		t_Graph& _data = idata->m_data;
		
		for(unsigned int i=0; i<_data.graphPara.numNode; i++)
		{
			float value_x = _data.node[i].pos[0];
			float value_y = _data.node[i].pos[1];
			int sel;

			if(!_data.graphInfo.is_merge)
			{
				if( i < _data.graphPara.numNode_1 )
					sel = 0;
				else
					sel = 1;
				if((value_x>m_newOrtho[0] && value_x<m_newOrtho[2]) && (value_y>m_newOrtho[1] && value_y<m_newOrtho[3]))
				{	
					rf->m_cf->UpdateBrush(_data.graphInfo.items[sel],_data.vertex_IdToValue[i]);
					rf->m_cf->UpdateFilter();
				}

			}
			else
			{
				if(i <_data.graphPara.numNode_1)
				{
					if((value_x>m_newOrtho[0] && value_x<m_newOrtho[2]) && (value_y>m_newOrtho[1] && value_y<m_newOrtho[3]))
					{	

						rf->m_cf->UpdateBrush(_data.graphInfo.items[0],_data.vertex_IdToValue[i]);
						rf->m_cf->UpdateFilter();

						if( _data.duplicatedS_D.find(_data.vertex_IdToValue[i]) != _data.duplicatedS_D.end())
						{
							rf->m_cf->UpdateBrush(_data.graphInfo.items[1],_data.vertex_IdToValue[_data.duplicatedS_D[_data.vertex_IdToValue[i]]]);
							rf->m_cf->UpdateFilter();
						}
					}
				}
				else
				{
					if((value_x>m_newOrtho[0] && value_x<m_newOrtho[2]) && (value_y>m_newOrtho[1] && value_y<m_newOrtho[3]))
					{	

						rf->m_cf->UpdateBrush(_data.graphInfo.items[1],_data.vertex_IdToValue[i]);
						rf->m_cf->UpdateFilter();

						if( _data.duplicatedD_S.find(_data.vertex_IdToValue[i]) != _data.duplicatedD_S.end())
						{
							rf->m_cf->UpdateBrush(_data.graphInfo.items[0],_data.vertex_IdToValue[_data.duplicatedD_S[_data.vertex_IdToValue[i]]]);
							rf->m_cf->UpdateFilter();
						}
					}
				}
			}
		}
	}

	void CGraphCanvas::InitGL()
	{
		SetCurrent();
		GLenum err = glewInit();
		if (GLEW_OK != err)
			wxLogError( "%s\n", glewGetErrorString(err) );

		if ( GLEW_ARB_vertex_buffer_object )
			wxLogMessage( "VBO extersion is supported!\n");
		if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader && GL_EXT_geometry_shader4)
			wxLogMessage("Ready for GLSL - vertex, fragment, and geometry units.\n");
		else {
			wxLogError("Not totally ready :( \n");
		}

		glClearColor( WHITE[0],WHITE[1],WHITE[2],WHITE[3] );
		glClearDepth(1.0);

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_BLEND);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_NORMALIZE);


		HWND hWnd = (HWND) GetHandle();
		m_hDC = GetDC(hWnd);

		BuildFont();
	}


}
