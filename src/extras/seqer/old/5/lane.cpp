
#include <wx/wx.h>
#include "lane.h"
#include "window.h"

Lane(Window* window): wxWindow(window, wxID_ANY)
{
	this->Bind(wxEVT_PAINT, &OnPaint);
}

~Lane()
{
}

void Lane::OnPaint(wxPaintEvent& event)
{
}

