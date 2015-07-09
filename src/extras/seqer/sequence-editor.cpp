
#include <math.h>
#include <algorithm>
#include <vector>
#include <wx/wx.h>
#include <midifile.h>
#include "seqer.h"
#include "sequence-editor.h"
#include "step-size.h"
#include "event-type.h"
#include "color.h"

#if defined(__WXMSW__)
#define CANVAS_BORDER wxBORDER_THEME
#else
#define CANVAS_BORDER wxBORDER_DEFAULT
#endif

wxString GetNoteNameFromNumber(int note_number)
{
	const char* note_names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
	return wxString::Format("%s%d", note_names[note_number % 12], (note_number / 12) - 1);
}

int GetNoteNumberFromName(wxString note_name)
{
	const char* note_names[] = {"C#", "C", "Db", "D#", "D", "Eb", "E", "F#", "F", "Gb", "G#", "G", "Ab", "A#", "A", "Bb", "B"};
	const int chromatics[] = {1, 0, 1, 3, 2, 3, 4, 6, 5, 6, 8, 7, 8, 10, 9, 10, 11};
	int chromatic = -1;
	int octave = -1;
	int length;

	for (int i = 0; i < (sizeof note_names / sizeof (char*)); i++)
	{
		length = strlen(note_names[i]);

		if (strncmp(note_names[i], note_name.c_str(), length) == 0)
		{
			chromatic = chromatics[i];
			break;
		}
	}

	if (chromatic < 0) return -1;
	if (sscanf(note_name.c_str() + length, "%d", &octave) != 1) return -1;
	return ((octave + 1) * 12) + chromatic;
}

int GetNoteOctave(int note_number)
{
	return (note_number / 12) - 1;
}

int SetNoteOctave(int note_number, int octave)
{
	return (note_number % 12) + ((octave + 1) * 12);
}

int GetNoteChromatic(int note_number)
{
	return note_number % 12;
}

int SetNoteChromatic(int note_number, int chromatic)
{
	return ((note_number / 12) * 12) + chromatic;
}

wxString GetKeyNameFromNumber(int key_number, bool is_minor)
{
	while (key_number < -6) key_number += 12;
	while (key_number > 6) key_number -= 12;
	const char* key_names[] = {"Gb", "Db", "Ab", "Eb", "Bb", "F", "C", "G", "D", "A", "E", "B", "F#"};
	return wxString::Format("%s%s", key_names[key_number + 6], is_minor ? "m" : "");
}

int GetChromaticFromDiatonicInKey(int diatonic, int key_number)
{
	while (key_number < -6) key_number += 12;
	while (key_number > 6) key_number -= 12;
	const int diatonic_to_chromatic[] = {0, 2, 4, 5, 7, 9, 11};
	const int key_diatonics[] = {4, 1, 5, 2, 6, 3, 0, 4, 1, 5, 2, 6, 3};
	const int key_chromatics[] = {6, 1, 8, 3, 10, 5, 0, 7, 2, 9, 4, 11, 6};
	return (key_chromatics[key_number] + diatonic_to_chromatic[(7 + diatonic - key_diatonics[key_number]) % 7]) % 12;
}

SequenceEditor::SequenceEditor(Window* window): wxScrolledCanvas(window, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | CANVAS_BORDER)
{
	this->window = window;
	this->event_list = new EventList(this);
	this->piano_roll = new PianoRoll(this);
	this->step_size = new StepsPerMeasureSize(this);
	this->current_row_number = 0;
	this->DisableKeyboardScrolling();
	this->SetBackgroundColour(*wxWHITE);
	this->Prepare();
}

bool SequenceEditor::Load(wxString filename)
{
	MidiFile_t new_midi_file = MidiFile_load((char *)(filename.ToStdString().c_str()));
	if (new_midi_file == NULL) return false;
	if (this->sequence->midi_file != NULL) MidiFile_free(this->sequence->midi_file);
	this->sequence->midi_file = new_midi_file;
	this->step_size = new StepsPerMeasureSize(this);
	this->Prepare();
	return true;
}

void SequenceEditor::Prepare()
{
	this->rows.clear();
	this->steps.clear();

	if (this->sequence->midi_file != NULL)
	{
		long last_step_number = -1;

		for (MidiFileEvent_t event = MidiFile_getFirstEvent(this->sequence->midi_file); event != NULL; event = MidiFileEvent_getNextEventInFile(event))
		{
			if (!this->Filter(event)) continue;

			long step_number = this->GetStepNumberFromTick(MidiFileEvent_getTick(event));

			while (last_step_number < step_number - 1)
			{
				last_step_number++;
				this->rows.push_back(Row(last_step_number, NULL));
				this->steps.push_back(Step(this->rows.size() - 1));
			}

			this->rows.push_back(Row(step_number, event));

			if (step_number == last_step_number)
			{
				this->steps[this->steps.size() - 1].last_row_number++;
			}
			else
			{
				this->steps.push_back(Step(this->rows.size() - 1));
			}

			last_step_number = step_number;
		}
	}

	this->event_list->Prepare();
	this->piano_roll->Prepare();
	this->Refresh();
}

void SequenceEditor::OnDraw(wxDC& dc)
{
	this->event_list->OnDraw(dc);
	this->piano_roll->OnDraw(dc);
}

long SequenceEditor::GetVisibleWidth()
{
	return this->GetClientSize().GetWidth();
}

long SequenceEditor::GetVisibleHeight()
{
	return this->GetClientSize().GetHeight();
}

long SequenceEditor::GetFirstVisibleY()
{
	return this->event_list->GetYFromRowNumber(this->event_list->GetFirstVisibleRowNumber());
}

long SequenceEditor::GetLastVisibleY()
{
	return this->GetFirstVisibleY() + this->GetVisibleHeight();
}

long SequenceEditor::GetFirstRowNumberFromStepNumber(long step_number)
{
	if (this->steps.size() == 0)
	{
		return step_number;
	}
	else if (step_number < this->steps.size())
	{
		return this->steps[step_number].first_row_number;
	}
	else
	{
		return this->steps[this->steps.size() - 1].last_row_number + step_number - this->steps.size() + 1;
	}
}

long SequenceEditor::GetLastRowNumberFromStepNumber(long step_number)
{
	if (this->steps.size() == 0)
	{
		return step_number;
	}
	else if (step_number < this->steps.size())
	{
		return this->steps[step_number].last_row_number;
	}
	else
	{
		return this->steps[this->steps.size() - 1].last_row_number + step_number - this->steps.size() + 1;
	}
}

long SequenceEditor::GetStepNumberFromRowNumber(long row_number)
{
	if (this->rows.size() == 0)
	{
		return row_number;
	}
	else if (row_number < this->rows.size())
	{
		return this->rows[row_number].step_number;
	}
	else
	{
		return this->rows[this->rows.size() - 1].step_number + row_number - this->rows.size() + 1;
	}
}

long SequenceEditor::GetStepNumberFromTick(long tick)
{
	return (long)(this->GetFractionalStepNumberFromTick(tick));
}

double SequenceEditor::GetFractionalStepNumberFromTick(long tick)
{
	return this->step_size->GetStepFromTick(tick);
}

MidiFileEvent_t SequenceEditor::GetLatestTimeSignatureEventForRowNumber(long row_number)
{
	return MidiFile_getLatestTimeSignatureEventForTick(this->sequence->midi_file, /* TODO */ 0);
}

bool SequenceEditor::Filter(MidiFileEvent_t event)
{
	if (this->filtered_event_types.size() > 0)
	{
		bool matched = false;

		for (int i = 0; i < this->filtered_event_types.size(); i++)
		{
			if (event_types[this->filtered_event_types[i]]->Matches(event))
			{
				matched = true;
				break;
			}
		}

		if (!matched) return false;
	}
	else
	{
		bool matched = false;

		for (int i = 0; event_types[i] != NULL; i++)
		{
			if (event_types[i]->Matches(event))
			{
				matched = true;
				break;
			}
		}

		if (!matched) return false;
	}

	if (this->filtered_tracks.size() > 0)
	{
		bool matched = false;

		for (int i = 0; i < this->filtered_tracks.size(); i++)
		{
			if (this->filtered_tracks[i] == MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)))
			{
				matched = true;
				break;
			}
		}

		if (!matched) return false;
	}

	if ((this->filtered_channels.size() > 0) && MidiFileEvent_isVoiceEvent(event))
	{
		bool matched = false;

		for (int i = 0; i < this->filtered_channels.size(); i++)
		{
			if (this->filtered_channels[i] == MidiFileVoiceEvent_getChannel(event))
			{
				matched = true;
				break;
			}
		}

		if (!matched) return false;
	}

	return true;
}

EventList::EventList(SequenceEditor* sequence_editor)
{
	this->sequence_editor = sequence_editor;
	this->font = this->sequence_editor->window->application->default_event_list_font;
}

void EventList::Prepare()
{
	wxClientDC dc(this->sequence_editor);
	dc.SetFont(this->font);
	this->row_height = dc.GetCharHeight();

	this->column_widths[0] = dc.GetTextExtent("Marker#").GetWidth();
	this->column_widths[1] = dc.GetTextExtent("000:0.000#").GetWidth();
	this->column_widths[2] = dc.GetTextExtent("00#").GetWidth();
	this->column_widths[3] = dc.GetTextExtent("00#").GetWidth();
	this->column_widths[4] = dc.GetTextExtent("C#0#").GetWidth();
	this->column_widths[5] = dc.GetTextExtent("000#").GetWidth();
	this->column_widths[6] = dc.GetTextExtent("000:0.000#").GetWidth();
	this->column_widths[7] = dc.GetTextExtent("000#").GetWidth();

	this->sequence_editor->SetScrollbars(0, this->row_height, 0, this->sequence_editor->rows.size());
}

void EventList::OnDraw(wxDC& dc)
{
	long first_y = this->sequence_editor->GetFirstVisibleY();
	long last_y = this->sequence_editor->GetLastVisibleY();
	long first_step_number = this->sequence_editor->GetStepNumberFromRowNumber(this->GetRowNumberFromY(first_y));
	long last_step_number = this->sequence_editor->GetStepNumberFromRowNumber(this->GetRowNumberFromY(last_y));
	long first_row_number = this->GetFirstVisibleRowNumber();
	long last_populated_row_number = this->GetLastVisiblePopulatedRowNumber();
	long piano_roll_width = this->sequence_editor->piano_roll->GetWidth();
	long width = this->GetVisibleWidth();

	dc.SetPen(wxPen(this->sequence_editor->piano_roll->lightest_line_color));

	for (long step_number = first_step_number; step_number <= last_step_number; step_number++)
	{
		long y = this->sequence_editor->piano_roll->GetYFromStepNumber(step_number);
		if (y == 0) continue;
		dc.DrawLine(piano_roll_width, y, piano_roll_width + width, y);
	}

	dc.SetFont(this->font);

	for (long row_number = first_row_number; row_number <= last_populated_row_number; row_number++)
	{
		Row row = this->sequence_editor->rows[row_number];

		if (row.event == NULL)
		{
			dc.DrawText(MidiFile_getMeasureBeatStringFromTick(this->sequence_editor->sequence->midi_file, MidiFile_getTickFromBeat(this->sequence_editor->sequence->midi_file, /* TODO: step size */ row.step_number)), this->GetXFromColumnNumber(1), this->GetYFromRowNumber(row_number) + 1);
		}
		else
		{
			bool matched = false;

			for (int event_type_number = 0; event_types[event_type_number] != NULL; event_type_number++)
			{
				if (event_types[event_type_number]->Matches(row.event))
				{
					matched = true;

					for (int column_number = 0; column_number < 8; column_number++)
					{
						wxString column_text = event_types[event_type_number]->GetEventListColumnText(this, row.event, column_number);
						if (!column_text.IsEmpty()) dc.DrawText(column_text, this->GetXFromColumnNumber(column_number) + (column_number == 0 ? 1 : 0), this->GetYFromRowNumber(row_number) + 1);
					}

					break;
				}
			}

			if (!matched) dc.DrawText(MidiFile_getMeasureBeatStringFromTick(this->sequence_editor->sequence->midi_file, MidiFileEvent_getTick(row.event)), this->GetXFromColumnNumber(1), this->GetYFromRowNumber(row_number) + 1);
		}
	}
}

long EventList::GetVisibleWidth()
{
	return this->sequence_editor->GetVisibleWidth() - this->sequence_editor->piano_roll->GetWidth();
}

long EventList::GetFirstVisibleRowNumber()
{
	return this->sequence_editor->GetViewStart().y;
}

long EventList::GetLastVisibleRowNumber()
{
	return this->GetFirstVisibleRowNumber() + this->GetRowNumberFromY(this->sequence_editor->GetVisibleHeight()) + 1;
}

long EventList::GetLastVisiblePopulatedRowNumber()
{
	return std::min(this->GetLastVisibleRowNumber(), (long)(this->sequence_editor->rows.size() - 1));
}

long EventList::GetColumnWidth(long column_number)
{
	return this->column_widths[column_number];
}

long EventList::GetXFromColumnNumber(long column_number)
{
	long x = this->sequence_editor->piano_roll->GetWidth();
	while (--column_number >= 0) x += this->column_widths[column_number];
	return x;
}

long EventList::GetYFromRowNumber(long row_number)
{
	return row_number * this->row_height;
}

long EventList::GetRowNumberFromY(long y)
{
	return y / this->row_height;
}

PianoRoll::PianoRoll(SequenceEditor* sequence_editor)
{
	this->sequence_editor = sequence_editor;
	this->first_note = this->sequence_editor->window->application->default_piano_roll_first_note;
	this->last_note = this->sequence_editor->window->application->default_piano_roll_last_note;
	this->key_width = this->sequence_editor->window->application->default_piano_roll_key_width;
}

void PianoRoll::Prepare()
{
	wxColour button_color = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
	this->darker_line_color = ColorShade(button_color, 205 * 100 / 255);
	this->lighter_line_color = ColorShade(button_color, 215 * 100 / 255);
	this->lightest_line_color = ColorShade(button_color, 218 * 100 / 255);
	this->white_key_color = *wxWHITE;
	this->black_key_color = ColorShade(button_color, 230 * 100 / 255);
	this->shadow_color = ColorShade(button_color, 192 * 100 / 255);
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
	dc.SetPen(wxPen(this->lightest_line_color));

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
				double event_step_number = this->sequence_editor->GetFractionalStepNumberFromTick(MidiFileEvent_getTick(event));
				long event_y = this->GetYFromStepNumber(event_step_number);

				MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);
				double end_event_step_number = this->sequence_editor->GetFractionalStepNumberFromTick(MidiFileEvent_getTick(end_event));
				long end_event_y = this->GetYFromStepNumber(end_event_step_number);

				if ((event_y <= last_y) && (end_event_y >= first_y))
				{
					dc.DrawRectangle((MidiFileNoteOnEvent_getNote(event) - this->first_note) * this->key_width - 1 + pass_offsets[pass], event_y + pass_offsets[pass], this->key_width + 1, end_event_y - event_y);
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

Sequence::Sequence(SequenceEditor* sequence_editor)
{
	this->sequence_editor = sequence_editor;
	this->midi_file = NULL;
}

Row::Row(long step_number, MidiFileEvent_t event)
{
	this->step_number = step_number;
	this->event = event;
}

Step::Step(long row_number)
{
	this->first_row_number = row_number;
	this->last_row_number = row_number;
}

