
#include <wx/wx.h>
#include "timeline.h"
#include "window.h"

Timeline::Timeline(Window* window): wxWindow(window, wxID_ANY)
{
	this->Bind(wxEVT_PAINT, &OnPaint);
}

Timeline::~Timeline()
{
}

void Timeline::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
}

