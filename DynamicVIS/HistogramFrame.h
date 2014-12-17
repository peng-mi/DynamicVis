#pragma once
#include "Frame.h"
#include "HistogramData.h"



namespace VIS
{
	class CHistogramFrame :public CFrame
	{
		friend class CHistogramCanvas;

	public:
		static CHistogramFrame *Create(CControlFrame* _cf,const wxString& title, const wxPoint& pos,
			const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);

		CHistogramFrame(CControlFrame* _cf, const wxString& title, const wxPoint& pos,
			const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);

		~CHistogramFrame(){};

		CHistogramCanvas *GetCanvas() { return (CHistogramCanvas*)m_canvas; }
		unsigned int GetItem(){ return m_item;} 

		void OnListboxSelect(wxListEvent &event);
		void OnSearchButton(wxCommandEvent& event);
		void OnSearchText(wxCommandEvent& event);
		void OnVerticalScaleScroll( wxScrollEvent &event );
		void OnLogScaleCheck( wxCommandEvent& event );
		void OnValueSelect( wxCommandEvent& event );

		void RefleshPanel();
		void CreateDataForNewFilter();
		//void CleanDataLists();
	 

	private:
		wxListCtrl	*m_pListCtrl;
		wxListBox	*m_valueSelectBox;
		wxCheckBox	*m_scalecheckbox;
		wxSlider	*m_verticalscale_slider;
		wxTextCtrl *m_search_Text,*m_selectedValue;
		wxButton *m_search_Button;


		bool m_new_search; // If this is a new search
		int m_previous_scroll;// previous scroll position
		int m_item;


		DECLARE_EVENT_TABLE()
	};

	class CHistogramCanvas: public CCanvas
	{
		friend class CHistogramFrame;
	public:
		CHistogramCanvas(wxWindow *parent, wxWindowID id = wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0, const wxString& name = _T("CHistogramCanvas"));
		~CHistogramCanvas(){};


		void SetLogScale ( bool _value );
		void SetVerticalScale( float _value );

		//	protected:

		//override functions
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
		//void GenerateRenderData();

	private:
		float m_vertical_scale;
		unsigned int m_maxvalue;

		void SetShader();
		
		DECLARE_EVENT_TABLE()

	};
}