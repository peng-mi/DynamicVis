#include <GL/glew.h>

#include "HistogramFrame.h"
#include <wx/notebook.h>
#include <wx/listbox.h>
#include <wx/datectrl.h>
#include <wx/combo.h>
#include <wx/listctrl.h>


#include "ControlFrame.h"
#include "datamanager.h"
#include "color.h"
#include "glhelper.h"
#include "PerformanceTimer.h"
#include "colorhelper.h"
#include "RawData.h"
#include "FilteredData.h"
#include "HistogramData.h"
#include "shaderhelper.h"
#include "VBOData.h"
#include "TimeData.h"

namespace VIS
{

	/******************************************************************************************************
	********************CHistogramFrame*******************************************************************
	******************************************************************************************************/

	enum
	{
		ID_QUIT  = wxID_EXIT,
		ID_HIDE,
		ID_ListCtrl,
		ID_ValueSelect_Listbox,
		ID_Scale_CheckBox,
		ID_VertialScale_Slider,
		ID_Search_Text,
		ID_Search_Button,
	};

	BEGIN_EVENT_TABLE(CHistogramFrame, CFrame)
		EVT_MENU(ID_QUIT, CFrame::OnExit)
		EVT_SIZE(CFrame::OnSize)
		EVT_MENU(ID_HIDE, CFrame::OnHide)
		EVT_CHAR_HOOK(CFrame::OnKeyDown)

		EVT_LIST_ITEM_SELECTED ( ID_ListCtrl, CHistogramFrame::OnListboxSelect )
		EVT_BUTTON ( ID_Search_Button, CHistogramFrame::OnSearchButton )
		EVT_TEXT	 ( ID_Search_Text, CHistogramFrame::OnSearchText)
		EVT_COMMAND_SCROLL  (ID_VertialScale_Slider, CHistogramFrame::OnVerticalScaleScroll)
		EVT_CHECKBOX( ID_Scale_CheckBox, CHistogramFrame::OnLogScaleCheck)
		EVT_LISTBOX (ID_ValueSelect_Listbox, CHistogramFrame::OnValueSelect)
		END_EVENT_TABLE()


	CHistogramFrame *CHistogramFrame::Create(CControlFrame* _cf, const wxString& title, const wxPoint& pos,
		const wxSize& size, long style )
	{
		CHistogramFrame *rf = new CHistogramFrame(_cf, title, pos, size, style);
		rf->Show(true);
		return rf;
	}

	CHistogramFrame::CHistogramFrame(CControlFrame* _cf, const wxString& title, const wxPoint& pos,
		const wxSize& size, long style)
		: CFrame(_cf,title, pos, size, style ),m_new_search(true),m_previous_scroll(0),m_item(1)
	{
		CDataManager *dm = GetDataManager();
		m_baseIData = new CHistogramIData();
		dm->m_BasedIDataList.push_back(m_baseIData);
		AddRelation( dm->m_BasedFilteredData, m_baseIData, true);

		m_canvas = new CHistogramCanvas( this, wxID_ANY,wxDefaultPosition, wxSize(size.GetWidth(),size.GetWidth()));

		m_panelScale = 0.4f;
		m_mgr.AddPane(m_canvas, wxAuiPaneInfo().
			Name(wxT("canvas")).Caption(wxT("canvas")).
			MaxSize(size).Layer(0).
			CloseButton(true).MaximizeButton(true));

		wxSize panelSize(size.GetWidth(),size.GetHeight()*m_panelScale);
		m_panels = new wxPanel(this, wxID_ANY, wxDefaultPosition, panelSize, wxTAB_TRAVERSAL);
		m_panels->SetBackgroundColour(wxColour(255,255,255));


		wxStaticText *control;
		control = new wxStaticText(m_panels, wxID_ANY,  _T("Item Names:"), wxPoint(0,3), wxDefaultSize, wxBORDER_NONE );
		control = new wxStaticText(m_panels, wxID_ANY,  _T("Value Names:"), wxPoint(160,3), wxDefaultSize, wxBORDER_NONE );


		wxSize listboxsize(150,size.GetHeight()*m_panelScale-30);
		m_pListCtrl = new wxListCtrl( m_panels, ID_ListCtrl, wxPoint(0, 25), listboxsize, wxLC_REPORT | wxLC_SINGLE_SEL );
		wxListItem itemCol;
		itemCol.SetText(_T("Name"));
		itemCol.SetImage(-1);
		m_pListCtrl->InsertColumn(0, itemCol);

		itemCol.SetText(_T("Type"));
		itemCol.SetImage(-1);
		m_pListCtrl->InsertColumn(1, itemCol);

		m_valueSelectBox = new wxListBox( m_panels, ID_ValueSelect_Listbox, wxPoint(160, 25), listboxsize, 0, NULL, wxLB_EXTENDED | wxLB_HSCROLL, wxDefaultValidator );
		m_scalecheckbox = new wxCheckBox(m_panels, ID_Scale_CheckBox, _T("Log Scale"), wxPoint(320, 4), wxSize(100, 25));
		m_scalecheckbox->SetValue( true );

		control = new wxStaticText(m_panels, wxID_ANY,  _T("Vertical Scale:"), wxPoint(420,7), wxSize( 80, 25), wxBORDER_NONE );
		m_verticalscale_slider = new wxSlider( m_panels, ID_VertialScale_Slider, 1, 1, 10, wxPoint( 495, 7),  wxSize(size.GetWidth()-520, 25) );
		m_verticalscale_slider->SetRange( 1, 5000 );
		m_verticalscale_slider->SetValue( 1 );

		m_search_Text = new wxTextCtrl( m_panels, ID_Search_Text, _T(""), wxPoint( 320, 35), wxSize( size.GetWidth()-450, 25) );
		m_search_Button = new wxButton( m_panels, ID_Search_Button, _T("Search"), wxPoint( 320, 65), wxSize( 150, 25) );

		control = new wxStaticText(m_panels, wxID_ANY,  _T("Selected Value:"), wxPoint(320,size.GetHeight()*m_panelScale-30-25), wxSize( 120, 25), wxBORDER_NONE );
		m_selectedValue = new wxTextCtrl(m_panels, wxID_ANY,  _T(""), wxPoint(320,size.GetHeight()*m_panelScale-30), wxSize( size.GetWidth() - 520, 25));//, wxBORDER_NONE );

		m_sizer = new wxBoxSizer(wxVERTICAL);
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

	void CHistogramFrame::OnLogScaleCheck( wxCommandEvent& event )
	{
		GetCanvas()->SetLogScale(event.IsChecked());
	}

	void CHistogramFrame::OnListboxSelect(wxListEvent& event)
	{
		int nSel = event.m_itemIndex+1;
		m_item = nSel;
		CDataManager *dm =  GetDataManager();
		if ( !dm->m_ifDataReady )
			return;

		m_valueSelectBox->Clear();
		for ( unsigned int i = 0; i < dm->m_RawData->m_item_desc[nSel].num_values; i++ )
		{
			m_valueSelectBox->Append( _T(dm->m_RawData->m_item_desc[nSel].value_names[i].c_str()) );
		}

		set<CAbstractData*> _set;
		CAbstractData::GetAllParents(m_canvas->m_RDataList,_set,0, m_canvas->m_RDataList.size()-1);
		set<CAbstractData*>::iterator it = _set.begin();
		for(; it != _set.end(); it++)
		{
			CHistogramIData* idata = dynamic_cast<CHistogramIData*>(*it);
			assert(idata != NULL);
			idata->SetItem(nSel);
			idata->Update();
		}

		m_verticalscale_slider->SetRange( 1, dm->m_RawData->m_item_desc[nSel].num_values );
		GetCanvas()->Refresh(false);
	}

	void CHistogramFrame::OnSearchButton(wxCommandEvent& event)
	{
		CDataManager *dm =  GetDataManager();//CDataManager::Instance();
		if(dm->m_ifDataReady)
		{
			wxString search_wxstr;

			if(!(m_search_Text->IsEmpty()))
			{
				bool found = false;
				search_wxstr = m_search_Text->GetValue();
				char buf[100];
				strcpy( buf, (const char*)search_wxstr.mb_str(wxConvUTF8) );
				string search_str = buf;
				int i=0;

				int _sel = GetItem(); 
				if (!m_new_search)
					i = m_previous_scroll+1;
				i = m_valueSelectBox->GetScrollPos(0);
				m_previous_scroll = i;
				i++;
				for (;i<dm->m_RawData->m_item_desc[_sel].num_values;i++)
				{
					if((dm->m_RawData->m_item_desc[_sel].value_names[i]).find(search_str)!=string::npos)
					{	
						m_valueSelectBox->ScrollLines(i-m_previous_scroll);

						m_previous_scroll = i;
						found = true;
						break;
					}
				}
				if (!found)
				{
					if(m_new_search)
						wxMessageBox( _("The input text value is not found!"),
						_("Message"),wxOK | wxICON_INFORMATION, this );
					else// scroll back to find the first item
					{
						m_valueSelectBox->ScrollLines(0-m_previous_scroll);
						m_previous_scroll =0;
						i = 0;
						for (;i<dm->m_RawData->m_item_desc[_sel].num_values;i++)
						{
							if((dm->m_RawData->m_item_desc[_sel].value_names[i]).find(search_str)!=string::npos)
							{	
								m_valueSelectBox->ScrollLines(i-m_previous_scroll);
								m_previous_scroll = i;
								found = true;
								break;
							}
						}
					}
				}
				if(m_new_search)
					m_new_search = false;
			}
			else
				wxMessageBox( _("You need to input the search text!"),
				_("Message"),wxOK | wxICON_INFORMATION, this );
		}
	}

	void CHistogramFrame::OnSearchText(wxCommandEvent& event)
	{
		m_new_search = true;
	}

	void CHistogramFrame::OnVerticalScaleScroll( wxScrollEvent &event )
	{
		int _scale = m_verticalscale_slider->GetValue();
		GetCanvas()->SetVerticalScale( (float) _scale );
	}

	void CHistogramFrame::OnValueSelect( wxCommandEvent& event )
	{
		int _sel = event.GetSelection();
		wxString str = event.GetString();

		uint _item = GetItem(); 
		m_cf->UpdateBrush( _item, _sel );
		m_selectedValue->SetValue( str );
		m_cf->UpdateFilter();
		m_cf->RefreshAll();
	}


	void CHistogramFrame::RefleshPanel()
	{
		m_pListCtrl->DeleteAllItems();
		m_valueSelectBox->Clear();
		CDataManager *dm = GetDataManager();
		uint _item = GetItem();

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
		m_verticalscale_slider->SetRange( 1,(int)(dm->m_RawData->m_item_desc[_item].num_values) );
	}

	void CHistogramFrame::CreateDataForNewFilter()
	{
		CDataManager *dm = GetDataManager();
		CHistogramIData *idata = new CHistogramIData();
		
		dm->m_HighlightIDataList.push_back(idata);
		AddRelation(dm->m_FilteredDataList[dm->m_FilteredDataList.size()-1], idata, true);
		AddRelation(m_baseIData, idata, false);
		CHistogramIData* base = dynamic_cast<CHistogramIData*>(m_baseIData);
		idata->SetItem(base->GetItem());

		CHistogramRData* _rdata = new CHistogramRData(m_canvas);
		m_canvas->m_RDataList.push_back(_rdata);
		
		set<CAbstractData*> _set;
		CAbstractData::GetAllParents(m_canvas->m_RDataList, _set,1, m_canvas->m_RDataList.size() -2);
		set<CAbstractData*>::iterator it = _set.begin();
		for(; it != _set.end(); it++ )
			AddRelation(*it , _rdata, false );
		AddRelation( idata, _rdata, true );
	}

	/**************************************************************************************************************************************************
	************************** CHistogramCanvas *************************************************************************************************************
	**************************************************************************************************************************************************/

	BEGIN_EVENT_TABLE(CHistogramCanvas, CCanvas)
		EVT_SIZE(CCanvas::OnSize)
		EVT_PAINT(CCanvas::OnPaint)
		EVT_ERASE_BACKGROUND(CCanvas::OnEraseBackground)
		EVT_KEY_DOWN(CCanvas::OnKeyDown)
		EVT_MOUSE_EVENTS(CCanvas::OnMouse)
		EVT_IDLE(CCanvas::OnIdle) 
	END_EVENT_TABLE()

	CHistogramCanvas::CHistogramCanvas( wxWindow *parent, wxWindowID id,
		const wxPoint& pos, const wxSize& size, long style, const wxString& name)
		: CCanvas(parent, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE , name ), m_vertical_scale(1.0f)
	{
		rf = (CHistogramFrame*) parent;
		InitOrtho();
		CHistogramRData* _rdata = new CHistogramRData(this);
		_rdata->m_vbo->SetCanvas(this);
		m_RDataList.push_back(_rdata);
		AddRelation( rf->m_baseIData, _rdata, true );	
	}

	void CHistogramCanvas::SetLogScale ( bool _value ) 
	{ 
		for(int i = 0; i < m_RDataList.size(); i++)
		{
			CHistogramRData * hrdata = dynamic_cast<CHistogramRData*>(m_RDataList[i]);
			hrdata->SetLogScale(_value);
			m_RDataList[i]->Update();
		}
		Refresh(false);
	}
	
	void CHistogramCanvas::SetVerticalScale( float _value ) 
	{
		m_vertical_scale = _value;
		for(int i = 0; i < m_RDataList.size(); i++)
		{
			CHistogramRData * hrdata = dynamic_cast<CHistogramRData*>(m_RDataList[i]);
			hrdata->SetVerticalScale(_value);
			m_RDataList[i]->Update();
		}
		Refresh(false);
	}

	void CHistogramCanvas::DrawAxis()
	{
		CDataManager *dm = GetDataManager();//CDataManager::Instance();

		unsigned int _sel = ((CHistogramFrame*)rf)->GetItem();
		unsigned int nValues = dm->m_RawData->m_item_desc[_sel].num_values;

		if ( nValues == 0 || _sel == 0)
			return;

		glColor4fv( BLACK );
		glBegin( GL_LINES );
		glVertex3f( 0.0, 0.0, 0.0 );
		glVertex3f( 0.0, 1.0, 0.0 );
		glVertex3f( 0.0, 0.0, 0.0 );
		glVertex3f( 1.0, 0.0, 0.0 );
		glEnd();
		float step = 1.0f / 6.0f;

		for ( int i = 1; i < 7; i++ )
		{
			glBegin( GL_LINES );
			glVertex3f( 0.0, i * step, 0.0 );
			glVertex3f( -0.01, i * step, 0.0 );
			glEnd();
		}

		m_maxvalue = CHistogramIData::s_maxValue;
		char str[256];
		strcpy( str, dm->m_RawData->m_item_desc[_sel].name.c_str() );
		glRasterPos2f( 0.4f, -0.08f);
		glPrint( "%s", str );

		sprintf( str, "%d", (int)(m_maxvalue / m_vertical_scale));
		glRasterPos2f( -0.1f, 0.956f );
		glPrint( "%s", str );
		sprintf( str, "0");
		glRasterPos2f( -0.1f, 0.0f );
		glPrint( "%s", str );
		for ( int i = 1; i < 6; i++ )
		{
			float temp, _value;
			CHistogramRData *rdata = dynamic_cast<CHistogramRData*>(m_RDataList[0]);
			if ( rdata->m_if_logScale )
			{
				temp = log10((float)(m_maxvalue+1)) * i / 6.0f;
				_value = pow( 10.0f, temp);
			}
			else
				_value = (float)m_maxvalue * i / 6.0f;

			_value /= m_vertical_scale;
			sprintf( str, "%d", (int)_value );
			glRasterPos2f( -0.1, i*step);
			glPrint( "%s", str );
		}
	}

	void CHistogramCanvas::RenderVBO()
	{
		glEnableClientState(GL_VERTEX_ARRAY);
		CDataManager *dm = GetDataManager();
		CHistogramRData * hrdata;
		for(unsigned int i = m_RDataList.size(); i >0 ; i-- )
		{
			if(i == m_RDataList.size()) 
			{
				glColor4fv(GRAY);
				hrdata = dynamic_cast<CHistogramRData*>(m_RDataList[0]);
			}
			else
			{
				glColor4ub(dm->m_colorTable[i-1].first.red,dm->m_colorTable[i-1].first.green,dm->m_colorTable[i-1].first.blue,dm->m_colorTable[i-1].first.alpha);
				hrdata = dynamic_cast<CHistogramRData*>(m_RDataList[i]);
			}
			glBindBuffer(GL_ARRAY_BUFFER, hrdata->m_vbo->m_vertexBufferId);
			glVertexPointer(hrdata->m_vbo->m_vertexFromat,GL_FLOAT,0,0);
			
			glUseProgram(p);
			glDrawArrays(GL_LINES,0,hrdata->m_vbo->m_vertexNum);
			glUseProgram(NULL);
		}
		glDisableClientState(GL_VERTEX_ARRAY);
	}


	void CHistogramCanvas::DrawIndication()
	{
		//the text lable info
		uint _sel = ((CHistogramFrame*)rf)->GetItem();
		CDataManager *dm = GetDataManager();

		CHistogramIData* idataBase  = dynamic_cast<CHistogramIData*>(m_RDataList[0]->m_parent[0]);
		assert(idataBase != NULL);

		float step = 1.0f / idataBase->m_numHistogram;
		float height =0.0f;
		m_maxvalue = CHistogramIData::s_maxValue;

		for(int i = 0; i < idataBase->m_numHistogram; i++)
		{
			for(int j = 1; j < m_RDataList.size(); j++)
			{
				CHistogramRData* hrdata = dynamic_cast<CHistogramRData*>(m_RDataList[j]);
				assert(hrdata != NULL);
				CHistogramIData* hidata = dynamic_cast<CHistogramIData*>(hrdata->m_parent[hrdata->m_parent.size()-1]);
				assert(hidata != NULL);
				if(hidata->m_histogram[i] != 0)
				{
					if( hrdata->m_if_logScale )
						height = log10((float)hrdata->m_acuHeight[i] +1.0) /log10(m_maxvalue +1.0f);
					else
						height = (float) (hrdata->m_acuHeight[i]) / (CHistogramIData::s_maxValue);
					height *= m_vertical_scale;
					height += 0.01;
					float _pos[3] ={step*(i+1) - 0.5*step, height, 0.5};
					glColor4fv(BLACK);
					char str[256];
					glRasterPos3fv(_pos);
					sprintf( str, "%d of %d", hidata->m_histogram[i], idataBase->m_histogram[i] );
					glPrint("%s",str);
					_pos[1] = -0.03;
					glRasterPos3fv(_pos);
					unsigned int index = idataBase->m_index[idataBase->m_reference[i]];
					sprintf( str, "%s", (char*)(dm->m_RawData->m_item_desc[_sel].value_names[index]).c_str() );
					glPrint("%s",str);
				}
			}
		}	
	}

	void CHistogramCanvas::DrawCanvas()
	{
		RenderVBO();
		DrawIndication();
	}


	void CHistogramCanvas::SelectData()
	{	
		CDataManager *dm = GetDataManager();
		if(dm->m_ifDataReady == false)
			return;
		uint _sel = ((CHistogramFrame*)rf)->GetItem();
		CHistogramIData* idata = dynamic_cast<CHistogramIData*>(m_RDataList[0]->m_parent[0]);
		CHistogramRData* rdata = dynamic_cast<CHistogramRData*>(m_RDataList[0]);
		assert(idata != NULL);

		unsigned int *_data = idata->m_histogram;
		uint _nValues = idata->m_numHistogram;
		m_maxvalue = CHistogramIData::s_maxValue;
		float step = 1.0f / _nValues;

		int x1 = round_value( m_newOrtho[0], 0.0f, step );
		int x2 = round_value( m_newOrtho[2], 0.0f, step );
		if (x1 < 0)
			x1 = 0;
		if (x2 >= _nValues)
			x2 = _nValues -1;
		else if( x1 > 0)
		{
			if( m_newOrtho[0] > (x1*step  + 0.9f*step ))
				x1++;
		}
		if (x2 < 0)
			x2 = 0;
		else if( x2 > 0)
		{
			if(m_newOrtho[2] < (x2*step + 0.1*step ))
				x2--;
		}

		float y1, y2;
		y1 = m_newOrtho[1];
		y2 = m_newOrtho[3];

		if (y2 < 0)
			return;
		unsigned int index;
		if(dm->m_BrushType <= Brush_OR_EX)
		{
			for(int i = x1; i <= x2; i++)
			{
				float _height;
				if( rdata->m_if_logScale )
					_height = log10( (float)(_data[i] + 1)) / log10((float)m_maxvalue + 1);
				else
					_height = (float)_data[i] / m_maxvalue;
				_height *= m_vertical_scale;

				if(_height <= PRECISION)
					continue;

				if( y1 < _height)
				{
					index = idata->m_index[idata->m_reference[i]];
					rf->m_cf->UpdateBrush(_sel,index);
					rf->m_cf->UpdateFilter();
				}
			}
		}
		else
		{
			map<uint, uint> _totalData;
			for(uint i= dm->m_TimeWindowData->m_StartIdx; i<= dm->m_TimeWindowData->m_EndIdx; i++)
			{
				uint _value = dm->m_RawData->GetDataValue(i,_sel);
				if( _totalData.find(_value) != _totalData.end() )
					_totalData[_value] = _totalData[_value] +1;
				else
					_totalData[_value] = 1;
			}

			for(int i = x1; i <= x2; i++)
			{
				float _height;
				if( rdata->m_if_logScale )
					_height = log10( (float)(_totalData[i] + 1)) / log10((float)m_maxvalue + 1);
				else
					_height = (float)_totalData[i] / m_maxvalue;
				_height *= m_vertical_scale;

				if(_height <= PRECISION)
					continue;

				if( y1 < _height)
				{
					
					index = idata->m_index[idata->m_reference[i]];
					rf->m_cf->UpdateBrush(_sel,index);
					rf->m_cf->UpdateFilter();
				}
			}
		}	
	}

	void CHistogramCanvas::InitGL()
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

		glClearColor( WHITE[0], WHITE[1], WHITE[2], WHITE[3] );
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
		SetShader(); 
	}

	void CHistogramCanvas::SetShader()
	{
		string _vertShaerName ="GLSL/dynamic.vert";
		string _fragShaerName ="GLSL/dynamic.frag";
		string _geomShaerName ="GLSL/dynamic.geom";

		char *vs = NULL, *fs = NULL, *gs = NULL;

		//First, create our shaders 
		v = glCreateShader(GL_VERTEX_SHADER);
		f = glCreateShader(GL_FRAGMENT_SHADER);
		g = glCreateShader(GL_GEOMETRY_SHADER_EXT);

		//Read in the programs
		vs = textFileRead(_vertShaerName.c_str());
		fs = textFileRead(_fragShaerName.c_str());
		gs = textFileRead(_geomShaerName.c_str());

		//Setup a few constant pointers for below
		const char * ff = fs;
		const char * vv = vs;
		const char * gg = gs;

		glShaderSource(v, 1, &vv, NULL);
		glShaderSource(f, 1, &ff, NULL);
		glShaderSource(g, 1, &gg, NULL);

		free(vs);free(fs);
		free(gs);

		glCompileShader(v);
		glCompileShader(f);
		glCompileShader(g);

		p = glCreateProgram();

		glAttachShader(p,f);
		glAttachShader(p,v);
		glAttachShader(p,g);

		glProgramParameteriEXT(p,GL_GEOMETRY_INPUT_TYPE_EXT,GL_LINES);
		glProgramParameteriEXT(p,GL_GEOMETRY_OUTPUT_TYPE_EXT,GL_TRIANGLES);

		int temp;
		glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT,&temp);
		glProgramParameteriEXT(p,GL_GEOMETRY_VERTICES_OUT_EXT,temp);

		glLinkProgram(p);

		char *infoLog;
		infoLog = printShaderInfoLog(v);
		if ( infoLog != NULL )
		{
			wxLogMessage("Vertex Shader InfoLog: %s\n",infoLog);
			delete(infoLog);
		}
		infoLog = printShaderInfoLog(f);
		if ( infoLog != NULL )
		{
			wxLogMessage("Fragment Shader InfoLog: %s\n",infoLog);
			delete(infoLog);
		}
		infoLog = printShaderInfoLog(g);
		if ( infoLog != NULL )
		{
			wxLogMessage("printShaderInfoLog: %s\n",infoLog);
			delete(infoLog);
		}
		infoLog = printProgramInfoLog(p);
		if ( infoLog != NULL )
		{
			wxLogMessage("Program InfoLog: %s\n",infoLog);
			delete(infoLog);
		}
	}
}