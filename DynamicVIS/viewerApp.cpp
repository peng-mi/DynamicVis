#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/app.h" 
#include "viewerApp.h"
#include "logframe.h"
#include "datamanager.h"
#include "ControlFrame.h"
#include "Frame.h"
#include "tinyxml/tinystr.h"
#include "tinyxml/tinyxml.h"

#include <vector>
using namespace std;

namespace VIS
{

IMPLEMENT_APP(CViewApp)

bool CViewApp::OnInit()
{
	LoadConfigFile();

	return true;
}

void CViewApp::LoadConfigFile()
{
	TiXmlDocument doc(CONFIGURATION);

	if(!doc.LoadFile())
		exit(0);

	TiXmlHandle hDoc(&doc);
	TiXmlElement* pElem;
	TiXmlHandle hRoot(0);

	pElem=hDoc.FirstChildElement().Element();
	if (!pElem)
		exit(0);

	const char* _name;
	//vector<t_FrameInfo> framesVec;

	VIS::CControlFrame *controlFrame;
	unsigned int controlframe_pos[2], controlframe_size[2];
	uint time_step = 1;
	uint range_step = 1;

	hRoot=TiXmlHandle(pElem);
	TiXmlNode* child, *dataset;
	TiXmlElement* element;
	bool _controlFrameBool = false;
	string datafile;
	if(hRoot.ToNode()!= NULL)
	{
		dataset = hRoot.FirstChild().ToElement();
		for(dataset; dataset; dataset = dataset->NextSibling())
		{
			_controlFrameBool = false;
			t_configPara _cfg;

			child = dataset->FirstChild();
			for(child; child; child = child->NextSibling())
			{
				_name = child->Value();
				if(strcmp(_name,"controlFrame")==0)
				{
					_controlFrameBool = true;
					element =child->FirstChildElement("position");
					controlframe_pos[0] = atoi(element->Attribute("xpos"));
					element =element->NextSiblingElement("position");
					controlframe_pos[1] = atoi(element->Attribute("ypos"));

					element =child->FirstChildElement("size");
					controlframe_size[0] = atoi(element->Attribute("xpos"));
					element =element->NextSiblingElement("size");
					controlframe_size[1] = atoi(element->Attribute("ypos"));

					TiXmlNode* frames;
					_cfg.subframes.clear();
					frames = child->FirstChildElement("subframes")->FirstChild()->ToElement();
					for(frames; frames; frames  = frames->NextSibling())
					{
						t_FrameInfo _frame;
						_frame.name = frames->Value();
						element =frames->FirstChildElement("position");
						_frame.pos[0] = atoi(element->Attribute("xpos"));
						element =element->NextSiblingElement("position");
						_frame.pos[1] = atoi(element->Attribute("ypos"));
						element =frames->FirstChildElement("size");
						_frame.size[0] = atoi(element->Attribute("xpos"));
						element =element->NextSiblingElement("size");
						_frame.size[1] = atoi(element->Attribute("ypos"));
						_cfg.subframes.push_back(_frame);
					}
				}
				else if(strcmp(_name,"data")==0)
				{
					TiXmlNode* _node;
					_node = child->FirstChildElement("file")->ToElement();
					if(_node->FirstChild() != NULL)
						datafile = _node->FirstChild()->Value();
					else
						datafile ="";

					_node = child->FirstChildElement("time_step")->ToElement();
					if(_node->FirstChild() != NULL)
						time_step = atoi(_node->FirstChild()->Value());


					_node = child->FirstChildElement("range_step")->ToElement();
					if(_node->FirstChild() != NULL)
						range_step = atoi(_node->FirstChild()->Value());

				}
			}
			if(_controlFrameBool)
			{
				_cfg.title ="Control Panel ( " + datafile + " )";
				_cfg.data_folder = (char*)datafile.c_str();
				_cfg.pos[0] = controlframe_pos[0];
				_cfg.pos[1] = controlframe_pos[1];
				_cfg.size[0] = controlframe_size[0];
				_cfg.size[1] = controlframe_size[1];
				_cfg.time_step = time_step;
				_cfg.range_step = range_step;


				controlFrame = VIS::CControlFrame::Create(_cfg);
				VIS::CControlFrame::s_ControlFrameList.push_back(controlFrame);

			}

		}
	}


	wxIdleEvent::SetMode( wxIDLE_PROCESS_SPECIFIED );

}

}