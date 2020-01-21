
#include <wx/wx.h>
#include "label-lane.h"
#include "lane.h"
#include "window.h"

LabelLane::LabelLane(Window* window): Lane(window)
{
}

LabelLane::~LabelLane()
{
}

void LabelLane::PaintBackground(wxDC& dc, int width, int height)
{
	dc.SetBackground(this->window->application->background_brush);
	dc.Clear();
}

void LabelLane::PaintEvents(wxDC& dc, int width, int height, int selected_events_x_offset, int WXUNUSED(selected_events_y_offset))
{
	this->PopulateLabels();
	this->LayoutLabels(selected_events_x_offset);

	int row_height = dc.GetCharHeight() + 4;
	dc.SetBackgroundMode(wxTRANSPARENT);

	for (int label_number = 0; label_number < this->labels.size(); label_number++)
	{
		Label& label = this->labels[label_number];

		if (MidiFileEvent_isSelected(label.midi_event))
		{
			dc.SetPen(this->window->application->selected_event_pen);
			dc.SetBrush(this->window->application->selected_event_brush);
			dc.SetTextForeground(this->window->application->selected_event_text_color);
		}
		else
		{
			dc.SetPen(this->window->application->unselected_event_pen);
			dc.SetBrush(this->window->application->unselected_event_brush);
			dc.SetTextForeground(this->window->application->unselected_event_text_color);
		}

		dc.DrawRectangle(label.x, label.row * row_height, label.width, row_height);
		dc.DrawText(label.text, label.x + 2, (label.row * row_height) + 2);
	}
}

MidiFileEvent_t LabelLane::GetEventFromXY(int x, int y)
{
	wxPaintDC dc(this);
	int row_height = dc.GetCharHeight() + 4;

	for (int label_number = this->labels.size() - 1; label_number >= 0; label_number--)
	{
		Label& label = this->labels[label_number];
		wxRect rect = wxRect(label.x, label.row * row_height, label.width, row_height);
		if (rect.Contains(x, y)) return label.midi_event;
	}

	return NULL;
}

MidiFileEvent_t LabelLane::AddEventAtXY(int x, int WXUNUSED(y))
{
	return this->AddEvent(this->window->GetTickFromX(x));
}

void LabelLane::MoveEventByXY(MidiFileEvent_t midi_event, int x_offset, int WXUNUSED(y_offset))
{
	MidiFileEvent_setTick(midi_event, MidiFileEvent_getTick(midi_event) + x_offset);
}

void LabelLane::SelectEventsInRect(int x, int y, int width, int height)
{
	wxPaintDC dc(this);
	int row_height = dc.GetCharHeight() + 4;
	wxRect bounds(x, y, width, height);

	for (int label_number = 0; label_number < this->labels.size(); label_number++)
	{
		Label& label = this->labels[label_number];
		wxRect rect = wxRect(label.x, label.row * row_height, label.width, row_height);

		if (rect.Intersects(bounds))
		{
			MidiFileEvent_setSelected(label.midi_event, 1);
		}
	}
}

void LabelLane::LayoutLabels(int selected_events_x_offset)
{
	wxClientDC dc(this);
	std::vector<int> row_ends;

	for (int label_number = 0; label_number < this->labels.size(); label_number++)
	{
		Label& label = this->labels[label_number];
		label.x = this->window->GetXFromTick(MidiFileEvent_getTick(label.midi_event));
		if (MidiFileEvent_isSelected(label.midi_event)) label.x += selected_events_x_offset;
		label.width = dc.GetTextExtent(label.text).GetWidth() + 4;

		for (label.row = 0; label.row < row_ends.size(); label.row++)
		{
			if (label.x >= row_ends[label.row]) break;
		}

		int row_end = label.x + label.width;

		if (label.row < row_ends.size())
		{
			row_ends[label.row] = row_end;
		}
		else
		{
			row_ends.push_back(row_end);
		}
	}
}

