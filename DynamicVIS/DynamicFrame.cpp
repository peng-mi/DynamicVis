#include <GL/glew.h>

//dynamic
#include "DynamicFrame.h"
#include "DynamicData.h"

//wxwidgets
#include <wx/notebook.h>
#include <wx/listbox.h>
#include <wx/datectrl.h>
#include <wx/combo.h>
#include <wx/listctrl.h>

//helpers
#include "color.h"
#include "glhelper.h"
#include "shaderhelper.h"

//data and controlframe
#include "ControlFrame.h"
#include "datamanager.h"
#include "RawData.h"
#include "FilteredData.h"
#include "VBOData.h"
#include "TimeData.h"

namespace VIS
{
	enum
	{
		ID_QUIT  = wxID_EXIT,
		ID_HIDE,

		ID_Dynamic_ScaleCheckBox,
		ID_Dynamic_Listbox,
		ID_DynamicSpeical_Listbox,


	};

	BEGIN_EVENT_TABLE(CDynamicFrame, CFrame)
		EVT_MENU(ID_QUIT, CFrame::OnExit)
		EVT_SIZE(CFrame::OnSize)
		EVT_MENU(ID_HIDE, CFrame::OnHide)
		EVT_CHAR_HOOK(CFrame::OnKeyDown)
		EVT_CHECKBOX( ID_Dynamic_ScaleCheckBox, CDynamicFrame::OnDynamicLogScaleCheck)
		EVT_LIST_ITEM_SELECTED (ID_Dynamic_Listbox, CDynamicFrame::OnDynamicSelect)
		EVT_LISTBOX (ID_DynamicSpeical_Listbox, CDynamicFrame::OnDynamicSpecialSelect)
		END_EVENT_TABLE()

	CDynamicFrame *CDynamicFrame::Create(CControlFrame* _cf, const wxString& title, const wxPoint& pos,
		const wxSize& size, long style )
	{
		CDynamicFrame *rf = new CDynamicFrame(_cf,title, pos, size, style);
		rf->Show(true);
		return rf;
	}

	CDynamicFrame::CDynamicFrame(CControlFrame* _cf,const wxString& title, const wxPoint& pos,
		const wxSize& size, long style)
		: CFrame(_cf,title, pos, size, style )
	{
		CDataManager *dm = GetDataManager();
		m_baseIData = new CDynamicIData();
		dm->m_BasedIDataList.push_back(m_baseIData);
		AddRelation( dm->m_BasedFilteredData, m_baseIData, true);
		AddRelation( dm->m_TimeStepData, m_baseIData, false);
		m_canvas = new CDynamicCanvas( this, wxID_ANY,wxDefaultPosition, wxSize(size.GetWidth(),size.GetWidth()));

		m_panelScale = 0.4f;

		unsigned int disp = 145;
		wxSize listboxsize( 150, size.GetHeight()-disp );
		wxSize selectionboxsize ( size.GetWidth() - 170, size.GetHeight()-disp );

		wxSize panelSize(size.GetWidth(),size.GetHeight()*m_panelScale);
		m_panels = new wxPanel(this, wxID_ANY, wxDefaultPosition, panelSize, wxTAB_TRAVERSAL);
		m_panels->SetBackgroundColour(wxColour(255,255,255));

		wxStaticText  *control;
		control= new wxStaticText(m_panels, wxID_ANY, _T("Item Names:"), wxPoint(0,3), wxDefaultSize, wxBORDER_NONE);
		control= new wxStaticText(m_panels, wxID_ANY, _T("Special Names:"), wxPoint(160,3), wxDefaultSize, wxBORDER_NONE);
		m_ScaleCheckBox = new wxCheckBox(m_panels, ID_Dynamic_ScaleCheckBox, _T("Log Scale"), wxPoint(320, 3), wxSize(100, 25));
		m_DynamicListBox = new wxListCtrl( m_panels, ID_Dynamic_Listbox, wxPoint(0, 25), listboxsize, wxLC_REPORT | wxLC_SINGLE_SEL );
		wxListItem itemCol;
		itemCol.SetText(_T("Name"));
		itemCol.SetImage(-1);
		m_DynamicListBox->InsertColumn(0, itemCol);

		itemCol.SetText(_T("Type"));
		itemCol.SetImage(-1);
		m_DynamicListBox->InsertColumn(1, itemCol);
		m_DynamicSpecialListBox = new wxListBox( m_panels, ID_DynamicSpeical_Listbox, wxPoint(160, 25), listboxsize, 0, NULL, wxLB_EXTENDED | wxLB_HSCROLL, wxDefaultValidator );

		m_sizer = new wxBoxSizer(wxVERTICAL);
		m_sizer->Layout();
		m_panels->SetSizer(m_sizer);
		SetExtraStyle(wxWS_EX_PROCESS_IDLE );

		m_mgr.AddPane(m_canvas, wxAuiPaneInfo().
			Name(wxT("canvas")).Caption(wxT("canvas")).
			MaxSize(size).Layer(0).
			CloseButton(true).MaximizeButton(true));

		m_mgr.AddPane(m_panels, wxAuiPaneInfo().
			Name(wxT("panel")).Caption(wxT("panel")).
			MaxSize(panelSize).Layer(0).
			CloseButton(true).MaximizeButton(false));

		m_mgr.GetPane(wxT("canvas")).Show().Center().Layer(0).Position(0);
		m_mgr.GetPane(wxT("panel")).Show().Center().Layer(0).Position(0);
		m_mgr.Update();
	}

	void CDynamicFrame::RefleshPanel()
	{
		m_DynamicListBox->DeleteAllItems();
		m_DynamicSpecialListBox->Clear();
		CDataManager *dm = GetDataManager();

		for(unsigned int i=1; i < dm->m_RawData->m_item_desc.size(); i++ )
		{
			long tmp = m_DynamicListBox->InsertItem( i-1, wxString::FromUTF8(dm->m_RawData->m_item_desc[i].name.c_str()), 0 );

			if ( dm->m_RawData->m_item_desc[i].num_values != 0 )
			{
				m_DynamicListBox->SetItem( tmp, 1, _T("CAT") );
				wxListItem item;
				item.m_itemId = i-1;
				item.SetTextColour(*wxLIGHT_GREY);
				item.SetFont(*wxITALIC_FONT);
				m_DynamicListBox->SetItem( item );
			}
			else
			{
				m_DynamicListBox->SetItem( tmp, 1, _T("NUM") );
			}
		}
		//m_DynamicSpecialListBox->Append( _T("Market Cap"));

	}
	
	void CDynamicFrame::CreateDataForNewFilter()
	{
		CDataManager *dm = GetDataManager();
		CDynamicIData *idata = new CDynamicIData();
		
		dm->m_HighlightIDataList.push_back(idata);
		AddRelation(dm->m_FilteredDataList[dm->m_FilteredDataList.size()-1], idata, true);
		AddRelation( dm->m_TimeStepData, idata, false);
		CDynamicIData* itmp = NULL;
		CDynamicRData* rtmp = NULL;
		itmp = dynamic_cast<CDynamicIData*>(m_canvas->m_RDataList[0]->m_parent[0]);
		assert(itmp != NULL);
		idata->m_Item = itmp->m_Item;
		rtmp = dynamic_cast<CDynamicRData*>(itmp->m_child[0]);
		assert(rtmp != NULL);

		CDynamicRData* rdata = new CDynamicRData(m_canvas);
		rdata->m_if_logScale = rtmp->m_if_logScale;
		m_canvas->m_RDataList.push_back(rdata);

		set<CAbstractData*> _set;
		CAbstractData::GetAllParents(m_canvas->m_RDataList,_set,1,m_canvas->m_RDataList.size()-2);
		set<CAbstractData*>::iterator it = _set.begin();
		for(; it != _set.end(); it++)
			AddRelation(*it,rdata, false);
		AddRelation( idata, rdata , true);
	}

	void CDynamicFrame::OnDynamicLogScaleCheck( wxCommandEvent& event )
	{
		GetCanvas()->SetLogScale(event.IsChecked());	 
	}

	void CDynamicFrame::OnDynamicSelect(wxListEvent& event)
	{
		int nSel = event.m_itemIndex+1;

		CDataManager *dm = GetDataManager();
		if ( !dm->m_ifDataReady )
			return;
		m_DynamicSpecialListBox->DeselectAll();

		CDynamicIData* idata = NULL;
		for(unsigned int  i = 0; i < m_canvas->m_RDataList.size(); i++)
		{
			idata = dynamic_cast<CDynamicIData*>(m_canvas->m_RDataList[i]->m_exclusiveParents[0]);
			assert(idata != NULL);
			if ( dm->m_RawData->m_item_desc[nSel].num_values == 0 ) // this is numerical
			{
				CDynamicIData::s_minvalue = dm->m_RawData->m_item_desc[nSel].max;
				CDynamicIData::s_maxvalue = dm->m_RawData->m_item_desc[nSel].min;
			}
			else //if ( nSel == idata->m_Item ) // this is categorical
			{
				CDynamicIData::s_minvalue = 0.0f;
				CDynamicIData::s_maxvalue = 6.0f;
				nSel = -1;
			}
			idata->m_Item = nSel;
			idata->Update();	
		}
		GetCanvas()->Refresh( false );
	}

	void CDynamicFrame::OnDynamicSpecialSelect(wxCommandEvent& event)
	{
		int nSel = event.GetSelection()+1;
		CDataManager *dm = GetDataManager();
		if ( !dm->m_ifDataReady )
			return;

		CDynamicIData* idata = NULL;
		for(unsigned int i = 0; i < m_canvas->m_RDataList.size(); i++)
		{
			idata  = dynamic_cast<CDynamicIData*>(m_canvas->m_RDataList[i]->m_parent[m_canvas->m_RDataList[i]->m_parent.size()-1]);
			assert(idata != NULL);
			if(nSel == idata->m_SpecialItem)
			{
				m_DynamicSpecialListBox->DeselectAll();
				idata->m_SpecialItem = -1;
			}
			else 
				idata->m_SpecialItem = nSel;
			idata->Update();
		}
		GetCanvas()->Refresh( false );
	}


	/**************************************************************************************************************************************************
	************************** The Canvas *************************************************************************************************************
	**************************************************************************************************************************************************/


	BEGIN_EVENT_TABLE(CDynamicCanvas, CCanvas)
		EVT_SIZE(CCanvas::OnSize)
		EVT_PAINT(CCanvas::OnPaint)
		EVT_ERASE_BACKGROUND(CCanvas::OnEraseBackground)
		EVT_KEY_DOWN(CCanvas::OnKeyDown)
		EVT_MOUSE_EVENTS(CCanvas::OnMouse)
		EVT_IDLE(CCanvas::OnIdle) 
		END_EVENT_TABLE()

	CDynamicCanvas::CDynamicCanvas( wxWindow *parent, wxWindowID id,
		const wxPoint& pos, const wxSize& size, long style, const wxString& name)
		: CCanvas(parent, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE , name )
	{
		rf = (CDynamicFrame*) parent;
		InitOrtho();
		CDynamicRData* _rdata = new CDynamicRData(this);
		m_RDataList.push_back(_rdata);
		AddRelation(rf->m_baseIData, _rdata, true);
	}

	void CDynamicCanvas::DrawAxis()
	{
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

		CDynamicIData* idata = dynamic_cast<CDynamicIData*>(m_RDataList[0]->m_parent[0]);
		CDynamicRData* rdata = dynamic_cast<CDynamicRData*>(m_RDataList[0]);
		float maxvalue = CDynamicIData::s_maxvalue;
		float minvalue = CDynamicIData::s_minvalue;

		char str[256];
		if ( idata->m_Item == -1 && idata->m_SpecialItem == -1 ) // Categorical Item
		{
			sprintf( str, "%d", (int)maxvalue);
			glRasterPos2f( -0.1, 0.956 );
			glPrint( "%s", str );
			sprintf( str, "0");
			glRasterPos2f( -0.1, 0.0 );
			glPrint( "%s", str );
			for ( int i = 1; i < 6; i++ )
			{
				float temp, _value;
				if ( rdata->m_if_logScale )
				{
					temp = log10((float)(maxvalue+1)) * i / 6.0f;
					_value = pow( 10.0f, temp);
				}
				else
					_value = (float)maxvalue * i / 6.0f;

				sprintf( str, "%d", (int)_value );
				glRasterPos2f( -0.1, i*step);
				glPrint( "%s", str );
			}
		}
		else // Numercial Data
		{
			float _max = maxvalue;
			float _min = minvalue;
			sprintf( str, "%f", _max );
			glRasterPos2f( -0.1, 0.956 );
			glPrint( "%s", str );
			sprintf( str, "%f", _min );
			glRasterPos2f( -0.1, 0.0 );
			glPrint( "%s", str );

			float _step = (_max - _min ) * step;

			for ( int i = 1; i < 6; i++ )
			{
				float temp, _value;
				if ( rdata->m_if_logScale )
				{
					temp = log10((float)(maxvalue+1)) * i / 6.0f;
					_value = pow( 10.0f, temp);
				}
				else
					_value = _min+i*_step;

				sprintf( str, "%f", _value );
				glRasterPos2f( -0.1, i*step);
				glPrint( "%s", str );
			}
		}
	}


	void CDynamicCanvas::InitGL()
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

	void CDynamicCanvas::SetShader()
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

	void CDynamicCanvas::DrawCanvas()
	{
		RenderVBO();
		DrawIndication();
	}

	void CDynamicCanvas::SelectData()
	{
	}

	void CDynamicCanvas::RenderVBO()
	{
		glEnableClientState(GL_VERTEX_ARRAY);
		CDataManager *dm = GetDataManager();

		
		CDynamicRData * drdata;
		for(unsigned int i = m_RDataList.size(); i > 0; i-- )
		{
			if( i == m_RDataList.size())
			{
				glColor4fv(GRAY);
				drdata = dynamic_cast<CDynamicRData*>(m_RDataList[0]);
			}
			else
			{
				glColor4ub(dm->m_colorTable[i-1].first.red,dm->m_colorTable[i-1].first.green,dm->m_colorTable[i-1].first.blue,dm->m_colorTable[i-1].first.alpha);
				drdata = dynamic_cast<CDynamicRData*>(m_RDataList[i]);
			}

			glBindBuffer(GL_ARRAY_BUFFER, drdata->m_vbo->m_vertexBufferId);
			glVertexPointer(drdata->m_vbo->m_vertexFromat,GL_FLOAT,0,0);
			CDynamicIData* didata = dynamic_cast<CDynamicIData*>(drdata->m_parent[0]);
			if ( didata->m_Item == -1 && didata->m_SpecialItem == -1 )
				glUseProgram(p);
			glDrawArrays(GL_LINE_STRIP,0,drdata->m_vbo->m_vertexNum);
			glUseProgram(NULL);
		}
		glDisableClientState(GL_VERTEX_ARRAY);
	}

	void CDynamicCanvas::DrawIndication()
	{
		CDataManager *dm = GetDataManager();
		CDynamicIData *idata = dynamic_cast<CDynamicIData*>(m_RDataList[0]->m_parent[0]);
		bool if_categorical = (idata->m_Item == -1 || idata->m_SpecialItem != -1);
		CDynamicRData *rdata = dynamic_cast<CDynamicRData*>(m_RDataList[0]);
		float step = 1.0f / (idata->m_size - 1);
		unsigned int idx;
		char tmpstr[32];

		if ( !dm->m_ifSkip )
			idx = (dm->m_CurTimeData->GetCurTime() - dm->m_MinTime) / dm->m_TimeStepData->GetTimeStep();
		else
			idx = dm->m_CurrentTimesliderIdx;

		float _max = CDynamicIData::s_maxvalue;
		float _min = CDynamicIData::s_minvalue;
		if( fabs(_max - _min) <= PRECISION)
			_max += 1.0f;
		float _height, _tmp;

		if ( idx*step > m_Ortho[1] )
		{
			float delta = 0.8f * (m_Ortho[1] - m_Ortho[0]);
			m_Ortho[0] += delta;
			m_Ortho[1] += delta;
		}

		else if(idx*step < m_Ortho[0])
		{
			float delta = 0.8f * (m_Ortho[1] - m_Ortho[0]);
			m_Ortho[0] -= delta;
			m_Ortho[1] -= delta;
		
		}

		time_t cur_time = dm->m_CurTimeData->GetCurTime();
		struct tm *timeinfo = gmtime( (const time_t *)(&cur_time) );
		int _year = timeinfo->tm_year+1900;
		int _month = timeinfo->tm_mon+1;
		int _hour = timeinfo->tm_hour +1;

		glColor4fv( BLACK );
		sprintf( tmpstr, "%4d/%2d/%2d,%2d:%2d:%2d", _year,_month,timeinfo->tm_mday,_hour,timeinfo->tm_min,timeinfo->tm_sec );
		glRasterPos2f( idx*step, -0.02f);
		glPrint( tmpstr );

		//color indicator 
		for(int i = 0; i < m_RDataList.size(); i++)
		{
			rdata = dynamic_cast<CDynamicRData*>(m_RDataList[i]);
			CDynamicIData* idata = dynamic_cast<CDynamicIData*>(rdata->m_exclusiveParents[0]);
			
			if( rdata->m_if_logScale )
			{
				_tmp = 1.0f / log10(_max-_min+1.0f);
				if( if_categorical )
					_height = log10(rdata->m_acuHeight[idx] + idata->m_data[idx] -_min+ 1.0f) * _tmp;
				else
					_height = log10(idata->m_data[idx] -_min+ 1.0f) * _tmp;

			}
			else
			{
				_tmp = 1.0f / (_max-_min);
				if( if_categorical )
					_height = (rdata->m_acuHeight[idx] + idata->m_data[idx] -_min) * _tmp;
				else
					_height = (idata->m_data[idx] -_min) * _tmp;
			}
			
			glPointSize( 3.0f );
			glColor4fv( BLUE );
			glBegin( GL_POINTS );
			glVertex3f( idx*step, _height, 0.0f );
			glEnd();

			glBegin( GL_LINES );
			glVertex3f( idx*step, _height, 0.0f );
			glVertex3f( idx*step, 0.0f, 0.0f );
			glEnd();

			glColor4fv( BLACK );
			glRasterPos2f( idx*step, _height);

			if ( if_categorical )
			{
				if ( (unsigned int) idata->m_data[idx] != 0 )
				{
					sprintf( tmpstr, "%d", (unsigned int) idata->m_data[idx] );
					glPrint( tmpstr );
				}
			}
			else
			{
				sprintf( tmpstr, "%f", idata->m_data[idx] );
				glPrint( tmpstr );
			}
		}
	}

	void CDynamicCanvas::SetLogScale(bool _value)
	{
		for(int i = 0; i < m_RDataList.size(); i++)
		{
			CDynamicRData *rdata = dynamic_cast<CDynamicRData*>(m_RDataList[i]);
			rdata->SetLogScale(_value);
			rdata->Incremental();
		}
		Refresh(false);
	}
}