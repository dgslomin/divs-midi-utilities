#ifndef APPLICATION_INCLUDED
#define APPLICATION_INCLUDED

class Application;

#include <wx/wx.h>

class Application: public wxApp
{
public:
	bool OnInit();
};

#endif
