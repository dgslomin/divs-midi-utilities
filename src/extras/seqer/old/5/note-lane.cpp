
#include <wx/wx.h>
#include "lane.h"
#include "midifile.h"
#include "note-lane.h"
#include "window.h"

NoteLane::NoteLane(wxWindow* parent, Window* window): Lane(parent, window)
{
}

NoteLane::~NoteLane()
{
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

void NoteLane::PaintEvents(wxDC& dc, int width, int height, int selected_events_x_offset, int selected_events_y_offset)
{
	wxRect bounds(0, 0, width, height);

	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		if (MidiFileEvent_isNoteStartEvent(midi_event))
		{
			wxRect rect = this->GetRectFromEvent(midi_event, selected_events_x_offset, selected_events_y_offset);

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
			wxRect rect = this->GetRectFromEvent(midi_event, 0, 0);
			if (rect.Contains(x, y)) return midi_event;
		}
	}

	return NULL;
}

MidiFileEvent_t NoteLane::AddEventAtXY(int x, int y)
{
	int start_tick = this->window->GetTickFromX(x);
	int end_tick = MidiFile_getTickFromBeat(this->window->sequence->midi_file, MidiFile_getBeatFromTick(this->window->sequence->midi_file, start_tick) + 1);
	int note = this->GetNoteFromY(y);
	return MidiFileTrack_createNoteStartAndEndEvents(this->track, start_tick, end_tick, this->channel, note, this->velocity, 0);
}

void NoteLane::MoveEventByXY(MidiFileEvent_t midi_event, int x_offset, int y_offset)
{
	MidiFileEvent_t note_end_midi_event = MidiFileNoteStartEvent_getNoteEndEvent(midi_event);
	MidiFileEvent_setTick(midi_event, this->window->GetTickFromX(this->window->GetXFromTick(MidiFileEvent_getTick(midi_event)) + x_offset));
	MidiFileEvent_setTick(note_end_midi_event, this->window->GetTickFromX(this->window->GetXFromTick(MidiFileEvent_getTick(note_end_midi_event)) + x_offset));
	MidiFileNoteStartEvent_setNote(midi_event, this->GetNoteFromY(this->GetYFromNote(MidiFileNoteStartEvent_getNote(midi_event)) + y_offset));
}

void NoteLane::SelectEventsInRect(int x, int y, int width, int height)
{
	wxRect bounds(x, y, width, height);

	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		if (MidiFileEvent_isNoteStartEvent(midi_event))
		{
			wxRect rect = this->GetRectFromEvent(midi_event, 0, 0);

			if (rect.Intersects(bounds))
			{
				MidiFileEvent_setSelected(midi_event, 1);
			}
		}
	}
}

wxRect NoteLane::GetRectFromEvent(MidiFileEvent_t midi_event, int selected_events_x_offset, int selected_events_y_offset)
{
	int start_x = this->window->GetXFromTick(MidiFileEvent_getTick(midi_event));
	int end_x = this->window->GetXFromTick(MidiFileEvent_getTick(MidiFileNoteStartEvent_getNoteEndEvent(midi_event)));
	int y = this->GetYFromNote(MidiFileNoteStartEvent_getNote(midi_event));

	if (MidiFileEvent_isSelected(midi_event))
	{
		start_x += selected_events_x_offset;
		end_x += selected_events_x_offset;
		y += selected_events_y_offset;
	}

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

