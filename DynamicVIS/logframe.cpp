#include "logframe.h"

namespace VIS
{

	BEGIN_EVENT_TABLE(CLogPanel, wxPanel)
		EVT_SIZE      (CLogPanel::OnSize)
		END_EVENT_TABLE()

		const int  ID_BOOK              = 1000;

	void CLogPanel::OnSize( wxSizeEvent& event )
	{
		int x = 0;
		int y = 0;
		GetClientSize( &x, &y );

		int Border = 0;
		if (m_book) m_book->SetSize( Border, Border, x-2*Border, y-2*Border );
		if (m_logtext) m_logtext->SetSize( Border, Border, x-2*Border, y-2*Border );
	}

	CLogPanel::CLogPanel(wxFrame *frame, const wxPoint& pos, const wxSize& size )
		: wxPanel( frame, wxID_ANY, pos, size )
	{
		m_logtext = NULL;
		m_book = NULL;

		m_book = new wxBookCtrl(this, ID_BOOK);

		wxPanel *panel = new wxPanel(m_book);
		m_logtext = new wxTextCtrl(panel, wxID_ANY, _T(""),
			pos, size, wxTE_MULTILINE | wxTE_READONLY );
		m_logtext->SetBackgroundColour(wxT("wheat"));
		m_logTargetOld_log = wxLog::SetActiveTarget(new wxLogTextCtrl(m_logtext));
		m_book->AddPage(panel, _T("Log Messages") );
	}

	/*****************************************************************************************
	********************************LogFrame**************************************************
	*****************************************************************************************/
	enum
	{
		ID_QUIT  = wxID_EXIT,
	};

	BEGIN_EVENT_TABLE(CLOGFrame, CFrame)
		EVT_MENU(ID_QUIT, CFrame::OnExit)
		EVT_SIZE(CFrame::OnSize)
		END_EVENT_TABLE()


		CLOGFrame *CLOGFrame::Create(CControlFrame* _cf, const wxString& title, const wxPoint& pos,
		const wxSize& size, long style )
	{
		CLOGFrame *rf = new CLOGFrame(_cf,title, pos, size, style);
		rf->Show(true);
		return rf;
	}

	CLOGFrame::CLOGFrame(CControlFrame* _cf,const wxString& title, const wxPoint& pos,
		const wxSize& size, long style)
		: CFrame(_cf,title, pos, size, style )
	{
		m_logpanel = new CLogPanel( this, wxDefaultPosition, size );



		m_mgr.Update();
	}

	CLOGFrame::~CLOGFrame()
	{
		//CFrame::~CFrame();
	}

}