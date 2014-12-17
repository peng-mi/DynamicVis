#include <GL/glew.h>

//dynamic
#include "TSPlotFrame.h"
#include "TimeSeriesData.h"

//wxwidgets
#include <wx/listctrl.h>

//helpers
#include "color.h"
#include "glhelper.h"
#include "shaderhelper.h"

//data and controlframe
#include "Data.h"
#include "ControlFrame.h"
#include "datamanager.h"
#include "RawData.h"
#include "Filter.h"
#include "FilteredData.h"
#include "VBOData.h"


#define ONE_SECOND 1

namespace VIS
{

enum
{
    ID_QUIT  = wxID_EXIT,
	ID_TSPlotBuildKey_Listbox,
	ID_TSPlotItem_Listbox,
	ID_BuildTimeSeries_Button,
	ID_TimeSeriesPlot_ScaleCheckBox,
	ID_BuildAllCheckBox,
};

BEGIN_EVENT_TABLE(CTimeSeriesPlotFrame, CFrame)
    EVT_MENU(ID_QUIT, CFrame::OnExit)
	EVT_CHECKBOX( ID_TimeSeriesPlot_ScaleCheckBox, CTimeSeriesPlotFrame::OnTimeSeriesPlotLogScaleCheck)
//	EVT_CHECKBOX( ID_BuildAllCheckBox, CTimeSeriesPlotFrame::OnBuildAllCheck)
	EVT_LIST_ITEM_SELECTED (ID_TSPlotBuildKey_Listbox, CTimeSeriesPlotFrame::OnTSPlotBuildKeySelect)
	EVT_LIST_ITEM_SELECTED (ID_TSPlotItem_Listbox, CTimeSeriesPlotFrame::OnTSPlotItemSelect)
	EVT_BUTTON ( ID_BuildTimeSeries_Button, CTimeSeriesPlotFrame::OnBuildTimeSeries)
END_EVENT_TABLE()


CTimeSeriesPlotFrame *CTimeSeriesPlotFrame::Create( CControlFrame* _cf, const wxString& title, const wxPoint& pos,
										 const wxSize& size, long style )
{
	CTimeSeriesPlotFrame *rf = new CTimeSeriesPlotFrame(_cf, title, pos, size, style);
    rf->Show(true);
    return rf;
}

CTimeSeriesPlotFrame::CTimeSeriesPlotFrame(CControlFrame* _cf, const wxString& title, const wxPoint& pos,
						   const wxSize& size, long style)
						   : CFrame(_cf, title, pos, size, style )
{
	CDataManager *dm = GetDataManager();
	m_baseIData = new CTimeSeriesIData();
	dm->m_BasedIDataList.push_back(m_baseIData);
	AddRelation( dm->m_RawData, m_baseIData, true );
	AddRelation( dm->m_ExclusiveFilter, m_baseIData, false );
	AddRelation( dm->m_NegExclusiveFilter, m_baseIData, false );
	m_canvas = new CTimeSeriesPlotCanvas( this, wxID_ANY,wxDefaultPosition, wxSize(size.GetWidth(),size.GetWidth()));

	m_panelScale = 0.4f;

	unsigned int disp = 145;
	wxSize listboxsize( 150, size.GetHeight()-disp );

	m_mgr.AddPane(m_canvas, wxAuiPaneInfo().
		Name(wxT("canvas")).Caption(wxT("canvas")).
		MaxSize(size).Layer(0).
		CloseButton(true).MaximizeButton(true));
	
	wxSize panelSize(size.GetWidth(),size.GetHeight()*m_panelScale);
	m_panels = new wxPanel(this, wxID_ANY, wxDefaultPosition, panelSize, wxTAB_TRAVERSAL);
	m_panels->SetBackgroundColour(wxColour(255,255,255));

	wxStaticText *control;
	control = new wxStaticText(m_panels, wxID_ANY,  _T("Select Key:"), wxPoint(0,3), wxDefaultSize, wxBORDER_NONE );
	control = new wxStaticText(m_panels, wxID_ANY,  _T("Select Item:"), wxPoint(160,3), wxDefaultSize, wxBORDER_NONE );

	{
		m_TSPlotBuildKeySelectBox = new wxListCtrl( m_panels, ID_TSPlotBuildKey_Listbox, wxPoint(0, 25), listboxsize, wxLC_REPORT | wxLC_SINGLE_SEL );
		wxListItem itemCol;
		itemCol.SetText(_T("Name"));
		itemCol.SetImage(-1);
		m_TSPlotBuildKeySelectBox->InsertColumn(0, itemCol);

		itemCol.SetText(_T("Type"));
		itemCol.SetImage(-1);
		m_TSPlotBuildKeySelectBox->InsertColumn(1, itemCol);
	}

	{
		m_TSPlotItemSelectBox = new wxListCtrl( m_panels, ID_TSPlotItem_Listbox, wxPoint(160, 25), listboxsize, wxLC_REPORT | wxLC_SINGLE_SEL );
		wxListItem itemCol;
		itemCol.SetText(_T("Name"));
		itemCol.SetImage(-1);
		m_TSPlotItemSelectBox->InsertColumn(0, itemCol);

		itemCol.SetText(_T("Type"));
		itemCol.SetImage(-1);
		m_TSPlotItemSelectBox->InsertColumn(1, itemCol);
	}
//	m_buildTS_Button = new wxButton( m_panels, ID_BuildTimeSeries_Button, _T("Build Time Series"), wxPoint( 320, 25), wxSize( 150, 25) );
//	m_buildAllcheckbox = new wxCheckBox(m_panels, ID_BuildAllCheckBox, _T("Include All Records"), wxPoint(320, 3), wxSize(150, 25));
//	m_buildAllcheckbox->SetValue( true );
	m_timeseriesplot_scalecheckbox = new wxCheckBox(m_panels, ID_TimeSeriesPlot_ScaleCheckBox, _T("Log Scale"), wxPoint(320, 3), wxSize(100, 25));

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


void CTimeSeriesPlotFrame::OnTimeSeriesPlotLogScaleCheck( wxCommandEvent& event )
{
	GetCanvas()->SetLogScale(event.IsChecked());
}
/*
void CTimeSeriesPlotFrame::OnBuildAllCheck( wxCommandEvent& event )
{
	CDataManager *dm = GetDataManager();
	if ( !dm->m_ifDataReady )
		return;
	dm->m_ifBuildAllHistoryData = event.IsChecked();

	set<CAbstractData*> _parents;
	CAbstractData::GetAllParents( m_canvas->m_RDataList, _parents, 0, m_canvas->m_RDataList.size() - 1 );

	for ( set<CAbstractData*>::iterator it = _parents.begin(); it != _parents.end(); it++ )
		(*it)->UpdateSelf();
	for ( unsigned int i = 0; i < m_canvas->m_RDataList.size(); i++ )
		m_canvas->m_RDataList[i]->UpdateSelf();

	GetCanvas()->Refresh( false );
}
*/

void CTimeSeriesPlotFrame::OnTSPlotBuildKeySelect(wxListEvent &event)
{
    int nSel = event.m_itemIndex+1;
	CDataManager *dm = GetDataManager();
	if ( !dm->m_ifDataReady )
		return;

	if ( dm->m_RawData->m_item_desc[nSel].num_values != 0 ) // Categorical item
	{
		CTimeSeriesIData *base = dynamic_cast<CTimeSeriesIData*>(m_baseIData);
		base->m_CurrentBuildKey = nSel;
		if ( base->m_CurrentItem != -1 )
			base->Update();
	
		for (int i = 0; i < dm->m_HighlightIDataList.size(); i++ )
		{
			CTimeSeriesIData *idata = dynamic_cast<CTimeSeriesIData*>(dm->m_HighlightIDataList[i]);
			if ( idata != NULL )
			{
				idata->m_CurrentBuildKey = nSel;
				if ( idata->m_CurrentItem != -1 )
					idata->Update();
			}
		}

		GetCanvas()->Refresh( false );
	}
		
}

void CTimeSeriesPlotFrame::OnTSPlotItemSelect(wxListEvent &event)
{
    int nSel = event.m_itemIndex+1;
	CDataManager *dm = GetDataManager();
	if ( !dm->m_ifDataReady )
		return;

	if ( dm->m_RawData->m_item_desc[nSel].num_values == 0 ) // Numerical item
	{
		CTimeSeriesIData *base = dynamic_cast<CTimeSeriesIData*>(m_baseIData);
		base->m_CurrentItem = nSel;
		if ( base->m_CurrentBuildKey != -1 )
			base->Update();
		for ( int i = 0; i < dm->m_HighlightIDataList.size(); i++ )
		{
			CTimeSeriesIData *idata = dynamic_cast<CTimeSeriesIData*>(dm->m_HighlightIDataList[i]);
			if ( idata != NULL )
			{
				idata->m_CurrentItem = nSel;
				if ( idata->m_CurrentBuildKey != -1 )
					idata->Update();
			}
		}
		GetCanvas()->Refresh( false );
	}
}

void CTimeSeriesPlotFrame::OnBuildTimeSeries(wxCommandEvent& event)
{
	CDataManager *dm = GetDataManager();
	if ( !dm->m_ifDataReady )
		return;
	
	CTimeSeriesIData *base = dynamic_cast<CTimeSeriesIData*>(m_baseIData);
	base->Update();
	
	for ( unsigned int i = 0; i < dm->m_HighlightIDataList.size(); i++ )
	{
		CTimeSeriesIData *idata = dynamic_cast<CTimeSeriesIData*>(dm->m_HighlightIDataList[i]);
		if ( idata != NULL )
			idata->Update();
	}
	GetCanvas()->Refresh( false );
}

void CTimeSeriesPlotFrame::RefleshPanel()
{
	CDataManager *dm = GetDataManager();
	for ( unsigned int i = 1; i < dm->m_RawData->m_item_desc.size(); i++ )
	{
		long tmp1;
		tmp1 = m_TSPlotBuildKeySelectBox->InsertItem( i-1, wxString::FromUTF8(dm->m_RawData->m_item_desc[i].name.c_str()), 0 );
		if ( dm->m_RawData->m_item_desc[i].num_values != 0 )
		{
			m_TSPlotBuildKeySelectBox->SetItem( tmp1, 1, _T("CAT") );
		}
		else
		{
			wxListItem item;
			item.m_itemId = i-1;
			item.SetTextColour(*wxLIGHT_GREY);
			item.SetFont(*wxITALIC_FONT);
			m_TSPlotBuildKeySelectBox->SetItem( tmp1, 1, _T("NUM") );
			m_TSPlotBuildKeySelectBox->SetItem( item );
		}

		tmp1 = m_TSPlotItemSelectBox->InsertItem( i-1, wxString::FromUTF8(dm->m_RawData->m_item_desc[i].name.c_str()), 0 );
		if ( dm->m_RawData->m_item_desc[i].num_values != 0 )
		{
			wxListItem item;
			item.m_itemId = i-1;
			item.SetTextColour(*wxLIGHT_GREY);
			item.SetFont(*wxITALIC_FONT);
			m_TSPlotItemSelectBox->SetItem( tmp1, 1, _T("CAT") );
			m_TSPlotItemSelectBox->SetItem( item );
		}
		else
		{
			m_TSPlotItemSelectBox->SetItem( tmp1, 1, _T("NUM") );
		}
	}
	//m_DynamicSpecialListBox->Append( _T("Market Cap"));
}

void CTimeSeriesPlotFrame::CreateDataForNewFilter()
{
	CDataManager *dm = GetDataManager();
	CTimeSeriesIIndicesData *iidata = new CTimeSeriesIIndicesData();
	
	dm->m_HighlightIDataList.push_back(iidata);
	AddRelation(dm->m_Filters[dm->m_Filters.size()-1], iidata, true);
	CTimeSeriesIData* idata = dynamic_cast<CTimeSeriesIData*>(m_canvas->m_RDataList[0]->m_parent[0]);
	AddRelation( idata, iidata , false);

	if ( m_canvas->m_RDataList.size() == 1 )
	{
		CTimeSeriesRColorData* rcdata = new CTimeSeriesRColorData();
		AddRelation( iidata, rcdata, false);
	}
	else
		AddRelation( iidata, m_canvas->m_RDataList[1], false);
}

BEGIN_EVENT_TABLE(CTimeSeriesPlotCanvas, CCanvas)
    EVT_SIZE(CCanvas::OnSize)
    EVT_PAINT(CCanvas::OnPaint)
    EVT_ERASE_BACKGROUND(CCanvas::OnEraseBackground)
	EVT_KEY_DOWN(CCanvas::OnKeyDown)
    EVT_MOUSE_EVENTS(CCanvas::OnMouse)
	EVT_IDLE(CCanvas::OnIdle) 
END_EVENT_TABLE()

CTimeSeriesPlotCanvas::CTimeSeriesPlotCanvas( wxWindow *parent, wxWindowID id,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : CCanvas(parent, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE , name )
{
	rf = (CTimeSeriesPlotFrame*) parent;
	InitOrtho();
	m_newOrtho[0] = -1.0f;
	CTimeSeriesRData* rdata = new CTimeSeriesRData(this);
	m_RDataList.push_back(rdata);
	AddRelation(rf->m_baseIData, rdata, true);
}

void CTimeSeriesPlotCanvas::InitGL()
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

  //  glEnable(GL_LINE_SMOOTH);
  //  glEnable(GL_BLEND);

  //  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //  glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
  //  glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
 	//glEnable(GL_NORMALIZE);

	HWND hWnd = (HWND) GetHandle();
	m_hDC = GetDC(hWnd);

	BuildFont();
	SetShader();
}

void CTimeSeriesPlotCanvas::SetShader()
{
	string _vertShaerName ="GLSL/curveline.vert";
	string _fragShaerName ="GLSL/curveline.frag";
	string _geomShaerName ="GLSL/curveline.geom";


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
	glProgramParameteriEXT(p,GL_GEOMETRY_OUTPUT_TYPE_EXT,GL_LINE_STRIP);

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

void CTimeSeriesPlotCanvas::DrawAxis()
{
	glMatrixMode (GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity ();
	glOrtho( -0.1f, 1.06f, -0.1f, 1.06f, -1.0f, 1.0f );
	
	glColor4fv( BLACK );
	glBegin( GL_LINES );
		glVertex3f( 0.0, 0.0, 0.1 );
		glVertex3f( 0.0, 1.0, 0.1 );
		glVertex3f( 0.0, 0.0, 0.1 );
		glVertex3f( 1.0, 0.0, 0.1 ); 
	glEnd();
	float step = 1.0f / 6.0f;

	for ( int i = 1; i < 7; i++ )
	{
		glBegin( GL_LINES );
			glVertex3f( 0.0, i * step, 0.1 );
			glVertex3f( -0.01, i * step, 0.1 );
		glEnd();
	}

	CDataManager *dm = GetDataManager();
	CTimeSeriesIData* idata = dynamic_cast<CTimeSeriesIData*>(m_RDataList[0]->m_parent[0]);
	CTimeSeriesRData* rdata = dynamic_cast<CTimeSeriesRData*>(m_RDataList[0]);

	float _max = 1.0f;
	float _min = 0.0f;
	if ( idata->m_CurrentItem != -1 )
	{
		_max = dm->m_RawData->m_item_desc[idata->m_CurrentItem].max;
		_min = dm->m_RawData->m_item_desc[idata->m_CurrentItem].min;
	}

	char str[256];
	sprintf( str, "%f", _max );
	glRasterPos2f( -0.1, 0.956 );
	glPrint( "%s", str );
	sprintf( str, "%f", _min );
	glRasterPos2f( -0.1, 0.0 );
	glPrint( "%s", str );

	
	for ( int i = 1; i < 6; i++ )
	{
		float temp, _value;
		if ( rdata->m_if_logScale )
		{
			temp = log10(_max-_min+1.0f) * i / 6.0f;
			_value = pow( 10.0f, temp);
		}
		else
			_value = (_max-_min) * i / 6.0f;

		sprintf( str, "%f", _value );
		glRasterPos2f( -0.1, i*step);
		glPrint( "%s", str );
	}

	glPopMatrix();
	glMatrixMode (GL_MODELVIEW);

	if ( m_if_dragging || m_if_selecting)
	{
		if ( m_if_dragging )
			glColor4fv( BLACK );
		else
			glColor4fv( RED );
	
		glBegin( GL_LINE_LOOP );
		glVertex3f( m_newOrtho[0], m_newOrtho[1], 0.9 );
		glVertex3f( m_newOrtho[0], m_newOrtho[3], 0.9 );
		glVertex3f( m_newOrtho[2], m_newOrtho[3], 0.9 );
		glVertex3f( m_newOrtho[2], m_newOrtho[1], 0.9 );
		glEnd();
	}
}

void CTimeSeriesPlotCanvas::RenderVBO()
{
	glEnableClientState(GL_VERTEX_ARRAY);
	CDataManager *dm = GetDataManager();
	
	CTimeSeriesRData *rdata = dynamic_cast<CTimeSeriesRData*>(m_RDataList[0]);
	CTimeSeriesIData *idata = dynamic_cast<CTimeSeriesIData*>(m_RDataList[0]->m_parent[0]);
	CTimeSeriesRColorData *rcdata = NULL;
	if( m_RDataList.size() == 2 )
		rcdata = dynamic_cast<CTimeSeriesRColorData*>(m_RDataList[1]);

	glEnableClientState(GL_VERTEX_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, rdata->m_vbo->m_vertexBufferId);
	glVertexPointer(rdata->m_vbo->m_vertexFromat,GL_FLOAT,0,0);

	glUseProgram(p);
	for ( unsigned int i = 0; i < idata->m_numTSs; i++ )
	{
		//glColor4ubv( m_ColorData[4*i] );
		if ( !idata->m_ifHasData[i] )
			continue;
		unsigned int cindex;
		if ( rcdata != NULL )
		{
			cindex = rcdata->m_ColorIndices[i];
			if ( cindex > 0 )
				glColor4ub(dm->m_colorTable[cindex-1].first.red,
						   dm->m_colorTable[cindex-1].first.green,
						   dm->m_colorTable[cindex-1].first.blue,
						   dm->m_colorTable[cindex-1].first.alpha);
			else
				glColor4fv( GRAY );
		}
		else
			glColor4fv( GRAY );

		glDrawArrays(GL_LINE_STRIP, i*idata->m_numPoints, idata->m_numPoints);
	}
	glUseProgram(NULL);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void CTimeSeriesPlotCanvas::DrawCanvas()
{
	RenderVBO();
	DrawIndication();
}

void CTimeSeriesPlotCanvas::DrawIndication()
{
}

void CTimeSeriesPlotCanvas::SelectData()
{
}

void CTimeSeriesPlotCanvas::SetLogScale(bool _value)
{
	for(int i = 0; i < m_RDataList.size(); i++)
	{
		CTimeSeriesRData *rdata = dynamic_cast<CTimeSeriesRData*>(m_RDataList[i]);
		rdata->SetLogScale(_value);
		rdata->Update();
	}
	Refresh(false);
}

} // end of namespace
