
#include <wx/wx.h>
#include "lane.h"
#include "window.h"

Lane::Lane(Window* window): wxWindow(window, wxID_ANY)
{
	this->window = window;
	this->Bind(wxEVT_PAINT, [=](wxPaintEvent& event) { this->OnPaint(event); });
	this->Bind(wxEVT_LEFT_DOWN, [=](wxMouseEvent& event) { this->OnLeftDown(event); });
	this->Bind(wxEVT_LEFT_UP, [=](wxMouseEvent& event) { this->OnLeftUp(event); });
	this->Bind(wxEVT_MOTION, [=](wxMouseEvent& event) { this->OnMotion(event); });
	this->Bind(wxEVT_CHAR, [=](wxKeyEvent& event) { this->OnChar(event); });
}

Lane::~Lane()
{
}

void Lane::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	int width;
	int height;
	this->GetSize(&width, &height);

	this->PaintBackground(dc, width, height);

	bool should_paint_selection_rect = false;
	int selected_events_x_offset = 0;
	int selected_events_y_offset = 0;

	if (this->mouse_down && (this->mouse_down_midi_event != NULL))
	{
		should_paint_selection_rect = true;
	}
	else
	{
		if (this->mouse_drag_x_allowed) selected_events_x_offset = this->mouse_drag_x - this->mouse_down_x;
		if (this->mouse_drag_y_allowed) selected_events_y_offset = this->mouse_drag_y - this->mouse_down_y;
	}

	this->PaintEvents(dc, width, height, selected_events_x_offset, selected_events_y_offset);

	dc.SetPen(this->window->application->cursor_pen);
	dc.DrawLine(dc, this->cursor_x, 0, this->cursor_x, height);

	if (should_paint_selection_rect)
	{
		dc.SetPen(this->window->application->selection_rect_pen);
		dc.SetBrush(this->window->application->selection_rect_brush);
		dc.DrawRectangle(dc, this->mouse_down_x, this->mouse_down_y, this->mouse_drag_x - this->mouse_down_x, this->mouse_drag_y - this->mouse_down_y);
	}

	event.Skip();
}

void Lane::OnLeftDown(wxMouseEvent& event)
{
	this->CaptureMouse();
	this->mouse_down = true;
	this->mouse_down_x = event.GetX();
	this->mouse_down_y = event.GetY();
	this->mouse_down_midi_event = this->GetEventFromXY(this->mouse_down_x, this->mouse_down_y);
	this->mouse_down_midi_event_is_new = false;
	this->mouse_drag_x_allowed = false;
	this->mouse_drag_y_allowed = false;

	if ((this->mouse_down_midi_event == NULL) && !event.ShiftDown())
	{
		this->window->SelectNone();

		if ((this->mouse_down_x == this->cursor_x) && (this->mouse_down_y == this->cursor_y))
		{
			this->mouse_down_midi_event = this->AddEventAtXY(this->mouse_down_x, this->mouse_down_y);
			MidiFileEvent_setSelected(this->mouse_down_midi_event, 1);
			this->mouse_down_midi_event_is_new = true;
		}
	}

	event.Skip();
}

void Lane::OnLeftUp(wxMouseEvent& event)
{
	int mouse_x = event.GetX();
	int mouse_y = event.GetY();

	if (this->mouse_down_midi_event == NULL)
	{
		if ((mouse_x == this->mouse_down_x) && (mouse_y == this->mouse_down_y))
		{
			this->cursor_x = mouse_y;
			this->cursor_y = mouse_y;
		}
		else
		{
			this->SelectEventsInRect(min(this->mouse_down_x, mouse_x), min(this->mouse_down_y, mouse_y), abs(mouse_x - this->mouse_down_x), abs(mouse_y - this->mouse_down_y));
		}
	}
	else
	{
		if ((this->mouse_drag_x_allowed && (mouse_x != this->mouse_down_x)) || (this->mouse_drag_y_allowed && (mouse_y != this->mouse_down_y)))
		{
			int x_offset = this->mouse_drag_x_allowed ? (this->mouse_drag_x - this->mouse_down_x) : 0
			int y_offset = this->mouse_drag_y_allowed ? (this->mouse_drag_y - this->mouse_down_y) : 0

			for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
			{
				if (MidiFileEvent_isSelected(midi_event))
				{
					this->MoveEventByXY(midi_event, x_offset, y_offset);
				}
			}
		}
		else
		{
			if (!event->ShiftDown() && !this->mouse_down_midi_event_is_new && (mouse_x == this->cursor_x) && (mouse_y == this->cursor_y))
			{
				this->window->FocusPropertyEditor();
			}
		}
	}

	this->mouse_down = false;
	this->ReleaseMouse();
	event.Skip();
}

void Lane::OnMotion(wxMouseEvent& event)
{
	if (this->mouse_down)
	{
		this->mouse_drag_x = event.GetX();
		this->mouse_drag_y = event.GetY();
		if (abs(this->mouse_drag_x - this->mouse_down_x) > this->window->application->mouse_drag_threshold) this->mouse_drag_x_allowed = true;
		if (abs(this->mouse_drag_y - this->mouse_down_y) > this->window->application->mouse_drag_threshold) this->mouse_drag_y_allowed = true;
		this->window->Refresh();
	}

	event.Skip();
}

void Lane::OnChar(wxKeyEvent& event)
{
}

