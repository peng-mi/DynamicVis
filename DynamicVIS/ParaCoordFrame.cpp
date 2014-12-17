#include <GL/glew.h>

//parallel coordinate
#include "ParaCoordFrame.h"
#include "ParaCoordData.h"

//wxwidgets
#include <wx/listbox.h>

//helpers
#include "color.h"
#include "glhelper.h"
#include "PerformanceTimer.h"
#include "colorhelper.h"
#include "shaderhelper.h"

//data and controlframe
#include "ControlFrame.h"
#include "datamanager.h"
#include "RawData.h"
#include "FilteredData.h"
#include "Filter.h"
#include "LogFile.h"
#define GPU_INIT true

namespace VIS
{

/******************************************************************************************************
********************CParaCoordFrame*******************************************************************
******************************************************************************************************/
	enum
	{
		ID_QUIT  = wxID_EXIT,
		ID_HIDE,

		ID_ListBox,
		ID_GPU,
	};

	BEGIN_EVENT_TABLE(CParaCoordFrame, CFrame)
		EVT_MENU(ID_QUIT, CFrame::OnExit)
		EVT_SIZE(CFrame::OnSize)
		EVT_MENU(ID_HIDE, CFrame::OnHide)
		EVT_CHAR_HOOK(CFrame::OnKeyDown)

		EVT_LISTBOX (ID_ListBox, CParaCoordFrame::OnListboxSelect)
		EVT_CHECKBOX( ID_GPU, CParaCoordFrame::OnGPUCheck)
	END_EVENT_TABLE()

	CParaCoordFrame *CParaCoordFrame::Create(CControlFrame* _cf, const wxString& title, const wxPoint& pos,
										 const wxSize& size, long style )
	{
		CParaCoordFrame *rf = new CParaCoordFrame(_cf,title, pos, size, style);
		rf->Show(true);
		return rf;
	}

	void CParaCoordFrame::OnGPUCheck(wxCommandEvent& event)
	{
		SetGPU(event.IsChecked());
		GetCanvas()->Refresh( false );
	}

	void CParaCoordFrame::SetGPU(bool _gpu)
	{
		GetCanvas()->SetGPU(_gpu);
	}

	CParaCoordFrame::CParaCoordFrame(CControlFrame* _cf,const wxString& title, const wxPoint& pos,
						   const wxSize& size, long style)
						   : CFrame(_cf,title, pos, size, style ),m_if_gpu(GPU_INIT)
	{
		CDataManager *dm = GetDataManager();
		m_baseIData = new CParaCoordIData(m_if_gpu);
		dm->m_BasedIDataList.push_back(m_baseIData);
		AddRelation( dm->m_BasedFilteredData, m_baseIData, true );
		m_canvas = new CParaCoordCanvas( this, wxID_ANY,wxDefaultPosition, wxSize(size.GetWidth(),size.GetWidth()));

		m_panelScale = 0.4f;

		m_mgr.AddPane(m_canvas, wxAuiPaneInfo().
					  Name(wxT("canvas")).Caption(wxT("canvas")).
					  MaxSize(size).Layer(0).
					  CloseButton(true).MaximizeButton(true));

		wxSize panelSize(size.GetWidth(),size.GetHeight()*m_panelScale);
		m_panels = new wxPanel(this, wxID_ANY, wxDefaultPosition, panelSize, wxTAB_TRAVERSAL);
		m_panels->SetBackgroundColour(wxColour(255,255,255));

		//wxTextCtrl *control;
		wxStaticText  *control = new wxStaticText(m_panels, wxID_ANY, _T("Item Names:"), wxPoint(0,3), wxDefaultSize, wxBORDER_NONE);
		wxSize listboxsize(150,size.GetHeight()*m_panelScale-30);
		m_pListBox = new wxListBox( m_panels, ID_ListBox, wxPoint(5, 25), listboxsize, 0, NULL, wxLB_MULTIPLE|wxWANTS_CHARS );
		
		m_GPUCheckBox = new wxCheckBox(m_panels, ID_GPU, _T("GPU Processing"), wxPoint(160, 4), wxSize(100, 25));
		m_GPUCheckBox->SetValue( true );

		m_sizer = new wxBoxSizer(wxVERTICAL);
		m_sizer->Layout();
		m_panels->SetSizer(m_sizer);
		SetExtraStyle(wxWS_EX_PROCESS_IDLE );

		m_mgr.AddPane(m_panels, wxAuiPaneInfo().
					  Name(wxT("panel")).Caption(wxT("panel")).
					  MaxSize(panelSize).Layer(0).
					  CloseButton(true).MaximizeButton(false));


		m_mgr.GetPane(wxT("canvas")).Show().Center().Layer(0).Position(0);
		m_mgr.GetPane(wxT("panel")).Show().Center().Layer(0).Position(0);
		m_mgr.Update();
	}

	void CParaCoordFrame::OnListboxSelect(wxCommandEvent &event)
	{
		int _sel = event.GetSelection();
		CDataManager *dm = GetDataManager();
		if ( !dm->m_ifDataReady )
			return;

		CParaCoordIData* _pidata = NULL;
		vector<CParaCoordIData*> vec;
		_pidata = dynamic_cast<CParaCoordIData*>(m_baseIData);
		vec.push_back(_pidata);
	
		for(unsigned int i = 0; i < dm->m_HighlightIDataList.size(); i++)
		{
			_pidata = dynamic_cast<CParaCoordIData*>(dm->m_HighlightIDataList[i]);
			if(_pidata != NULL)
				vec.push_back(_pidata);
		}

		bool found = false;
		for(unsigned int i = 0; i < vec.size(); i++)
		{
			found = false;
			for ( t_uintvector::iterator it = vec[i]->m_selectedItems.begin(); it != vec[i]->m_selectedItems.end(); it++)
			{
				if ( (*it) == _sel+1 )
				{
					vec[i]->m_selectedItems.erase( it );
					found = true;
					break;
				}
			}
			if ( !found )
				vec[i]->m_selectedItems.push_back( _sel+1 );
			if(vec[i]->m_selectedItems.size() > 1)
				vec[i]->Update();
		}	
		 GetCanvas()->Refresh(false);
	}

	void CParaCoordFrame::RefleshPanel()
	{
		m_pListBox->Clear();
		CDataManager *dm =  GetDataManager();
		for ( unsigned int i = 1; i < dm->m_RawData->m_item_desc.size(); i++ )
		{
			m_pListBox->Append( wxString::FromUTF8(dm->m_RawData->m_item_desc[i].name.c_str()));
		}
		CParaCoordIData* idata = NULL;
		idata = dynamic_cast<CParaCoordIData*> ( m_canvas->m_RDataList[0]->m_parent[0] );
		assert(idata != NULL);
		idata->m_selectedItems.clear();
	}

	void CParaCoordFrame::CreateDataForNewFilter()
	{
		CDataManager* dm = GetDataManager();
		CParaCoordIData* base = dynamic_cast<CParaCoordIData*>(m_baseIData);
		CParaCoordIData *idata = new CParaCoordIData();
		if(base != NULL)
		{
			for(unsigned int i = 0; i < base->m_selectedItems.size(); i++)
				idata->m_selectedItems.push_back(base->m_selectedItems[i]);
		}
		
		dm->m_HighlightIDataList.push_back(idata);
		AddRelation(dm->m_FilteredDataList[dm->m_FilteredDataList.size()-1], idata, true);

		CParaCoordRData *rdata = new CParaCoordRData(m_canvas,m_if_gpu);
		m_canvas->m_RDataList.push_back(rdata);
		AddRelation( idata, rdata, true );
	}


/**************************************************************************************************************************************************
************************** The Canvas *************************************************************************************************************
**************************************************************************************************************************************************/

	BEGIN_EVENT_TABLE(CParaCoordCanvas, CCanvas)
		EVT_SIZE(CCanvas::OnSize)
		EVT_PAINT(CCanvas::OnPaint)
		EVT_ERASE_BACKGROUND(CCanvas::OnEraseBackground)
		EVT_MOUSE_EVENTS(CCanvas::OnMouse)
		EVT_IDLE(CCanvas::OnIdle) 
	END_EVENT_TABLE()



	CParaCoordCanvas::CParaCoordCanvas( wxWindow *parent, wxWindowID id,
		const wxPoint& pos, const wxSize& size, long style, const wxString& name)
		: CCanvas(parent, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE , name ),m_if_gpu(GPU_INIT)
	{
		rf = (CParaCoordFrame*) parent;
		InitOrtho();
		CParaCoordFrame* frame = dynamic_cast<CParaCoordFrame*>(rf);
		CParaCoordRData* _rdata = new CParaCoordRData(this,frame->m_if_gpu);
		m_RDataList.push_back(_rdata);
		AddRelation(rf->m_baseIData, _rdata, true);
	}

	void CParaCoordCanvas::DrawAxis()
	{
		CDataManager* dm = GetDataManager();
		CParaCoordIData* _pidata = NULL;
		_pidata = dynamic_cast<CParaCoordIData*>(m_RDataList[0]->m_parent[0]);
		t_uintvector &si = _pidata->m_selectedItems;

		float step = 0.0f, start;
		if ( si.size() == 1 )
		{
			start = 0.5;
		}
		else
		{
			step = 0.8f / (si.size() - 1 );
			start = 0.1f;
		}
		for ( int i = 0; i < si.size(); i++ )
		{
			glColor4fv( GRAY );
			glBegin( GL_LINES );
				glVertex3f( start+(i*step), 0.05, 0.0 );
				glVertex3f( start+(i*step), 0.95, 0.0 );
			glEnd();
			glColor4fv( BLACK );
			glRasterPos2f( start-0.06/si.size()+(i*step), 0.03);
			glPrint( dm->m_RawData->m_item_desc[si[i]].name.c_str() );

			if ( dm->m_RawData->m_item_desc[si[i]].num_values == 0 )
			{
				char str[32];
				glRasterPos2f( start+(i*step)+0.01, 0.05);
				sprintf( str, "%.2f", dm->m_RawData->m_item_desc[si[i]].min );
				glPrint( str );

				glRasterPos2f( start+(i*step)+0.01, 0.95);
				sprintf( str, "%.2f", dm->m_RawData->m_item_desc[si[i]].max );
				glPrint( str );
			}
		}
	}

	void CParaCoordCanvas::InitGL()
	{
		SetCurrent();
		GLenum err = glewInit();
		if (GLEW_OK != err)
			wxLogError( "%s\n", glewGetErrorString(err) );

		if ( GLEW_ARB_vertex_buffer_object )
			wxLogMessage( "VBO extersion is supported!\n");
		if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader && GL_EXT_geometry_shader4)
			wxLogMessage("Ready for GLSL - vertex, fragment, and geometry units.\n");
		else {
			wxLogError("Not totally ready :( \n");
		}

		glClearColor( WHITE[0],WHITE[1],WHITE[2],WHITE[3] );
		glClearDepth(1.0);
		glEnable(GL_DEPTH_TEST);

		HWND hWnd = (HWND) GetHandle();
		m_hDC = GetDC(hWnd);

		BuildFont();
	}

	void CParaCoordCanvas::RenderVBO()
	{
		CDataManager* dm = GetDataManager();
		SetCurrent();
		glEnableClientState(GL_VERTEX_ARRAY);
		CParaCoordRData* rdata = NULL;
		CParaCoordIData* idata = NULL;
		
		for(unsigned int i = 0; i < m_RDataList.size() ; i++)
		{
			rdata = dynamic_cast<CParaCoordRData*>(m_RDataList[i]);
			idata = dynamic_cast<CParaCoordIData*>(rdata->m_parent[0]);
			if(idata->m_selectedItems.size() < 2)
				continue;
			if(i == 0)
				glColor4fv(GRAY);
			else
				glColor4ub(dm->m_colorTable[i-1].first.red,dm->m_colorTable[i-1].first.green,dm->m_colorTable[i-1].first.blue,dm->m_colorTable[i-1].first.alpha);
		
		//	if(rdata->m_vbo->m_vertexNum != 0 && rdata->m_vbo->m_indexNum != 0)
			{
				glBindBuffer(GL_ARRAY_BUFFER, rdata->m_vbo->m_vertexBufferId);
				glVertexPointer(rdata->m_vbo->m_vertexFromat,GL_FLOAT,0,0);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rdata->m_vbo->m_indexBufferId);
				glDrawElements(GL_LINES, rdata->m_vbo->m_indexNum*rdata->m_vbo->m_indexFormat, GL_UNSIGNED_INT, NULL);
			}
		}

		glDisableClientState(GL_VERTEX_ARRAY);
	}

	void CParaCoordCanvas::DrawIndication()
	{
		float pos[3] = {0.0f, 0.0f, 0.6f};
		CFilter* filter;
		CDataManager* dm = GetDataManager();
		CParaCoordIData* idata;
		idata = dynamic_cast<CParaCoordIData*>(m_RDataList[0]->m_parent[0]);
		assert(idata != NULL);
		t_uintvector &si = idata->m_selectedItems;
		if(si.size() < 2)
			return;
		float step  = 0.8f / (si.size() -1);
		float start = 0.1f;
		vector<float> steps;
		steps.resize(si.size());
		for(unsigned int i = 0; i < si.size(); i++)
			steps[i] = 0.9f / (dm->m_RawData->m_item_desc[si[i]].num_values -1);

		for(unsigned int i = 0; i < dm->m_colorTable.size() + 1; i++)
		{
			t_color color;
			if( i < dm->m_colorTable.size())
			{
				filter = dm->m_colorTable[i].second;
				color = dm->m_colorTable[i].first;
			}
			else
			{
				color.red = 0; color.green = 0; color.blue = 0; color.alpha =1.0f;
				filter = dm->m_ExclusiveFilter;
			}

			for(unsigned int j = 0; j < filter->m_Filter.size(); j++)
			{
				t_AndFilterSet &afs = filter->m_Filter[j];
				for( t_AndFilterSet::iterator it = afs.begin(); it != afs.end(); it++ )
				{
					for( unsigned int k = 0; k < si.size(); k++)
					{
						if(it->first == si[k])
						{
							pos[0] = start + k*step;
							if(dm->m_RawData->m_item_desc[it->first].num_values != 0)
								pos[1] = it->second*steps[k] +0.05f;
							else
							{
								float value = dm->m_RawData->GetNumDataValue(it->second, it->first);
								value = log10(value - dm->m_RawData->m_item_desc[it->first].min +1.0f) * dm->m_RawData->m_item_desc[it->first].logcoeff;
								pos[1] = value*0.9f + 0.05f;
							}

							glColor4ub(color.red, color.green, color.blue, color.alpha);
							glPointSize(5);
							glBegin(GL_POINTS);
							glVertex3fv(pos);
							glEnd();
							glColor4fv(BLACK);
							pos[0] += 0.01f*(m_Ortho[1]-m_Ortho[0]);
							glRasterPos3fv(pos);
							char str[256];
							if(dm->m_RawData->m_item_desc[it->first].num_values != 0)
								sprintf(str,"%s",(char*)(dm->m_RawData->m_item_desc[it->first].value_names[it->second].c_str()));
							else
							{
								float value = dm->m_RawData->GetNumDataValue(it->second, it->first);
								sprintf(str, "%f", value);
							}
							glPrint("%s",str);
						}
					}
				}
			}
		}
	}

	void CParaCoordCanvas::DrawCanvas()
	{
		static int timeinit=0;
		static PerformanceTimer pTimer;
		if(!timeinit)
		{
			pTimer.StartTimer();	
			timeinit=1;
		}

		PerformanceTimer timer;
		timer.StartTimer();
		RenderVBO();
		DrawIndication();
		CLogFile::GetLogFile()->Add("RenderingTime",timer.GetTimeElapsed()*1000.0f);
		CLogFile::GetLogFile()->Add("Totaltime",pTimer.GetTimeElapsed()*1000.0f);
		CLogFile::GetLogFile()->NextLine();
		pTimer.StartTimer();
	}


	void CParaCoordCanvas::SelectData()
	{
		CDataManager *dm = GetDataManager();
		CParaCoordIData* _data = NULL;
		_data = dynamic_cast<CParaCoordIData*>(m_RDataList[0]->m_parent[0]);
		assert(_data != NULL);

		uint itemidx =0;
		int valueidx =0;
		t_uintvector &si = _data->m_selectedItems;
		if(si.size() <2)
			return;

		float step =0.8f/(si.size() -1);
		float start = 0.1f;

		int x1 = round_value( m_newOrtho[0], start, step );
		int x2 = round_value( m_newOrtho[2], start, step );

		if ( x2 != x1+1 )
		{
			Refresh(false);
			return;
		}

		itemidx = si[x2];
		float y1, y2;

		if ( dm->m_RawData->m_item_desc[itemidx].num_values != 0 )
		{
			step = 0.9f / (dm->m_RawData->m_item_desc[itemidx].num_values - 1);
			start = 0.05;

			y1 = (float)round_value( m_newOrtho[1], start, step );
			y2 = (float)round_value( m_newOrtho[3], start, step );
		}
		else
		{
			y1 = m_newOrtho[1];
			y2 = m_newOrtho[3];
		}

		set<int> recordset;
		
		//if(dm->m_BrushType <= Brush_OR_EX)
		{
			if(m_if_gpu)
			{
				for(unsigned int i=0;  i<_data->m_compactNum; i++)
				{
					valueidx = _data->m_compactRecords[x2*_data->m_compactNum+i];
					recordset.insert( (int)valueidx );
				}
			}
			else
			{
				for ( t_recordset::iterator it = _data->m_records.begin(); it != _data->m_records.end(); it++ )
				{
					valueidx = (*it).first[x2];
					recordset.insert( (int)valueidx );
				}
			}

			
			CFilter* filter_ptr;
			for(unsigned int i = 0; i < dm->m_Filters.size() +2; i++) //total filters
			{
				if( i == dm->m_Filters.size())
					filter_ptr = dm->m_ExclusiveFilter;
				else if( i == dm->m_Filters.size() + 1 )
					filter_ptr = dm->m_NegExclusiveFilter;
				else
					filter_ptr = dm->m_Filters[i];

				for(unsigned int j = 0; j < filter_ptr->m_Filter.size(); j++)
				{
					t_AndFilterSet &afs = filter_ptr->m_Filter[j];
					for ( t_AndFilterSet::iterator it = afs.begin(); it != afs.end(); it++ )
					{
						if ( it->first == si[x2] )
							recordset.insert((int)it->second);
					}
				}
			}
		}
		

		for(set<int>::iterator it= recordset.begin(); it != recordset.end(); it++)
		{
			valueidx = *it;
			if(dm->m_RawData->m_item_desc[itemidx].num_values !=0)
			{
				if(y1 < valueidx && valueidx <= y2)
				{
					rf->m_cf->UpdateBrush(itemidx,valueidx);
					rf->m_cf->UpdateFilter();
				}
			}
			else
			{
				float value = dm->m_RawData->GetNumDataValue(valueidx, itemidx);
				value = log10( value - dm->m_RawData->m_item_desc[itemidx].min + 1.0f ) * dm->m_RawData->m_item_desc[itemidx].logcoeff;
				value = value*0.9f + 0.05f;

				if ( y1 < value && value < y2 )
				{
					rf->m_cf->UpdateBrush(itemidx,valueidx);
					rf->m_cf->UpdateFilter();
				}
			}
		} 
	}

	void CParaCoordCanvas::SetGPU(bool _gpu)
	{
		CParaCoordRData* rdata;
		CParaCoordIData* idata;
		for(unsigned int i = 0; i < m_RDataList.size(); i++)
		{
			rdata = dynamic_cast<CParaCoordRData*>(m_RDataList[i]);
			assert(rdata != NULL);
			idata = dynamic_cast<CParaCoordIData*>(rdata->m_parent[0]);
			assert(idata != NULL);
			idata->CleanSelf();
			rdata->CleanSelf();
			idata->SetGPU(_gpu);
			rdata->SetGPU(_gpu);
			idata->Update();
			Refresh(false);
		}
	}
}