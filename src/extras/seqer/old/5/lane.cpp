
#include <wx/wx.h>
#include "lane.h"
#include "window.h"

Lane::Lane(Window* window): wxWindow(window, wxID_ANY)
{
	this->window = window;
	this->Bind(wxEVT_PAINT, [=](wxPaintEvent& event) { this->OnPaint(event); });
}

Lane::~Lane()
{
}

void Lane::OnPaint(wxPaintEvent& event)
{
}

