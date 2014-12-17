#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <GL/glew.h>
#include "Frame.h"
#include "ControlFrame.h"
#include "datamanager.h"
#include "GL/freeglut.h"
#include "color.h"
#include "glhelper.h"
#include "PerformanceTimer.h"
#include "Data.h"

#include "HistogramFrame.h"
#include "ParaCoordFrame.h"

#include <typeinfo>


namespace VIS
{

	/******************************************************************************************************
	********************CFrame****************************************************************************
	******************************************************************************************************/


	enum
	{
		ID_QUIT  = wxID_EXIT,
		ID_HIDE,
		ID_BUTTON,
	};

	BEGIN_EVENT_TABLE(CFrame, wxFrame)
		EVT_MENU(ID_QUIT, CFrame::OnExit)
		EVT_MENU(ID_HIDE, CFrame::OnHide)

		EVT_CHAR_HOOK(CFrame::OnKeyDown)
		EVT_CLOSE	(CFrame::OnClose)
		EVT_SIZE(CFrame::OnSize)
		END_EVENT_TABLE()



	CFrame *CFrame::Create(CControlFrame* _rf,const wxString &title, const wxPoint &pos, 
		const wxSize &size, long style)
	{
		CFrame *rf = new CFrame(_rf,title,pos,size,style);
		rf->Show(true);
		return rf;
	}

	CFrame::~CFrame()
	{
		if(m_canvas != NULL)
			delete  m_canvas;
		m_canvas = NULL;

		m_mgr.UnInit();
	}

	void CFrame::OnSize(wxSizeEvent& event)
	{
		this->Refresh(false);
		event.Skip();
	}

	void CFrame::OnKeyDown( wxKeyEvent& event )
	{
		char ch = event.GetKeyCode();
		if ( ch == ' ' )
		{
			GetCanvas()->InitOrtho();	
			GetCanvas()->Refresh(false);
		} 
	}

	CDataManager* CFrame::GetDataManager()
	{
		return m_cf->GetDataManager();
	}

	CFrame::CFrame(CControlFrame* _cf,const wxString& title, const wxPoint& pos,
		const wxSize& size, long style)
		: wxFrame( NULL, wxID_ANY, title, pos, size, style ), m_cf(NULL),m_hide(true),\
		m_canvas(NULL), m_baseIData(NULL)
	{
		m_cf = _cf;
		m_mgr.SetManagedWindow(this);

		m_name =  string(title.mb_str());

		wxMenu *fileMenu = new wxMenu;

		fileMenu->AppendCheckItem(ID_HIDE, wxT("H&ide"));
		fileMenu->Append(ID_QUIT, wxT("E&xit"));
		wxMenuBar *menuBar = new wxMenuBar;
		menuBar->Append(fileMenu, wxT("&File"));
		SetMenuBar(menuBar);
	}

	void CFrame::OnExit( wxCommandEvent& WXUNUSED(event) )
	{
		Close(true);
	}

	void CFrame::OnHide( wxCommandEvent& WXUNUSED(event) )
	{
		if(m_hide)
		{
			m_mgr.GetPane(wxT("panel")).Hide();
			m_mgr.GetPane(wxT("canvas")).Show().Center().Layer(0).Position(0);
			m_hide = false;
		}
		else
		{
			m_mgr.GetPane(wxT("canvas")).Show().Center().Layer(0).Position(0);
			m_mgr.GetPane(wxT("panel")).Show().Center().Layer(0).Position(0);
			m_hide = true;
		}

		m_mgr.Update();
	}

	void CFrame::OnClose( wxCloseEvent& event )
	{
		this->Show(false);

		int begin = HISTOGRAM_ID;
		int size = m_cf->m_menuViews->GetMenuItemCount();
		int i;
		for(i = 0; i<size; i++)
		{
			string name = m_cf->m_menuViews->GetLabel(i+begin);
			if(name.compare(m_name)==0)
				break;
		}
		if(i<size)
		{
			m_cf->m_menuViews->Check(i+begin,false);
			
			vector<CFrame*>::iterator it_frame = m_cf->m_frames.begin();
			size = m_cf->m_frames.size();
			for(i=0; i<size; i++)
			{
				if(m_name.compare(m_cf->m_frames[i]->m_name) ==0)
					break;
			}
			if(i<size)
				m_cf->m_frames.erase(it_frame+i);
		}
		Destroy();
	}

	void CFrame::CleanDataLists()
	{
		if(m_canvas == NULL)
			return;
		for( vector<CAbstractData*>::iterator it = m_canvas->m_RDataList.begin() ; it != m_canvas->m_RDataList.end();)
		{
			if( CAbstractData::IsDeletedObject(*it) )
				it = m_canvas->m_RDataList.erase(it);
			else
				it++;
		}
	}

	/******************************************************************************************************
	********************CCanvas****************************************************************************
	******************************************************************************************************/

	BEGIN_EVENT_TABLE(CCanvas, wxGLCanvas)
		EVT_SIZE(CCanvas::OnSize)
		EVT_PAINT(CCanvas::OnPaint)
		EVT_ERASE_BACKGROUND(CCanvas::OnEraseBackground)
		EVT_KEY_DOWN(CCanvas::OnKeyDown)
		EVT_MOUSE_EVENTS(CCanvas::OnMouse)
		EVT_IDLE(CCanvas::OnIdle) 
		END_EVENT_TABLE()

		CCanvas::CCanvas(wxWindow *parent, wxWindowID id ,
		const wxPoint &pos, const wxSize &size, 
		long style , const wxString &name )
		:wxGLCanvas(parent, (wxGLCanvas*) NULL, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE , name )
	{
		rf = (CFrame*) parent;
		m_hDC = NULL;
		m_init = false;
		m_RDataList.resize(0);

		m_if_dragging = false;
		m_if_selecting = false;
		m_if_choseDrag = false;

		m_base = -1;

		m_Ortho[4] =-1.0;
		m_Ortho[5] =1.0;
	}

	CCanvas::~CCanvas()
	{
		if( m_base != -1)
		{
			KillFont();
			m_base = -1;

		}
	}

	GLvoid CCanvas::BuildFont(GLvoid)
	{
		HFONT	font;										// Windows Font ID
		HFONT	oldfont;									// Used For Good House Keeping

		m_base = glGenLists(96);								// Storage For 96 Characters

		font = CreateFont(	-12,							// Height Of Font
			0,								// Width Of Font
			0,								// Angle Of Escapement
			0,								// Orientation Angle
			FW_BOLD,						// Font Weight
			FALSE,							// Italic
			FALSE,							// Underline
			FALSE,							// Strikeout
			ANSI_CHARSET,					// Character Set Identifier
			OUT_TT_PRECIS,					// Output Precision
			CLIP_DEFAULT_PRECIS,			// Clipping Precision
			ANTIALIASED_QUALITY,			// Output Quality
			FF_DONTCARE|DEFAULT_PITCH,		// Family And Pitch
			"Arial");					// Font Name

		oldfont = (HFONT)SelectObject(m_hDC, font);           // Selects The Font We Want
		wglUseFontBitmaps(m_hDC, 32, 96, m_base);				// Builds 96 Characters Starting At Character 32
		SelectObject(m_hDC, oldfont);							// Selects The Font We Want
		DeleteObject(font);	
	}
	GLvoid CCanvas::KillFont(GLvoid)									// Delete The Font List
	{
		glDeleteLists(m_base, 96);							// Delete All 96 Characters
	}

	GLvoid CCanvas::glPrint(const char *fmt, ...)					// Custom GL "Print" Routine
	{
		char		text[256];								// Holds Our String
		va_list		ap;										// Pointer To List Of Arguments

		if (fmt == NULL)									// If There's No Text
			return;											// Do Nothing

		va_start(ap, fmt);									// Parses The String For Variables
		vsprintf(text, fmt, ap);						// And Converts Symbols To Actual Numbers
		va_end(ap);											// Results Are Stored In Text

		glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
		glListBase(m_base - 32);								// Sets The Base Character to 32
		glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
		glPopAttrib();										// Pops The Display List Bits
	}




	void CCanvas::DrawExtraInfo()
	{
		if ( m_if_dragging || m_if_selecting)
		{
			CDataManager *dm =  GetDataManager();//CDataManager::Instance();

			if ( m_if_dragging )
				glColor4fv( BLACK );
			else
			{
				if ( dm->m_BrushType >= Brush_One_EX )
					glColor4fv( BLUE );
				else 
				{
					glColor4ub(rf->m_cf->m_ColorData.GetColour().Red(),rf->m_cf->m_ColorData.GetColour().Green(),rf->m_cf->m_ColorData.GetColour().Blue(),rf->m_cf->m_ColorData.GetColour().Alpha() );
				}
			}

			glLineWidth(1.0f);
			glBegin( GL_LINE_LOOP );
			glVertex3f( m_newOrtho[0], m_newOrtho[1], 0.9 );
			glVertex3f( m_newOrtho[0], m_newOrtho[3], 0.9 );
			glVertex3f( m_newOrtho[2], m_newOrtho[3], 0.9 );
			glVertex3f( m_newOrtho[2], m_newOrtho[1], 0.9 );
			glEnd();
		}
	}

	void CCanvas::Render()
	{
		int w = m_size.x, h = m_size.y;

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glViewport(0, 0, w, h);

		CDataManager *dm =  GetDataManager();//CDataManager::Instance();

		glOrtho( m_Ortho[0], m_Ortho[1], m_Ortho[2], m_Ortho[3], m_Ortho[4], m_Ortho[5] );

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if(dm->m_ifDataReady)
		{	
			DrawAxis();
			DrawCanvas();
			DrawExtraInfo();
			CalculateFrameRate();
			DrawRenderingStatus();

			glFlush();
			SwapBuffers();
		}
		/*glFlush();
		SwapBuffers();*/
	}



	void CCanvas::OnIdle( wxIdleEvent& event )
	{
		Render();
		Refresh(false);
	}

	void CCanvas::OnPaint( wxPaintEvent& WXUNUSED(event) )
	{
		wxPaintDC dc(this);

		if ( !GetContext() )
			return;
		SetCurrent();
		if (!m_init)
		{
			InitGL();
			m_init = true;
		}

		Render();
	}

	void CCanvas::OnSize(wxSizeEvent& event)
	{
		int w, h;
		GetClientSize(&w, &h);
		m_size.SetHeight( h );
		m_size.SetWidth( w );

		glViewport(0, 0, w, h);
	}

	void CCanvas::OnEraseBackground(wxEraseEvent& WXUNUSED(event))
	{
		// Do nothing, to avoid flashing.
	}

	void CCanvas::OnKeyDown( wxKeyEvent& event )
	{
		SetCurrent();
		char ch = event.GetKeyCode();
		if ( ch == ' ' )
		{
			InitOrtho();	
			Refresh(false);
		} 
	}
	void CCanvas::OnMouse( wxMouseEvent& event )
	{
		SetCurrent();

		int mouse_x = event.GetX();
		int mouse_y = event.GetY();

		if ( event.MiddleDown())
		{
			if ( !m_if_dragging )
			{
				SetCurrent();
				GetWorldCoordinate( m_newOrtho[0], m_newOrtho[1], mouse_x, mouse_y );
				m_newOrtho[2] = m_newOrtho[0]; m_newOrtho[3] = m_newOrtho[1];
			}
		}
		else if ( event.Dragging() && event.MiddleIsDown() )
		{
			m_if_dragging = true;
			GetWorldCoordinate( m_newOrtho[2], m_newOrtho[3], mouse_x, mouse_y );
			Refresh(false);
		}
		else if ( event.MiddleUp() )
		{
			if ( m_if_dragging )
			{
				m_if_dragging = false;
				GetWorldCoordinate( m_newOrtho[2], m_newOrtho[3], mouse_x, mouse_y );
				if ( m_newOrtho[0] > m_newOrtho[2] )
				{
					float temp = m_newOrtho[0];
					m_newOrtho[0] = m_newOrtho[2];
					m_newOrtho[2] = temp;
				}
				if ( m_newOrtho[1] > m_newOrtho[3] )
				{
					float temp = m_newOrtho[1];
					m_newOrtho[1] = m_newOrtho[3];
					m_newOrtho[3] = temp;
				}				 
				if ( !event.ControlDown() ) // zoom in
				{
					m_Ortho[0] = m_newOrtho[0];
					m_Ortho[1] = m_newOrtho[2];
					m_Ortho[2] = m_newOrtho[1];
					m_Ortho[3] = m_newOrtho[3];
				}
				else
				{
					float ratiox = (m_Ortho[1]-m_Ortho[0]) / (m_newOrtho[2] - m_newOrtho[0] );
					float ratioy = (m_Ortho[3]-m_Ortho[2]) / (m_newOrtho[3] - m_newOrtho[1] );
					float x[2], y[2];
					x[0] = m_Ortho[0] - ( m_newOrtho[0] - m_Ortho[0] ) * (ratiox+1.0f);
					x[1] = m_Ortho[1] + ( m_Ortho[1] - m_newOrtho[2] ) * (ratiox+1.0f);
					y[0] = m_Ortho[2] - ( m_newOrtho[1] - m_Ortho[2] ) * (ratioy+1.0f);
					y[1] = m_Ortho[3] + ( m_Ortho[3] - m_newOrtho[3] ) * (ratioy+1.0f);

					m_Ortho[0] = x[0];
					m_Ortho[1] = x[1];
					m_Ortho[2] = y[0];
					m_Ortho[3] = y[1];
				}

				Refresh(false);
			}
		}

		if ( event.LeftDown())
		{
			if ( !m_if_selecting )
			{
				SetCurrent();
				GetWorldCoordinate( m_newOrtho[0], m_newOrtho[1], mouse_x, mouse_y );
				m_newOrtho[2] = m_newOrtho[0]; m_newOrtho[3] = m_newOrtho[1];
			}
		}
		else if ( event.Dragging() && event.LeftIsDown() )
		{
			m_if_selecting = true;
			GetWorldCoordinate( m_newOrtho[2], m_newOrtho[3], mouse_x, mouse_y );
			Refresh(false);
		}
		else if ( event.LeftUp() )
		{
			if ( m_if_selecting )
			{
				m_if_selecting = false;
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
				rf->m_cf->RefreshAll();
			}
		}
	}

	void CCanvas::CalculateFrameRate()
	{
		SetCurrent();
		static int timeinit=0;
		static PerformanceTimer pTimer;
		static int frame=0;
		if(!timeinit)
		{
			frame=0;
			pTimer.StartTimer();	
			timeinit=1;
		}
		frame++;
		double dif = pTimer.GetTimeElapsed();
		double fps = (dif)?(double)frame/dif:-1.0;
		if(dif>2.0 && frame >10) 
		{
			pTimer.StartTimer();
			frame  = 0;
		}
		sprintf(strFrameRate, "Frames Rate: %.3f",fps);
	}

	void CCanvas::DrawRenderingStatus()
	{
		CDataManager *dm =  GetDataManager();//CDataManager::Instance();
		glColor4fv(BLACK);

		vector<CAbstractData*> _vec;
		for(int i = 0; i < m_RDataList.size(); i++)
		{
			for(int j = 0; j < m_RDataList[i]->m_parent.size(); j++)
			{
				for(int k = 0; k < CAbstractData::s_UpdateObjects.size(); k++)
					if(CAbstractData::s_UpdateObjects[k] == m_RDataList[i]->m_parent[j])
						_vec.push_back(CAbstractData::s_UpdateObjects[k]);
			}
			for(int k = 0; k < CAbstractData::s_UpdateObjects.size(); k++)
				if(CAbstractData::s_UpdateObjects[k] == m_RDataList[i])
					_vec.push_back(CAbstractData::s_UpdateObjects[k]);
		}

		float mining_time = 0.0f;
		for(int i = 0; i < _vec.size(); i++)
			mining_time += _vec[i]->m_dataProcessingTime;

		if ( dm->m_ifDataReady )
		{
			glColor4fv( LIGHT_GRAY );
			glRasterPos2f( m_Ortho[0], 0.99*m_Ortho[2] );
			glPrint( "%s", strFrameRate );

			char str[64];
			sprintf( str, "Mining Time: %.3f ms", mining_time*1000.0f);
			glRasterPos2f( 0.7*m_Ortho[1]+ 0.3*m_Ortho[0], 0.99*m_Ortho[2] );
			glPrint( "%s", str );
		}
	}
}