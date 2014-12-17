#pragma once

#include "wx/wx.h"
#include <vector>
using namespace std;
#include <wx/glcanvas.h>
#include <time.h>
#include "wx/aui/aui.h"

class wxNotebook;
class wxListBox;
class wxPanel;
class wxBoxSizer;
class wxComboBox;
class wxDatePickerCtrl;
class wxListCtrl;
class wxListEvent;
class wxCheckBox;


namespace VIS
{
	class CCanvas;
	class CFrame;
	class CControlFrame;
	class CDataManager;
	class CAbstractData;
	class CHistogramCanvas;

	class CFrame: public wxFrame
	{
		friend class CCanvas;
		friend class CControlFrame;
		friend class CAbstractData;

	public:
		static CFrame *Create(CControlFrame* _rf, const wxString& title, const wxPoint& pos,
			const wxSize& size, long style = wxDEFAULT_FRAME_STYLE );

		CFrame(CControlFrame* _rf, const wxString& title, const wxPoint& pos,
			const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);

		~CFrame();

		CControlFrame *m_cf;
		string m_name;
		CAbstractData* m_baseIData;

		void OnExit(wxCommandEvent& event);
		void OnHide(wxCommandEvent& event);
		void OnClose( wxCloseEvent &event );
		void OnKeyDown( wxKeyEvent& event );

		//void virtual SetupData(){}
		//void virtual CleanData(){}
		virtual void CreateDataForNewFilter(){}
		void CleanDataLists();

		void SetCanvas( CCanvas *_canvas ) { m_canvas = _canvas; }
		CCanvas *GetCanvas() { return m_canvas; }
		void OnSize(wxSizeEvent& evt);
		CDataManager* GetDataManager();

		virtual void RefleshPanel(){};

	protected:
		CCanvas *m_canvas;
		bool	m_hide;
		wxAuiManager m_mgr;
		wxPanel *m_panels;
		wxBoxSizer* m_sizer;
		wxNotebook *m_pNotebook;
		float	 m_panelScale;

		DECLARE_EVENT_TABLE()
	};

	class CCanvas: public wxGLCanvas
	{
		friend CFrame;

	public:
		CCanvas ( wxWindow *parent, wxWindowID id = wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0, const wxString& name = _T("CCanvas") );

		~CCanvas();

		void OnPaint(wxPaintEvent& event);
		void OnSize(wxSizeEvent& event);
		void OnEraseBackground(wxEraseEvent& event);
		void OnKeyDown( wxKeyEvent& event );
		void OnMouse( wxMouseEvent& event );
		void OnIdle( wxIdleEvent& event );
		void Render();

		CDataManager* GetDataManager(){return rf->GetDataManager();}

		GLuint v,f,p,g;	//for shaders
		virtual void SetShader(){};

		CFrame *rf;
		vector<CAbstractData*> m_RDataList;

	protected:
		HDC m_hDC;
		GLuint	m_base;
		bool m_init;
		wxSize m_size;
		float m_Ortho[6];
		float m_newOrtho[4];
		float m_x, m_y;

		int framesPerSecond;        // This will store our fps
		long lastTime;           // This will hold the time from the last frame
		char strFrameRate[50];         // We will store the string here for the window title

		bool m_if_dragging;
		bool m_if_selecting;
		bool m_if_choseDrag;
		float m_drag[2]; // the offset of nodes;

		void virtual InitOrtho(){}
		void virtual DrawAxis(){}
		void virtual DrawCanvas(){}
		void virtual SelectData(){}
		void virtual InitGL(){}

		GLvoid BuildFont(GLvoid);
		GLvoid KillFont(GLvoid);									
		GLvoid glPrint(const char *fmt, ...);	

		void DrawExtraInfo();
		void virtual DrawRenderingStatus();
		void CalculateFrameRate();

		DECLARE_EVENT_TABLE()
	};	
}
