
#include <algorithm>
#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"
#include "event-list.h"
#include "piano-roll.h"
#include "music-math.h"
#include "color.h"

EventList::EventList(SequenceEditor* sequence_editor)
{
	this->sequence_editor = sequence_editor;
	this->font = this->sequence_editor->window->application->default_event_list_font;
}

void EventList::RefreshData()
{
	wxClientDC dc(this->sequence_editor);
	dc.SetFont(this->font);
	this->row_height = dc.GetCharHeight();

	wxColour button_color = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
	this->current_cell_border_color = ColorShade(button_color, 25);

	this->column_widths[0] = dc.GetTextExtent("Marker#").GetWidth();
	this->column_widths[1] = dc.GetTextExtent(this->sequence_editor->step_size->GetTimeStringFromTick(0) + "###").GetWidth();
	this->column_widths[2] = dc.GetTextExtent("00#").GetWidth();
	this->column_widths[3] = dc.GetTextExtent("00#").GetWidth();
	this->column_widths[4] = dc.GetTextExtent("C#0#").GetWidth();
	this->column_widths[5] = dc.GetTextExtent("000#").GetWidth();
	this->column_widths[6] = dc.GetTextExtent(this->sequence_editor->step_size->GetTimeStringFromTick(0) + "###").GetWidth();
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
	long last_row_number = this->GetLastVisibleRowNumber();
	long piano_roll_width = this->sequence_editor->piano_roll->GetWidth();
	long width = this->GetVisibleWidth();

	dc.SetPen(wxPen(this->sequence_editor->piano_roll->lighter_line_color));

	for (long step_number = first_step_number; step_number <= last_step_number; step_number++)
	{
		long y = this->sequence_editor->piano_roll->GetYFromStepNumber(step_number);
		if (y == 0) continue;
		dc.DrawLine(piano_roll_width, y, piano_roll_width + width, y);
	}

	dc.SetPen(this->current_cell_border_color);
	dc.DrawRectangle(this->GetXFromColumnNumber(this->sequence_editor->current_column_number) - 1, this->GetYFromRowNumber(this->sequence_editor->current_row_number), this->GetColumnWidth(this->sequence_editor->current_column_number) + 1, this->row_height + 1);

	dc.SetFont(this->font);

	for (long row_number = first_row_number; row_number <= last_row_number; row_number++)
	{
		for (long column_number = 0; column_number < 8; column_number++)
		{
			wxString cell_text = this->GetCellText(row_number, column_number);
			if (!cell_text.IsEmpty()) dc.DrawText(cell_text, this->GetXFromColumnNumber(column_number) + (column_number == 0 ? 1 : 0), this->GetYFromRowNumber(row_number) + 1);
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
	return std::min<int>(this->GetLastVisibleRowNumber(), (long)(this->sequence_editor->rows.size() - 1));
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

wxString EventList::GetCellText(long row_number, long column_number)
{
	if ((row_number >= this->sequence_editor->rows.size()) || (this->sequence_editor->rows[row_number].event == NULL))
	{
		long step_number = this->sequence_editor->GetStepNumberFromRowNumber(row_number);

		switch (column_number)
		{
			case 1:
			{
				return this->sequence_editor->step_size->GetTimeStringFromTick(this->sequence_editor->step_size->GetTickFromStep(step_number));
			}
			default:
			{
				return wxEmptyString;
			}
		}
	}
	else
	{
		Row row = this->sequence_editor->rows[row_number];

		switch (this->sequence_editor->GetEventType(row.event))
		{
			case EVENT_TYPE_NOTE:
			{
				switch (column_number)
				{
					case 0:
					{
						return wxString("Note");
					}
					case 1:
					{
						return this->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(row.event));
					}
					case 2:
					{
						return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event)));
					}
					case 3:
					{
						return wxString::Format("%d", MidiFileNoteStartEvent_getChannel(row.event) + 1);
					}
					case 4:
					{
						return GetNoteNameFromNumber(MidiFileNoteStartEvent_getNote(row.event));
					}
					case 5:
					{
						return wxString::Format("%d", MidiFileNoteStartEvent_getVelocity(row.event));
					}
					case 6:
					{
						return this->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(MidiFileNoteStartEvent_getNoteEndEvent(row.event)));
					}
					case 7:
					{
						return wxString::Format("%d", MidiFileNoteEndEvent_getVelocity(MidiFileNoteStartEvent_getNoteEndEvent(row.event)));
					}
					default:
					{
						return wxEmptyString;
					}
				}
			}
			case EVENT_TYPE_CONTROL_CHANGE:
			{
				switch (column_number)
				{
					case 0:
					{
						return wxString("Ctrl");
					}
					case 1:
					{
						return this->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(row.event));
					}
					case 2:
					{
						return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event)));
					}
					case 3:
					{
						return wxString::Format("%d", MidiFileControlChangeEvent_getChannel(row.event) + 1);
					}
					case 4:
					{
						return wxString::Format("%d", MidiFileControlChangeEvent_getNumber(row.event) + 1);
					}
					case 5:
					{
						return wxString::Format("%d", MidiFileControlChangeEvent_getValue(row.event));
					}
					default:
					{
						return wxEmptyString;
					}
				}
			}
			case EVENT_TYPE_PROGRAM_CHANGE:
			{
				switch (column_number)
				{
					case 0:
					{
						return wxString("Prog");
					}
					case 1:
					{
						return this->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(row.event));
					}
					case 2:
					{
						return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event)));
					}
					case 3:
					{
						return wxString::Format("%d", MidiFileProgramChangeEvent_getChannel(row.event) + 1);
					}
					case 4:
					{
						return wxString::Format("%d", MidiFileProgramChangeEvent_getNumber(row.event) + 1);
					}
					default:
					{
						return wxEmptyString;
					}
				}
			}
			case EVENT_TYPE_AFTERTOUCH:
			{
				switch (column_number)
				{
					case 0:
					{
						return wxString("Touch");
					}
					case 1:
					{
						return this->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(row.event));
					}
					case 2:
					{
						return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event)));
					}
					case 3:
					{
						if (MidiFileEvent_getType(row.event) == MIDI_FILE_EVENT_TYPE_KEY_PRESSURE)
						{
							return wxString::Format("%d", MidiFileKeyPressureEvent_getChannel(row.event) + 1);
						}
						else
						{
							return wxString::Format("%d", MidiFileChannelPressureEvent_getChannel(row.event) + 1);
						}
					}
					case 4:
					{
						if (MidiFileEvent_getType(row.event) == MIDI_FILE_EVENT_TYPE_KEY_PRESSURE)
						{
							return GetNoteNameFromNumber(MidiFileKeyPressureEvent_getNote(row.event));
						}
						else
						{
							return wxString("-");
						}
					}
					case 5:
					{
						if (MidiFileEvent_getType(row.event) == MIDI_FILE_EVENT_TYPE_KEY_PRESSURE)
						{
							return wxString::Format("%d", MidiFileKeyPressureEvent_getAmount(row.event));
						}
						else
						{
							return wxString::Format("%d", MidiFileChannelPressureEvent_getAmount(row.event));
						}
					}
					default:
					{
						return wxEmptyString;
					}
				}
			}
			case EVENT_TYPE_PITCH_BEND:
			{
				switch (column_number)
				{
					case 0:
					{
						return wxString("Bend");
					}
					case 1:
					{
						return this->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(row.event));
					}
					case 2:
					{
						return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event)));
					}
					case 3:
					{
						return wxString::Format("%d", MidiFilePitchWheelEvent_getChannel(row.event) + 1);
					}
					case 5:
					{
						return wxString::Format("%d", MidiFilePitchWheelEvent_getValue(row.event));
					}
					default:
					{
						return wxEmptyString;
					}
				}
			}
			case EVENT_TYPE_SYSTEM_EXCLUSIVE:
			{
				// TODO: render bytes as ascii plus hex escapes

				switch (column_number)
				{
					case 0:
					{
						return wxString("Sysex");
					}
					case 1:
					{
						return this->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(row.event));
					}
					case 2:
					{
						return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event)));
					}
					default:
					{
						return wxEmptyString;
					}
				}
			}
			case EVENT_TYPE_TEXT:
			{
				switch (column_number)
				{
					case 0:
					{
						return wxString("Text");
					}
					case 1:
					{
						return this->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(row.event));
					}
					case 2:
					{
						return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event)));
					}
					case 4:
					{
						return wxString(MidiFileTextEvent_getText(row.event));
					}
					default:
					{
						return wxEmptyString;
					}
				}
			}
			case EVENT_TYPE_LYRIC:
			{
				switch (column_number)
				{
					case 0:
					{
						return wxString("Lyric");
					}
					case 1:
					{
						return this->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(row.event));
					}
					case 2:
					{
						return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event)));
					}
					case 4:
					{
						return wxString(MidiFileLyricEvent_getText(row.event));
					}
					default:
					{
						return wxEmptyString;
					}
				}
			}
			case EVENT_TYPE_MARKER:
			{
				switch (column_number)
				{
					case 0:
					{
						return wxString("Marker");
					}
					case 1:
					{
						return this->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(row.event));
					}
					case 2:
					{
						return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event)));
					}
					case 4:
					{
						return wxString(MidiFileMarkerEvent_getText(row.event));
					}
					default:
					{
						return wxEmptyString;
					}
				}
			}
			case EVENT_TYPE_PORT:
			{
				switch (column_number)
				{
					case 0:
					{
						return wxString("Port");
					}
					case 1:
					{
						return this->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(row.event));
					}
					case 2:
					{
						return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event)));
					}
					case 4:
					{
						return wxString(MidiFilePortEvent_getName(row.event));
					}
					default:
					{
						return wxEmptyString;
					}
				}
			}
			case EVENT_TYPE_TEMPO:
			{
				switch (column_number)
				{
					case 0:
					{
						return wxString("Tempo");
					}
					case 1:
					{
						return this->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(row.event));
					}
					case 2:
					{
						return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event)));
					}
					case 4:
					{
						return wxString::Format("%5.3f", MidiFileTempoEvent_getTempo(row.event));
					}
					default:
					{
						return wxEmptyString;
					}
				}
			}
			case EVENT_TYPE_TIME_SIGNATURE:
			{
				switch (column_number)
				{
					case 0:
					{
						return wxString("Time");
					}
					case 1:
					{
						return this->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(row.event));
					}
					case 2:
					{
						return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event)));
					}
					case 4:
					{
						return wxString::Format("%d/%d", MidiFileTimeSignatureEvent_getNumerator(row.event), MidiFileTimeSignatureEvent_getDenominator(row.event));
					}
					default:
					{
						return wxEmptyString;
					}
				}
			}
			case EVENT_TYPE_KEY_SIGNATURE:
			{
				switch (column_number)
				{
					case 0:
					{
						return wxString("Key");
					}
					case 1:
					{
						return this->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(row.event));
					}
					case 2:
					{
						return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event)));
					}
					case 4:
					{
						return GetKeyNameFromNumber(MidiFileKeySignatureEvent_getNumber(row.event), (bool)(MidiFileKeySignatureEvent_isMinor(row.event)));
					}
					default:
					{
						return wxEmptyString;
					}
				}
			}
			default:
			{
				return wxEmptyString;
			}
		}
	}
}

