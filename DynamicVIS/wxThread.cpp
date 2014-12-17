#include "wxThread.h"
#include "datamanager.h"
#include "header.h"
#include <string.h>
namespace VIS
{
	CwxThread::CwxThread(wxFrame* _frame, wxTimer* _timer,CDataManager* _dm, char* _name): wxThread()
	{
		m_frame = _frame;
		m_dm = _dm;
		m_name = _strdup(_name);
		m_timer = _timer;
	}

	void CwxThread::OnExit()
	{
		free(m_name);
	}

	void *CwxThread::Entry()
	{
		wxLogMessage("Start loading the file %s.\n", m_name);
		m_dm->LoadData(m_name);	
		m_timer->Stop();
		wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, WORKER_EVENT );
		event.SetInt(-1); // that's all
		wxPostEvent( m_frame, event );
		return NULL;
	}


}