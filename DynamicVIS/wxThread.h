#pragma once

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include "wx/thread.h"
#include <wx/timer.h>
namespace VIS
{
	class CDataManager;
	class CwxThread : public wxThread
	{
	public:
		CwxThread(wxFrame* _frame, wxTimer* _timer,CDataManager* _dm, char* filename);
		~CwxThread(void){};

		virtual void *Entry();//thread exectuion starts here
		virtual void OnExit(); //called when thread exits
		//wxThreadError Run();

	public:
		wxFrame* m_frame;
		CDataManager* m_dm;
		char* m_name;
		wxTimer* m_timer;

	};

}