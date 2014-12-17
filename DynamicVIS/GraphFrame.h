#pragma once
#include "Frame.h"
#include "GraphData.h"

namespace VIS
{
	class CGraphFrame : public CFrame
	{
		friend class CGraphCanvas;

	public:
		static CGraphFrame *Create(CControlFrame* _cf,const wxString& title, const wxPoint& pos,
        const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);

		CGraphFrame(CControlFrame* _cf, const wxString& title, const wxPoint& pos,
        const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);
		
		~CGraphFrame(void);

		CGraphCanvas *GetCanvas() {return (CGraphCanvas*)m_canvas; }

		void DrawPanel();
		void RefleshPanel();
		void SetupData();
		void CleanData();

		void OnListboxSelect(wxListEvent &event);
		void OnGraphCreate(wxCommandEvent& event);
		void OnCleanGraph(wxCommandEvent& event);
		void OnLayout(wxCommandEvent& event);
		void OnIdle( wxIdleEvent &event );

		void OnLayoutAttScroll( wxScrollEvent &event );
		void OnLayoutRepScroll( wxScrollEvent &event );
		void OnLayoutGraScroll( wxScrollEvent &event );
		void OnLayoutRangeScroll( wxScrollEvent &event );
		void OnLayoutSizeScroll( wxScrollEvent &event );

		void CreateDataForNewFilter();

		bool	m_ifLayout;

	private:

		wxListBox	*m_pListBox;
		wxListCtrl	*m_pListCtrl;

		//graph force direct
		wxButton  *m_GraCreate,*m_Layout,*m_DragNode, *m_PinNode,*m_clean;
		wxCheckBox	*m_whole, *m_merge;
		wxSlider *m_Layout_att, *m_Layout_rep,*m_Layout_gra,*m_Layout_pointSize,*m_Layout_range;
		wxTextCtrl *m_Layout_att_text, *m_Layout_rep_text,*m_Layout_gra_text,*m_Layout_pointSize_text,*m_Layout_range_text;

		//graph info
		uint m_GraphItems[2];

		DECLARE_EVENT_TABLE()
	};

	class CGraphCanvas : public CCanvas
	{
		friend class CGraphFrame;

	public:

		CGraphCanvas(wxWindow *parent, wxWindowID id = wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0, const wxString& name = _T("CGraphCanvas"));
		~CGraphCanvas();

		void OnMouse( wxMouseEvent& event );

		void InitOrtho()
		{
			m_Ortho[0] = m_OrthoBackup[0];
			m_Ortho[1] = m_OrthoBackup[1];
			m_Ortho[2] = m_OrthoBackup[2];
			m_Ortho[3] = m_OrthoBackup[3];
			m_Ortho[4] = -1.0f;
			m_Ortho[5] = 1.0f;
		}

		void DrawAxis();
		void DrawCanvas();
		void SelectData();
		void InitGL();
		float m_OrthoBackup[4];
	private:
		bool m_dragNodes;
		bool m_nodeMove;
		float m_nodeTranslation[2];
		void RenderVBO();
		void DrawIndication();
		set<uint> m_nodes;
		DECLARE_EVENT_TABLE()


	};

}