#ifndef _LOGFRAME_H_
#define _LOGFRAME_H_

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "Frame.h"
#include "wx/bookctrl.h"

namespace VIS
{

	class CLogPanel: public wxPanel
	{
	public:
		CLogPanel(wxFrame *frame, const wxPoint& pos, const wxSize& size );
		void OnSize( wxSizeEvent& event );
		wxTextCtrl    *m_logtext;
		wxBookCtrl    *m_book;

	private:
		wxLog *m_logTargetOld_log;
		DECLARE_EVENT_TABLE()
	};


	class CLOGFrame: public CFrame
	{
	public:
		/* CLOGFrame( const wxString& title, const wxPoint& pos,
		const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);*/

		static CLOGFrame *Create(CControlFrame* _cf,const wxString& title, const wxPoint& pos,
			const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);

		CLOGFrame(CControlFrame* _cf, const wxString& title, const wxPoint& pos,
			const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);

		~CLOGFrame(void);

	private:
		CLogPanel *m_logpanel;

		DECLARE_EVENT_TABLE()
	};

}

#endif