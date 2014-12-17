#pragma once

namespace VIS
{
class CViewApp: public wxApp
{
public:
    bool OnInit();
	void LoadConfigFile();
};
}

