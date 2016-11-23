
#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"
#include "event-list.h"
#include "piano-roll.h"
#include "music-math.h"
#include "color.h"

PianoRoll::PianoRoll(SequenceEditor* sequence_editor)
{
	this->sequence_editor = sequence_editor;
	this->first_note = this->sequence_editor->window->application->default_piano_roll_first_note;
	this->last_note = this->sequence_editor->window->application->default_piano_roll_last_note;
	this->key_width = this->sequence_editor->window->application->default_piano_roll_key_width;
}

void PianoRoll::RefreshData()
{
	wxColour button_color = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
	this->darker_line_color = ColorShade(button_color, 80);
	this->lighter_line_color = ColorShade(button_color, 85);
	this->white_key_color = *wxWHITE;
	this->black_key_color = ColorShade(button_color, 90);
	this->shadow_color = ColorShade(button_color, 75);
}

void PianoRoll::OnDraw(wxDC& dc)
{
	wxPen pens[] = {wxPen(this->darker_line_color), wxPen(this->lighter_line_color)};
	wxBrush brushes[] = {wxBrush(this->white_key_color), wxBrush(this->black_key_color)};
	long key_pens[] = {0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1};
	long key_brushes[] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0};
	long first_y = this->sequence_editor->GetFirstVisibleY();
	long last_y = this->sequence_editor->GetLastVisibleY();
	long width = this->GetWidth();
	long height = this->sequence_editor->GetVisibleHeight();

	for (long note = this->first_note; note <= this->last_note; note++)
	{
		dc.SetPen(pens[key_pens[note % 12]]);
		dc.SetBrush(brushes[key_brushes[note % 12]]);
		dc.DrawRectangle((note - this->first_note) * this->key_width - 1, first_y - 1, this->key_width + 1, height + 2);
	}

	long first_step_number = this->sequence_editor->GetStepNumberFromRowNumber(this->sequence_editor->event_list->GetRowNumberFromY(first_y));
	long last_step_number = this->sequence_editor->GetStepNumberFromRowNumber(this->sequence_editor->event_list->GetRowNumberFromY(last_y));
	dc.SetPen(wxPen(this->lighter_line_color));

	for (long step_number = first_step_number; step_number <= last_step_number; step_number++)
	{
		long y = this->GetYFromStepNumber(step_number);
		if (y == 0) continue;
		dc.DrawLine(0, y, width, y);
	}

	const wxPen pass_pens[] = {wxPen(this->shadow_color), *wxBLACK_PEN};
	const wxBrush pass_brushes[] = {wxBrush(this->shadow_color), *wxWHITE_BRUSH};
	const long pass_offsets[] = {1, 0};

	for (int pass = 0; pass < 2; pass++)
	{
		dc.SetPen(pass_pens[pass]);
		dc.SetBrush(pass_brushes[pass]);

		for (MidiFileEvent_t event = MidiFile_getFirstEvent(this->sequence_editor->sequence->midi_file); event != NULL; event = MidiFileEvent_getNextEventInFile(event))
		{
			if (MidiFileEvent_isNoteStartEvent(event) && this->sequence_editor->Filter(event))
			{
				int note = MidiFileNoteOnEvent_getNote(event);

				if ((note >= this->first_note) && (note <= this->last_note))
				{
					double event_step_number = this->sequence_editor->GetFractionalStepNumberFromTick(MidiFileEvent_getTick(event));
					long event_y = this->GetYFromStepNumber(event_step_number);

					MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);
					double end_event_step_number = this->sequence_editor->GetFractionalStepNumberFromTick(MidiFileEvent_getTick(end_event));
					long end_event_y = this->GetYFromStepNumber(end_event_step_number);

					if ((event_y <= last_y) && (end_event_y >= first_y))
					{
						dc.DrawRectangle((note - this->first_note) * this->key_width - 1 + pass_offsets[pass], event_y + pass_offsets[pass], this->key_width + 1, end_event_y - event_y);
					}
				}
			}
		}
	}
}

long PianoRoll::GetWidth()
{
	return this->key_width * (this->last_note - this->first_note + 1);
}

long PianoRoll::GetYFromStepNumber(double step_number)
{
	long step_first_y = this->sequence_editor->event_list->GetYFromRowNumber(this->sequence_editor->GetFirstRowNumberFromStepNumber((long)(step_number)));
	long step_last_y = this->sequence_editor->event_list->GetYFromRowNumber(this->sequence_editor->GetLastRowNumberFromStepNumber((long)(step_number)) + 1);
	return step_first_y + (long)((step_last_y - step_first_y) * (step_number - (long)(step_number)));
}

