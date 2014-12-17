
#pragma once
#include "Frame.h"
#include "ParaCoordData.h"
namespace VIS
{
	class CParaCoordFrame :public CFrame
	{
		friend class CParaCoordCanvas;

	public:
		static CParaCoordFrame *Create(CControlFrame* _cf,const wxString& title, const wxPoint& pos,
			const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);

		CParaCoordFrame(CControlFrame* _cf, const wxString& title, const wxPoint& pos,
			const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);

		~CParaCoordFrame(){};

		void OnListboxSelect(wxCommandEvent &event);
		CParaCoordCanvas *GetCanvas() { return (CParaCoordCanvas*)m_canvas; }
		void RefleshPanel();
		void SetupData();
		void CleanData();
		void OnGPUCheck( wxCommandEvent& event );
		void SetGPU(bool _gpu);
		void CreateDataForNewFilter(); 


	private:
		wxListBox	*m_pListBox;
		wxCheckBox  *m_GPUCheckBox;
		bool		m_if_gpu;

		DECLARE_EVENT_TABLE()
	};

	class CParaCoordCanvas: public CCanvas
	{
		friend class CParaCoordFrame;
	public:
		CParaCoordCanvas(wxWindow *parent, wxWindowID id = wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0, const wxString& name = _T("CParaCoordCanvas"));
		~CParaCoordCanvas(){};

		//override functions
		void InitOrtho()
		{
			m_Ortho[0] = 0.0f;
			m_Ortho[1] = 1.0f;
			m_Ortho[2] = 0.0f;
			m_Ortho[3] = 1.0f;
			m_Ortho[4] = -1.0f;
			m_Ortho[5] = 1.0f; 
		}
		void DrawAxis();
		void DrawCanvas();
		void SelectData();
		void InitGL();
		void SetGPU(bool _gpu);
		void CleanData();
		void RenderVBO();
		
	private:
		bool m_if_gpu;
		void DrawIndication();
		DECLARE_EVENT_TABLE()

	};
}