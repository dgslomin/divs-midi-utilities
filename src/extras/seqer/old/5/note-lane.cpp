
#include <wx/wx.h>
#include "lane.h"
#include "midifile.h"
#include "note-lane.h"
#include "window.h"

NoteLane::NoteLane(Window* window): Lane(window)
{
}

NoteLane::~NoteLane()
{
}

void NoteLane::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	int width;
	int height;
	this->GetSize(&width, &height);
	this->PaintBackground(dc, width, height);
	this->PaintNotes(dc, width, height);
}

void NoteLane::PaintBackground(wxDC& dc, int width, int height)
{
	dc.SetBackground(this->window->application->background_brush);
	dc.Clear();

	bool black_note[] = { false, true, false, true, false, false, true, false, true, false, true, false };

	for (int note = this->GetNoteFromY(0), last_note = this->GetNoteFromY(height); note < last_note; note++)
	{
		dc.SetBrush(black_note[note % 12] ? this->window->application->black_note_background_brush : this->window->application->white_note_background_brush);
		dc.DrawRectangle(0, this->GetYFromNote(note), width, this->pixels_per_note);
	}
}

void NoteLane::PaintNotes(wxDC& dc, int width, int height)
{
	wxRect bounds = wxRect(0, 0, width, height);

	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		if (MidiFileEvent_isNoteStartEvent(midi_event))
		{
			wxRect rect = this->GetRectFromEvent(midi_event);

			if (rect.Intersects(bounds))
			{
				if (MidiFileEvent_isSelected(midi_event))
				{
					dc.SetPen(this->window->application->selected_event_pen);
					dc.SetBrush(this->window->application->selected_event_brush);
				}
				else
				{
					dc.SetPen(this->window->application->unselected_event_pen);
					dc.SetBrush(this->window->application->unselected_event_brush);
				}

				dc.DrawRectangle(rect);
			}
		}
	}
}

MidiFileEvent_t NoteLane::GetEventFromXY(int x, int y)
{
	for (MidiFileEvent_t midi_event = MidiFile_getLastEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getPreviousEventInFile(midi_event))
	{
		if (MidiFileEvent_isNoteStartEvent(midi_event))
		{
			wxRect rect = this->GetRectFromEvent(midi_event);
			if (rect.Contains(x, y)) return midi_event;
		}
	}

	return NULL;
}

wxRect NoteLane::GetRectFromEvent(MidiFileEvent_t midi_event)
{
	int start_x = this->window->GetXFromTick(MidiFileEvent_getTick(midi_event));
	int end_x = this->window->GetXFromTick(MidiFileEvent_getTick(MidiFileNoteStartEvent_getNoteEndEvent(midi_event)));
	int y = this->GetYFromNote(MidiFileNoteStartEvent_getNote(midi_event));
	return wxRect(start_x, y, end_x - start_x, this->pixels_per_note);
}

int NoteLane::GetYFromNote(int note)
{
	return (note * this->pixels_per_note) - this->scroll_y;
}

int NoteLane::GetNoteFromY(int y)
{
	return (y + this->scroll_y) / this->pixels_per_note;
}

