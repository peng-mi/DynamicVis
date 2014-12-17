#pragma once
#include "Frame.h"
#include "DynamicData.h"
namespace VIS
{
	class CDynamicFrame : public CFrame
	{
		friend class CDynamicCanvas;

	public:
		static CDynamicFrame *Create(CControlFrame* _cf,const wxString& title, const wxPoint& pos,
			const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);

		CDynamicFrame(CControlFrame* _cf, const wxString& title, const wxPoint& pos,
			const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);

		~CDynamicFrame(void){};

		CDynamicCanvas *GetCanvas() {return (CDynamicCanvas*)m_canvas; }

		void OnDynamicLogScaleCheck( wxCommandEvent& event );
		void OnDynamicSelect(wxListEvent &event);
		void OnDynamicSpecialSelect(wxCommandEvent &event);

		void RefleshPanel();
		void CreateDataForNewFilter();

	private:
		wxListBox	*m_DynamicSpecialListBox;
		wxListCtrl	*m_DynamicListBox;
		wxCheckBox	*m_ScaleCheckBox;

		DECLARE_EVENT_TABLE()
	};

	class CDynamicCanvas: public CCanvas
	{
		friend class CDynamicFrame;

	public:
		CDynamicCanvas(wxWindow *parent, wxWindowID id = wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0, const wxString& name = _T("CDynamicCanvas"));
		~CDynamicCanvas(){};
		void SetLogScale ( bool _value );

	private:
		void InitOrtho()
		{
			m_Ortho[0] = -0.1f;
			m_Ortho[1] = 1.06f;
			m_Ortho[2] = -0.1f;
			m_Ortho[3] = 1.06f;
			m_Ortho[4] = -1.0f;
			m_Ortho[5] = 1.0f;
		}
		void DrawAxis();
		void DrawCanvas();
		void SelectData();
		void InitGL();

		void RenderVBO();
		void DrawIndication();

		void SetShader();

		DECLARE_EVENT_TABLE()
	};


}
