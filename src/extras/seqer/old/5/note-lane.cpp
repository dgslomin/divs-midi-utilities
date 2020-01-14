
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
	bool black_note[] = { false, true, false, true, false, false, true, false, true, false, true, false };

	for (int note = this->GetNoteFromY(0), last_note = this->GetNoteFromY(height); note < last_note; note++)
	{
		dc.SetBrush(black_note[note % 12] ? this->black_note_background_brush : this->white_note_background_brush);
		dc.DrawRectangle(0, this->GetYFromNote(note), width, this->pixels_per_note);
	}
}

void NoteLane::PaintNotes(wxDC& dc, int width, int height)
{
	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		if (MidiFileEvent_isNoteStartEvent(midi_event))
		{
			int start_x = this->window->GetXFromTick(MidiFileEvent_getTick(midi_event));
			if (start_x > width) continue;
			int end_x = this->window->GetXFromTick(MidiFileEvent_getTick(MidiFileNoteStartEvent_getNoteEndEvent(midi_event)));
			if (end_x < 0) continue;
			int y = this->GetYFromNote(MidiFileNoteStartEvent_getNote(midi_event));
			if ((y < 0) || (y > height)) continue;
			dc.DrawRectangle(start_x, y, end_x - start_x, this->pixels_per_note);
		}
	}
}

int NoteLane::GetYFromNote(int note)
{
	return (note * this->pixels_per_note) - this->scroll_y;
}

int NoteLane::GetNoteFromY(int y)
{
	return (y + this->scroll_y) / this->pixels_per_note;
}

