
#include <wx/wx.h>
#include "application.h"
#include "window.h"

IMPLEMENT_APP(Application)

bool Application::OnInit()
{
	if (wxGetEnv("SEQER_DEBUG", NULL)) wxMessageBox("Attach debugger now.", "Seqer", wxOK);

	this->background_brush = wxBrush(*wxWHITE);
	this->white_note_background_brush = wxBrush(*wxWHITE);
	this->black_note_background_brush = wxBrush(*wxLIGHT_GREY);
	this->unselected_event_pen = wxPen(*wxBLACK);
	this->unselected_event_brush = wxBrush(*wxWHITE);
	this->unselected_event_text_color = *wxBLACK;
	this->selected_event_pen = wxPen(*wxBLACK);
	this->selected_event_brush = wxBrush(*wxBLUE);
	this->selected_event_text_color = *wxWHITE;
	this->cursor_pen = wxPen(*wxBLUE);
	this->selection_rect_pen = wxPen(*wxBLACK);
	this->selection_rect_brush = wxBrush(*wxBLUE);
	this->mouse_drag_threshold = 8;

	Window* window = new Window(this);
	window->Show(true);

	if (this->argc > 1)
	{
		if (!window->Load(this->argv[1]))
		{
			wxMessageBox("Cannot open the specified MIDI file.", "Error", wxOK | wxICON_ERROR);
		}
	}

	return true;
}

