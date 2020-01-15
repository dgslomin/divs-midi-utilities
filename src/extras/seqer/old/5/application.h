#ifndef APPLICATION_INCLUDED
#define APPLICATION_INCLUDED

class Application;

#include <wx/wx.h>

class Application: public wxApp
{
public:
	wxBrush background_brush;
	wxBrush white_note_background_brush;
	wxBrush black_note_background_brush;

	wxPen unselected_event_pen;
	wxBrush unselected_event_brush;
	wxColour unselected_event_text_color;

	wxPen selected_event_pen;
	wxBrush selected_event_brush;
	wxColour selected_event_text_color;

	virtual bool OnInit();
};

#endif
