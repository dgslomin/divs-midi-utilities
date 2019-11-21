
#include <wx/wx.h>

NoteLane::NoteLane(Window* window): wxWindow(window, wxID_ANY)
{
	this->Bind(wxEVT_PAINT, &OnPaint);
}

NoteLane::~NoteLane()
{
}

void NoteLane::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	this->PaintBackground(dc);
	this->PaintNotes(dc);
}

void NoteLane::PaintBackground(wxDC& dc)
{
	bool[] black_note = { false, true, false, true, false, false, true, false, true, false, true, false };

	for (int note = this->GetNoteForY(0), last_note = this->GetNoteForY(this->height); note < last_note; note++)
	{
		dc.SetBrush(black_note[note % 12] ? this->black_note_background_brush : this->white_note_background_brush);
		dc.drawRectangle(0, this->GetYForNote(note), this->window->width, this->note_height);
	}
}

void NoteLane::PaintNotes(wxDC& dc)
{
	for (MidiFileEvent_t midi_event = MidiFile_getFirstEventInFile(this->window->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		if (MidiFileEvent_isNoteStartEvent(midi_event))
		{
			int start_x = this->window->GetXForTick(MidiFileEvent_getTick(midi_event));
			if (start_x > this->window->width) continue;
			int end_x = this->window->GetXForTick(MidiFileEvent_getTick(MidiFileNoteStartEvent_getEndEvent(midi_event)));
			if (end_x < 0) continue;
			int y = this->GetYForNote(MidiFileNoteStartEvent_getNote(midi_event));
			if ((y < 0) || (y > this->height)) continue;
			dc.drawRectangle(start_x, y, end_x - start_x, this->note_height);
		}
	}
}

int NoteLane::GetYForNote(int note)
{
	return (note * this->note_height) - scroll_y;
}

int NoteLane::GetNoteForY(int y)
{
	return (y + scroll_y) / this->note_height;
}

